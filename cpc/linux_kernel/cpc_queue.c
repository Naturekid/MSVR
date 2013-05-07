#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/netfilter_ipv4.h>
#include <net/sock.h>
#include <net/route.h>
#include <net/icmp.h>
#include <linux/udp.h>

#include "cpc_mod.h"
#include "cpc_queue.h"
#include "cpc_rt.h"

#define CPC_QUEUE_MAX_DEFAULT 1024
#define NET_CPC_QUEUE_QMAX 2088

extern int routing_port;

struct rt_info {
	__u8 tos;
	__u32 daddr;
	__u32 saddr;
};

struct queue_entry {
	struct list_head l;
	struct sk_buff *skb;
	int (*okfn)(struct sk_buff *);
	struct rt_info rtinfo;
};

typedef int (*cpc_queue_cmpfn)(struct queue_entry *, unsigned long);

static unsigned int queue_maxlen = CPC_QUEUE_MAX_DEFAULT;
static rwlock_t queue_lock = RW_LOCK_UNLOCKED;
static unsigned int queue_total;
static LIST_HEAD(queue_list);

static inline int
__cpc_queue_enqueue_entry(struct queue_entry *e)
{
	if (queue_total >= queue_maxlen) {
		if (net_ratelimit())
			printk(KERN_WARNING "CPC: full at %d entries, "
					"dropping packet(s).\n", queue_total);
		return -ENOSPC;
	}

	list_add(&e->l, &queue_list);
	queue_total++;

	return 0;
}

static inline struct queue_entry *
__cpc_queue_find_entry(cpc_queue_cmpfn cmpfn, unsigned long data)
{
	struct list_head *p;

	list_for_each_prev(p, &queue_list) {
		struct queue_entry *e = (struct queue_entry *)p;

		if (!cmpfn || cmpfn(e, data))
			return e;
	}
	return NULL;
}

static inline struct queue_entry *
__cpc_queue_find_dequeue_entry(cpc_queue_cmpfn cmpfn, unsigned long data)
{
	struct queue_entry *e;

	e = __cpc_queue_find_entry(cmpfn, data);
	if (e == NULL)
		return NULL;

	list_del(&e->l);
	queue_total--;

	return e;
}

static inline void
__cpc_queue_flush(void)
{
	struct queue_entry *e;

	while ((e = __cpc_queue_find_dequeue_entry(NULL, 0))) {
		kfree_skb(e->skb);
		kfree(e);
	}
}

static inline void
__cpc_queue_reset(void)
{
	__cpc_queue_flush();
}

static struct queue_entry *
cpc_queue_find_dequeue_entry(cpc_queue_cmpfn cmpfn, unsigned long data)
{
	struct queue_entry *e;

	write_lock_bh(&queue_lock);
	e = __cpc_queue_find_dequeue_entry(cmpfn, data);
	write_unlock_bh(&queue_lock);

	return e;
}

void
cpc_queue_flush(void)
{
	write_lock_bh(&queue_lock);
	__cpc_queue_flush();
	write_unlock_bh(&queue_lock);
}

int
cpc_queue_enqueue_packet(struct sk_buff *skb, int (*okfn)(struct sk_buff *))
{
	int status = -EINVAL;
	struct queue_entry *e;
	struct iphdr *iph = (struct iphdr *)skb->network_header;

	e = kmalloc(sizeof(struct queue_entry), GFP_ATOMIC);
	if (e == NULL) {
		printk(KERN_ERR "CPC: Out of memory in cpc_queue_enqueue_packet()\n");
		return -ENOMEM;
	}

	e->okfn = okfn;
	e->skb = skb;
	e->rtinfo.tos = iph->tos;
	e->rtinfo.daddr = iph->daddr;
	e->rtinfo.saddr = iph->saddr;

	write_lock_bh(&queue_lock);

	status = __cpc_queue_enqueue_entry(e);
	if (status < 0)
		goto unlock;

	write_unlock_bh(&queue_lock);
	return status;

unlock:
	write_unlock_bh(&queue_lock);
	kfree(e);

	return status;
}

static inline int
dest_cmp(struct queue_entry *e, unsigned long daddr)
{
	return (daddr == e->rtinfo.daddr);
}

int
cpc_queue_find(__u32 daddr)
{
	struct queue_entry *e;
	int res = 0;

	read_lock_bh(&queue_lock);
	
	e = __cpc_queue_find_entry(dest_cmp, daddr);
	if (e != NULL)
		res = 1;

	read_unlock_bh(&queue_lock);

	return res;
}

int
cpc_queue_set_verdict(int verdict, __u32 daddr)
{
	struct queue_entry *e;
	int pkts = 0;

	if (verdict == CPC_QUEUE_DROP) {
		while (1) {
			e = cpc_queue_find_dequeue_entry(dest_cmp, daddr);
			if (e == NULL)
				return pkts;

			if (pkts == 0)
				icmp_send(e->skb, ICMP_DEST_UNREACH,
						ICMP_HOST_UNREACH, 0);

			kfree_skb(e->skb);
			kfree(e);
			pkts++;
		}
	} else if (verdict == CPC_QUEUE_SEND) {
		struct rt_entry rt;

		while (1) {
			e = cpc_queue_find_dequeue_entry(dest_cmp, daddr);
			if (e == NULL)
				return pkts;

			if (!cpc_rt_get(daddr, &rt)) {
				kfree_skb(e->skb);
				goto next;
			}

			ip_route_me_harder(e->skb, RTN_LOCAL);
			pkts++;

			e->okfn(e->skb);
next:
			kfree(e);
		}
	}

	return 0;
}

void cpc_queue_send_with_data(struct cpc_stateless_msg *rt)
{
	struct queue_entry *e;
    struct iphdr *iph;
    struct udphdr *udph;
    __u32 daddr;

    daddr = rt->dst;

    while (1) {
        struct cpchdr h;
        struct sk_buff *new_skb;

        e = cpc_queue_find_dequeue_entry(dest_cmp, daddr);
        if (e == NULL)
            return;

        /*
         * TODO: maybe it need to modify dest addr
         */
        iph = (struct iphdr *)((e->skb)->network_header);
        iph->daddr = daddr;

        /* save app port and routing port */
        udph = (struct udphdr *)((e->skb)->transport_header);
        h.app_port = udph->dest;
        udph->dest = htons(routing_port);

        h.rt_len = rt->n;
        h.rt_data = rt->p;

        new_skb = skb_copy_expand(e->skb, sizeof(h), 0, GFP_ATOMIC);
        if (new_skb == NULL) {
            printk(KERN_ERR "Out of memory\n");
            return;
        }

        memcpy(new_skb->head, &h, sizeof(h));

        ip_route_me_harder(new_skb, RTN_LOCAL);
        e->okfn(new_skb);

        kfree(e);
    }
}

static int
init_or_cleanup(int init)
{
	int status = -ENOMEM;

	if (!init)
		goto cleanup;

	queue_total = 0;

	return 1;

cleanup:
	synchronize_net();
	cpc_queue_flush();

	return status;
}

int
cpc_queue_init(void)
{
	return init_or_cleanup(1);
}

void
cpc_queue_fini(void)
{
	init_or_cleanup(0);
}
