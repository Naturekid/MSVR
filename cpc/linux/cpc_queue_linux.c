#include "cpc_queue_linux.h"
#include "nl.h"
#include "../cpc_debug.h"
#include "../cpc_queue.h"

void cpc_queue_send_linux(struct in_addr dst)
{
    nl_send_queue(dst);
}

void cpc_queue_drop_linux(struct in_addr dst)
{
    nl_drop_queue(dst);
}
