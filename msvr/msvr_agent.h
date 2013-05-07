#ifndef MSVR_AGENT_H
#define MSVR_AGENT_H

#include <common/packet.h>
#include <common/agent.h>
#include <common/mobilenode.h>
#include <classifier/classifier-port.h>
#include <trace/trace.h>
#include <tools/random.h>

#define HDR_ACCESS(p) hdr_msvr::access(p)

#include <cpc/ns/cpc_agent.h>

#include "neigh/msvr_neigh.h"
#include "neigh/msvr_nblist.h"
#include "map/msvr_map.h"
#include "gps/msvr_gps.h"

#define HDR_MSVR_SIZE sizeof(hdr_msvr)

struct hdr_msvr {
    u_int8_t type;
    ns_addr_t dtn_recent_;
	static int offset_;

	inline static int& offset() {
		return offset_;
	}

	inline static void* access(const Packet* p) {
		return (void*)p->access(offset_);
	}

};

class MsvrAgent : public CpcAgent {
	public:
		MsvrAgent();
        virtual ~MsvrAgent();
		int command(int argc, const char* const* argv);

        // gps module interface
        double getNodeX();
        double getNodeY();
        double getNodeSpeed();
        double getNodeHeading();
        MsvrGps getGps() const {
            return gps_;
        }

        // neighbor module interface
        inline int getNodeId() {
            return myAddr_.s_addr;
        }

        // map module
        inline const MsvrMap& getMap() const {
            return map_;
        }

        std::list<msvr_nbentry>& getNblist() {
            return nbl_;
        }


    protected:
        Trace* tracetarget_;

    private:
        // private functions
        void start();

        // members
        MobileNode* node_;
        /*PortClassifier* portDmux_;*/

        MsvrNeighTimer neighTimer_;
        RNG randSend_;

        MsvrMap map_;
        MsvrGps gps_;

        std::list<msvr_nbentry> nbl_;

};

// callback functions
int routing_proto_cb(char *p, int *n, CpcAgent *agent);
int request_routing_cb(char *p, struct in_addr src, struct in_addr dst, int applen, CpcAgent *agent);

#endif // MSVR_AGENT_H
