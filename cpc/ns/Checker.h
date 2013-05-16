/*
 * Checker.h
 *
 *  Created on: May 15, 2013
 *      Author: david
 */

#ifndef CHECKER_H_
#define CHECKER_H_

#define EXPIRETIME 5.0

#include "timer-handler.h"

class Checker: public TimerHandler {
public:
	Checker( );
	virtual ~Checker();

	int addBroken_pair( struct Broken_pair *the_pair);
	int existBroken_pair(struct Broken_pair *the_pair);
	int addBroadcast_buffer(struct Broadcast_buffer *the_buffer);
	int existBroadcast_buffer(struct Broadcast_buffer *the_buffer);
	int delBroken_pair(struct Broken_pair *the_pair);

protected:
	void expire(Event *e);

private:
	std::list<Broken_pair> broken_pair_list;
	std::list<Broadcast_buffer> broadcast_buffer_list;
};

struct Broken_pair {
	struct in_addr src;
	struct in_addr dst;
	struct timeval ts;
};

struct Broadcast_buffer {
	struct Packet packet;
	struct timeval ts;
};

#endif /* CHECKER_H_ */
