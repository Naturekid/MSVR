#ifndef NL_H
#define NL_H

#include <netinet/in.h>

#include "../cpc_prot.h"

int sock_init(void);
void send_rt_sock_init(void);

int nl_send_add_route_msg(struct in_addr dest, struct in_addr next_hop,
			  int metric, u_int32_t lifetime, int rt_flags,
			  int ifindex);
int nl_send_del_route_msg(struct in_addr dest, struct in_addr next_hop, int metric);

int nl_send_queue(struct in_addr dest);
int nl_drop_queue(struct in_addr dest);

int nl_send_with_data(char *p, int n, struct in_addr dst);

#endif /* NL_H */
