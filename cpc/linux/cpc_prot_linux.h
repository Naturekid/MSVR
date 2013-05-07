#ifndef CPC_PROT_LINUX_H
#define CPC_PROT_LINUX_H

#include <linux/netlink.h>
#include <netinet/in.h>

#include "../cpc_prot.h"

int cpc_prot_init_linux(void);
int cpc_prot_des_linux(void);

void cpc_prot_send_linux(char *p, int n, struct in_addr dst);
void cpc_prot_send_with_data_linux(char *p, int n, struct in_addr dst);

#endif /* CPC_PROT_LINUX_H */
