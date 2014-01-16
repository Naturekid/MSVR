/*
 * Copyright (C) 2007 
 * Mercedes-Benz Research & Development North America, Inc. and
 * University of Karlsruhe (TH)
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * The copyright of this module includes the following
 * linking-with-specific-other-licenses addition:
 *
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 */

/*
 * For further information see: 
 * http://dsn.tm.uni-karlsruhe.de/english/Overhaul_NS-2.php
 */



#include <iostream>
#include <stdio.h>
#include "random.h"
#include "pbc.h"
//#include "fec.h"
int hdr_pbc::offset_;

/**********************TCL Classes***************************/
static class PBCHeaderClass : public PacketHeaderClass {
public:
	PBCHeaderClass() : PacketHeaderClass("PacketHeader/PBC",
			sizeof(hdr_pbc)) {
		bind_offset(&hdr_pbc::offset_);
	}
} class_pbchdr;


static class PBCClass : public TclClass {
public:
	PBCClass() : TclClass("Agent/PBC") {}
	TclObject* create(int, const char*const*) {
		return (new PBCAgent());
	}
} class_pbc;

/**********************PBC Agent****Constructor*************************/
PBCAgent::PBCAgent() : Agent(PT_PBC), timer(this)
{
	bind("payloadSize", &size);
	bind("periodicBroadcastVariance", &msgVariance);
	bind("periodicBroadcastInterval", &msgInterval);
	bind("modulationScheme",&modulationScheme);
	periodicBroadcast = false;

	this->fec_list_head = NULL;
}

PBCAgent::~PBCAgent()
{

}

/**********************PBC Agent****Member functions *************************/
void PBCAgent::singleBroadcast()
{
	Packet* pkt = allocpkt();
	hdr_cmn *cmnhdr = hdr_cmn::access(pkt);
	hdr_pbc* pbchdr = hdr_pbc::access(pkt);
	hdr_ip*  iphdr  = hdr_ip::access(pkt);

	cmnhdr->next_hop() = IP_BROADCAST;

	cmnhdr->size()    = size;
	iphdr->src_.addr_ = here_.addr_;  //LL will fill MAC addresses in the MAC header
	iphdr->dst_.addr_ = IP_BROADCAST;
	iphdr->dst_.port_ = this->port();
	pbchdr->send_time	= Scheduler::instance().clock();

	switch (modulationScheme)
	{
	case BPSK:   cmnhdr->mod_scheme_ = BPSK;break;
	case QPSK:   cmnhdr->mod_scheme_ = QPSK;break;
	case QAM16:  cmnhdr->mod_scheme_ = QAM16;break;
	case QAM64:  cmnhdr->mod_scheme_ = QAM64;break;
	default :
		cmnhdr->mod_scheme_ = BPSK;
	}
	send(pkt,0);
}


void PBCAgent::singleUnicast(int addr)
{

	Packet* pkt = allocpkt();
	hdr_cmn *cmnhdr = hdr_cmn::access(pkt);
	hdr_pbc* pbchdr = hdr_pbc::access(pkt);
	hdr_ip*  iphdr  = hdr_ip::access(pkt);


	cmnhdr->addr_type() = NS_AF_ILINK;
	cmnhdr->next_hop()  = (u_int32_t)(addr);
	cmnhdr->size()      = size;
	iphdr->src_.addr_ = here_.addr_;  //MAC will fill this address
	iphdr->dst_.addr_ = (u_int32_t)(addr);
	iphdr->dst_.port_ = this->port();

	pbchdr->send_time 	= Scheduler::instance().clock();

	Fec *myfec;
	myfec= new Fec();
	myfec->Get_msg(myfec->message);
	myfec->Encode(myfec->COLBIT, myfec->BIT, myfec->ExptoFE, myfec->FEtoExp, myfec->packets, myfec->message);

	//memcpy(pbchdr->content , myfec->packets , sizeof(unsigned int) * 255);
	memcpy( pbchdr->content , myfec->packets,4*51);
	memcpy( pbchdr->content+4*51 , myfec->packets+51,4*51);
	memcpy( pbchdr->content+4*102 , myfec->packets+102,4*51);
	memcpy( pbchdr->content+4*153 , myfec->packets+153,4*51);
	memcpy( pbchdr->content+4*204 , myfec->packets+204,4*51);


	switch (modulationScheme)
	{
	case BPSK:   cmnhdr->mod_scheme_ = BPSK;break;
	case QPSK:   cmnhdr->mod_scheme_ = QPSK;break;
	case QAM16:  cmnhdr->mod_scheme_ = QAM16;break;
	case QAM64:  cmnhdr->mod_scheme_ = QAM64;break;
	default :
		cmnhdr->mod_scheme_ = BPSK;
	}
	send(pkt,0);


	//  Packet* pkt_1 = allocpkt();
	//    hdr_cmn *cmnhdr_1 = hdr_cmn::access(pkt_1);
	//    hdr_pbc* pbchdr_1 = hdr_pbc::access(pkt_1);
	//    hdr_ip*  iphdr_1  = hdr_ip::access(pkt_1);
	//
	//
	//    cmnhdr_1->addr_type() = NS_AF_ILINK;
	//    cmnhdr_1->next_hop()  = (u_int32_t)(addr);
	//    cmnhdr_1->size()      = size;
	//    iphdr_1->src_.addr_ = here_.addr_;  //MAC will fill this address
	//    iphdr_1->dst_.addr_ = (u_int32_t)(addr);
	//    iphdr_1->dst_.port_ = this->port();
	//
	//    pbchdr_1->send_time 	= Scheduler::instance().clock()+0.25;
	//    strcpy( pbchdr_1->content,"World\n");
	//
	//    switch (modulationScheme)
	//      {
	//      case BPSK:   cmnhdr_1->mod_scheme_ = BPSK;break;
	//      case QPSK:   cmnhdr_1->mod_scheme_ = QPSK;break;
	//      case QAM16:  cmnhdr_1->mod_scheme_ = QAM16;break;
	//      case QAM64:  cmnhdr_1->mod_scheme_ = QAM64;break;
	//      default :
	//        cmnhdr_1->mod_scheme_ = BPSK;
	//      }
	//    //send(pkt_1,0);
	//    Scheduler::instance().schedule(target_, pkt_1, 0.25 );
}

void PBCAgent::singleUnicast_fec(int addr)
{
	Fec *myfec;
	myfec= new Fec();
	myfec->Get_msg(myfec->message);
	myfec->Encode(myfec->COLBIT, myfec->BIT, myfec->ExptoFE, myfec->FEtoExp, myfec->packets, myfec->message);
	double the_time = Scheduler::instance().clock();
	for( int fecid = 0 ; fecid < myfec->Npackets; fecid ++){

		Packet* pkt = allocpkt();
		hdr_cmn *cmnhdr = hdr_cmn::access(pkt);
		hdr_pbc* pbchdr = hdr_pbc::access(pkt);
		hdr_ip*  iphdr  = hdr_ip::access(pkt);


		cmnhdr->addr_type() = NS_AF_ILINK;
		cmnhdr->next_hop()  = (u_int32_t)(addr);
		cmnhdr->size()      = size;
		iphdr->src_.addr_ = here_.addr_;  //MAC will fill this address
		iphdr->dst_.addr_ = (u_int32_t)(addr);
		iphdr->dst_.port_ = this->port();

		pbchdr->send_time 	= the_time;

		memcpy( pbchdr->content , myfec->packets+fecid*51 , 4*51);

		switch (modulationScheme)
			{
			case BPSK:   cmnhdr->mod_scheme_ = BPSK;break;
			case QPSK:   cmnhdr->mod_scheme_ = QPSK;break;
			case QAM16:  cmnhdr->mod_scheme_ = QAM16;break;
			case QAM64:  cmnhdr->mod_scheme_ = QAM64;break;
			default :
				cmnhdr->mod_scheme_ = BPSK;
			}
		Scheduler::instance().schedule(target_, pkt, 0.1 * fecid );
	}

}


void PBCAgent::recv(Packet* pkt, Handler*)
{
	hdr_ip*  iphdr  = hdr_ip::access(pkt);
	//if the destination is me, O, FEC things is on duty
	if(	iphdr->dst_.addr_ == here_.addr_){

		hdr_pbc* pbchdr = hdr_pbc::access(pkt);
		printf("arrived %d---fecid %d ---------%lf\n", here_.addr_,*pbchdr->content,pbchdr->send_time);

		int inthelist = 0;
		struct fec_list * temp = fec_list_head;
		struct fec_list * temp_delete = NULL;
		//check fec list queue, use the send time as the ID.
		//if the send_time is the same ,then check the fec_position,
		//if there are M different fec_packets enough, Decode it.Got the message.
		//if there aren't M enough , put in the list, continue

		while(temp != NULL){
			if(temp->time == pbchdr->send_time){
				inthelist = 1;
				int position = *pbchdr->content;
				if( temp->receive[position] == 1)
					break;
				else{
					temp->receive[position] = 1;
					temp->receive_n ++;
					memcpy( temp->content+4*51*position, pbchdr->content,4*51);
					if(temp->receive_n >= 3){
						temp_delete = temp;
						Fec * myfec = new Fec();
						memcpy( myfec->rec_packets,temp->content,255*4);
						myfec->Decode(myfec->COLBIT, myfec->BIT, myfec->ExptoFE, myfec->FEtoExp, myfec->rec_packets, &temp->receive_n, myfec->rec_message);
						printf("Got right message %s at %d \n",(char *)myfec->rec_message,here_.addr_);
					}
					break;
				}
			}else
				temp = temp->next;
		}
		if(inthelist == 0){
			struct fec_list * newnode = (struct fec_list *)malloc(sizeof(struct fec_list));
			memset( newnode , 0 , sizeof(struct fec_list) );
			newnode->time = pbchdr->send_time;
			newnode->receive_n++;
			int position = *pbchdr->content;
			newnode->receive[position] = 1;
			memcpy( newnode->content+4*51*position, pbchdr->content, 4*51);
			newnode->next = fec_list_head;
			fec_list_head = newnode;
		}
		if(temp_delete!=NULL){
			if(temp_delete == fec_list_head){
				fec_list_head = fec_list_head->next;
				free(temp_delete);
			}else{
				temp = fec_list_head;
				//while()
			}
		}

	}

	Packet::free(pkt);
}

/*************************PBC Timer *******************/
/****asyn_start() start() stop() setPeriod()***********/
void
PBCTimer::start(void)
{
	if(!started)
	{
		Scheduler &s = Scheduler::instance();
		started = 1;
		variance = agent->msgVariance;
		period = agent->msgInterval;
		double offset = Random::uniform(0.0,period);
		s.schedule(this, &intr, offset);
	}
}


void PBCTimer::stop(void)
{
	Scheduler &s = Scheduler::instance();
	if(started)
	{
		s.cancel(&intr);
	}
	started = 0;
}

void PBCTimer::setVariance(double v)
{
	if(v >= 0) variance = v;
}

void PBCTimer::setPeriod(double p)
{
	if(p >= 0) period = p;
}

void PBCTimer::handle(Event *e)
{
	agent->singleBroadcast();
	if(agent->periodicBroadcast)
	{
		Scheduler &s = Scheduler::instance();
		double t = period - variance + Random::uniform(variance*2);
		s.schedule(this, &intr, t>0.0 ? t : 0.0);
	}
}


/*************************************************************************/


int PBCAgent::command(int argc, const char*const* argv)
{
	if (argc == 2) 
	{
		if (strcmp(argv[1], "singleBroadcast") == 0)
		{
			singleBroadcast();
			return (TCL_OK);
		}
		if (strcmp(argv[1], "stop") == 0)
		{
			timer.stop();
			return (TCL_OK);
		}
	}
	if (argc == 3)
	{
		if(strcmp(argv[1],"unicast")==0)
		{
			int addr =atoi(argv[2]);
			singleUnicast(addr);
			return TCL_OK;
		}
		if(strcmp(argv[1],"unicast_fec")==0)
		{
			int addr =atoi(argv[2]);
			singleUnicast_fec(addr);
			return TCL_OK;
		}
		if(strcmp(argv[1],"PeriodicBroadcast") ==0)
		{
			if (strcmp(argv[2],"ON") == 0 )
			{
				periodicBroadcast = true;
				timer.start();
				return TCL_OK;
			}
			if(strcmp(argv[2],"OFF") ==0 )
			{
				periodicBroadcast = false;
				timer.stop();
				return TCL_OK;
			}
		}


	}
	// If the command hasn't been processed by PBCAgent()::command,
	// call the command() function for the base class
	return (Agent::command(argc, argv));
}
