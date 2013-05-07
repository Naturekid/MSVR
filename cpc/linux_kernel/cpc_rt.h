#ifndef CPC_RT_H
#define CPC_RT_H

#include <linux/list.h>

struct rt_entry {
	struct list_head l;
	unsigned long expires;
	__u32 daddr;
	__u32 nhop;
	int ifindex;
};

void cpc_rt_init(void);
void cpc_rt_flush(void);
void cpc_rt_fini(void);

int cpc_rt_get(__u32 daddr, struct rt_entry *rt);
int cpc_rt_add(__u32 daddr, __u32 nhop, unsigned long time, int ifindex);
int cpc_rt_update(__u32 daddr, __u32 nhop, unsigned long time, int ifindex);
int cpc_rt_del(__u32 daddr);

#endif /* CPC_RT_H */
