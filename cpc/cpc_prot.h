#ifndef CPC_PROT_H
#define CPC_PROT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef WIN32
#include <netinet/in.h>
#endif

#define CPC_TYPE_NS 1
#define CPC_TYPE_LINUX 2
#define CPC_TYPE_WIN 3

#include "cpc_defs.h"

struct cpc_prot {
	int type;
	int len;
	void *data;
};

#define NS_PROT_SIZE (sizeof(struct cpc_prot_ns))

void cpc_prot_send_packet(char *p, int n, struct in_addr dst);
void cpc_prot_send_packet_with_data(char *p, int n, struct in_addr src, struct in_addr dst, struct in_addr next);

#ifdef CPC_NS

class CpcAgent;

struct cpc_prot_ns {
	CpcAgent *agent;
};

int cpc_init_ns(CpcAgent *agent);
int cpc_des_ns();

#endif /* CPC_NS */

#ifdef CPC_LINUX

#include <linux/netlink.h>

struct prot_callback {
    process_route_prot prot_callback;
    process_route_table rt_callback;
    process_request_route route_req_callback;
};

struct prot_callback callback_set;

#define LINUX_PROT_SIZE (sizeof(struct cpc_prot_linux))

int cpc_init_linux(struct prot_callback *set);
int cpc_des_linux();

#endif /* CPC_LINUX */

#ifdef WIN32

#define ROUTING_PORT 10000

struct ProtCallback {
    process_route_prot prot_callback;
    process_route_table rt_callback;
    process_request_route route_req_callback;
};

struct ProtCallback callback_set;

int cpc_init_win(struct ProtCallback *set);
int cpc_des_win(void);

#endif /* WIN32 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CPC_PROT_H */
