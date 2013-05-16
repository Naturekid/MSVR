/*
 * Checker.cpp
 *
 *  Created on: May 15, 2013
 *      Author: david
 */

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
		if (*iter  == *the_buffer){
			gettimeofday(&iter->ts, NULL);
			return 1;
		}
	}
	return 0;
}

int Checker::existBroken_pair(struct Broken_pair *the_pair){
	for(std::list<Broken_pair>::iterator iter = broken_pair_list.begin();
			iter != broken_pair_list.end(); ++iter) {
		if ( iter->dst == the_pair->dst && iter->src == the_pair->src){
			gettimeofday(&iter->ts, NULL);
			return 1;
		}
	}
	return 0;
}

int Checker::addBroadcast_buffer(struct Broadcast_buffer *the_buffer){
	struct Broadcast_buffer x;
	gettimeofday(&x.ts, NULL);
	x.packet = the_buffer->packet;
	if(broadcast_buffer_list.push_back(x))
		return 1;
	else
		return 0;
}

int Checker::addBroken_pair(struct Broken_pair *the_pair){
	struct Broken_pair x;
	gettimeofday(&x.ts,NULL);
	x.dst = the_pair->dst;
	x.src = the_pair->src;
	if(broken_pair_list.push_back(x))
		return 1;
	else
		return 0;
}

int Checker::delBroken_pair(struct Broken_pair *the_pair ){
	for(std::list<Broken_pair>::iterator iter = broken_pair_list.begin();
			iter != broken_pair_list.end(); ++iter) {
		if ( iter->dst == the_pair->dst && iter->src == the_pair->src){
			iter = broken_pair_list.erase(iter);
			return 1;
		}
	}
	return 0;
}

void Checker::expire(){

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
