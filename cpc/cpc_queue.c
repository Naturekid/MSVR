#ifndef WIN32
#include <netinet/in.h>
#endif

#include "cpc_queue.h"

#ifdef CPC_NS

#include "ns/cpc_agent.h"
#include "ns/cpc_queue_ns.h"

int cpc_queue_init_ns(CpcAgent *agent)
{
	cpc_queue_ns_init(agent);
}

int cpc_queue_des_ns()
{
	cpc_queue_ns_des();
}

int cpc_queue_send_packets(struct in_addr dst)
{
	cpc_queue_ns_send_packets(dst);
}

int cpc_queue_drop_packets(struct in_addr dst)
{
	cpc_queue_ns_drop_packets(dst);
}

#endif /* CPC_NS */

#ifdef CPC_LINUX

#include "linux/cpc_queue_linux.h"

int cpc_queue_init_linux(void)
{
    return 0;
}

int cpc_queue_des_linux(void)
{
    return 0;
}

int cpc_queue_send_packets(struct in_addr dst)
{
    cpc_queue_send_linux(dst);

    return 0;
}

int cpc_queue_drop_packets(struct in_addr dst)
{
    cpc_queue_drop_linux(dst);

    return 0;
}

#endif /* CPC_LINUX */

#ifdef WIN32

#include <winsock2.h>

#include "win/CpcQueueWin.h"

int cpc_queue_init_win(void)
{
	return 0;
}

int cpc_queue_des_win(void)
{
	return 0;
}

int cpc_queue_send_packets(struct in_addr dst)
{
	CpcQueueSendPackets(dst);
	return 0;
}

int cpc_queue_drop_packets(struct in_addr dst)
{
	CpcQueueDropPackets(dst);
	return 0;
}

#endif /* WIN32 */