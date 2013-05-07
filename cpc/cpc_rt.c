#include <stdlib.h>

#include "cpc_rt.h"
#include "routing_table.h"

#ifdef CPC_NS

void cpc_rt_init()
{
	rt_init();
}

void cpc_rt_des()
{
	rt_des();
}

void cpc_rt_add(struct in_addr dst, struct in_addr next)
{
	/*
	 * TODO: need to determined platform.
	 */
	rt_add(dst, next);
}

void cpc_rt_remove(struct in_addr dst)
{
	struct rt_entry *rt;
	
	rt = rt_find(dst);
	if (rt == NULL)
		return;

	rt_remove(rt);
}

void cpc_rt_update(struct in_addr dst, struct in_addr next)
{
	struct rt_entry *rt;

	rt = rt_find(dst);
	if (rt == NULL)
		rt_add(dst, next);
	else
		rt_update(rt, next);
}

struct in_addr *cpc_rt_find(struct in_addr dst)
{
	struct rt_entry *rt;
	
	rt = rt_find(dst);
	if (rt == NULL)
		return NULL;
	else
		return &rt->next;
}

#endif /* CPC_NS */

#ifdef CPC_LINUX

#include "linux/cpc_rt_linux.h"

void cpc_rt_init()
{
}

void cpc_rt_des()
{
}

void cpc_rt_add(struct in_addr dst, struct in_addr next, int ifindex)
{
    cpc_rt_add_linux(dst, next, ifindex);
}

void cpc_rt_remove(struct in_addr dst, struct in_addr next)
{
    cpc_rt_del_linux(dst, next);
}

void cpc_rt_update(struct in_addr dst, struct in_addr next, int ifindex)
{
    cpc_rt_update_linux(dst, next, ifindex);
}

#endif /* CPC_LINUX */

#ifdef WIN32

#include "win/CpcRtWin.h"

void cpc_rt_add(struct in_addr dst, struct in_addr next)
{
	CpcRtAddWin(dst, next);
}

void cpc_rt_remove(struct in_addr dst)
{
	CpcRtRemoveWin(dst);
}

void cpc_rt_update(struct in_addr dst, struct in_addr next)
{
	CpcRtUpdateWin(dst, next);
}

struct in_addr *cpc_rt_find(struct in_addr dst)
{
	return CpcRtFindWin(dst);
}

#endif /* WIN32 */