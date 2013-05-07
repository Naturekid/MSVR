#ifndef CPC_LINUX_MOD_H
#define CPC_LINUX_MOD_H

#include <linux/types.h>

struct cpchdr {
    __be16 app_port;
    __u32 rt_len;
    char *rt_data;
};

#endif /* CPC_LINUX_MOD_H */
