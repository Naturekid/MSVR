#ifndef ECHO_AGENT_H
#define ECHO_AGENT_H

#include <common/packet.h>

#include "cpc_agent.h"

#define HDR_ECHO_SIZE sizeof(hdr_echo)

struct hdr_echo {
	double send_time;
	char state;

	static int offset_;

	inline static int& offset() {
		return offset_;
	}

	inline static hdr_echo* access(const Packet* p) {
		return (hdr_echo*) p->access(offset_);
	}
};

class EchoAgent : public CpcAgent {
	public:
		EchoAgent();
		int command(int argc, const char* const* argv);
		void recv(Packet*, Handler*);
};

#endif // ECHO_AGENT_H
