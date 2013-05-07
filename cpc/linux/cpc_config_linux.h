#ifndef CPC_CONFIG_LINUX_H
#define CPC_CONFIG_LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>

#if !defined(ROUTING_PORT)
#define ROUTING_PORT 10000
#endif

#if !defined(IFNAMSIZ)
#define IFNAMSIZ 16
#endif

#define MAX_NR_INTERFACES 10

struct dev_info {
	int enabled;
	int sock;
	unsigned int ifindex;
	char ifname[IFNAMSIZ];
	struct in_addr ipaddr;
	struct in_addr netmask;
	struct in_addr broadcast;
};

struct host_info {
	u_int32_t seqno;
	struct timeval bcast_time;
	struct timeval fwd_time;
	int nif;
	struct dev_info devs[MAX_NR_INTERFACES];
};

struct host_info this_host;

unsigned int dev_indices[MAX_NR_INTERFACES];

static inline int ifindex2devindex(unsigned int ifindex)
{
    int i;

    for (i = 0; i < this_host.nif; i++)
	if (dev_indices[i] == ifindex)
	    return i;

    return -1;
}

#define DEV_IFINDEX(ifindex) (this_host.devs[ifindex2devindex(ifindex)])
#define DEV_NR(n) (this_host.devs[n])

#endif /* CPC_CONFIG_LINUX_H */
