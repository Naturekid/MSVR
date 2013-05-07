#ifndef CPC_DEFS_H
#define CPC_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* callback function definitions */
typedef int (*process_route_table)(int cmd, struct in_addr dest_addr,
								   ... /* struct in_addr *next_addr */);
#ifdef CPC_NS
class CpcAgent;
typedef int (*process_route_prot)(char *p, int *n, CpcAgent *agent);
typedef int (*process_request_route)(char *p, struct in_addr src, struct in_addr dst,
                                     int applen, CpcAgent *agent);
#else
typedef int (*process_route_prot)(char *p, int *n);
typedef int (*process_request_route)(char *p, struct in_addr src,
                                     struct in_addr dst, int applen);
#endif /* CPC_NS */

#ifdef __cplusplus
}
#endif

#endif /* CPC_DEFS_H */
