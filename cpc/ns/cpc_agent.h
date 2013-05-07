#ifndef CPC_AGENT_H
#define CPC_AGENT_H

#include <common/agent.h>
#include <common/packet.h>
#include <classifier/classifier-port.h>

#include "../cpc_defs.h"
#include "../cpc_prot.h"

//#define DEBUG

#ifndef HDR_ACCESS
#define HDR_ACCESS(p) NULL
#endif /* HDR_ACCESS */

class CpcAgent : public Agent {

	public:
		CpcAgent(packet_t type);
		virtual ~CpcAgent();

		virtual void recv(Packet *p, Handler *h);
		virtual void dropPacket(Packet *p, const char *msg);
        virtual void dropPacket(struct in_addr dst, const char *msg);

		virtual void sendData(Packet* p, struct in_addr dst);

		virtual void sendProtPacket(char *p, int n, struct in_addr dst);
		virtual void sendProtPacketWithData(char *p, int n, struct in_addr src, struct in_addr dst, struct in_addr next);

        virtual struct in_addr getAddr() const {
            return myAddr_;
        }
        int DTNFlag_;
        int GetDTNFlag();
        void SetDTNFlag(int n);

	protected:
		process_route_prot protHandler_;
		process_route_table rtHandler_;
		process_request_route reqHandler_;
        void* (*access_cb)(const Packet* p);
		void processPacket(Packet *p);
		int command(int argc, const char * const *argv);

		struct in_addr myAddr_;
        PortClassifier* portDmux_;
};

#endif /* CPC_AGENT_H */
