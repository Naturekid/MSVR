#ifndef CPC_QUEUE_H
#define CPC_QUEUE_H

#include <common/packet.h>

#include "../list.h"

class CpcAgent;

struct queue_entry {
	list_t l;
	struct in_addr dst;
	Packet *p;
};

struct cpc_queue_ns {
	list_t head;
	unsigned int len;
	CpcAgent *agent;
};

int cpc_queue_ns_init(CpcAgent *agent);
int cpc_queue_ns_des();
int cpc_queue_ns_add(Packet *p, struct in_addr dst);
int cpc_queue_ns_remove(struct in_addr dst);

Packet* cpc_queue_ns_get(struct in_addr dst);

int cpc_queue_ns_send_packets(struct in_addr dst);
int cpc_queue_ns_drop_packets(struct in_addr dst);

#endif /* CPC_QUEUE_H */
