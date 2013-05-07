#include <vector>

#include "msvr_neigh.h"
#include "msvr_nblist.h"
#include "../msvr_agent.h"
#include "../msvr_packet.h"
#include "../gps/msvr_gps.h"

#ifdef CPC_NS
#include <common/ip.h>
#endif

MsvrNeighTimer::MsvrNeighTimer(MsvrAgent *agent) : TimerHandler()
{
    agent_ = agent;

    msvr_nblinit(agent_->getNblist());
}

MsvrNeighTimer::~MsvrNeighTimer()
{
    msvr_nbldes(agent_->getNblist());
}

void
MsvrNeighTimer::expire(Event *e)
{
    // Send a hello packet
    sendInfo();

    // Scan neighbor list to remove old neighbors
    msvr_nblaging(agent_->getNblist());

    resched(NEIGH_TIMER + randSend_.uniform(0.0,0.25));
}

void
MsvrNeighTimer::sendInfo()
{
    struct hdr_msvr_info info;
    struct in_addr dst;

    // Build a neighbor info packet
    info.type = MSVR_TYPE_INFO;

    // Get GPS info
    info.id = agent_->getNodeId();
    info.x = agent_->getNodeX();
    info.y = agent_->getNodeY();
    info.s = agent_->getNodeSpeed();
    info.h = agent_->getNodeHeading();
    info.dst = agent_->getAddr();
    
    // Call CpcAgent::sendProtPacket() function to send it
    dst.s_addr = IP_BROADCAST;
    agent_->sendProtPacket((char *)&info, info.size(), dst);
}

struct in_addr
msvr_get_next_hop(std::list<msvr_nbentry>& nbl, int roadid1,
        int roadid2, int roadid3, double x1, double y1, double x2, double y2, bool first)
{
    struct msvr_nbentry *ep;

    if (first) {
        // XXX: old find a next hop method
        /*ep = msvr_nbl_find_furthest_nhop(nbl, roadid1, roadid2, x1, y1);*/

        // XXX: new find a next hop method
        ep = msvr_nbl_find_next_hop(nbl, roadid1, roadid2, roadid3, x1, y1, x2, y2);

        if (ep == NULL) {
            struct in_addr res;
            res.s_addr = -1;
            return res;
        }

        return ep->nbe_ninfo.n_dst;
    }
}

bool
msvr_find_dst(std::list<msvr_nbentry>& nbl, struct in_addr dst)
{
    return msvr_nbl_find_dst(nbl, dst);
}
