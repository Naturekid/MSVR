#include "mybundle.h"

#define DEBUG
//#define UTIL_DEBUG

void Utival_cal_Timer::expire(Event*) {
	a_->m_uti_nodeInfo.uvalue_block_cal();
	a_->nb_purge();
	resched(1);
}

void MyHelloTimer::expire(Event*) {
	resched(HELLO_INTERVAL);//a_->helloInterval/1000.0);
	//a_->m_uti_nodeInfo.uvalue_block_cal();
	a_->sendHello();
}

void BundleTimer::expire(Event*) {
	a_->checkStorage();
	if (a_->myBundleBuffer->length() >= 1)
		resched(1);//0.001); // Check buffers every 1 ms
	else
		resched(1);//0.01);  // If empty, check buffers every 10 ms
}

int hdr_bundle::offset_;
static class BundleHeaderClass : public PacketHeaderClass {
public:
	BundleHeaderClass() : PacketHeaderClass("PacketHeader/Bundle",
			sizeof(hdr_bundle)) {
		bind_offset(&hdr_bundle::offset_);
	}
} class_bundlehdr;


static class BundleClass : public TclClass {
public:
	BundleClass() : TclClass("Agent/Bundle") {}
	TclObject* create(int, const char*const*) {
		return (new BundleAgent());
	}
} class_bundle;


BundleAgent::BundleAgent() : Agent(PT_DTNBUNDLE), utival_cal_Timer_(this),  helloTimer_(this), bundleTimer_(this) {


	bind("helloInterval_", &helloInterval);
	//  bind("retxTimeout_", &retxTimeout);
	//  bind("deleteForwarded_", &deleteForwarded);
	//  bind("cqsize_", &cqsize);
	//  bind("retxqsize_", &retxqsize);
	//  bind("qsize_", &qsize);
	//  bind("ifqCongestionDuration_", &ifqCongestionDuration);
	//  bind("sentBundles_", &sentBundles);
	//  bind("receivedBundles_", &receivedBundles);
	//  bind("duplicateReceivedBundles_", &duplicateReceivedBundles);
	//  bind("duplicateBundles_", &duplicateBundles);
	//  bind("deletedBundles_", &deletedBundles);
	//  bind("forwardedBundles_", &forwardedBundles);
	//  bind("sentReceipts_", &sentReceipts);
	//  bind("receivedReceipts_", &receivedReceipts);
	//  bind("duplicateReceivedReceipts_", &duplicateReceivedReceipts);
	//  bind("duplicateReceipts_", &duplicateReceipts);
	//  bind("forwardedReceipts_", &forwardedReceipts);
	//  bind("avBundleDelay_", &avBundleDelay);
	//  bind("avReceiptDelay_", &avReceiptDelay);
	//  bind("avBundleHopCount_", &avBundleHopCount);
	//  bind("avReceiptHopCount_", &avReceiptHopCount);
	//  bind("avNumNeighbors_", &avNumNeighbors);
	//  bind("avLinkLifetime_", &avLinkLifetime);
	//  bind("antiPacket_", &antiPacket);
	//  bind("routingProtocol_", &routingProtocol);
	//  bind("initialSpray_", &initialSpray);
	bind("bundleStorageSize_", &bundleStorageSize);
	//  bind("congestionControl_", &congestionControl);
	//  bind("bundleStorageThreshold_", &bundleStorageThreshold);
	//  bind("CVRR_", &CVRR);
	//  bind("limitRR_", &limitRR);
	//  bind("dropStrategy_", &dropStrategy);
	//  bundleDelaysum=0;
	//  receiptDelaysum=0;
	//  bundleHopCountsum=0;
	//  receiptHopCountsum=0;

	utival_cal_Timer_.resched((rand()%1000)/1000000.0);
	helloTimer_.resched((rand()%helloInterval)/1000.0); // To avoid synchronization
	bundleTimer_.resched((rand()%1000)/1000000.0);      // To avoid synchronization
	//  neighborFreeBytes=NULL;
	//  countRR=NULL;
	//  ownDP=NULL;
	//   neighborNeighborId=NULL;
	//   neighborDP=NULL;
	//   neighborLastSeen=NULL;
	//   neighborFirstSeen=NULL;
	//   neighborId=NULL;
	//   neighborBundleTableSize=NULL;
	//   neighborBundles=NULL;
	//   neighbors=0;
	//   bundleNeighborId=NULL;
	//   bundleNeighborEid=NULL;
	//   bundleNeighborFragments=NULL;
	//   bundleNeighborFragmentNumber=NULL;
	//   bundleNeighborLastSeen=NULL;
	//   bundleNeighbors=0;

	myBundleBuffer = new BundlePacketQueue();
	myBundleStorage = new BundlePacketQueue();
	midBundleBuffer = new BundlePacketQueue();
	controlPktBuffer = new BundlePacketQueue();
	LIST_INIT(&nbhead);

	//   bundleStorage=new PacketQueue();
	//   reTxBundleStorage=new PacketQueue();
	//   mgmntBundleStorage=new PacketQueue();
	//   bundleBuffer=new PacketQueue();
	//   reTxBundleBuffer=new PacketQueue();
	//   mgmntBundleBuffer=new PacketQueue();
	ifqueue=0;
	//  last_transmission=0;
	//  dropsRR=0;
	//  repsRR=0;
	//  lastCVRR=0;

	duplicateFrag = 0;
}

int BundleAgent::command(int argc, const char*const* argv) {

#ifdef DEBUG
	std::cout<<"argc::"<<argc<<' '<<argv[1]<<' '<<argv[2]<<"---------------------------------------------"<<std::endl;
#endif

	if(argc == 3) {
		if(strcmp(argv[1], "if-queue") == 0) {
			ifqueue=(PriQueue*) TclObject::lookup(argv[2]);
			if(ifqueue == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
		if(strcmp(argv[1], "index") == 0) {
			m_uti_nodeInfo.index = atoi(argv[2]);

			//registration!
			Packet* pkt=allocpkt();
			hdr_cmn* ch=hdr_cmn::access(pkt);
			ch->size() = 1;
			target_->recv(pkt, (Handler*)0);

			return TCL_OK;
		}
	}

	if (argc == 8) {
		if (strcmp(argv[1], "send") == 0) {
			sentBundles++; // Stats
			if ((myBundleBuffer->byteLength() + midBundleBuffer->byteLength() + atoi(argv[3])) > bundleStorageSize) {
				return (TCL_OK);
			}
			Packet* pkt=allocpkt();
			hdr_cmn* ch=hdr_cmn::access(pkt);
			ch->size()=atoi(argv[3]);
			hdr_bundle* bh=hdr_bundle::access(pkt);
			bh->originating_timestamp=CURRENT_TIME;
			bh->lifetime=atof(argv[4]);

			bh->type= TYPE_DATA;
			bh->src=here_.addr_;
			bh->prev=here_.addr_;
			bh->dst=atoi(argv[2]);
			//      bh->hop_count=0;
			bh->bundle_id=ch->uid(); // Unique bundle ID
			bh->custody_transfer=atoi(argv[5]);
			bh->return_receipt=atoi(argv[6]);


			bh->initial_timestamp= CURRENT_TIME;
			myBundleBuffer->enque(pkt);

			Packet* pkt2 = pkt->copy();
			myBundleStorage->enque(pkt2);
			return (TCL_OK);
		}
	}
	return (Agent::command(argc, argv));
}



//void BundleAgent::sendRetReceipt(Packet* pkt) {
//  // Send a report to the source node
//  hdr_bundle* bh=hdr_bundle::access(pkt);
//  Packet* pktret=allocpkt();
//  hdr_cmn* chret=hdr_cmn::access(pktret);
//  hdr_bundle* bhret=hdr_bundle::access(pktret);
//  chret->size()=10; // Should be a parameter
//  bhret->originating_timestamp=NOW;
//  bhret->lifetime=retxTimeout - (NOW - bh->originating_timestamp); // min(retxTimeout - bundledelay, lifetime)
//  if (bhret->lifetime > bh->lifetime)
//    bhret->lifetime=bh->lifetime;
//  bhret->type=2;
//  bhret->src=here_.addr_;
//  bhret->prev=here_.addr_;
//  bhret->dst=bh->src;
////  bhret->hop_count=0;
//  bhret->bundle_id=chret->uid();
////  bhret->reverse_eid=bh->bundle_id;
//  bhret->custody_transfer=0;
//  bhret->return_receipt=0;
////  bhret->priority=2;
////  bhret->sent_tos=0;
////  for(int n=0; n < 100; n++) {
////    bhret->sent_to[n]=-1;
////    bhret->sent_when[n]=0;
////  }
////  bhret->spray=initialSpray;
////  bhret->nretx=0;
//  bhret->initial_timestamp=NOW;
////  mgmntBundleStorage->enque(pktret);
//  sentReceipts++; // Stats
//}

Packet* BundleAgent::copyBundle(Packet* pkt) {
	Packet* newpkt=allocpkt();
	hdr_cmn* ch=hdr_cmn::access(pkt);
	hdr_bundle* bh=hdr_bundle::access(pkt);
	hdr_cmn* newch=hdr_cmn::access(newpkt);
	hdr_bundle* newbh=hdr_bundle::access(newpkt);
	newch->size()=ch->size();
	newbh->originating_timestamp=bh->originating_timestamp;
	newbh->lifetime=bh->lifetime;
	newbh->type=bh->type;
	newbh->src=bh->src;
	newbh->prev=bh->prev;
	newbh->dst=bh->dst;
	//  newbh->hop_count=bh->hop_count;
	newbh->bundle_id=bh->bundle_id;
	//  newbh->reverse_eid=bh->reverse_eid;
	newbh->custody_transfer=bh->custody_transfer;
	newbh->return_receipt=bh->return_receipt;
	//  newbh->priority=bh->priority;
	//  newbh->sent_tos=bh->sent_tos;
	//  for(int n=0; n < 100; n++) {
	//    newbh->sent_to[n]=bh->sent_to[n];
	//    newbh->sent_when[n]=bh->sent_when[n];
	//  }
	//  newbh->spray=bh->spray;
	//  newbh->nretx=bh->nretx;
	newbh->initial_timestamp=bh->initial_timestamp;
	return newpkt;
}


/*
 *first���Ҫ������ʼ��Ƭ��last
 */
void BundleAgent::sendBundle(Packet* pkt, int next_hop, int first, int last) {
	hdr_cmn* ch=hdr_cmn::access(pkt);
	hdr_bundle* bh=hdr_bundle::access(pkt);
	//fragments: �ֳɶ���Ƭ
	int fragments=(ch->size()/1460)+1; // Fragment to IP packets, save 40 bytes for overhead
	if ((ch->size()%1460) == 0)
		fragments--;
	//  int retx=0;

	if ((first==0)&&(last==0))
		last=fragments;
	//  else
	//retxΪ1��ʾֻ�Ƿ���Bundle��һ���ַ�Ƭ
	//    retx=1;
	for(int i=first; i < last; i++) {
		Packet* newpkt=allocpkt();
		hdr_bundle* newbh=hdr_bundle::access(newpkt);
		hdr_ip* newiph=hdr_ip::access(newpkt);
		hdr_cmn* newch=hdr_cmn::access(newpkt);

		if(fragments > 1)
			newbh->is_fragment = true;
		else
			newbh->is_fragment = false;

		if ((i==(fragments-1))&&((ch->size()%1460) > 0))
			newch->size()=(ch->size()%1460)+40;
		else
			newch->size()=1500;
		//ΪʲôҪ��һ����Bundle??: ÿ�������bundleͷ����
		newbh->nfragments=fragments;

		//i��ʾ�ڼ��Ρ�
		newbh->fragment=i;
		newbh->bundle_size=ch->size();
		newbh->originating_timestamp=bh->originating_timestamp;
		newbh->lifetime=bh->lifetime;
		newbh->type = bh->type;
		newbh->src=bh->src;
		newbh->prev=here_.addr_;
		newbh->dst=bh->dst;
		//    newbh->hop_count=bh->hop_count+1;
		newbh->bundle_id=bh->bundle_id;
		newbh->ack_bundle_id = bh->ack_bundle_id;
		//    newbh->reverse_eid=bh->reverse_eid;
		newbh->custody_transfer=bh->custody_transfer;
		newbh->return_receipt=bh->return_receipt;
		//    newbh->priority=bh->priority;
		//    newbh->sent_tos=0;
		//    for(int n=0; n < 100; n++) {
		//      newbh->sent_to[n]=-1;
		//      newbh->sent_when[n]=0;
		//    }
		//    newbh->spray=bh->spray;
		//    newbh->nretx=bh->nretx;
		newbh->initial_timestamp=bh->initial_timestamp;
		newch->iface()=UNKN_IFACE.value();
		newch->direction()=hdr_cmn::NONE;
		newiph->saddr()=here_.addr_;
		newiph->sport()=here_.port_;
		newiph->dport()=here_.port_;
		newiph->daddr()=next_hop;
		//    int buffered=0;

		target_->recv(newpkt);
	}
}

void BundleAgent::dropOldest() {
	//  int n=0;
	//  Packet* pkt=0;
	//  double max_lifetime=0;
	//  int max_n=0;
	//  while (n < bundleStorage->length()) {
	//    pkt=bundleStorage->lookup(n);
	//    hdr_bundle* bh=hdr_bundle::access(pkt);
	//    if (NOW - bh->originating_timestamp > max_lifetime) {
	//      max_lifetime=(NOW - bh->originating_timestamp);
	//      max_n=n;
	//    }
	//    n++;
	//  }
	//  pkt=bundleStorage->lookup(max_n);
	//  bundleStorage->remove(pkt);
	//  Packet::free(pkt);
}

//void BundleAgent::dropMostSpread() {
//  int n=0;
//  Packet* pkt=0;
//  int max_sent_tos=0;
//  int max_n=0;
//  while (n < bundleStorage->length()) {
//    pkt=bundleStorage->lookup(n);
//    hdr_bundle* bh=hdr_bundle::access(pkt);
////    if (bh->sent_tos > max_sent_tos) {
////      max_sent_tos=bh->sent_tos;
////      max_n=n;
////    }
//    n++;
//  }
//  pkt=bundleStorage->lookup(max_n);
//  bundleStorage->remove(pkt);
//  Packet::free(pkt);
//}

//void BundleAgent::dropLeastSpread() {
//  int n=0;
//  Packet* pkt=0;
//  int min_sent_tos=10000;
//  int min_n=0;
//  while (n < bundleStorage->length()) {
//    pkt=bundleStorage->lookup(n);
//    hdr_bundle* bh=hdr_bundle::access(pkt);
////    if (bh->sent_tos < min_sent_tos) {
////      min_sent_tos=bh->sent_tos;
////      min_n=n;
////    }
//    n++;
//  }
//  pkt=bundleStorage->lookup(min_n);
//  bundleStorage->remove(pkt);
//  Packet::free(pkt);
//}

void BundleAgent::dropRandom() {
	//  int n=Random::integer(bundleStorage->length());
	//  Packet* pkt=bundleStorage->lookup(n);
	//  bundleStorage->remove(pkt);
	//  Packet::free(pkt);
}

void BundleAgent::removeExpiredBundles() {
	int n=0;
	Packet* pkt=0;

#ifdef DEBUG
	//for debugging
	//	std::cout<<"CHeck EXPIRE! node "<<m_uti_nodeInfo.index<<std::endl;
#endif

	while (n < myBundleStorage->length()) {
		pkt = myBundleStorage->lookup(n);
		hdr_bundle* bh = hdr_bundle::access(pkt);
		if (CURRENT_TIME - bh->originating_timestamp > bh->lifetime) {
			//�ش�
			bh->originating_timestamp = NOW;

			Packet* pkt2 = pkt->copy();
			myBundleBuffer->delete_by_id(bh->bundle_id);
			myBundleBuffer->enque(pkt2);

#ifdef DEBUG
			//for debugging
			hdr_bundle* bh2 = hdr_bundle::access(pkt2);
			std::cout<<"Im RETRANS!!"<<std::endl;
			std::cout<<bh2->dst<<std::endl;
			//    	if(myBundleBuffer->length())
			//    			std::cout<<"Im node "<<m_uti_nodeInfo.index <<" My bundleBuffer "<<myBundleBuffer->length()<<endl;

#endif

		}
		n++;
	}

	//  n=0;
	//  Packet* midpkt=0;
	//  while (n < midBundleBuffer->length()) {
	//	  midpkt = myBundleStorage->lookup(n);
	//	  hdr_bundle* bh = hdr_bundle::access(midpkt);
	//	  if (NOW - bh->originating_timestamp > bh->lifetime) {
	//
	//		  midBundleBuffer->remove(midpkt);
	//
	//    }
	//    n++;
	//  }
}

void BundleAgent::checkStorage() {
	//ifqCongestionDuration=NOW-last_transmission; // Stats

	//bundleStore_size = myBundleStore->byteLength();
	//midBuffer_size	 = midBundleBuffer->byteLength();

#ifdef DEBUG
	//for debugging
	if(myBundleBuffer->length())
		std::cout<<"Im node "<<m_uti_nodeInfo.index <<" My bundleBuffer "<<myBundleBuffer->length()<<endl;
#endif

	removeExpiredBundles();
	//	removeExpiredBundles(midBundleBuffer);
	//����
	Packet* pkt= 0;
	Packet* controlPkt = 0;

	if(myBundleBuffer->length() > 0)
		pkt = myBundleBuffer->deque();
	if(controlPktBuffer->length() > 0)
		controlPkt = controlPktBuffer->deque();
	if (pkt!=0){

#ifdef DEBUG
		//fordebugging
		hdr_bundle* bh_de = hdr_bundle::access(pkt);
		assert(1||bh_de->dst);
#endif

		forward(pkt);
	}

	if (controlPkt!=0){
		forward(controlPkt);
	}

	//����Ƿ��г�ʱfragment���������ش�����
	//ֻ������һ�����ش�����
	if (!fragMap.empty())
	{
		std::map<int, std::map<int,packet_info> >::iterator itr;
		for (itr=fragMap.begin(); itr!=fragMap.end();itr++)
		{
			std::map<int,packet_info>::iterator itr_iner;
			itr_iner = itr->second.begin();
			if (CURRENT_TIME - itr_iner->second.recv_time > MAX_BUFFER_TIME)
			{
				hdr_bundle* bh_nak = hdr_bundle::access(itr_iner->second.p);
				sendNAK(bh_nak->prev, itr->first);  //��ʱ���ǽ����BUNDLE�ش�
			}
		}
	}
	//
}

/*
 *	���ͺ���:
 *		1. ѡ����һ��
 *		2. ����
 *		3. ע���Ƭ1500����sendBundle�н��С�
 */
void BundleAgent::forward(Packet *p){
	//	hdr_cmn* ch=hdr_cmn::access(p);
	//	hdr_ip* iph=hdr_ip::access(p);
	hdr_bundle* bh=hdr_bundle::access(p);

	DTN_Neighbor *nb = nbhead.lh_first;
	DTN_Neighbor *nbn;
	double temp_max_value = -1.0;
	int temp_max_node = m_uti_nodeInfo.index;

#ifdef DEBUG

	//for debugging
	for(; nb; nb = nbn) {
		std::cout<<"my index:"<<m_uti_nodeInfo.index<<" AT "<<m_uti_nodeInfo.locX<<' '<<m_uti_nodeInfo.locY<<std::endl;
		std::cout<<nb->nb_addr<<' '<<nb->locX<<' '<<nb->locY<<endl;
		nbn = nb->nb_link.le_next;
	}
	nb = nbhead.lh_first;
#endif

	//��Ŀ�ĵ�����Ч��ֵ�ıȶ���������һ��, if dst node is in my nblist ,then send pkt straight
	for(nb = nbhead.lh_first; nb; nb = nbn) {
		if(nb->nb_addr == bh->dst){
			sendBundle(p, bh->dst, 0, 0);
			return;
		}
		nbn = nb->nb_link.le_next;
	}

	for(nb = nbhead.lh_first; nb; nb = nbn) {
		//ȡ��Ŀ�Ľڵ����
		Node *dstnode;
		double dst_x, dst_y, dst_z;
		dstnode = Node::get_node_by_address(bh->dst);
		((MobileNode *)dstnode)->getLoc(&dst_x, &dst_y, &dst_z);

#ifdef DEBUG
		//for debugging
		std::cout<<"my index:"<<m_uti_nodeInfo.index<<" location: "<<m_uti_nodeInfo.locX<<' '<<m_uti_nodeInfo.locY<<endl;
		//		std::cout<<"dst's index:"<<bh->dst<<" location: "<<dst_x<<' '<<dst_y<<endl;
		//		std::cout<<"nb's index:"<<nb->nb_addr<<" location: "<<nb->locX<<' '<<nb->locY<<endl;
#endif

		double a = sqrt(pow(dst_x - m_uti_nodeInfo.getLocX(), 2.0) + pow(dst_y - m_uti_nodeInfo.getLocY(), 2.0)) - sqrt(pow(dst_x - nb->locX, 2.0) + pow(dst_y - nb->locY, 2.0));
		a = a/sqrt(pow(dst_x - nb->locX, 2.0) + pow(dst_y - nb->locY, 2.0));

		//		nb->orient = m_uti_nodeInfo.orient_block_cal(nb->locX - m_uti_nodeInfo.getLocX(), nb->locY - m_uti_nodeInfo.getLocY());
		int orient = m_uti_nodeInfo.orient_block_cal(dst_x - m_uti_nodeInfo.getLocX(), dst_y - m_uti_nodeInfo.getLocY());

		//		double b = (nb->syn_utilValue[nb->orient] - m_uti_nodeInfo.getSynValue(nb->orient))/m_uti_nodeInfo.getSynValue(nb->orient);
#ifdef DEBUG
		//for debugging
		std::cout<<nb->nb_addr<<"nb->syn_utilValue[orient]:"<<nb->syn_utilValue[orient]<<" m_uti_nodeInfo.getSynValue(orient): "<<m_uti_nodeInfo.getSynValue(orient)<<endl;
		//		for (int orient11=0; orient11<BLOCK; orient11++)
		//		{
		//			std::cout<<nb->syn_utilValue[orient11]<<endl;
		//		}
#endif
		double b = 0;
		if(m_uti_nodeInfo.getSynValue(orient))
			b = (nb->syn_utilValue[orient] - m_uti_nodeInfo.getSynValue(orient));// /abs(m_uti_nodeInfo.getSynValue(orient));
		else
			b = nb->syn_utilValue[orient];
		//��ֹ���Խ��ԽԶ
		if (a < 0)
		{
			if(b < 0)
				b = b * (1 + DISTENCE_INFLU);
			else
				b = b * DISTENCE_INFLU;
		}
		//speed == 0 ,then forward by distance
		if (m_uti_nodeInfo.speed < 0.5){
			if(nb->syn_utilValue[orient] < 0)
				b = nb->syn_utilValue[orient];
			else //if(nb->syn_utilValue[orient] <= 0.6)
				b = nb->syn_utilValue[orient] * SPEED_INFLU;
		}
		//nb's sum > NB_THRESHOLD, then reduce utility's influence
		if(this->nb_sum() > NB_THRESHOLD && b > 0)
			b = b * NB_INFLU;

		double temp_value = a + b;
		int temp_node = nb->nb_addr;
		//
		if (temp_value < 0 || temp_value < temp_max_value){
			nbn = nb->nb_link.le_next;
			continue;
		}
		else{
			temp_max_node = temp_node;
			temp_max_value = temp_value;
		}
		nbn = nb->nb_link.le_next;
	}//end for
	if(temp_max_node == m_uti_nodeInfo.index)
		//û�и����һ��ڵ�
		myBundleBuffer->enque(p);
	else
		//����Bundle
		sendBundle(p, temp_max_node, 0, 0);
}

void BundleAgent::sendNAK(nsaddr_t prev, int bundle_id){
	Packet* pktNak = allocpkt();
	hdr_cmn* chNak = hdr_cmn::access(pktNak);
	chNak->ptype() = PT_DTNBUNDLE;
	chNak->size() = 22; // Should be a parameter

	hdr_bundle* bhNak = hdr_bundle::access(pktNak);
	bhNak->type = TYPE_NAK;
	bhNak->src = m_uti_nodeInfo.index;
	bhNak->dst = prev;
	bhNak->ack_bundle_id = bundle_id;

	//target_->recv(pktNak);
	//myBundleBuffer->enque(pktNak);
	controlPktBuffer->enque(pktNak);
	//Scheduler::instance().schedule(target_, pktNak, 0.0);
}

void BundleAgent::sendCustAck(int bundle_id, int dst) {
	// Send a report to the previous node

	Packet* pktret = allocpkt();
	hdr_cmn* chret = hdr_cmn::access(pktret);
	chret->ptype() = PT_DTNBUNDLE;
	chret->size()=10; // Should be a parameter

	hdr_ip* ipret = hdr_ip::access(pktret);
	ipret->saddr() = here_.addr_;
	ipret->sport() = here_.port_;
	ipret->daddr() = dst;
	ipret->dport() = 0;

	hdr_bundle* bret = hdr_bundle::access(pktret);
	bret->type = TYPE_ACK;
	bret->ack_bundle_id = bundle_id;

	target_->recv(pktret);
	//Scheduler::instance().schedule(target_, pktret, 0.0);
	//mgmntBundleBuffer->enque(pktret);
}


void BundleAgent::sendHello() {
	Packet* pkt=allocpkt();
	hdr_cmn* ch=hdr_cmn::access(pkt);

#ifdef DEBUG
	//for debugging
	//	std::cout<<"Im node "<<m_uti_nodeInfo.index<<" send a Hello!";
	//	std::cout<<" at location "<<m_uti_nodeInfo.locX<<' '<<m_uti_nodeInfo.locY<<std::endl;
#endif

	//ch->ptype()=PT_MESSAGE; // This is a hack that puts Hello messages first in the IFQ
	ch->size()=20; // Should be a parameter
	ch->ptype() = PT_DTNBUNDLE;

	hdr_ip* iph=hdr_ip::access(pkt);
	iph->saddr()=here_.addr_;
	iph->sport()=here_.port_;
	iph->daddr()=IP_BROADCAST;
	iph->dport()=0;
	//TTL�����ã�
	if (nb_sum() < NB_THRESHOLD)
		//iph->ttl_ = HELLO_TTL_L;
		iph->ttl_ = 2;
	else
		//iph->ttl_ = HELLO_TTL_S;
		iph->ttl_ = 2;

	hdr_bundle* bh=hdr_bundle::access(pkt);
	bh->type= TYPE_HELLO;
	bh->src = here_.addr_;
	//д��ڵ��Ч��ֵ
	bh->node_info.locX = m_uti_nodeInfo.locX;
	bh->node_info.locY = m_uti_nodeInfo.locY;
	for (int i=0; i<BLOCK; i++)
	{
		bh->node_info.utiValue[i] = m_uti_nodeInfo.getSynValue(i);
	}

	target_->recv(pkt);
	//Scheduler::instance().schedule(target_, pkt, 0.0);
}

void BundleAgent::recv(Packet* pkt, Handler*) {
	//	hdr_cmn* ch=hdr_cmn::access(pkt);
	//	hdr_ip* iph=hdr_ip::access(pkt);
	hdr_bundle* bh=hdr_bundle::access(pkt);

#ifdef DEBUG
	//for debugging
	//std::cout<<"Im node "<<m_uti_nodeInfo.index<<" recv a "<<bh->type<<std::endl;
	if(m_uti_nodeInfo.index == 7 ){
		if(bh->type != 3)
			std::cout<<"Im node "<<m_uti_nodeInfo.index<<" recv a "<<bh->type<<std::endl;
	}
#endif

	switch(bh->type){
	case TYPE_ACK:
		recvACK(pkt);
		break;
	case TYPE_DATA:
		recvData(pkt);
		break;
	case TYPE_DISRU:
		recvDisruption(pkt);
		break;
	case TYPE_HELLO:
		recvHello(pkt);
		break;
	case TYPE_NAK:
		recvNAK(pkt);
		break;
	default:
		fprintf(stderr, "Invalid DTN type (%x)\n", bh->type);
		exit(1);
	}
}

/*
 *	����ȷ�ϣ�ɾ����Ӧ���
 *	�����Bundleidɾ��ڵ��յ������BUNDLE�Żᷢack��
 */
void BundleAgent::recvACK(Packet * p){
	hdr_bundle* bh=hdr_bundle::access(p);

#ifdef DEBUG
	//for debugging
	std::cout<<"Im node "<<m_uti_nodeInfo.index<<" recv a ACK!!"<<std::endl;
#endif
	myBundleStorage->delete_by_id(bh->ack_bundle_id);
	midBundleBuffer->delete_by_id(bh->ack_bundle_id);

	Packet::free(p);
}
//void BundleAgent::delete_bundleInStorage(BundlePacketQueue* storage, int eid) {
//	int n=0;
//	Packet* pkt = storage->lookup_by_id(eid);
//	if(pkt)
//		storage->remove(pkt);
//	while (n < storage->length()) {
//		Packet* pkt=storage->lookup(n);
//		hdr_bundle* bh=hdr_bundle::access(pkt);
//		if (bh->bundle_id == eid){
//
//#ifdef DEBUG
//			//for debugging
//			std::cout<<"Im node "<<m_uti_nodeInfo.index<<" Im deleting!!!!!"<<std::endl;
//#endif
//
//			storage->remove(pkt);
//			Packet::free(pkt);
//		}
//		n++;
//	}
//}

/*
 *	���?����ݣ���Ϊ������
 *				Ŀ�ĵ�Ϊ���ڵ�/Ϊָ����һ��
 *				�м仺�����
 */
void BundleAgent::recvData(Packet * p){
	//	hdr_cmn* ch=hdr_cmn::access(p);
	hdr_ip* iph=hdr_ip::access(p);
	hdr_bundle* bh=hdr_bundle::access(p);

#ifdef DEBUG
	//for debugging
	std::cout<<"Im node "<<m_uti_nodeInfo.index<<" recv a DATA!!"<<" FROM "<<iph->src().addr_<<std::endl;
#endif

	//is fragment?
	if (check_bundle(p))
	{
		if(bh->is_fragment){
			//�յ�һ����Ƭ��
			//��������
			//�ǲ����ظ��ķ�Ƭ?
			if(fragMap.count(bh->bundle_id)) {

				if(!fragMap[bh->bundle_id].count(bh->fragment)){
					packet_info_n.recv_time = CURRENT_TIME;
					packet_info_n.p 		= p;
					(fragMap[bh->bundle_id])[bh->fragment] = packet_info_n;
				}else{
					duplicateFrag++;
				}
#ifdef DEBUG
				// std::map<int, std::map<int,packet_info> > fragMap
				std::map<int,packet_info>::iterator itr = fragMap[bh->bundle_id].begin();
				if(m_uti_nodeInfo.index == 7){
					std::cout<<fragMap[bh->bundle_id].size()<<std::endl;
					while(itr!= fragMap[bh->bundle_id].end()){

						std::cout<<itr->first<<' '<<itr->second.recv_time <<std::endl;
						itr++;
					}
				}
#endif
				//����Ƭ�Ƿ�����ȫ
				if(fragMap[bh->bundle_id].size() == (unsigned int)bh->nfragments){
					//����ȫ,������װ����
					//check_bundle(process_for_reassembly(int bundle_id));
					//p����process�б��ͷ�
					int temp_bundle_id = bh->bundle_id;
					nsaddr_t temp_prev = bh->prev;

					process_for_reassembly(temp_bundle_id);
					sendCustAck(temp_bundle_id, temp_prev);
				}
				else{
					//δ��ȫ���ش������⴦�?��CheckStorage()

				}

			}else{
				packet_info_n.recv_time = CURRENT_TIME;
				packet_info_n.p 		= p;
				//				std::map<int,packet_info> tempMap;
				//				fragMap.insert(pair<int, std::map<int,packet_info> >(bh->bundle_id, tempMap));
				//				(fragMap[bh->bundle_id]).insert(pair<int,packet_info>(bh->fragment, packet_info_n));
				(fragMap[bh->bundle_id])[bh->fragment] = packet_info_n;
#ifdef DEBUG
				// std::map<int, std::map<int,packet_info> > fragMap
				std::map<int,packet_info>::iterator itr = fragMap[bh->bundle_id].begin();
				if(m_uti_nodeInfo.index == 7){
					std::cout<<fragMap[bh->bundle_id].size()<<std::endl;
					while(itr!= fragMap[bh->bundle_id].end()){

						std::cout<<itr->first<<' '<<itr->second.recv_time <<std::endl;
						itr++;
					}
				}
#endif
			}
		}else{
			//����bundle,û�з�Ƭ
			//myBundleStore.enque(p);
			//����������ظ�������

			sendCustAck(bh->bundle_id, bh->prev);
		}
	}
	//	Packet::free(p);
}

/*
 *	�����·��(�ش�)��Ϊ���ڵ��·������ڵ��·
 *		��·���ǰ���(src, dst) ����һ��DTN???, Ŀ�ĵأ���Է���
 */
void BundleAgent::recvDisruption(Packet * p){
	hdr_bundle *disbh = hdr_bundle::access(p);
	//	Packet* newpkt = midBundleBuffer->lookup_by_SrcDst(bh->src, bh->dst);
	int n=0;
	while (n < midBundleBuffer->length()) {
		Packet* pkt = midBundleBuffer->lookup(n);
		hdr_bundle* bh=hdr_bundle::access(pkt);
		if (bh->src == disbh->src && bh->dst == disbh->dst){
			myBundleBuffer->enque(pkt);//�յ�ȷ�Ϻ���ɾ��midBuffer
		}
		n++;
	}
	//
	if(n == 0){
		sendNAK(disbh->src, disbh->bundle_id);

	}
}

/*
 *	�ش�ĳBundle�򲿷�
 */
void BundleAgent::recvNAK(Packet * p){
#ifdef DEBUG
	//for debugging
	std::cout<<"Im node "<<m_uti_nodeInfo.index<<" recv a NNNAK!! myBundleStorage"<<myBundleStorage->length()<<std::endl;
#endif
	hdr_bundle* bh = hdr_bundle::access(p);
	Packet* pkt = myBundleStorage->lookup_by_id(bh->ack_bundle_id);
	if(pkt)
		myBundleBuffer->enque(pkt);
	else{
		//send Nak to src!! exam the prev and src
	}
}

/*
 *	����㲥Hello��ݰ�ά���ھ��б?�Է������Ԥ��?��
 */
void BundleAgent::recvHello(Packet * p){
	hdr_bundle *bh = hdr_bundle::access(p);
	DTN_Neighbor *nb ;//= nbhead.lh_first;//;

#ifdef DEBUG
	//for debugging
	//	std::cout<<"recvHello::Im node "<<m_uti_nodeInfo.index<<std::endl;
	//	std::cout<<"Im node "<<m_uti_nodeInfo.index<<std::endl;
	//
	//				DTN_Neighbor *nbn;
	//				for(; nb; nb = nbn) {
	//					std::cout<<nb->nb_addr<<' '<<nb->locX<<' '<<nb->locY<<endl;
	//					nbn = nb->nb_link.le_next;
	//				}
#endif

	nb = nb_lookup(bh->src);
	if(nb == 0) {
		if(bh->src == m_uti_nodeInfo.index)
			assert(1);
		else
			nb_insert(p);
	}
	else {
		//���ñ�����
		nb->nb_expire = CURRENT_TIME + 0.7;
		//(1.5 * ALLOWED_HELLO_LOSS * HELLO_INTERVAL);
		//���õ�ַ
		nb->locX = bh->node_info.locX;
		nb->locY = bh->node_info.locY;

		//����Ч��ֵ
		for (int i=0; i<BLOCK; i++)
		{
			nb->syn_utilValue[i] = bh->node_info.utiValue[i];
		}
	}

	Packet::free(p);
}

void BundleAgent::nb_insert(Packet * p) {
	struct hdr_bundle *bh = hdr_bundle::access(p);
	DTN_Neighbor *nb = new DTN_Neighbor(bh->src);

	assert(nb);
	nb->locX = bh->node_info.locX;
	nb->locY = bh->node_info.locY;
	for (int i=0; i<BLOCK; i++)
	{
		nb->syn_utilValue[i] = bh->node_info.utiValue[i];
	}

	nb->nb_expire = CURRENT_TIME +
			(1.5 * ALLOWED_HELLO_LOSS * HELLO_INTERVAL);
	LIST_INSERT_HEAD(&nbhead, nb, nb_link);
}


DTN_Neighbor* BundleAgent::nb_lookup(int id) {
	DTN_Neighbor *nb = nbhead.lh_first;

	for(; nb; nb = nb->nb_link.le_next) {
		if(nb->nb_addr == id) break;
	}
	return nb;
}

/*
 * Called when we receive *explicit* notification that a Neighbor
 * is no longer reachable.
 */
void BundleAgent::nb_delete(int id) {
	DTN_Neighbor *nb = nbhead.lh_first;

	//log_link_del(id);
	//seqno += 2;     // Set of neighbors changed
	//assert ((seqno%2) == 0);

	for(; nb; nb = nb->nb_link.le_next) {
		if(nb->nb_addr == id) {
			LIST_REMOVE(nb,nb_link);
			delete nb;
			break;
		}
	}

	//handle_link_failure(id);

}


/*
 * Purges all timed-out Neighbor Entries - runs every
 * HELLO_INTERVAL * 1.5 seconds.
 */
void BundleAgent::nb_purge() {
	DTN_Neighbor *nb = nbhead.lh_first;
	DTN_Neighbor *nbn;
	double now = CURRENT_TIME;

	for(; nb; nb = nbn) {
		nbn = nb->nb_link.le_next;
		if(nb->nb_expire <= now) {
#ifdef DEBUG
			//for debugging
			std::cout<<"Im node "<<m_uti_nodeInfo.index<<" PURGE "<<nb->nb_addr<<std::endl;
#endif
			nb_delete(nb->nb_addr);
		}
	}
}

/*
 *	�����ھ�����
 */
int BundleAgent::nb_sum(){
	int sum = 0;
	DTN_Neighbor *nb = nbhead.lh_first;
	DTN_Neighbor *nbn;
	for(; nb; nb = nbn) {
		nbn = nb->nb_link.le_next;
		sum++;
	}
	return sum;
}

/*
 *
 *
 */

/*
 *	�����ھ�·�ɱ?�ݲ����á�
 *		1. �����ÿһ��������һ��
 *		2. ����Ϊ���ڵ�λ��
 *		3. �ֿ飬��Ϊ���²���
 */
void BundleAgent::rt_update(int locX, int locY){
	//����ÿ���ھ���Ŀ飬ȡ��ÿһ������
	DTN_Neighbor *nb = nbhead.lh_first;
	DTN_Neighbor *nbn;
	//	int temp_max[BLOCK];//��ÿһ�鵱ǰ�����ھ�
	for (int i=0; i<BLOCK; i++)
	{
		//		temp_max[i] = -1;
	}
	for(; nb; nb = nbn) {
		nb->orient = m_uti_nodeInfo.orient_block_cal(nb->locX - locX, nb->locY - locY);
		//int a = (nb->syn_utilValue[nb->orient] - m_uti_nodeInfo.getSynValue(nb->orient))/m_uti_nodeInfo.getSynValue(nb->orient);
		//int b = ()//���ʱ��û��Ŀ�Ľڵ����꣡��
		nbn = nb->nb_link.le_next;

	}
}

/*
 *	��װ������Bundle����ֱ�Ӵ�����ж��Ƿ��ء���������Ӧ���
 */
Packet* BundleAgent::process_for_reassembly(int bundle_id){
	Packet* newpkt = allocpkt();
	hdr_cmn* newch = hdr_cmn::access(newpkt);
	hdr_bundle* newbh = hdr_bundle::access(newpkt);
	//	hdr_ip* iph=hdr_ip::access(newpkt);

	std::map<int, packet_info>::iterator itr = fragMap[bundle_id].begin();
	std::map<int, std::map<int,packet_info> >::iterator itr_L = fragMap.begin();

	hdr_bundle* fragbh = hdr_bundle::access(itr->second.p);

	//��Ƭ���͵�ʱ��û�иĶ�bundle_size
	newbh->bundle_size = fragbh->bundle_size;
	newch->size() = fragbh->bundle_size;

	newbh->is_fragment = false;
	newbh->nfragments = 1;

	newbh->originating_timestamp = fragbh->originating_timestamp;
	newbh->lifetime = fragbh->lifetime;
	newbh->type = fragbh->type;
	newbh->src = fragbh->src;
	//	newbh->prev = here_.addr_;
	newbh->dst = fragbh->dst;
	//    newbh->hop_count=bh->hop_count+1;
	newbh->bundle_id = fragbh->bundle_id;
	newbh->custody_transfer = fragbh->custody_transfer;
	newbh->return_receipt = fragbh->return_receipt;

	newbh->initial_timestamp = fragbh->initial_timestamp;

	if(fragbh->dst == m_uti_nodeInfo.index){
		//end
		receivedBundles++;
		Packet::free(newpkt);
	}else{
		//enque
		Packet* pkt2 = newpkt->copy();
		myBundleStorage->enque(pkt2);
		myBundleBuffer->enque(newpkt);

	}
	//�ͷ���Դ
	for(; itr != fragMap[bundle_id].end(); itr++){
		//		ch->size() =
		Packet::free(itr->second.p);
	}
	//ɾ�����
	recivedBundle.push_back(bundle_id);
	itr_L = fragMap.find(bundle_id);
	if(itr_L != fragMap.end())
		fragMap.erase(itr_L);

	return 0;
}

/*
 *	�ж�Bundle��Դ����Ŀ�ĵ�Ϊ���ڵ�/Ϊָ����һ�� ����
 *				�м仺�����,��������ѡ�����ĸ�����
���ǹ�·��ݷ���true
 */
bool BundleAgent::check_bundle(Packet * p){
	hdr_bundle* bh=hdr_bundle::access(p);
	hdr_ip* iph=hdr_ip::access(p);

	for(unsigned int i=0; i<recivedBundle.size(); i++){
		if(recivedBundle[i] == bh->bundle_id){
			duplicateFrag++;
			return false;
		}
	}
	if (!bh->is_fragment && bh->dst == here_.addr_)
	{
		receivedBundles++;
		Packet::free(p);
		return false;
	}else if(bh->is_fragment)
		return true;
	else if(iph->daddr() == here_.addr_){

#ifdef DEBUG
		//for debugging
		std::cout<<"Im node "<<m_uti_nodeInfo.index<<" enque!!"<<std::endl;
#endif

		myBundleBuffer->enque(p);
		return true;
	}else{
		midBundleBuffer->enque(p);
		return false;
	}
	return 0;
}

//void Util_nodeInfo::increaseCal(Util_nodeInfo &matrix, double increase, int orient){
// int inNum1 = (orient+1)%BLOCK;
// int inNum2 = (orient+BLOCK-1)%BLOCK;
//
// matrix.setUtilValue(inNum1, matrix.getUtilValue(inNum1)+increase*0.5);
// matrix.setUtilValue(inNum2, matrix.getUtilValue(inNum2)+increase*0.5);
//}

/*
 *	ͨ����ʸ���������Ӧ��ֵ
 */
int Util_nodeInfo::orient_block_cal(double locX, double locY){
	int orient;
	double tempOrient = 0.0;
	if(locY > 0){
		tempOrient = atan2(locY,locX);
		if (tempOrient < PI/6)
			orient = 0;
		else if(tempOrient <= PI/3)
			orient = 1;
		else if(tempOrient <= PI/2)
			orient = 2;
		else if(tempOrient <= PI*2/3)
			orient = 3;
		else if(tempOrient <= PI*5/6)
			orient = 4;
		else if(tempOrient <= PI)
			orient = 5;
	}else{
		tempOrient = atan2(abs(locY),locX);
		if (tempOrient < PI/6)
			orient = 11;
		else if(tempOrient <= PI/3)
			orient = 10;
		else if(tempOrient <= PI/2)
			orient = 9;
		else if(tempOrient <= PI*2/3)
			orient = 8;
		else if(tempOrient <= PI*5/6)
			orient = 7;
		else if(tempOrient <= PI)
			orient = 6;
	}
	return orient;
}
/*
 *	Ч��ֵ���㣬�����ڼ��㡾@!@��δ���ǵ�ͼ��
 */

void Util_nodeInfo::uvalue_block_cal(){

#ifdef UTIL_DEBUG
	//for debugging
	//	std::cout<<"I am node "<<index<<"uvalue_block_cal"<<endl;
	int temorient = orient_block_cal(orientX, orientY);
	if(index == 1){
		f1<<"ORIENT: "<<temorient<<"\r\n";
		//		if(orient != 11){
		//			assert(1);
		//			std::cout<<"ORIENT!!!!!!!!!!!!!!!"<<orient<<std::endl;
		//		}
	}
	if(index == 1&&CURRENT_TIME>155.0){
		cout<<"Time: "<<CURRENT_TIME<<"\r\n";
		for (int orient11=0; orient11<BLOCK; orient11++)
		{
			//			std::cout<<orient11<<": "<<getSynValue(orient11)<<std::endl;//"                      "<<this->getUtilValue(orient11)<<std::endl;
			cout<<'O'<<orient11<<": "<<getSynValue(orient11)<<std::endl;
		}
	}
	if(index == 1){
		//		std::cout<<"I am node "<<index<<"++++++++++++++++++++++++++++++ AT "<<this->locX<<" "<<this->locY<<std::endl;
		//		std::cout<<"nb->syn_utilValue[orient]:"<<nb->syn_utilValue[orient]<<" m_uti_nodeInfo.getSynValue(orient): "<<m_uti_nodeInfo.getSynValue(orient)<<endl;
		f1<<"Time: "<<CURRENT_TIME<<"\r\n";
		for (int orient11=0; orient11<BLOCK; orient11++)
		{
			//			std::cout<<orient11<<": "<<getSynValue(orient11)<<std::endl;//"                      "<<this->getUtilValue(orient11)<<std::endl;
			f1<<'O'<<orient11<<": "<<getSynValue(orient11)<<"\r\n";
		}
		f1<<"\r\n";
		f1.flush();

	}

#endif

	//��ȡ�ڵ��ٶȺͷ���ʸ����ͬʱ���±��ڵ���ٶȺ�λ��
	Node *thisnode; //
	thisnode = Node::get_node_by_address(index);//index);
	speed = ((MobileNode *)thisnode)->speed();
	//����ʸ��
	((MobileNode *)thisnode)->getVelo(&orientX, &orientY, &orientZ);
	//��ַ
	((MobileNode *)thisnode)->getLoc(&locX, &locY, &locZ);

	//ͨ����ʸ���������Ӧ��ֵ
	int orient = orient_block_cal(orientX, orientY);

#ifdef UTIL_DEBUG
	//	if(orient == 10){
	//		orient = orient_block_cal(orientX, orientY);
	//	}


#endif

	//��ȡ�����
	double dTemple0 = this->getUtilValue(orient);
	double dTemple1 = this->getUtilValue((orient+1)%BLOCK);
	double dTemple2 = this->getUtilValue((orient+BLOCK-1)%BLOCK);
	double dA0 = exp(0.35*dTemple0*dTemple0)-0.2;
	double dA1 = exp(0.35*dTemple1*dTemple1)-0.2;
	double dA2 = exp(0.35*dTemple2*dTemple2)-0.2;
	//��Ч��ֵС������Ӵ�����
	if(dTemple0 < 0)
		dA0 *= 5;
	if(dTemple1 < 0)
		dA1 *= 5;
	if(dTemple2 < 0)
		dA2 *= 5;
	double uti_increase0 = dA0 * speed * 0.002;
	double uti_increase1 = dA1 * speed * 0.002 *0.5;
	double uti_increase2 = dA2 * speed * 0.002 *0.5;
	dTemple0 += uti_increase0;
	dTemple1 += uti_increase1;
	dTemple2 += uti_increase2;
	//����˥��
	if (dTemple0<1){
		setUtilValue(orient, dTemple0);
	}
	if (dTemple1<1){
		setUtilValue((orient+1)%BLOCK, dTemple1);
	}
	if (dTemple2<1){
		setUtilValue((orient+BLOCK-1)%BLOCK, dTemple2);
	}
	//	increaseCal(dA*speed, orient);
	decayCal2();
	decayCal(uti_increase0, orient);
	synCal(orient);

}
/*
 *	�ۺ�Ч��ֵ����,ͬ��ͬ��
 */
void Util_nodeInfo::synCal(int ori_orient){
	bool de_flag = 1;
	double synValue = -1;
	for (int orient=0; orient<BLOCK; orient++, de_flag = 1)
	{
		if(ori_orient == orient || ori_orient == (orient+1)%BLOCK || ori_orient == (orient+2)%BLOCK
				|| ori_orient ==  (orient+BLOCK-1)%BLOCK || ori_orient == (orient+BLOCK-2)%BLOCK)
			de_flag = 0;
		double synNum0 = getUtilValue(orient);
		double synNum1 = getUtilValue((orient+1)%BLOCK);
		//		double synNum2 = getUtilValue((orient+2)%BLOCK); //0;
		double synNum3 = getUtilValue((orient+BLOCK-1)%BLOCK);
		//		double synNum4 = getUtilValue((orient+BLOCK-2)%BLOCK);//0;
		double sum_1_3 = synNum1 + synNum3;
		//		double sum_2_4 = synNum2 + synNum4;

		if(this->getSynValue(orient) < -0.5){
			sum_1_3 = 0;
			if(synNum1 > 0)
				sum_1_3 += synNum1;
			if(synNum3 > 0)
				sum_1_3 += synNum3;

			synValue = synNum0 + (sum_1_3)*0.5 ;//+ (sum_2_4)*0.2;
			if(synValue > 1.0)
				synValue = 0.99;
			if(synValue < -1.0)
				synValue = -0.99;
			//			setSynValue(orient,synValue);

		}
		//		else if(this->getSynValue(orient) < 0.3){
		//			sum_1_3 = 0;
		////			sum_2_4 = 0;
		//			if(abs(synNum1) > abs(synNum3)){
		//				if(synNum1 > 0)
		//					sum_1_3 += abs(synNum1) - abs(synNum3);
		//				else
		//					sum_1_3 += -(abs(synNum1) - abs(synNum3));
		//			}else
		//				if(synNum3 > 0)
		//					sum_1_3 += abs(synNum3) - abs(synNum1);
		//				else
		//					sum_1_3 += -(abs(synNum3) - abs(synNum1));
		//			synValue = synNum0 + (sum_1_3)*0.5 ;//+ (sum_2_4)*0.2;
		//			if(synValue > 1.0)
		//				synValue = 0.99;
		//			if(synValue < -1.0)
		//				synValue = -0.99;
		//
		////			setSynValue(orient,synValue);
		//
		////			if(abs(synNum2) > abs(synNum4)){
		////				if(synNum2 > 0)
		////					sum_2_4 += synNum2 * 0.5;
		////			}else
		////				if(synNum4 > 0)
		////					sum_2_4 += synNum4 * 0.5;
		//		}
		else if(this->getSynValue(orient) > 0.7){
			sum_1_3 = 0;
			if(abs(synNum1) > abs(synNum3)){
				if(synNum1 < 0)
					sum_1_3 += synNum1;
			}else
				if(synNum3 < 0)
					sum_1_3 += synNum3;

			synValue = synNum0 + (sum_1_3)*0.3 ;//+ (sum_2_4)*0.2;
			if(synValue > 1.0)
				synValue = 0.99;
			if(synValue < -1.0)
				synValue = -0.99;
			//			setSynValue(orient,synValue);

			//			if(abs(synNum2) > abs(synNum4)){
			//				if(synNum2 < 0)
			//					sum_2_4 += synNum2;
			//			}else
			//				if(synNum4 < 0)
			//					sum_2_4 += synNum4;
		}else {
			synValue = synNum0 + (sum_1_3)*0.3 ;//+ (sum_2_4)*0.2;
			if(synValue > 1.0)
				synValue = 0.99;
			if(synValue < -1.0)
				synValue = -0.99;
			if(synValue > 0.7)
				synValue = synNum0;

		}
		if(de_flag)
			synValue = synNum0;
		setSynValue(orient,synValue);
	}
}

void Util_nodeInfo::decayCal(double increase, int orient){
	int deNum0 = (orient+3)%BLOCK;//
	int deNum1 = (orient+4)%BLOCK;//
	int deNum2 = (orient+5)%BLOCK;//
	int deNum3 = (orient+6)%BLOCK;
	int deNum4 = (orient+7)%BLOCK;
	int deNum5 = (orient+8)%BLOCK;
	int deNum6 = (orient+9)%BLOCK;//

	double temp0 = getUtilValue(deNum0)-increase*1;
	if(temp0 < -1.0)
		temp0 = -1.0;
	double temp1 = getUtilValue(deNum1)-increase*1.2;
	if(temp1 < -1.0)
		temp1 = -1.0;
	double temp2 = getUtilValue(deNum2)-increase*1.4;
	if(temp2 < -1.0)
		temp2 = -1.0;
	double temp3 = getUtilValue(deNum3)-increase*1.6;
	if(temp3 < -1.0)
		temp3 = -1.0;
	double temp4 = getUtilValue(deNum4)-increase*1.4;
	if(temp4 < -1.0)
		temp4 = -1.0;
	double temp5 = getUtilValue(deNum5)-increase*1.2;
	if(temp5 < -1.0)
		temp5 = -1.0;
	double temp6 = getUtilValue(deNum6)-increase*1;
	if(temp6 < -1.0)
		temp6 = -1.0;

	setUtilValue(deNum0, temp0);
	setUtilValue(deNum1, temp1);
	setUtilValue(deNum2, temp2);
	setUtilValue(deNum3, temp3);
	setUtilValue(deNum4, temp4);
	setUtilValue(deNum5, temp5);
	setUtilValue(deNum6, temp6);
}

void Util_nodeInfo::decayCal2(){
	for (int i=0; i<BLOCK; i++)
	{
		if(getUtilValue(i) > 0)
			setUtilValue(i, getUtilValue(i)*CON_DERATE);
		if(getUtilValue(i) < 0)
			setUtilValue(i, getUtilValue(i)*0.9);//increase !
	}
}

