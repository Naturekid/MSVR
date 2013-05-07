#ifndef CPC_RT_H
#define CPC_RT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
#include <netinet/in.h>
#endif

void cpc_rt_init();
void cpc_rt_des();

#ifdef CPC_NS
void cpc_rt_add(struct in_addr dst, struct in_addr next);
void cpc_rt_remove(struct in_addr dst);
void cpc_rt_update(struct in_addr dst, struct in_addr next);
#endif /* CPC_NS */

#ifdef CPC_LINUX
void cpc_rt_add(struct in_addr dst, struct in_addr next, int ifindex);
void cpc_rt_remove(struct in_addr dst, struct in_addr next);
void cpc_rt_update(struct in_addr dst, struct in_addr next, int ifindex);
#endif /* CPC_LINUX */

#ifdef WIN32
void cpc_rt_add(struct in_addr dst, struct in_addr next);
void cpc_rt_remove(struct in_addr dst);
void cpc_rt_update(struct in_addr dst, struct in_addr next);
#endif /* WIN32 */

struct in_addr *cpc_rt_find(struct in_addr dst);

#ifdef __cplusplus
}
#endif

#endif /* CPC_RT_H */
