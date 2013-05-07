#ifndef NEIGH_LIST_H
#define NEIGH_LIST_H

#include <sys/time.h>

#include <common/packet.h>

#include <list>

#define MSVR_MAX_NB_AGING 4.0

struct msvr_ninfo {
    int    n_id;     /* NOTE: it need a char * type */

    /* NOTE: define a struct to store them */
    double n_x;
    double n_y;
    double n_speed;
    double n_heading;
    int n_roadid;
    struct in_addr n_dst;
};

struct msvr_nbentry {
    struct msvr_ninfo nbe_ninfo;
    struct timeval nbe_ts;
};

void msvr_nblinit(std::list<msvr_nbentry>& nbl);
void msvr_nbldes(std::list<msvr_nbentry>& nbl);
void msvr_nbladd(std::list<msvr_nbentry>& nbl, struct msvr_ninfo *ip);
void msvr_nbldel(std::list<msvr_nbentry>& nbl, struct msvr_nbentry *ep);
void msvr_nblupdate(std::list<msvr_nbentry>& nbl, struct msvr_nbentry *ep, struct msvr_ninfo *ip);
struct msvr_nbentry *msvr_nblfind(std::list<msvr_nbentry>& nbl, int id);
void msvr_nblaging(std::list<msvr_nbentry>& nbl);

bool msvr_nbl_find_dst(std::list<msvr_nbentry>& nbl, struct in_addr dst);
struct msvr_nbentry *msvr_nbl_find_furthest_nhop(std::list<msvr_nbentry>& nbl, int roadid1,
                                            int roadid2, double x1, double y1);
struct msvr_nbentry *msvr_nbl_find_next_hop(std::list<msvr_nbentry>& nbl, int roadid1,
                                            int roadid2, int roadid3, double x1, double y1, double x2, double y2);

void msvr_nbl_print(std::list<msvr_nbentry>& nbl);

#endif /* NEIGH_LIST_H */
