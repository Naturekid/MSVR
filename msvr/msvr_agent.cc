#include <common/agent.h>
#include <common/packet.h>
#include <trace/cmu-trace.h>

#include <vector>
#include <iostream>

#include "msvr_agent.h"
#include "msvr_packet.h"
#include "gps/msvr_gps.h"
#include "map/msvr_map.h"
#include "neigh/msvr_nblist.h"

MsvrMap *MSVRMAP = NULL;

int hdr_msvr::offset_;

static class MsvrHeaderClass : public PacketHeaderClass {
public:
	MsvrHeaderClass() : PacketHeaderClass("PacketHeader/MSVR", HDR_MSVR_SIZE) {
		bind_offset(&hdr_msvr::offset_);
	}
} class_msvrhdr;

static class MsvrClass : public TclClass {
public:
	MsvrClass() : TclClass("Agent/MSVR") {}
	TclObject* create(int, const char* const*) {
		return (new MsvrAgent);
	}
} class_msvr;

MsvrAgent::MsvrAgent() :
    										CpcAgent(PT_MSVR),neighTimer_(this),
    										map_(), gps_(this)
{
	// init callback function pointers
	protHandler_ = routing_proto_cb;
	reqHandler_ = request_routing_cb;
	access_cb = &hdr_msvr::access;

	msvr_nblinit(nbl_);

	MSVRMAP = &map_;
}

MsvrAgent::~MsvrAgent()
{
	msvr_nbldes(nbl_);
}

int MsvrAgent::command(int argc, const char* const* argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			start();

			return TCL_OK;
		}
	}

	if (argc == 3) {
		TclObject *obj;
		if (strcasecmp(argv[1], "addr") == 0) {
			myAddr_.s_addr = Address::instance().str2addr(argv[2]);
			return TCL_OK;
		}

		if ((obj = TclObject::lookup(argv[2])) == 0)
			goto base;

		if (strcasecmp(argv[1], "node") == 0) {
			node_ = static_cast<MobileNode*>(obj);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "port-dmux") == 0) {
			portDmux_ = static_cast<PortClassifier*>(obj);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "tracetarget") == 0) {
			tracetarget_ = static_cast<Trace*>(obj);
			return TCL_OK;
		}
	}

	base:
	return CpcAgent::command(argc, argv);
}

void
MsvrAgent::start()
{
	neighTimer_.resched(randSend_.uniform(0.0, 1.0));
}

double
MsvrAgent::getNodeX()
{
	return gps_.getPos(myAddr_.s_addr).x;
}

double
MsvrAgent::getNodeY()
{
	return gps_.getPos(myAddr_.s_addr).y;
}

double
MsvrAgent::getNodeSpeed()
{
	return gps_.getSpeed(myAddr_.s_addr);
}

double
MsvrAgent::getNodeHeading()
{
	return gps_.getHeading(myAddr_.s_addr);
}

void
recv_info(char* p, CpcAgent *agent)
{
	hdr_msvr_info* mih = (hdr_msvr_info*) p;
	struct msvr_ninfo info;

	memset(&info, 0, sizeof(struct msvr_ninfo));

	info.n_id = mih->id;
	info.n_x = mih->x;
	info.n_y = mih->y;
	info.n_speed = mih->s;
	info.n_heading = mih->h;
	Road r = MSVRMAP->getRoadByPos(mih->x, mih->y);
	info.n_roadid = r.id_;
	info.n_dst = mih->dst;

	msvr_nbladd(((MsvrAgent*)(agent))->getNblist(), &info);
	/*fprintf(stderr, "id %d:\n", agent->getAddr());*/
	/*msvr_nbl_print(((MsvrAgent *)(agent))->getNblist());*/
}

void
recv_routing(char *p)
{
	// NOTE: never reach here because MSVR is a stateless routing protocol.
	fprintf(stderr, "recv routing packet\n");
}

int
routing_proto_cb(char *p, int *n, CpcAgent *agent)
{
	hdr_msvr* mh = (hdr_msvr*)(p);

	switch (mh->type) {
	case MSVR_TYPE_INFO:
		recv_info(p, agent);
		break;
	case MSVR_TYPE_ROUTING:
		recv_routing(p);
		break;
	default:
		fprintf(stderr, "Error with msvr packet type.\n");
		return 1;//maybe got dtn broadcast things
		//recv_dtnbroad(p,agent);
		break;
	}

	// cpc_agent does not check return value.
	return 0;
}

char *
encode_path(const std::vector<int>& paths)
{
	char *enpath = NULL;

	enpath = (char *)malloc(sizeof(char) * paths.size() * 2);

	enpath[0] = paths.size() * 2 - 1;
	int i;
	std::vector<int>::const_iterator iter;
	for (i = 1, iter = paths.begin();
			iter != paths.end(); ++iter, i += 2) {
		enpath[i] = *iter;
		enpath[i + 1] = ',';
	}

	return enpath;
}

void
decode_path(std::vector<int>& paths, char* enpaths)
{
	int len = enpaths[0];

	for (int i = 1; i <= len; i += 2) {
		paths.push_back(enpaths[i]);
	}
}

int
request_routing_cb(char *p, struct in_addr src, struct in_addr dst, int applen, CpcAgent *agent)
{
	std::vector<int> paths;
	std::vector<int> paths_new;
	MobileNode *srcNode, *dstNode, *relayNode, *nextNode;
	struct point srcPos, dstPos, relayPos, nextPos;
	struct in_addr next;
	struct hdr_msvr_routing packet;
	struct hdr_msvr_routing *recvp = (struct hdr_msvr_routing *)p;
	char* enpaths = NULL;

	struct point nextPos_new;
	struct in_addr next_new;
	struct hdr_msvr_routing packet_new;
	bool Packet_New_Send = false;
	char* enpaths_new = NULL;

	// firstly we find dst node in neighbour list
	printf( "here I am node :%d\n", agent->addr());
	if (msvr_find_dst(((MsvrAgent *)(agent))->getNblist(), dst)) {
		// get src and dst node position
		srcNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(src.s_addr)));
		dstNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(dst.s_addr)));
		srcPos.x = srcNode->X();
		srcPos.y = srcNode->Y();
		dstPos.x = dstNode->X();
		dstPos.y = dstNode->Y();
		relayPos.x = srcPos.x;
		relayPos.y = srcPos.y;

		// get path
		paths = MSVRMAP->getPaths(srcPos.x, srcPos.y, dstPos.x, dstPos.y);

		// encode path
		enpaths = encode_path(paths);

		// send packet to dst node directly
		next = dst;
	} else {
		// if p == NULL, the packet is handled in src node
		// if failed, the packet is handled in relay node
		if (p == NULL) {
			// get src and dst node position
			srcNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(src.s_addr)));
			dstNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(dst.s_addr)));
			srcPos.x = srcNode->X();
			srcPos.y = srcNode->Y();
			dstPos.x = dstNode->X();
			dstPos.y = dstNode->Y();
			relayPos.x = srcPos.x;
			relayPos.y = srcPos.y;

			// get path
			paths = MSVRMAP->getPaths(srcPos.x, srcPos.y, dstPos.x, dstPos.y);
			//#if 0
			fprintf(stderr, "所有路口ID: ");
			for (std::vector<int>::iterator iter = paths.begin();
					iter != paths.end(); ++iter)
				fprintf(stderr, "---- %d ", *iter);
			fprintf(stderr, "\n");
			//#endif
			Packet_New_Send = true;
			paths_new = MSVRMAP->getPaths_new(srcPos.x, srcPos.y, dstPos.x, dstPos.y,paths);

			fprintf(stderr, "new path 所有路口ID: ");
			for (std::vector<int>::iterator iter = paths_new.begin();
					iter != paths_new.end(); ++iter)
				fprintf(stderr, "---- %d ", *iter);
			fprintf(stderr, "\n");

			// find next hop
			int roadid1 = MSVRMAP->getRoadByNode(paths[0], paths[1]);
			nextPos.x = MSVRMAP->getMap()[paths[1]].x_;
			nextPos.y = MSVRMAP->getMap()[paths[1]].y_;

			int roadid1_new = MSVRMAP->getRoadByNode(paths_new[0], paths_new[1]);
			nextPos_new.x = MSVRMAP->getMap()[paths_new[1]].x_;
			nextPos_new.y = MSVRMAP->getMap()[paths_new[1]].y_;

			int roadid2;
			if (paths.size() > 2) {
				roadid2 = MSVRMAP->getRoadByNode(paths[1], paths[2]);
				nextPos.x = MSVRMAP->getMap()[paths[2]].x_;
				nextPos.y = MSVRMAP->getMap()[paths[2]].y_;
			} else
				roadid2 = -1;
			if (roadid2 == -1 && roadid1 != MSVRMAP->getRoadByPos(dstPos.x, dstPos.y).id_)
				roadid2 = MSVRMAP->getRoadByPos(dstPos.x, dstPos.y).id_;


			int roadid2_new;
			if (paths_new.size() > 2) {
				roadid2_new = MSVRMAP->getRoadByNode(paths_new[1], paths_new[2]);
				nextPos_new.x = MSVRMAP->getMap()[paths_new[2]].x_;
				nextPos_new.y = MSVRMAP->getMap()[paths_new[2]].y_;
			} else
				roadid2_new = -1;
			if (roadid2_new == -1 && roadid1_new != MSVRMAP->getRoadByPos(dstPos.x, dstPos.y).id_)
				roadid2_new = MSVRMAP->getRoadByPos(dstPos.x, dstPos.y).id_;


			int roadid3 = MSVRMAP->getRoadByPos(srcPos.x, srcPos.y).id_;
			int roadid3_new = roadid3;
			if( MSVRMAP->nodeInRoad(srcPos.x,srcPos.y,roadid1))
				roadid3 = roadid1;
			if( MSVRMAP->nodeInRoad(srcPos.x,srcPos.y,roadid1_new))
				roadid3_new = roadid1_new;
			// XXX: call old find a next hop method
			/*next = msvr_get_next_hop(((MsvrAgent *)(agent))->getNblist(), roadid1, roadid2, srcPos.x, srcPos.y, true);*/

			// XXX:: call new find a next hop method
			next = msvr_get_next_hop(((MsvrAgent *)(agent))->getNblist(), roadid1, roadid2, roadid3, srcPos.x, srcPos.y, nextPos.x, nextPos.y, true);
			next_new = msvr_get_next_hop(((MsvrAgent *)(agent))->getNblist(), roadid1_new, roadid2_new, roadid3_new, srcPos.x, srcPos.y, nextPos_new.x, nextPos_new.y, true);

		} else {
			// get src and dst node position
			srcNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(src.s_addr)));
			dstNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(dst.s_addr)));
			relayNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(agent->getAddr().s_addr)));

			srcPos.x = srcNode->X();
			srcPos.y = srcNode->Y();
			dstPos.x = dstNode->X();
			dstPos.y = dstNode->Y();
			relayPos.x = relayNode->X();
			relayPos.y = relayNode->Y();

			// get path from recved packet
			int pathlen = recvp->path[0];
			enpaths = (char *)malloc(sizeof(char) * (pathlen + 1));
			memcpy(enpaths, recvp->path, pathlen + 1);

			// decode path, which is from recved packet
			decode_path(paths, enpaths);

			fprintf(stderr , "paths.size():%d;...", paths.size());
			for( int tempx = 0 ; tempx < paths.size() ; tempx ++ )
				fprintf(stderr , "--%d--",paths[tempx]);
			fprintf(stderr , "\n");

			if (paths.size() < 2) {
				agent->dropPacket(dst, DROP_RTR_NO_ROUTE);
				return -1;
			}

			// find next hop
			int roadid1 = MSVRMAP->getRoadByNode(paths[0], paths[1]);
			nextPos.x = MSVRMAP->getMap()[paths[1]].x_;
			nextPos.y = MSVRMAP->getMap()[paths[1]].y_;
			int roadid2;

			if (paths.size() > 2) {
				roadid2 = MSVRMAP->getRoadByNode(paths[1], paths[2]);
				nextPos.x = MSVRMAP->getMap()[paths[2]].x_;
				nextPos.y = MSVRMAP->getMap()[paths[2]].y_;
			} else
				roadid2 = -1;

			if (roadid2 == -1 && roadid1 != MSVRMAP->getRoadByPos(dstPos.x, dstPos.y).id_)
				roadid2 = MSVRMAP->getRoadByPos(dstPos.x, dstPos.y).id_;

			int roadid3 = MSVRMAP->getRoadByPos(srcPos.x, srcPos.y).id_;


			// XXX: call old find a next hop method
			/*next = msvr_get_next_hop(((MsvrAgent *)(agent))->getNblist(), roadid1, roadid2, srcPos.x, srcPos.y, true);*/

			// XXX: call new find a next hop method
			/*next = msvr_get_next_hop(((MsvrAgent *)(agent))->getNblist(), roadid1, roadid2, relayPos.x, relayPos.y, dstPos.x, dstPos.y, true);*/
			next = msvr_get_next_hop(((MsvrAgent *)(agent))->getNblist(), roadid1, roadid2, roadid3, relayPos.x, relayPos.y, nextPos.x, nextPos.y, true);
		}
	}

	if (next.s_addr == -1)
	{
		next = dst;
		return -1;
	}
	// remove passed path
	nextNode = static_cast<MobileNode*>(Node::get_node_by_address(static_cast<nsaddr_t>(next.s_addr)));
	nextPos.x = nextNode->X();
	nextPos.y = nextNode->Y();
	if (MSVRMAP->getRoadByNode(paths[0], paths[1]) !=
			MSVRMAP->getRoadByPos(nextPos.x, nextPos.y).id_ &&
			MSVRMAP->getRoadByPos(relayPos.x, relayPos.y).id_ ==
					MSVRMAP->getRoadByNode(paths[0], paths[1]))
		paths.erase(paths.begin());

	// encode path
	enpaths = encode_path(paths);

	// construct routing packet
	packet.type = MSVR_TYPE_ROUTING;
	packet.length = packet.size() + paths.size() * 2 + applen;
	packet.hlen = packet.size() + paths.size() * 2;
	packet.seq = time(NULL);
	packet.m = 0x0;
	packet.t = 0x0;
	packet.c = 0x0;
	packet.sx = srcPos.x;
	packet.sy = srcPos.y;
	packet.dx = dstPos.x;
	packet.dy = dstPos.y;
	packet.path = (char *)malloc(sizeof(char *) * paths.size() * 2);
	memcpy(packet.path, enpaths, paths.size() * 2);

	if (enpaths != NULL)
		free(enpaths);
	//fprintf(stderr, "src:%d----dst:%d----next:%d----length:%d\n",src,dst,next,packet.length);
	printf("src:%d----dst:%d----next:%d----length:%d\n",src,dst,next,packet.length);
	agent->sendProtPacketWithData((char *)&packet, packet.length, src, dst, next);

	if(Packet_New_Send == true){
	//if(false){
		enpaths_new = encode_path(paths_new);
		packet_new.type = MSVR_TYPE_ROUTING;
		packet_new.length = packet_new.size() + paths_new.size() * 2 + applen;
		packet_new.hlen = packet_new.size() + paths_new.size() * 2;
		packet_new.seq = time(NULL)+1;
		packet_new.m = 0x0;
		packet_new.t = 0x0;
		packet_new.c = 0x0;
		packet_new.sx = srcPos.x;
		packet_new.sy = srcPos.y;
		packet_new.dx = dstPos.x;
		packet_new.dy = dstPos.y;
		packet_new.path = (char *)malloc(sizeof(char *) * paths_new.size() * 2);
		memcpy(packet_new.path, enpaths_new , paths_new.size() * 2);

		if (enpaths_new != NULL)
			free(enpaths_new);
		//fprintf(stderr, "src:%d----dst:%d----next:%d----length:%d\n",src,dst,next,packet.length);
		printf("src:%d----dst:%d----next:%d----length:%d\n",src,dst,next_new,packet_new.length);
		agent->sendProtPacketWithData((char *)&packet_new, packet_new.length, src, dst, next_new);

	}
}
