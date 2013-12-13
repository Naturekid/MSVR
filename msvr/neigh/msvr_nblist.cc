#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <time.h>
#include <list>

#include "../msvr_agent.h"
#include "msvr_nblist.h"
#include "../gps/msvr_gps.h"

/*std::list<msvr_nbentry> msvr_nbl;*/

void
msvr_nblinit(std::list<msvr_nbentry>& nbl)
{
    // empty
}

void
msvr_nbldes(std::list<msvr_nbentry>& nbl)
{
    // empty
}

void
msvr_nbladd(std::list<msvr_nbentry>& nbl, struct msvr_ninfo *ip)
{
    struct msvr_nbentry ep;

    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if (iter->nbe_ninfo.n_id == ip->n_id) {
            /*iter->nbe_ninfo.n_id = ip->n_id;*/
            iter->nbe_ninfo.n_x = ip->n_x;
            iter->nbe_ninfo.n_y = ip->n_y;
            iter->nbe_ninfo.n_speed = ip->n_speed;
            iter->nbe_ninfo.n_heading = ip->n_heading;
            iter->nbe_ninfo.n_roadid = ip->n_roadid;
            memcpy(&ep.nbe_ninfo.n_dst, &ip->n_dst, sizeof(struct in_addr));
            gettimeofday(&iter->nbe_ts, NULL);
            return;
        }
    }

    memset(&ep, 0, sizeof(struct msvr_nbentry));

    ep.nbe_ninfo.n_id = ip->n_id;
    ep.nbe_ninfo.n_x = ip->n_x;
    ep.nbe_ninfo.n_y = ip->n_y;
    ep.nbe_ninfo.n_speed = ip->n_speed;
    ep.nbe_ninfo.n_heading = ip->n_heading;
    ep.nbe_ninfo.n_roadid = ip->n_roadid;
    memcpy(&ep.nbe_ninfo.n_dst, &ip->n_dst, sizeof(struct in_addr));
    gettimeofday(&ep.nbe_ts, NULL);

    nbl.push_front(ep);
}

void
msvr_nbldel(std::list<msvr_nbentry>& nbl, struct msvr_nbentry *ep)
{
    if (ep == NULL)
        return;

    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if (iter->nbe_ninfo.n_id == ep->nbe_ninfo.n_id)
            nbl.erase(iter);
    }
}

void
msvr_nblupdate(std::list<msvr_nbentry>& nbl, struct msvr_nbentry *ep, struct msvr_ninfo *ip)
{
    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if (iter->nbe_ninfo.n_id == ip->n_id) {
            iter->nbe_ninfo.n_x = ip->n_x;
            iter->nbe_ninfo.n_y = ip->n_y;
            iter->nbe_ninfo.n_speed = ip->n_speed;
            iter->nbe_ninfo.n_heading = ip->n_heading;
            iter->nbe_ninfo.n_roadid = ip->n_roadid;
            iter->nbe_ninfo.n_dst = ip->n_dst;
            gettimeofday(&iter->nbe_ts, NULL);
        }
    }
}

struct msvr_nbentry *
msvr_nblfind(std::list<msvr_nbentry>& nbl, int id)
{
    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if (iter->nbe_ninfo.n_id == id)
            return &(*iter);
    }

    return NULL;
}

bool
msvr_nbl_find_dst(std::list<msvr_nbentry>& nbl, struct in_addr dst)
{
    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if (iter->nbe_ninfo.n_dst.s_addr == dst.s_addr)
            return true;
    }

    return false;
}

void
msvr_nblaging(std::list<msvr_nbentry>& nbl)
{
    struct timeval now;

    gettimeofday(&now, NULL);

tryagain:
    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if (float(now.tv_sec - iter->nbe_ts.tv_sec) >= MSVR_MAX_NB_AGING) {
            /*msvr_nbldel(nbl, &(*iter));*/
            nbl.erase(iter);
            goto tryagain;
        }
    }
}

struct msvr_nbentry *
msvr_nbl_find_furthest_nhop(std::list<msvr_nbentry>& nbl, int roadid1,
                       int roadid2, double x1, double y1)
{
    struct msvr_nbentry *res = NULL;
    double len = 0.0;

    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {

        if (iter->nbe_ninfo.n_roadid == roadid1) {
            double tmplen = msvr_cal_dist(x1, y1,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);

            // TODO: need to improved to select a better next hop
            // here we just further next hop
            if (tmplen >= len/* && tmplen <= 100.0*/) {
                len = tmplen;
                res = &(*iter);
            }
        } else if (roadid2 != -1 &&
                   iter->nbe_ninfo.n_roadid == roadid2) {
            double tmplen = msvr_cal_dist(x1, y1,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);
            if (tmplen >= len) {
                len = tmplen;
                res = &(*iter);
            }
        }
    }

    return res;
}

struct msvr_nbentry *
msvr_nbl_find_next_hop(std::list<msvr_nbentry>& nbl, int roadid1,
                       int roadid2, int roadid3, double x1, double y1, double x2, double y2)
{
    struct msvr_nbentry *res = NULL;
    double len1 = 200.0;
    double len2 = msvr_cal_dist(x1, y1, x2, y2);
    double goal = 100.0;
    extern MsvrMap *MSVRMAP;

    //msvr_nbl_print(nbl);//added by yyq
    assert(nbl.size());
    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
    	//printf("x:%f y:%f roadid:%d roadid1:%d\n",iter->nbe_ninfo.n_x,iter->nbe_ninfo.n_y,iter->nbe_ninfo.n_roadid,roadid1);
        //if (iter->nbe_ninfo.n_roadid == roadid1) {
    	if (MSVRMAP->nodeInRoad(iter->nbe_ninfo.n_x,iter->nbe_ninfo.n_y,roadid1)) {
            double tmplen = msvr_cal_dist(x1, y1,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);
            double dstlen = msvr_cal_dist(x2, y2,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);

            if (abs(tmplen - goal) < len1 &&
                dstlen < len2) {
                len1 = abs(tmplen - goal);
                res = &(*iter);
            } 
        } else if (roadid2 != -1 &&
                   iter->nbe_ninfo.n_roadid == roadid2) {
            double tmplen = msvr_cal_dist(x1, y1,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);
            double dstlen = msvr_cal_dist(x2, y2,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);

            if (abs(tmplen - goal) < len1 &&
                dstlen < len2) {
                len1 = abs(tmplen - goal);
                res = &(*iter);
            }
        } else if (iter->nbe_ninfo.n_roadid == roadid3) {
            double tmplen = msvr_cal_dist(x1, y1,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);
            double dstlen = msvr_cal_dist(x2, y2,
                    iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y);

            if (abs(tmplen - goal) < len1 &&
                dstlen < len2) {
                len1 = abs(tmplen - goal);
                res = &(*iter);
            }
        }
    }

    return res;
}

void
msvr_nbl_print(std::list<msvr_nbentry>& nbl)
{
    fprintf(stderr, "====\n");
    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        fprintf(stderr, "id %d (%f, %f):(%f %f) roadid %d dst %d\n",
                iter->nbe_ninfo.n_id, iter->nbe_ninfo.n_x, iter->nbe_ninfo.n_y,
                iter->nbe_ninfo.n_speed, iter->nbe_ninfo.n_heading,
                iter->nbe_ninfo.n_roadid, iter->nbe_ninfo.n_dst);
    }
    fprintf(stderr, "====\n");
}
