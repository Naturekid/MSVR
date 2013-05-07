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
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <linux/sockios.h> 

#include "nl.h"
#include "defs.h"
#include "cpc_config_linux.h"
#include "../cpc_debug.h"
#include "../linux_kernel/cpc_netlink.h"

struct nl_sock {
    int sock;
    int seq;
    struct sockaddr_nl local;
};

struct sockaddr_nl peer = {
    AF_NETLINK,
    0,
    0,
    0,
};

struct nl_sock nlsock;
struct nl_sock rtsock;

#define BUFLEN 256

#define SO_RECVBUF_SIZE 256*1024
/*
 * TODO: it maybe need to be enlarged
 */
#define RECV_BUF_SIZE 256

static void nl_cpc_callback(int sock)
{
    int len;
    socklen_t addrlen;
    struct nlmsghdr *nlm;
    struct nlmsgerr *nlmerr;
    char buf[BUFLEN];
    struct in_addr dst, src;
    struct cpc_rt_msg *m;
    /*rt_table_t *rt, *fwd_rt, *rev_rt;*/

    addrlen = sizeof(struct sockaddr_nl);

    len = recvfrom(sock, buf, BUFLEN, 0,
            (struct sockaddr *)&peer, &addrlen);
    if (len <= 0)
        return;

    nlm = (struct nlmsghdr *)buf;

    switch (nlm->nlmsg_type) {
        case NLMSG_ERROR:
            nlmerr = (struct nlmsgerr *)NLMSG_DATA(nlm);
            if (nlmerr->error == 0) {
                fprintf(stderr, "NLMSG_ACK\n");
            } else {
                fprintf(stderr, "NLMSG_ERROR, error=%d type=%s",
                        nlmerr->error, 
                        cpc_msg_type_to_str(nlmerr->msg.nlmsg_type));
            }
            break;
        case CPC_ROUTE_REQ:
            m = (struct cpc_rt_msg *)NLMSG_DATA(nlm);
            dst.s_addr = m->dst;

            callback_set.route_req_callback(dst);
            break;
        case CPC_ROUTE_UPDATE:
            m = (struct cpc_rt_msg *)NLMSG_DATA(nlm);
            dst.s_addr = m->dst;
            src.s_addr = m->src;

            /*
             * TODO: need to think
             */
            break;
        case CPC_SEND_ERR:
            m = (struct cpc_rt_msg *)NLMSG_DATA(nlm);
            dst.s_addr = m->dst;
            src.s_addr = m->src;

            /*
             * TODO: need to think
             */
            break;
        default:
            fprintf(stderr, "CPC: Got msg type %d\n", nlm->nlmsg_type);
            break;
    }
}

    static void
nl_rt_callback(int sock)
{
    int len, attrlen;
    socklen_t addrlen;
    struct nlmsghdr *nlm;
    struct nlmsgerr *nlmerr;
    char buf[BUFLEN];
    struct ifaddrmsg *ifm;
    struct rtattr *rta;

    addrlen = sizeof(struct sockaddr_nl);

    len = recvfrom(sock, buf, BUFLEN, 0,
            (struct sockaddr *)&peer, &addrlen);
    if (len <= 0)
        return;

    nlm = (struct nlmsghdr *)buf;

    switch (nlm->nlmsg_type) {
        case NLMSG_ERROR:
            nlmerr = (struct nlmsgerr *)NLMSG_DATA(nlm);
            if (nlmerr->error == 0) {
                fprintf(stderr, "NLMSG_ACK\n");
            } else {
                fprintf(stderr, "NLMSG_ERROR, error=%d type=%s",
                        nlmerr->error, 
                        cpc_msg_type_to_str(nlmerr->msg.nlmsg_type));
            }
            break;
        case RTM_NEWLINK:
            cpc_debug("RTM_NEWLINK\n");
            break;
        case RTM_NEWADDR:
            ifm = (struct ifaddrmsg *)NLMSG_DATA(nlm);

            rta = (struct rtattr *)((char *)ifm + sizeof(ifm));
            attrlen = nlm->nlmsg_len - sizeof(struct nlmsghdr) -
                sizeof(struct ifaddrmsg);

            for ( ; RTA_OK(rta, attrlen); rta = RTA_NEXT(rta, attrlen)) {
                struct in_addr ifaddr;

                memcpy(&ifaddr, RTA_DATA(rta), RTA_PAYLOAD(rta));

                fprintf(stderr, "Interface index %d changed address to %s",
                        ifm->ifa_index, ip_to_str(ifaddr));
            }
            break;
        case RTM_NEWROUTE:
            break;
        default:
            fprintf(stderr, "CPC: Got msg type %d\n", nlm->nlmsg_type);
            break;
    }
}

    static int
nl_send(struct nl_sock *nl, struct nlmsghdr *n)
{
    int res;
    struct iovec iov = { (void *) n, n->nlmsg_len };
    struct msghdr msg =
    { (void *) &peer, sizeof(peer), &iov, 1, NULL, 0, 0 };

    if (!nl)
        return -1;

    n->nlmsg_seq = ++nl->seq;
    n->nlmsg_pid = nl->local.nl_pid;

    /* Request an acknowledgement by setting NLM_F_ACK */
    n->nlmsg_flags |= NLM_F_ACK;

    /* Send message to netlink interface. */
    res = sendmsg(nl->sock, &msg, 0);

    if (res < 0) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int prefix_length(int family, void *nm)
{
    int prefix = 0;

    if (family == AF_INET) {
        unsigned int tmp;
        memcpy(&tmp, nm, sizeof(unsigned int));

        while (tmp) {
            tmp = tmp << 1;
            prefix++;
        }
        return prefix;

    } else {
        cpc_debug("Unsupported address family\n");
    }

    return 0;
}

/* Utility function  comes from iproute2. 
Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
int addattr(struct nlmsghdr *n, int type, void *data, int alen)
{
    struct rtattr *attr;
    int len = RTA_LENGTH(alen);

    attr = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
    attr->rta_type = type;
    attr->rta_len = len;
    memcpy(RTA_DATA(attr), data, alen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

    return 0;
}

/* Function to add, remove and update entries in the kernel routing
 * table */
int nl_kern_route(int action, int flags, int family,
        int index, struct in_addr *dst, struct in_addr *gw,
        struct in_addr *nm, int metric)
{
    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char attrbuf[1024];
    } req;

    if (!dst || !gw)
        return -1;

    req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.nlh.nlmsg_type = action;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | flags;
    req.nlh.nlmsg_pid = 0;

    req.rtm.rtm_family = family;

    if (!nm)
        req.rtm.rtm_dst_len = sizeof(struct in_addr) * 8;
    else
        req.rtm.rtm_dst_len = prefix_length(AF_INET, nm);

    req.rtm.rtm_src_len = 0;
    req.rtm.rtm_tos = 0;
    req.rtm.rtm_table = RT_TABLE_MAIN;
    req.rtm.rtm_protocol = 100;
    req.rtm.rtm_scope = RT_SCOPE_LINK;
    req.rtm.rtm_type = RTN_UNICAST;
    req.rtm.rtm_flags = 0;

    addattr(&req.nlh, RTA_DST, dst, sizeof(struct in_addr));

    if (memcmp(dst, gw, sizeof(struct in_addr)) != 0) {
        req.rtm.rtm_scope = RT_SCOPE_UNIVERSE;
        addattr(&req.nlh, RTA_GATEWAY, gw, sizeof(struct in_addr));
    }

    if (index > 0)
        addattr(&req.nlh, RTA_OIF, &index, sizeof(index));

    addattr(&req.nlh, RTA_PRIORITY, &metric, sizeof(metric));

    return nl_send(&rtsock, &req.nlh);
}

int sock_init(void)
{
    int status;
    unsigned int addrlen;

    memset(&peer, 0, sizeof(struct sockaddr_nl));
    peer.nl_family = AF_NETLINK;
    peer.nl_pid = 0;
    peer.nl_groups = 0;

    /*
     * this socket communicates with cpc kernel module
     */
    nlsock.seq = 0;
    nlsock.local.nl_family = AF_NETLINK;
    nlsock.local.nl_pid = getpid();
    nlsock.local.nl_groups = CPC_NOTIFY;

    nlsock.sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_CPC);
    if (nlsock.sock < 0) {
        perror("CPC: Unable to craete kernel netlink socket");
        exit(-1);
    }

    status = bind(nlsock.sock,
            (struct sockaddr *)&nlsock.local,
            sizeof(nlsock.local));
    if (status == -1) {
        perror("CPC: Bind for kernel netlink socket failed");
        exit(-1);
    }

    addrlen = sizeof(nlsock.local);

    if (getsockname(nlsock.sock,
                (struct sockaddr *)&nlsock.local,
                &addrlen) < 0) {
        perror("CPC: Getsockname failed");
        exit(-1);
    }

    if (register_callback_func(nlsock.sock, nl_cpc_callback) < 0) {
        perror("CPC: Could not register callback");
        exit(-1);
    }

    /*
     * This socket communicates with rt netlink
     */
    rtsock.seq = 0;
    rtsock.local.nl_family = AF_NETLINK;
    rtsock.local.nl_pid = getpid();
    rtsock.local.nl_groups =
        RTMGRP_NOTIFY | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;

    rtsock.sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (rtsock.sock < 0) {
        perror("CPC: Unable to create rt netlink socket");
        exit(-1);
    }

    addrlen = sizeof(rtsock.local);

    status = bind(rtsock.sock,
            (struct sockaddr *)&rtsock.local,
            addrlen);
    if (status == -1) {
        perror("CPC: Bind for rt netlink failed");
        exit(-1);
    }

    if (getsockname(rtsock.sock,
                (struct sockaddr *)&rtsock.local,
                &addrlen) < 0) {
        perror("CPC: Getsockname failed");
        exit(-1);
    }

    if (register_callback_func(rtsock.sock, nl_rt_callback) < 0) {
        perror("CPC: could not register callback");
        exit(-1);
    }

    return 0;
}

#define RECV_BUF_SIZE 1024
static char recv_buf[RECV_BUF_SIZE];

static void routing_packet_read(int fd)
{
    struct in_addr src, dst;
    int i, len, ttl = -1;
    /*AODV_msg *aodv_msg;*/
    struct dev_info *dev;
    struct msghdr msgh;
    struct cmsghdr *cmsg;
    struct iovec iov;
    char ctrlbuf[CMSG_SPACE(sizeof(int)) +
        CMSG_SPACE(sizeof(struct in_pktinfo))];
    struct sockaddr_in src_addr;

    iov.iov_base = recv_buf;
    iov.iov_len = RECV_BUF_SIZE;
    msgh.msg_name = &src_addr;
    msgh.msg_namelen = sizeof(src_addr);
    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    msgh.msg_control = ctrlbuf;
    msgh.msg_controllen = sizeof(ctrlbuf);

    len = recvmsg(fd, &msgh, 0);

    if (len < 0) {
        fprintf(stderr, "receive ERROR len=%d!", len);
        return;
    }

    src.s_addr = src_addr.sin_addr.s_addr;

    /* Get the ttl and destination address from the control message */
    for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL;
            cmsg = CMSG_NXTHDR_FIX(&msgh, cmsg)) {
        if (cmsg->cmsg_level == SOL_IP) {
            switch (cmsg->cmsg_type) {
                case IP_TTL:
                    ttl = *(CMSG_DATA(cmsg));
                    break;
                case IP_PKTINFO:
                    dst.s_addr =
                        ((struct in_pktinfo *) CMSG_DATA(cmsg))->ipi_addr.s_addr;
            }
        }
    }

    if (ttl < 0) {
        fprintf(stderr, "No TTL, packet ignored!");
        return;
    }

    /* Ignore messages generated locally */
    for (i = 0; i < MAX_NR_INTERFACES; i++)
        if (this_host.devs[i].enabled &&
                memcmp(&src, &this_host.devs[i].ipaddr,
                    sizeof(struct in_addr)) == 0)
            return;

    /*aodv_msg = (AODV_msg *) recv_buf;*/

    /*dev = devfromsock(fd);*/

    callback_set.prot_callback(recv_buf, RECV_BUF_SIZE);

#if 0 
    if (!dev) {
        DEBUG(LOG_ERR, 0, "Could not get device info!\n");
        return;
    }
#endif
}

    void
send_rt_sock_init(void)
{
    struct sockaddr_in addr;
    struct ifreq ifr;
    int i, retval = 0;
    int on = 1;
    int tos = IPTOS_LOWDELAY;
    int bufsize = SO_RECVBUF_SIZE;
    socklen_t optlen = sizeof(bufsize);

    if (this_host.nif == 0) {
        fprintf(stderr, "CPC: No interfaces configured\n");
        exit(-1);
    }

    for (i = 0; i < MAX_NR_INTERFACES; i++) {
        if (!DEV_NR(i).enabled)
            continue;

        DEV_NR(i).sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (DEV_NR(i).sock < 0) {
            perror("CPC: create udp socket failed");
            exit(-1);
        }

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(ROUTING_PORT);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        retval = bind(DEV_NR(i).sock, (struct sockaddr *)&addr,
                sizeof(struct sockaddr));
        if (retval < 0) {
            perror("CPC: Bind faild");
            exit(-1);
        }

        if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_BROADCAST,
                    &on, sizeof(int)) < 0) {
            perror("CPC: SO_BROADCAST failed");
            exit(-1);
        }

        memset(&ifr, 0, sizeof(struct ifreq));
        strcpy(ifr.ifr_name, DEV_NR(i).ifname);

        if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_BINDTODEVICE,
                    &ifr, sizeof(ifr)) < 0) {
            fprintf(stderr, "CPC: SO_BINDTODEVICE failed for %s", DEV_NR(i).ifname);
            exit(-1);
        }

        if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_PRIORITY,
                    &tos, sizeof(int)) < 0) {
            perror("CPC: SO_PRIORITY failed");
            exit(-1);
        }

        if (setsockopt(DEV_NR(i).sock, SOL_IP, IP_RECVTTL,
                    &on, sizeof(int)) < 0) {
            perror("CPC: IP_RECVTTL failed");
            exit(-1);
        }

        if (setsockopt(DEV_NR(i).sock, SOL_IP, IP_PKTINFO,
                    &on, sizeof(int)) < 0) {
            perror("CPC: IP_PKTINFO failed");
            exit(-1);
        }

        for ( ; ; bufsize -= 1024) {
            if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_RCVBUF,
                        (char *) &bufsize, optlen) == 0) {
                fprintf(stderr, "Receive buffer size set to %d", bufsize);
                break;
            }

            if (bufsize < RECV_BUF_SIZE) {
                fprintf(stderr, "Could not set receive buffer size");
                exit(-1);
            }
        }

        register_callback_func(DEV_NR(i).sock, routing_packet_read);
    }
}

int nl_send_add_route_msg(struct in_addr dest, struct in_addr next_hop,
        int metric, u_int32_t lifetime, int rt_flags,
        int ifindex)
{
    struct {
        struct nlmsghdr n;
        struct cpc_rt_msg m;
    } areq;

    memset(&areq, 0, sizeof(areq));

    areq.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct cpc_rt_msg));
    areq.n.nlmsg_type = CPC_ROUTE_ADD;
    areq.n.nlmsg_flags = NLM_F_REQUEST;

    areq.m.dst = dest.s_addr;
    areq.m.nhop = next_hop.s_addr;
    areq.m.time = lifetime;
    areq.m.ifindex = ifindex;

    if (nl_send(&nlsock, &areq.n) < 0) {
        cpc_debug("Failed to send netlink message\n");
        return -1;
    }

    return nl_kern_route(RTM_NEWROUTE, NLM_F_CREATE,
            AF_INET, ifindex, &dest, &next_hop, NULL, metric);
}

int nl_send_del_route_msg(struct in_addr dest, struct in_addr next_hop, int metric)
{
    int index = -1;
    struct {
        struct nlmsghdr n;
        struct cpc_rt_msg m;
    } areq;

    fprintf(stderr, "Send DEL_ROUTE to kernel: %s\n", ip_to_str(dest));

    memset(&areq, 0, sizeof(areq));

    areq.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct cpc_rt_msg));
    areq.n.nlmsg_type = CPC_ROUTE_DEL;
    areq.n.nlmsg_flags = NLM_F_REQUEST;

    areq.m.dst = dest.s_addr;
    areq.m.nhop = next_hop.s_addr;
    areq.m.time = 0;

    if (nl_send(&nlsock, &areq.n) < 0) {
        cpc_debug("Failed to send netlink message\n");
        return -1;
    }

    return nl_kern_route(RTM_DELROUTE, 0, AF_INET, index, &dest, &next_hop,
            NULL, metric);
}

int nl_send_queue(struct in_addr dest)
{
    struct {
        struct nlmsghdr n;
        struct cpc_rt_msg m;
    } req;

    fprintf(stderr, "Send SEND_QUEUE to kernel: %s\n", ip_to_str(dest));

    memset(&req, 0, sizeof(req));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct cpc_rt_msg));
    req.n.nlmsg_type = CPC_SEND_QUEUE;
    req.n.nlmsg_flags = NLM_F_REQUEST;

    req.m.dst = dest.s_addr;

    if (nl_send(&nlsock, &req.n) < 0) {
        cpc_debug("Failed to send netlink message\n");
        return -1;
    }

    return 0;
}

int nl_drop_queue(struct in_addr dest)
{
    struct {
        struct nlmsghdr n;
        struct cpc_rt_msg m;
    } req;

    fprintf(stderr, "Send DROP_QUEUE to kernel: %s\n", ip_to_str(dest));

    memset(&req, 0, sizeof(req));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct cpc_rt_msg));
    req.n.nlmsg_type = CPC_DROP_QUEUE;
    req.n.nlmsg_flags = NLM_F_REQUEST;

    req.m.dst = dest.s_addr;

    if (nl_send(&nlsock, &req.n) < 0) {
        cpc_debug("Failed to send netlink message\n");
        return -1;
    }

    return 0;
}

int nl_send_with_data(char *p, int n, struct in_addr dst)
{
    struct {
        struct nlmsghdr n;
        struct cpc_stateless_msg m;
    } req;

    fprintf(stderr, "Send DROP_QUEUE to kernel: %s\n", ip_to_str(dst));

    memset(&req, 0, sizeof(req));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct cpc_stateless_msg));
    req.n.nlmsg_type = CPC_SEND_WITH_DATA;
    req.n.nlmsg_flags = NLM_F_REQUEST;

    req.m.p = p;
    req.m.n = n;
    req.m.dst = dst.s_addr;

    if (nl_send(&nlsock, &req.n) < 0) {
        cpc_debug("Failed to send netlink message\n");
        return -1;
    }
    return 0;
}
