#ifndef CPC_QUEUE_LINUX_H
#define CPC_QUEUE_LINUX_H

#include <netinet/in.h>

void cpc_queue_send_linux(struct in_addr dst);
void cpc_queue_drop_linux(struct in_addr dst);

#endif /* CPC_QUEUE_LINUX_H */
