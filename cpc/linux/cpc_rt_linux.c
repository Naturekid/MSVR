#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>

#include "../cpc_debug.h"
#include "../cpc_rt.h"
#include "cpc_rt_linux.h"
#include "nl.h"

void cpc_rt_add_linux(struct in_addr dst, struct in_addr next, int ifindex)
{
    nl_send_add_route_msg(dst, next, 1, 1000, 0, ifindex);
}

void cpc_rt_del_linux(struct in_addr dst, struct in_addr next)
{
    nl_send_del_route_msg(dst, next, 1);
}

void cpc_rt_update_linux(struct in_addr dst, struct in_addr next, int ifindex)
{
    nl_send_add_route_msg(dst, next, 1, 1000, 0, ifindex);
}
