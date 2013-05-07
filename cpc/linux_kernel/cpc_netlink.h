#ifndef CPC_NETLINK_H
#define CPC_NETLINK_H

#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

static struct sock *cpcnl;

#define NETLINK_CPC (MAX_LINKS-1)

enum {
	CPC_NOTIFY = 1,
#define CPC_NOTIFY	CPC_NOTIFY
	__CPC_MAX
};

#define CPC_MAX (__CPC_MAX - 1)

enum {
	CPC_BASE = 100,
#define CPC_BASE CPC_BASE
	CPC_ROUTE_ADD,
#define CPC_ROUTE_ADD CPC_ROUTE_ADD
	CPC_ROUTE_DEL,
#define CPC_ROUTE_DEL CPC_ROUTE_DEL
	CPC_TIMEOUT,
#define CPC_TIMEOUT CPC_TIMEOUT
	CPC_ROUTE_REQ,
#define CPC_ROUTE_REQ CPC_ROUTE_REQ
	CPC_NOROUTE_FOUND,
#define CPC_NOROUTE_FOUND CPC_NOROUTE_FOUND
	CPC_ROUTE_UPDATE,
#define CPC_ROUTE_UPDATE CPC_ROUTE_UPDATE
	CPC_SEND_ERR,
#define CPC_SEND_ERR CPC_SEND_ERR
    CPC_SEND_QUEUE,
#define CPC_SEND_QUEUE CPC_SEND_QUEUE
    CPC_DROP_QUEUE,
#define CPC_DROP_QUEUE CPC_DROP_QUEUE
    CPC_SEND_WITH_DATA,
#define CPC_SEND_WITH_DATA CPC_SEND_WITH_DATA
	__CPC_NETLINK_MAX,
#define CPC_NETLINK_MAX __CPC_NETLINK_MAX
};

static struct {
	int type;
	char *name;
} msgtype[CPC_NETLINK_MAX] = {
	{ CPC_ROUTE_ADD, "Add route" },
	{ CPC_ROUTE_DEL, "Del route" },
	{ CPC_TIMEOUT, "Timeout" },
	{ CPC_ROUTE_REQ, "Request route" },
	{ CPC_NOROUTE_FOUND, "No route found" },
	{ CPC_ROUTE_UPDATE, "Update route" },
    { CPC_SEND_ERR, "Send route error" },
    { CPC_SEND_QUEUE, "Send packet" },
    { CPC_DROP_QUEUE, "Drop packet" },
};

static inline char *
cpc_msg_type_to_str(int type)
{
	int i;

	for (i = 0; i < CPC_NETLINK_MAX; i++)
		if (type == msgtype[i].type)
			return msgtype[i].name;

	return "Unknown message type";
}

struct cpc_rt_msg {
	u_int8_t type;
	u_int32_t src;
	u_int32_t dst;
	u_int32_t nhop;
	int ifindex;
	long time;
};

struct cpc_stateless_msg {
    char *p;
    int n;
    u_int32_t dst;
};

#define PKT_INBOUND 1
#define PKT_OUTBOUND 2

/*
 * TODO: maybe it is not nessesary
 */
struct cpc_conf_msg {
	int active_route_timeout;
	int qual_th;
	int is_gateway;
};

#ifdef __KERNEL__

int cpc_netlink_init(void);
void cpc_netlink_fini(void);

void cpc_netlink_send_rt_msg(int type, __u32 src, __u32 dst);
void cpc_netlink_send_rt_update_msg(int type, __u32 src,
									__u32 dst, int ifindex);
void cpc_netlink_send_err_msg(int type, __u32 src, __u32 dst, int ifindex);

#endif /* __KERNEL__ */

#endif /* CPC_NETLINK_H */
