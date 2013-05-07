#ifndef _GPS_COMMON_H
#define _GPS_COMMON_H

class MsvrAgent;

struct point {
    double x;
    double y;
};

class MsvrGps {
    public:
        MsvrGps(MsvrAgent* agent) {
            agent_ = agent;
        }
        virtual ~MsvrGps() {}

        struct point getPos(int id);
        double getSpeed(int id);
        double getHeading(int id);
    private:
        MsvrAgent* agent_;
};

double msvr_cal_dist(struct point p1, struct point p2);
double msvr_cal_dist(double x1, double y1, double x2, double y2);

#endif /* _GPS_COMMON_H */
