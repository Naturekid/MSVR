#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "routing_table.h"

struct routing_table rt_tbl;

void rt_init()
{
	rt_tbl.nentries = 0;
	INIT_LIST_HEAD(&rt_tbl.list);
}

void rt_des()
{
	list_t *tmp, *pos;

	list_foreach_safe(pos, tmp, &rt_tbl.list) {
		struct rt_entry *rt = (struct rt_entry *)pos;

		rt_remove(rt);
	}
}

void rt_add(struct in_addr dst, struct in_addr next)
{
	list_t *pos;
	struct rt_entry *rt;

	list_foreach(pos, &rt_tbl.list) {
		rt = (struct rt_entry *)pos;

		if (memcmp(&rt->dst, &dst, sizeof(struct in_addr)) == 0) {
			fprintf(stderr, "already exist in routing table\n");
			return;
		}
	}

	rt = (struct rt_entry *)malloc(sizeof(struct rt_entry));
	if (rt == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(-1);
	}

	memset(rt, 0, sizeof(struct rt_entry));

	rt->dst = dst;
	rt->next = next;

	rt_tbl.nentries++;
	list_add(&rt_tbl.list, &rt->list);

	/*
	 * TODO: think that how to handle in linux and whether or not
	 * send packet in the queue.
	 */
}

void rt_remove(struct rt_entry *rt)
{
	if (rt == NULL)
		return;

	list_detach(&rt->list);

	/*
	 * TODO: think that how to handle in linux.
	 */

	rt_tbl.nentries--;
	free(rt);
}

void rt_update(struct rt_entry *rt, struct in_addr next)
{
	rt->next = next;

	/*
	 * TODO: think whether or not send packet in the queue.
	 */
}

struct rt_entry *rt_find(struct in_addr dst)
{
	list_t *pos;

	list_foreach(pos, &rt_tbl.list) {
		struct rt_entry *rt = (struct rt_entry *)pos;

		if (memcmp(&rt->dst, &dst, sizeof(struct in_addr)) == 0)
			return rt;
	}

	return NULL;
}
