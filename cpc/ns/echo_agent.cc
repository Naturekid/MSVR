#include <common/agent.h>
#include <common/packet.h>

#include "echo_agent.h"

int hdr_echo::offset_;

static class EchoHeaderClass : public PacketHeaderClass {
	public:
		EchoHeaderClass() : PacketHeaderClass("PacketHeader/ECHO", HDR_ECHO_SIZE) {
			bind_offset(&hdr_echo::offset_);
		}
} class_echohdr;

static class EchoClass : public TclClass {
	public:
		EchoClass() : TclClass("Agent/ECHO") {}
		TclObject* create(int, const char* const*) {
			return (new EchoAgent);
		}
} class_echo;

EchoAgent::EchoAgent() : CpcAgent(PT_ECHO)
{
}

int EchoAgent::command(int argc, const char* const* argv)
{
	if (argc == 2) {
		if (strcmp(argv[1], "send") == 0) {
			Packet* p = allocpkt();
			struct hdr_echo* hdrcom = (hdr_echo*)hdr_echo::access(p);
			hdrcom->state = 0;
			hdrcom->send_time = Scheduler::instance().clock();
			send(p, 0);
			return (TCL_OK);
		}
	}

	return CpcAgent::command(argc, argv);
}

void EchoAgent::recv(Packet* p, Handler *)
{
	struct hdr_echo* hdrecho = (hdr_echo*)hdr_echo::access(p);
	if (hdrecho->state == 0) {
		double stime = hdrecho->send_time;
		Packet::free(p);
		Packet* ret = allocpkt();
		struct hdr_echo* hdrret = (hdr_echo*) hdr_echo::access(p);
		hdrret->state = 1;
		hdrret->send_time = stime;
		send(ret, 0);
	} else {
		free(p);
	}
}
