#ifndef MSVR_PAKCET_H
#define MSVR_PAKCET_H

#include <common/packet.h>

#include "msvr_agent.h"

#define MSVR_TYPE_INFO      0x01 // neighbor info packet
#define MSVR_TYPE_ROUTING   0x02 // routing packet

#define HDR_MSVR(p)         ((struct hdr_msvr*) hdr_msvr::access(p))
#define HDR_MSVR_INFO(p)    ((struct hdr_msvr_info*) hdr_msvr::access(p))
#define HDR_MSVR_ROUTING(p) ((struct hdr_msvr_routing*) hdr_msvr::access(p))

struct hdr_msvr_info {
    u_int8_t type;
    u_int32_t id;
    float x;
    float y;
    float s;
    float h;
    struct in_addr dst;

    inline int size() {
        int sz = sizeof(u_int8_t) +
            sizeof(u_int32_t) + 8 * sizeof(float) + sizeof(struct in_addr);

        return sz;
    }
};

struct hdr_msvr_routing {
    u_int8_t  type;
    u_int16_t length;
    u_int8_t  hlen; 
    int       m : 1;
    int       t : 2;
    int       c : 1;
    u_int32_t seq;
    float     sx;
    float     sy;
    float     dx;
    float     dy;
    char      *path;
    char      app[0];

    inline int size() {
        int sz = sizeof(u_int8_t) * 2 +
                 sizeof(u_int16_t) +
                 sizeof(u_int32_t) +
                 sizeof(int) +
                 sizeof(float) * 4;
        return sz;
    }
};

union hdr_all_msvr {
    hdr_msvr mh;
    hdr_msvr_info mih;
    hdr_msvr_routing mrh;
};

#endif // MSVR_PAKCET_H
