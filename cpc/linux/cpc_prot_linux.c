#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <linux/sockios.h> 

#include "cpc_prot_linux.h"
#include "cpc_config_linux.h"
#include "defs.h"
#include "nl.h"
#include "../cpc_prot.h"
#include "../cpc_rt.h"
#include "../cpc_debug.h"
#include "../linux_kernel/cpc_netlink.h"

#define CALLBACK_FUNCS 5

static struct callback {
	int fd;
	callback_func func;
} callbacks[CALLBACK_FUNCS];

static int nr_callbacks= 0;

int
register_callback_func(int fd, callback_func func)
{
	if (nr_callbacks >= CALLBACK_FUNCS) {
		fprintf(stderr, "CPC: callback register limit reached\n");
		exit(-1);
	}
	callbacks[nr_callbacks].fd = fd;
	callbacks[nr_callbacks].func = func;
	nr_callbacks++;

	return 0;
}

/*
 * Returns information on a network interface given its name...
 */
static struct sockaddr_in *
get_if_info(char *ifname, int type)
{
	int skfd;
	struct sockaddr_in *ina;
	static struct ifreq ifr;

	/* Get address of interface... */
	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, type, &ifr) < 0) {
		fprintf(stderr, "Could not get address of %s ", ifname);
		close(skfd);
		return NULL;
	} else {
		ina = (struct sockaddr_in *) &ifr.ifr_addr;
		close(skfd);
		return ina;
	}
}

static void
load_modules(char *ifname)
{
	struct stat st;
	char buf[1024]; 
    /*char *l = NULL;*/
	/*int found = 0;*/
	/*FILE *m;*/

	memset(buf, '\0', 64);

	if (stat("./cpc.ko", &st) == 0)
		sprintf(buf, "/sbin/insmod cpc.ko routing_port=%d ifname=%s &>/dev/null", ROUTING_PORT, ifname);
	else if (stat("./cpc.o", &st) == 0)
		sprintf(buf, "/sbin/insmod cpc.o routing_port=%d ifname=%s &>/dev/null", ROUTING_PORT, ifname);
	else
		sprintf(buf, "/sbin/modprobe cpc routing_port=%d ifname=%s &>/dev/null", ROUTING_PORT, ifname);

	system(buf);

	usleep(100000);

	/* Check result */
#if 0	
	m = fopen("/proc/modules", "r");
	while (fgets(buf, sizeof(buf), m)) {
		l = strtok(buf, " \t");
		if (!strcmp(l, "kaodv"))
			found++;
		if (!strcmp(l, "ipchains")) {
			fprintf(stderr,
					"The ipchains kernel module is loaded and prevents AODV-UU from functioning properly.\n");
			exit(-1);
		}
	}
	fclose(m);

	if (found < 1) {
		fprintf(stderr,
				"A kernel module could not be loaded, check your installation... %d\n",
				found);
		exit(-1);
	}
#endif
}

static void
remove_modules(void)
{
	int ret;

	ret = system("/sbin/rmmod cpc &>/dev/null");

	if (ret != 0)
		fprintf(stderr, "CPC: Could not remove kernel module cpc\n");
}

static void
host_init(void)
{
	struct sockaddr_in *ina;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	int i, iw_sock, if_sock;
	char buf[1024], tmp_ifname[IFNAMSIZ];
	char ifnames[(IFNAMSIZ + 1) * MAX_NR_INTERFACES], *iface;
	char *ifname;

	memset(&this_host, 0, sizeof(struct host_info));
	memset(dev_indices, 0, sizeof(unsigned int) * MAX_NR_INTERFACES);

	/* No interface was given... search for first wireless. */
	iw_sock = socket(PF_INET, SOCK_DGRAM, 0);
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(iw_sock, SIOCGIFCONF, &ifc) < 0) {
		fprintf(stderr, "Could not get wireless info\n");
		exit(-1);
	}
	ifr = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof(struct ifreq); i >= 0; i--, ifr++) {
		struct iwreq req;

		strcpy(req.ifr_name, ifr->ifr_name);
		if (ioctl(iw_sock, SIOCGIWNAME, &req) >= 0) {
			strcpy(tmp_ifname, ifr->ifr_name);
			break;
		}
	}
	/* Did we find a wireless interface? */
	if (!strlen(tmp_ifname)) {
		fprintf(stderr, "\nCould not find a wireless interface!\n");
		fprintf(stderr, "Use -i <interface> to override...\n\n");
		exit(-1);
	}
	strcpy(ifreq.ifr_name, tmp_ifname);
	if (ioctl(iw_sock, SIOCGIFINDEX, &ifreq) < 0) {
		fprintf(stderr, "Could not get index of %s", tmp_ifname);
		close(if_sock);
		exit(-1);
	}
	close(iw_sock);

	ifname = tmp_ifname;

	strcpy(ifnames, ifname);

	/*
	 * TODO: maybe this need to remove
	 */
	this_host.seqno = 1;

	this_host.nif = 0;

	gettimeofday(&this_host.bcast_time, NULL);

	if_sock = socket(AF_INET, SOCK_DGRAM, 0);

	iface = strtok(ifname, ",");

	do {
		strcpy(ifreq.ifr_name, iface);
		if (ioctl(if_sock, SIOCGIFINDEX, &ifreq) < 0) {
			fprintf(stderr, "Could not get index of %s", iface);
			close(if_sock);
			exit(-1);
		}
		this_host.devs[this_host.nif].ifindex = ifreq.ifr_ifindex;
		dev_indices[this_host.nif++] = ifreq.ifr_ifindex;

		strcpy(DEV_IFINDEX(ifreq.ifr_ifindex).ifname, iface);

		/* Get IP-address of interface... */
		ina = get_if_info(iface, SIOCGIFADDR);
		if (ina == NULL)
			exit(-1);

		DEV_IFINDEX(ifreq.ifr_ifindex).ipaddr = ina->sin_addr;

		/* Get netmask of interface... */
		ina = get_if_info(iface, SIOCGIFNETMASK);
		if (ina == NULL)
			exit(-1);

		DEV_IFINDEX(ifreq.ifr_ifindex).netmask = ina->sin_addr;

		ina = get_if_info(iface, SIOCGIFBRDADDR);
		if (ina == NULL)
			exit(-1);

		DEV_IFINDEX(ifreq.ifr_ifindex).broadcast = ina->sin_addr;
		DEV_IFINDEX(ifreq.ifr_ifindex).enabled = 1;

		if (this_host.nif >= MAX_NR_INTERFACES)
			break;
	} while ((iface = strtok(NULL, ",")));

	close(if_sock);

	load_modules(ifnames);

#if 0	
	if (set_kernel_options() < 0) {
		fprintf(stderr, "CPC: Could not set kernel options\n");
		exit(-1);
	}
#endif
}

static void
start_event_loop(void)
{
	fd_set rfds, readers;
	int nfds = 0;
	int i, n;

	FD_ZERO(&readers);
	for (i = 0; i < nr_callbacks; i++) {
		FD_SET(callbacks[i].fd, &readers);

		if (callbacks[i].fd >= nfds)
			nfds = callbacks[i].fd + 1;
	}

	while (1) {
		memcpy((char *)&rfds, (char *)&readers, sizeof(rfds));

		if ((n = select(nfds, &rfds, NULL, NULL, NULL)) < 0) {
			if (errno != EINTR)
				fprintf(stderr, "CPC: Failed select (event loop)\n");

			continue;
		}

		if (n > 0) {
			for (i = 0; i < nr_callbacks; i++) {
				if (FD_ISSET(callbacks[i].fd, &rfds)) {
					(callbacks[i].func)(callbacks[i].fd);
				}
			}
		}
	}
}

int
cpc_prot_init_linux(void)
{
	/*atexit((void *)cleanup);*/
	cpc_rt_init();
	host_init();
	send_rt_sock_init();
	sock_init();

	start_event_loop();

    return 0;
}

static void
stop_event_loop(void)
{
	int i;

	for (i = 0; i < MAX_NR_INTERFACES; i++) {
		if (!DEV_NR(i).enabled)
			continue;
		close(DEV_NR(i).sock);
	}
}

int
cpc_prot_des_linux(void)
{
	cpc_rt_des();
	stop_event_loop();
	remove_modules();	

	return 0;
}

/*
 * this func just send routing packet without data
 */
void cpc_prot_send_linux(char *p, int n, struct in_addr dst)
{
	struct sockaddr_in dst_addr;
	int i;

	memset(&dst_addr, 0, sizeof(dst_addr));
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_addr = dst;
	dst_addr.sin_port = htons(ROUTING_PORT);

	for (i = 0; i < MAX_NR_INTERFACES; i++) {
		if (!DEV_NR(i).enabled)
			continue;

		sendto(DEV_NR(i).sock, p, n, 0,
				(struct sockaddr *)&dst_addr, sizeof(dst_addr));
	}
}

void cpc_prot_send_with_data_linux(char *p, int n, struct in_addr dst)
{
    nl_send_with_data(p, n, dst);
}
