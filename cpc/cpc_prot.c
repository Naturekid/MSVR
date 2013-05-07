#include "cpc_prot.h"
#include "cpc_debug.h"

struct cpc_prot cpc_prot;

#ifdef CPC_NS

#include "ns/cpc_agent.h"

void cpc_prot_send_packet(char *p, int n, struct in_addr dst)
{
	struct cpc_prot_ns *ns;

	ns = (struct cpc_prot_ns *)cpc_prot.data;
	ns->agent->sendProtPacket(p, n, dst);
}

void cpc_prot_send_packet_with_data(char *p, int n, struct in_addr src, struct in_addr dst, struct in_addr next)
{
	struct cpc_prot_ns *ns;

	ns = (struct cpc_prot_ns *)cpc_prot.data;
	ns->agent->sendProtPacketWithData(p, n, src, dst, next);
}

int cpc_init_ns(CpcAgent *agent)
{
	struct cpc_prot_ns *ns;

	cpc_prot.type = CPC_TYPE_NS;
	cpc_prot.len = NS_PROT_SIZE;
	cpc_prot.data = malloc(NS_PROT_SIZE);
	if (cpc_prot.data == NULL) {
		cpc_debug("Out of memotry\n");
		return -1;
	}

	ns = (struct cpc_prot_ns *)cpc_prot.data;
	ns->agent = agent;
}

int cpc_des_ns()
{
	if (cpc_prot.data != NULL)
		free(cpc_prot.data);
}

#endif /* CPC_NS */

#ifdef CPC_LINUX

#include "linux/cpc_prot_linux.h"

int cpc_init_linux(struct prot_callback *set)
{
    callback_set.prot_callback = set->prot_callback;
    callback_set.rt_callback = set->rt_callback;
    callback_set.route_req_callback = set->route_req_callback;

    cpc_prot_init_linux();

    return 0;
}

int cpc_des_linux()
{
    cpc_prot_des_linux();

    return 0;
}

void cpc_prot_send_packet(char *p, int n, struct in_addr dst)
{
    cpc_prot_send_linux(p, n, dst);
}

void cpc_prot_send_packet_with_data(char *p, int n, struct in_addr dst)
{
    cpc_prot_send_with_data_linux(p, n, dst);
}

#endif /* CPC_LINUX */

#ifdef WIN32

#include <winsock2.h>

#include "win/CpcProtWin.h"

void cpc_prot_send_packet(char *p, int n, struct in_addr dst)
{
	CpcProtSendWin(p, n, dst);
}

void cpc_prot_send_packet_with_data(char *p, int n, struct in_addr dst)
{
	CpcProtSendWinWithData(p, n, dst);
}

int cpc_init_win(struct ProtCallback *set)
{
	callback_set.prot_callback = set->prot_callback;
	callback_set.route_req_callback = set->route_req_callback;
	callback_set.rt_callback = set->rt_callback;

	CpcProtInitWin();
	return 0;
}

int cpc_des_win(void)
{
	CpcProtDesWin();
	return 0;
}

#endif /* WIN32 */
