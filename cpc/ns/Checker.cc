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
		if (*iter  == *the_buffer)
			return 1;
	}
	return 0;
}

int Checker::existBroken_pair(struct Broken_pair *the_pair){
	for(std::list<Broken_pair>::iterator iter = broken_pair_list.begin();
			iter != broken_pair_list.end(); ++iter) {
		if ( iter->dst == the_pair->dst && iter->src == the_pair->src)
			return 1;
	}
	return 0;
}

int Checker::addBroadcast_buffer(struct Broadcast_buffer *the_buffer){
	return 0;
}

int Checker::addBroken_pair(struct Broken_pair *the_pair){
	return 0;
}

void Checker::expire(){


}
