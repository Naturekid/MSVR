#ifndef MSVR_MAP_H
#define MSVR_MAP_H

#include <algorithm>

#include <boost/config.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

using namespace boost;

/*
 * store vertex position.
 */
struct Point {
    int id_;
    double x_;
    double y_;

    Point() : id_(-1), x_(0.0), y_(0.0) {}
    Point(int id, double x, double y) : id_(id), x_(x), y_(y) {}
    Point(const Point& p) : id_(p.id_), x_(p.x_), y_(p.y_) {}
};

/*
 * store road information as edge property.
 */
struct Road {
    int id_;
    int type_;   // store road's type

    Road() : id_(-1), type_(0) {}
    Road(const Road& r) : id_(r.id_), type_(r.type_) {}
    Road(int id, int t) : id_(id), type_(t) {}
};

/*typedef adjacency_list<listS, vecS, directedS,*/
        /*Point, Road> Map;*/
typedef adjacency_list<listS, vecS, undirectedS,
        Point, Road> Map;
typedef graph_traits<Map>::vertex_descriptor vertex_descriptor;
typedef graph_traits<Map>::edge_descriptor edge_descriptor;
typedef std::pair<int, int> Edge;


class MsvrMap {
    public:
        MsvrMap();
        virtual ~MsvrMap();

        Road getRoadByPos(double x, double y);
        int getRoadByNode(int src, int dst);
        std::vector<int> getPaths(double x1, double y1,
                                   double x2, double y2);
        std::pair<int, int> getSrcAndDst(double x1, double y1,
                                          double x2, double y2);

        inline const Map& getMap() const {
            return map_;
        }
    private:
        bool hasNextPath(vertex_descriptor& src, vertex_descriptor& dst);
        double meanSquare(const std::vector<int>& types);
        int findMeanMin(const std::vector<std::vector<int> >& types, const std::vector<double>& means);

        Map map_;
};

#endif // MSVR_MAP_H
