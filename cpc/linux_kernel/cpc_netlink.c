#include <linux/if.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/netlink.h>
#include <linux/version.h>

#include <linux/security.h>

#include <net/sock.h>

#include "cpc_netlink.h"
#include "cpc_rt.h"
#include "cpc_queue.h"

static int peer_pid;
struct net net;
struct mutex cb_mutex;
static DECLARE_MUTEX(cpcnl_sem);

static struct sk_buff *
cpc_netlink_build_msg(int type, void *data, int len)
{
	unsigned char *old_tail;
	size_t size = 0;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	void *m;

	size = NLMSG_SPACE(len);

	skb = alloc_skb(size, GFP_ATOMIC);
	if (skb == NULL)
		goto nlmsg_failure;

	old_tail = skb->tail;
	nlh = NLMSG_PUT(skb, 0, 0, type, size - sizeof(*nlh));

	m = NLMSG_DATA(nlh);
	memcpy(m, data, len);
	nlh->nlmsg_len = skb->tail - old_tail;
	NETLINK_CB(skb).pid = 0;

	return skb;

nlmsg_failure:
	if (skb != NULL)
		kfree_skb(skb);

	printk(KERN_ERR "CPC: error creating rt timeout message\n");
	return NULL;
}

void
cpc_netlink_send_rt_msg(int type, __u32 src, __u32 dst)
{
	struct sk_buff *skb = NULL;
	struct cpc_rt_msg m;

	memset(&m, 0, sizeof(struct cpc_rt_msg));

	m.src = src;
	m.dst = dst;

	skb = cpc_netlink_build_msg(type, &m, sizeof(struct cpc_rt_msg));
	if (skb == NULL) {
		printk("CPC: cpc_netlink skb = NULL\n");
		return;
	}

	netlink_broadcast(cpcnl, skb, 0, CPC_NOTIFY, GFP_USER);
}

void
cpc_netlink_send_rt_update_msg(int type, __u32 src, __u32 dst, int ifindex)
{
	struct sk_buff *skb = NULL;
	struct cpc_rt_msg m;

	memset(&m, 0, sizeof(struct cpc_rt_msg));

	m.type = type;
	m.src = src;
	m.dst = dst;
	m.ifindex = ifindex;

	skb = cpc_netlink_build_msg(CPC_ROUTE_UPDATE, &m,
								sizeof(struct cpc_rt_msg));
	if (skb == NULL) {
		printk("CPC: cpc_netlink skb = NULL\n");
		return;
	}

	netlink_broadcast(cpcnl, skb, 0, CPC_NOTIFY, GFP_USER);
}

void
cpc_netlink_send_err_msg(int type, __u32 src, __u32 dst, int ifindex)
{
	struct sk_buff *skb = NULL;
	struct cpc_rt_msg m;

	memset(&m, 0, sizeof(struct cpc_rt_msg));

	m.type = type;
	m.src = src;
	m.dst = dst;
	m.ifindex = ifindex;

	skb = cpc_netlink_build_msg(CPC_SEND_ERR, &m,
								sizeof(struct cpc_rt_msg));
	if (skb == NULL) {
		printk("CPC: cpc_netlink skb = NULL\n");
		return;
	}

	netlink_broadcast(cpcnl, skb, 0, CPC_NOTIFY, GFP_USER);
}

static int
cpc_netlink_receive_peer(unsigned char type, void *msg, unsigned int len)
{
	int ret = 0;
	struct cpc_rt_msg *m;
    struct cpc_stateless_msg *rt;
	struct rt_entry e;

	switch (type) {
		case CPC_ROUTE_ADD:
			if (len < sizeof(struct cpc_rt_msg))
				return -EINVAL;

			m = (struct cpc_rt_msg *)msg;

			ret = cpc_rt_get(m->dst, &e);
			if (ret < 0)
				ret = cpc_rt_update(m->dst, m->nhop, m->time, m->ifindex);
			else
				ret = cpc_rt_add(m->dst, m->nhop, m->time, m->ifindex);

			cpc_queue_set_verdict(CPC_QUEUE_SEND, m->dst);

			break;
		case CPC_ROUTE_DEL:
			if (len < sizeof(struct cpc_rt_msg))
				return -EINVAL;

			m = (struct cpc_rt_msg *)msg;

			cpc_rt_del(m->dst);
			cpc_queue_set_verdict(CPC_QUEUE_DROP, m->dst);

			break;
		case CPC_NOROUTE_FOUND:
			if (len < sizeof(struct cpc_rt_msg))
				return -EINVAL;

			m = (struct cpc_rt_msg *)msg;
			cpc_queue_set_verdict(CPC_QUEUE_DROP, m->dst);

			break;
        case CPC_SEND_QUEUE:
            if (len < sizeof(struct cpc_rt_msg))
                return -EINVAL;

            m = (struct cpc_rt_msg *)msg;
			cpc_queue_set_verdict(CPC_QUEUE_SEND, m->dst);

            break;
        case CPC_DROP_QUEUE:
            if (len < sizeof(struct cpc_rt_msg))
                return -EINVAL;

            m = (struct cpc_rt_msg *)msg;
			cpc_queue_set_verdict(CPC_QUEUE_DROP, m->dst);

            break;
        case CPC_SEND_WITH_DATA:
            if (len < sizeof(struct cpc_stateless_msg))
                return -EINVAL;

            rt = (struct cpc_stateless_msg *)msg;
            cpc_queue_send_with_data(rt);

            break;
		default:
			printk("CPC: cpc_netlink Unknown message type\n");
			ret = -EINVAL;
			break;
	}

	return ret;
}

static int
cpc_netlink_rcv_nl_event(struct notifier_block *this,
						unsigned long event, void *ptr)
{
	struct netlink_notify *n = ptr;

	if (event == NETLINK_URELEASE &&
		n->protocol == NETLINK_CPC &&
		n->pid) {
		if (n->pid == peer_pid) {
			peer_pid = 0;
			cpc_rt_flush();
			cpc_queue_flush();
		}

		return NOTIFY_DONE;
	}

	return NOTIFY_DONE;
}

static struct notifier_block cpc_nl_notifier = {
	.notifier_call = cpc_netlink_rcv_nl_event,
};

#define RCV_SKB_FAIL(err) do { \
	netlink_ack(skb, nlh, (err)); \
	return; \
} while (0);

static inline void
cpc_netlink_rcv_skb(struct sk_buff *skb)
{
	int status, type, pid, flags, nlmsglen, skblen;
	struct nlmsghdr *nlh;

	skblen = skb->len;
	if (skblen < sizeof(struct nlmsghdr)) {
		printk("skblen to small\n");
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlmsglen = nlh->nlmsg_len;
	
	if (nlmsglen < sizeof(struct nlmsghdr) || skblen < nlmsglen) {
		printk("nlsmsg=%d skblen=%d to small\n", nlmsglen, skblen);
		return;
	}

	pid = nlh->nlmsg_pid;
	flags = nlh->nlmsg_flags;

	if (pid <= 0 || !(flags & NLM_F_REQUEST) || flags & NLM_F_MULTI)
		RCV_SKB_FAIL(-EINVAL);


	if (flags & MSG_TRUNC)
		RCV_SKB_FAIL(-ECOMM);

	type = nlh->nlmsg_type;

/* 	printk("kaodv_netlink: type=%d\n", type); */
	/* if (type < NLMSG_NOOP || type >= IPQM_MAX) */
/* 		RCV_SKB_FAIL(-EINVAL); */
#ifdef KERNEL26
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
	if (security_netlink_recv(skb))
		RCV_SKB_FAIL(-EPERM);
#else
	if (security_netlink_recv(skb, CAP_NET_ADMIN))
		RCV_SKB_FAIL(-EPERM);
#endif
#endif
	//write_lock_bh(&queue_lock);
	
	if (peer_pid) {
		if (peer_pid != pid) {
			//write_unlock_bh(&queue_lock);
			RCV_SKB_FAIL(-EBUSY);
		}
	} else
		peer_pid = pid;

	//write_unlock_bh(&queue_lock);

	status = cpc_netlink_receive_peer(type, NLMSG_DATA(nlh),
					    skblen - NLMSG_LENGTH(0));
	if (status < 0)
		RCV_SKB_FAIL(status);

	if (flags & NLM_F_ACK)
		netlink_ack(skb, nlh, 0);
	return;
}

static void
cpc_netlink_rcv_sk(struct sk_buff *sk)
{
	printk(KERN_ALERT "cpc: enter cpc_netlink_rcv_sk\n");

	do {
		struct sk_buff *skb;

		if (down_trylock(&cpcnl_sem))
			return;

		while ((skb = skb_dequeue(&sk->sk->sk_receive_queue)) != NULL) {
			cpc_netlink_rcv_skb(skb);
			kfree_skb(skb);
		}

		up(&cpcnl_sem);

	} while (cpcnl && cpcnl->sk_receive_queue.qlen);

	return;
}

int
cpc_netlink_init(void)
{
	cpcnl = netlink_kernel_create(&net, NETLINK_CPC, CPC_MAX,
			cpc_netlink_rcv_sk, &cb_mutex, THIS_MODULE);
	if (cpcnl == NULL) {
		printk(KERN_ERR "CPC: Failed to create netlink socket\n");
		return -1;
	}

	return 0;
}

void
cpc_netlink_fini(void)
{
	sock_release(cpcnl->sk_socket);
}
