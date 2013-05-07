#include <trace/cmu-trace.h>

#include "cpc_queue_ns.h"
#include "cpc_agent.h"

struct cpc_queue_ns q;

int cpc_queue_ns_init(CpcAgent *agent)
{
	INIT_LIST_HEAD(&q.head);
	q.len = 0;
	q.agent = agent;
}

int cpc_queue_ns_des()
{
	list_t *pos, *tmp;

	list_foreach_safe(pos, tmp, &q.head) {
		struct queue_entry *e = (struct queue_entry *)pos;
		list_detach(pos);

		q.agent->dropPacket(e->p, DROP_END_OF_SIMULATION);

		free(e);
		q.len--;
	}
}

int cpc_queue_ns_add(Packet *p, struct in_addr dst)
{
	struct queue_entry *e;
	struct hdr_ip *ih;

	ih = HDR_IP(p);

	assert(ih->daddr() == dst.s_addr);

	/*
	 * TODO: check queue length
	 */

	e = (struct queue_entry *)malloc(sizeof(struct queue_entry));
	if (e == NULL) {
		fprintf(stderr, "Out of memotry\n");
		exit(-1);
	}

	e->p = p;
	e->dst = dst;

	list_add_tail(&q.head, &e->l);
	q.len++;
}

int cpc_queue_ns_remove(struct in_addr dst)
{
	list_t *pos, *tmp;

	list_foreach_safe(pos, tmp, &q.head) {
		struct queue_entry *e = (struct queue_entry *)pos;

		if (e->dst.s_addr == dst.s_addr) {
			list_detach(&e->l);

			q.len--;
			free(e);
		}
	}
}

int cpc_queue_ns_send_packets(struct in_addr dst)
{
	int flag = 0;
	list_t *pos, *tmp;

	list_foreach_safe(pos, tmp, &q.head) {
		struct queue_entry *e = (struct queue_entry *)pos;

		if (e->dst.s_addr == dst.s_addr) {
			/*
			 * TODO: need to think.
			 */
			q.agent->sendData(e->p, dst);
			flag = 1;
		}
	}

	if (flag)
		cpc_queue_ns_remove(dst);
}

int cpc_queue_ns_drop_packets(struct in_addr dst)
{
	cpc_queue_ns_remove(dst);
}

Packet* cpc_queue_ns_get(struct in_addr dst)
{
	list_t *pos, *tmp;

	list_foreach_safe(pos, tmp, &q.head) {
		struct queue_entry *e = (struct queue_entry *)pos;

		if (memcmp(&e->dst, &dst, sizeof(struct in_addr)) == 0)
			return e->p;
	}

	return NULL;
}
