#ifndef ROUTE_TABLE_H
#define ROUTE_TABLE_H

#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

#include "list.h"

struct rt_entry {
	list_t list;
	struct in_addr dst;
	struct in_addr next;

	/*
	 * TODO: It need to add a timer.
	 *
	 * timer in Linux/Windows are different. So it need to think about it.
	 */
};

struct routing_table {
	unsigned int nentries;
	list_t list;
};

void rt_init();
void rt_des();
void rt_add(struct in_addr dst, struct in_addr next);
void rt_remove(struct rt_entry *rt);
void rt_update(struct rt_entry *rt, struct in_addr next);
struct rt_entry *rt_find(struct in_addr dst);

#endif /* ROUTE_TABLE_H */
