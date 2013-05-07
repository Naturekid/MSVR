#ifndef CPC_QUEUE_WIN_H
#define CPC_QUEUE_WIN_H

void CpcQueueSendPackets(struct in_addr dst);
void CpcQueueDropPackets(struct in_addr dst);

#endif /* CPC_QUEUE_WIN_H */