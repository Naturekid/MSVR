#include <stdio.h>
#include <math.h>

#include <common/mobilenode.h>

#include "msvr_gps.h"
#include "../msvr_agent.h"

struct point
MsvrGps::getPos(int id)
{
    struct point p;
    MobileNode* node;

    node = static_cast<MobileNode*>(Node::get_node_by_address(
                static_cast<nsaddr_t>(id)));

    p.x = node->X();
    p.y = node->Y();

    return p;
}

double
MsvrGps::getSpeed(int id)
{
    MobileNode* node;

    node = static_cast<MobileNode*>(Node::get_node_by_address(
                static_cast<nsaddr_t>(id)));

    return node->speed();
}

#define PI (3.1415926)

double cal_angle(double dx, double dy)
{
	double angle;
	double at;

	if (-0.01 < dx && dx < 0.01) {
		if (dy > 0.0)
			return 0;
		else
			return 180;
	} else if (-0.01 < dy && dy < 0.01) {
		if (dx > 0.0)
			return 90;
		else
			return 270;
	} else if (-0.01 < dx && dx < 0.01 &&
			-0.01 < dy && dy < 0.01)
		return 0;
	else {
		at = atan(dx / dy);
		if (at < 0.0)
			at = at + PI / 2;
		angle = at * 180 / PI;
		if (dx > 0.0 && dy > 0.0)
			return angle;
		else if (dx > 0.0 && dy < 0.0)
			return angle + 90;
		else if (dx < 0.0 && dy < 0.0)
			return angle + 180;
		else
			return angle + 270;
	}
}

double
MsvrGps::getHeading(int id)
{
	return cal_angle(agent_->getNodeX(), agent_->getNodeY());
}

double
msvr_cal_dist(struct point p1, struct point p2)
{
    return sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
}

double
msvr_cal_dist(double x1, double y1, double x2, double y2)
{
    point p1, p2;

    p1.x = x1;
    p1.y = y1;
    p2.x = x2;
    p2.y = y2;

    return msvr_cal_dist(p1, p2);
}
