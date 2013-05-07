#ifndef CPC_RT_LINUX_H
#define CPC_RT_LINUX_H

void cpc_rt_add_linux(struct in_addr dst, struct in_addr next, int ifindex);
void cpc_rt_del_linux(struct in_addr dst, struct in_addr next);
void cpc_rt_update_linux(struct in_addr dst, struct in_addr next, int ifindex);

#endif /* CPC_RT_LINUX_H */
