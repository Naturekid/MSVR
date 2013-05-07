#ifndef MSVR_NEIGH_H
#define MSVR_NEIGH_H

#include <vector>
#include <list>
#include <timer-handler.h>
#include <tools/random.h>

#include "msvr_nblist.h"

#define NEIGH_TIMER 1.0

class MsvrAgent;

class MsvrNeighTimer : public TimerHandler {
    public:
        MsvrNeighTimer(MsvrAgent *);
        virtual ~MsvrNeighTimer();

    protected:
        virtual void expire(Event *e);
        MsvrAgent *agent_;

    private:
        void sendInfo();

        RNG randSend_;
};

/*struct in_addr msvr_get_next_hop(std::list<msvr_nbentry>& nbl, int roadid1, int roadid2,*/
                                 /*double x1, double y1, bool first);*/
struct in_addr msvr_get_next_hop(std::list<msvr_nbentry>& nbl, int roadid1, int roadid2, int roadid3,
                                 double x1, double y1, double x2, double y2, bool first);
bool msvr_find_dst(std::list<msvr_nbentry>& nbl, struct in_addr dst);

#endif /* MSVR_NEIGH_H */
