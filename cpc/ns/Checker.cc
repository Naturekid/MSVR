/*
 * Checker.cpp
 *
 *  Created on: May 15, 2013
 *      Author: david
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <sys/time.h>
#include <list>
#include "Checker.h"

Checker::Checker( ) {
	// TODO Auto-generated constructor stub
}

Checker::~Checker( ) {
	// TODO Auto-generated destructor stub
}

int Checker::existBroadcast_buffer(struct Broadcast_buffer *the_buffer){

	for(std::list<Broadcast_buffer>::iterator iter = broadcast_buffer_list.begin();
			iter != broadcast_buffer_list.end(); ++iter) {
		struct hdr_cmn *ch1 = HDR_CMN(iter->pkt);
		struct hdr_cmn *ch2 = HDR_CMN(the_buffer->pkt);
		if ( ch1->uid_ == ch2->uid_){
			gettimeofday(&iter->ts, NULL);
			return 1;
		}
	}
	return 0;
}

int Checker::existBroken_pair(struct Broken_pair *the_pair){
	for(std::list<Broken_pair>::iterator iter = broken_pair_list.begin();
			iter != broken_pair_list.end(); ++iter) {
		struct in_addr x = the_pair->src;
		struct in_addr y = the_pair->dst;
		if ( iter->dst.s_addr == x.s_addr && iter->src.s_addr == y.s_addr){
			gettimeofday(&iter->ts, NULL);
			return 1;
		}
	}
	return 0;
}

int Checker::addBroadcast_buffer(struct Broadcast_buffer *the_buffer){
	struct Broadcast_buffer x;
	gettimeofday(&x.ts, NULL);
	x.pkt = the_buffer->pkt->copy();
	broadcast_buffer_list.push_back(x);
		return 1;
}

int Checker::addBroken_pair(struct Broken_pair *the_pair){
	struct Broken_pair x;
	gettimeofday(&x.ts,NULL);
	x.dst = the_pair->dst;
	x.src = the_pair->src;
	broken_pair_list.push_back(x);
		return 1;
}

int Checker::delBroken_pair(struct Broken_pair *the_pair ){
	for(std::list<Broken_pair>::iterator iter = broken_pair_list.begin();
			iter != broken_pair_list.end(); ++iter) {
		struct in_addr x = the_pair->src;
		struct in_addr y = the_pair->dst;
		if ( iter->dst.s_addr == y.s_addr && iter->src.s_addr == x.s_addr){
			iter = broken_pair_list.erase(iter);
			return 1;
		}
	}
	return 0;
}

void Checker::expire( Event *e){

	struct timeval now;
	gettimeofday(&now, NULL);

	for(std::list<Broken_pair>::iterator iter = broken_pair_list.begin();
				iter != broken_pair_list.end(); ++iter){
		if(float( now.tv_sec - iter->ts.tv_sec) >= EXPIRETIME)
			iter = broken_pair_list.erase(iter);
	}

	for(std::list<Broadcast_buffer>::iterator iter = broadcast_buffer_list.begin();
				iter != broadcast_buffer_list.end(); ++iter) {
		if(float(now.tv_sec - iter->ts.tv_sec) >= EXPIRETIME )
			iter = broadcast_buffer_list.erase(iter);
	}

}
