#ifndef CPC_DEBUG_H
#define CPC_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define cpc_debug(fmt, ...) do {	\
	fprintf(stderr, "%s(%d): ", __FILE__, __LINE__); \
	fprintf(stderr, fmt, ##__VA_ARGS__); \
} while (0);
#else /* !DEBUG */
#define cpc_debug(args)
#endif /* DEBUG */

#ifdef CPC_LINUX

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* This function lets you print more than one IP address at the same time */
inline char *ip_to_str(struct in_addr addr)
{
    static char buf[16 * 4];
    static int index = 0;
    char *str;

    addr.s_addr = htonl(addr.s_addr);
    strcpy(&buf[index], inet_ntoa(addr));
    str = &buf[index];
    index += 16;
    index %= 64;
    return str;
}

#endif /* CPC_LINUX */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CPC_DEBUG_H */
