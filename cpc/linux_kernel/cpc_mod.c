#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/net.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/stat.h>

/* need linux kernel version > 2.6 */
#include <linux/moduleparam.h>

#include <net/sock.h>

#include "cpc_mod.h"
#include "cpc_if_list.h"
#include "cpc_rt.h"
#include "cpc_queue.h"
#include "cpc_netlink.h"

MODULE_DESCRIPTION
	("CPC linux kernel support");
MODULE_AUTHOR("gnehzuil");

/* module parameter definition */
int routing_port;
module_param(routing_port, int, S_IRUGO);

#define MAX_INTERFACES 10
static char *ifname[MAX_INTERFACES] = { "eth0", NULL, };
static int nifname = 0;
module_param_array(ifname, charp, &nifname, 0444);

/**
 * TODO: TBD
 */
static unsigned int
cpc_hook(unsigned int hooknum, struct sk_buff *skb,
		 const struct net_device *in,
		 const struct net_device *out,
		 int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph = (struct iphdr *)((skb)->network_header);
	struct in_addr ifaddr, bcaddr;
	struct rt_entry e;
	int res = 0;

	if (iph == NULL)
		return NF_ACCEPT;

	memset(&ifaddr, 0, sizeof(struct in_addr));
	memset(&bcaddr, 0, sizeof(struct in_addr));

	if (hooknum == NF_INET_PRE_ROUTING)
		res = if_info_from_ifindex(&ifaddr, &bcaddr, in->ifindex);
	else
		res = if_info_from_ifindex(&ifaddr, &bcaddr, out->ifindex);

	if (res < 0)
		return NF_ACCEPT;

	switch (hooknum) {
		case NF_INET_PRE_ROUTING:
			/*printk(KERN_ALERT "CPC: hook fn, pre routing\n");*/
			if (iph->saddr == ifaddr.s_addr)
				return NF_ACCEPT;

			if (iph && iph->protocol == IPPROTO_UDP) {
				struct udphdr *udph;

				/*udph = (struct udphdr *)((char *)iph + (iph->ihl << 2));*/
                udph = (struct udphdr *)((skb)->transport_header);

				if (ntohs(udph->dest) == routing_port ||
					ntohs(udph->source) == routing_port) {
					printk(KERN_ALERT "CPC: recv a routing packet\n");

					/*
					 * TODO: process routing packet
					 */
                    return NF_ACCEPT;
				}
			}

			break;
		case NF_INET_POST_ROUTING:
			/* maybe this procedure is not required */
			/*printk(KERN_ALERT "CPC: hook fn, post routing\n");*/
			break;
		case NF_INET_LOCAL_OUT:
			if (!cpc_rt_get(iph->daddr, &e)) {
				if (!cpc_queue_find(iph->daddr))
					cpc_netlink_send_rt_msg(CPC_ROUTE_REQ, 0, iph->daddr);

				cpc_queue_enqueue_packet(skb, okfn);

				printk(KERN_ALERT "CPC: local out route request\n");
				return NF_STOLEN;
			} else
				/*
				 * TODO: there is not a routing table in stateless routing
				 */
				printk(KERN_ALERT "CPC: local out route me\n");
				ip_route_me_harder(skb, RTN_LOCAL);
			break;
		default:
			break;
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops cpc_ops[] = {
	{
		/* packet come in network layer from datalink layer */
		.hook = cpc_hook,
#ifdef KERNEL26
		.owner = THIS_MODULE,
#endif
		.pf = PF_INET,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST,
	},
	{
		/* packet go out network layer to datalink layer */
		.hook = cpc_hook,
#ifdef KERNEL26
		.owner = THIS_MODULE,
#endif
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER,
	},
	{
		/* packet come in network layer from transport layer */
		.hook = cpc_hook,
#ifdef KERNEL26
		.owner = THIS_MODULE,
#endif
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER,
	},
};

static int __init
cpc_linux_init(void)
{
	int i, ret = 0;
	struct net_device *dev = NULL;
	struct net *ifnet = sock_net(cpcnl);

	cpc_rt_init();
	ret = cpc_queue_init();
	if (ret < 0)
		return ret;

	ret = cpc_netlink_init();
	if (ret < 0)
		goto q;

	ret = nf_register_hook(&cpc_ops[0]);
	if (ret < 0)
		goto pre;

	ret = nf_register_hook(&cpc_ops[1]);
	if (ret < 0)
		goto post;

	ret = nf_register_hook(&cpc_ops[2]);
	if (ret < 0)
		goto out;

	for (i = 0; i < MAX_INTERFACES; i++) {
		if (!ifname[i])
			break;

		if (i == 1)
			break;

		dev = dev_get_by_name(ifnet, ifname[i]);
		if (!dev) {
			printk("CPC: No device %s available, ignoring\n",
					ifname[i]);
			continue;
		}

		if_info_add(dev);

		dev_put(dev);
	}

	printk(KERN_ALERT "CPC: Cpc linux kernel module init...\n");
	printk(KERN_ALERT "CPC: Routing port: %d\n", routing_port);

	return ret;

out:
	nf_unregister_hook(&cpc_ops[1]);
post:
	nf_unregister_hook(&cpc_ops[0]);
pre:
	cpc_netlink_fini();
q:
	cpc_queue_fini();

	return ret;
}

static void __exit
cpc_linux_exit(void)
{
	int i;

	if_info_purge();

	for (i = 0; i < sizeof(cpc_ops) / sizeof(struct nf_hook_ops); i++)
		nf_unregister_hook(&cpc_ops[i]);

	cpc_queue_fini();
	cpc_rt_fini();
	cpc_netlink_fini();
	printk(KERN_ALERT "CPC: Cpc linux kernel module exit...\n");
}

module_init(cpc_linux_init);
module_exit(cpc_linux_exit);
