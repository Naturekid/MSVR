#ifndef CPC_QUEUE_H
#define CPC_QUEUE_H

#define CPC_QUEUE_DROP 1
#define CPC_QUEUE_SEND 2

int cpc_queue_find(__u32 daddr);
int cpc_queue_enqueue_packet(struct sk_buff *skb,
							int (*okfn)(struct sk_buff *));
int cpc_queue_set_verdict(int verdict, __u32 daddr);
void cpc_queue_flush(void);
int cpc_queue_init(void);
void cpc_queue_fini(void);

#include "cpc_netlink.h"
void cpc_queue_send_with_data(struct cpc_stateless_msg *rt);

#endif /* CPC_QUEUE_H */
