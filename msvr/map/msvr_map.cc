#include <algorithm>
#include <iostream>
#include <limits>
#include <cmath>
#include <stdio.h>

#include <assert.h>

#include "msvr_map.h"
#include "../gps/msvr_gps.h"

/*#define MSVR_ROAD_PAD 15.0*/
#define MSVR_ROAD_PAD 30.0

MsvrMap::MsvrMap()
{
	// TODO: construct map member from file or by manual.
	// construct a simple map to test.
	typedef graph_traits<Map>::vertex_descriptor vertex_descriptor;
	typedef graph_traits<Map>::edge_descriptor edge_descriptor;
	typedef std::pair<int, int> Edge;

#if 0 
	const int num_nodes = 4;
	enum nodes { A, B, C, D };
	/*char name[] = "ABCD";*/
	Edge edge_array[] = {
			Edge(A, B), Edge(B, D), Edge(A, C), Edge(C, D),
	};
	Road weights[] = {
			Road(0, 20), Road(1, 100), Road(2, 60), Road(3, 60),
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	Point point_arr[] = {
			Point(0, 0.0, 0.0), Point(1, 1000.0, 0.0),
			Point(2, 0.0, 1000.0), Point(3, 1000.0, 1000.0),
	};
#endif
#if 0
	const int num_nodes = 5;
	enum nodes { A, B, C, D, E };
	/*char name[] = "ABCD";*/
	Edge edge_array[] = {
			Edge(A, B), Edge(B, C), Edge(C, D), Edge(D, E),
	};
	Road weights[] = {
			Road(0, 100), Road(1, 100), Road(2, 100), Road(3, 100),
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	Point point_arr[] = {
			Point(0, 0.0, 0.0), Point(1, 100.0, 0.0),
			Point(2, 1400.0, 0.0), Point(3, 1500.0, 1000.0),
			Point(4, 1550.0, 0.0),
	};
#endif
#if 0
	const int num_nodes = 4;
	enum nodes { A, B, C, D, };
	/*char name[] = "ABCD";*/
	Edge edge_array[] = {
			Edge(A, B), Edge(B, D), Edge(A, C), Edge(C, D),
	};
	Road weights[] = {
			Road(0, 100), Road(1, 100), Road(2, 60), Road(3, 100),
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	Point point_arr[] = {
			Point(0, 0.0, 0.0), Point(1, 800.0, 0.0), Point(2, 0.0, 1400.0), Point(3, 800.0, 1400.0),
	};
#endif
	//#if 0
	const int num_nodes = 25;
	enum nodes {
		A, B, C, D, E, F, G, H, I, J, K, L, M,
		N, O, P, Q, R, S, T, U, V, W, X, Y, };
	/*char name[] = "ABCD";*/
	Edge edge_array[] = {
			Edge(A, B), Edge(B, C), Edge(C, D), Edge(D, E),
			Edge(A, F), Edge(B, G), Edge(C, H), Edge(D, I), Edge(E, J),
			Edge(F, G), Edge(G, H),
			Edge(F, K), Edge(G, L), Edge(H, M), Edge(I, N), Edge(J, O),
			Edge(K, L), Edge(L, M), Edge(M, N), Edge(N, O),
			Edge(K, P), Edge(L, Q), Edge(M, R), Edge(N, S), Edge(O, T),
			Edge(P, Q),             Edge(R, S), Edge(S, T),
			Edge(P, U), Edge(Q, V), Edge(R, W), Edge(S, X), Edge(T, Y),
			Edge(U, V), Edge(V, W), Edge(W, X), Edge(X, Y),
	};
	Road weights[] = {
			Road(0, 20), Road(1, 20), Road(2, 20), Road(3, 20),
			Road(4, 20), Road(5, 6000), Road(6, 20), Road(7, 100), Road(8, 20),
			Road(9, 60), Road(10, 60),
//			Road(0, 20), Road(1, 20), Road(2, 20), Road(3, 20),
//			Road(4, 20), Road(5, 60), Road(6, 60), Road(7, 100), Road(8, 20),
//			Road(9, 60), Road(10, 60),
			Road(11, 2000), Road(12, 60), Road(13, 100), Road(14, 100), Road(15, 100),
			/*Road(16, 60), Road(17, 60), Road(18, 60), Road(19, 60),*/
			Road(16, 60), Road(17, 100), Road(18, 60), Road(19, 60),
			Road(20, 60), Road(21, 60), Road(22, 100), Road(23, 100), Road(24, 100),
			Road(25, 60),                Road(26, 60), Road(27, 60),
			/*Road(28, 60), Road(29, 60), Road(30, 100), Road(31, 100), Road(32, 100),*/
			Road(28, 60), Road(29, 60), Road(30, 100), Road(31, 60), Road(32, 100),
			Road(33, 60), Road(34, 60), Road(35, 60), Road(36, 60),
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	Point point_arr[] = {
			Point(0, 0.0, 0.0), Point(1, 500.0, 0.0), Point(2, 1000.0, 0.0), Point(3, 1500.0, 0.0), Point(4, 2000.0, 0.0),
			Point(5, 0.0, 500.0), Point(6, 500.0, 500.0), Point(7, 1000.0, 500.0), Point(8, 1500.0, 500.0), Point(9, 2000.0, 500.0),
			Point(10, 0.0, 1000.0), Point(11, 500.0, 1000.0), Point(12, 1000.0, 1000.0), Point(13, 1500.0, 1000.0), Point(14, 2000.0, 1000.0),
			Point(15, 0.0, 1500.0), Point(16, 500.0, 1500.0), Point(17, 1000.0, 1500.0), Point(18, 1500.0, 1500.0), Point(19, 2000.0, 1500.0),
			Point(20, 0.0, 2000.0), Point(21, 500.0, 2000.0), Point(22, 1000.0, 2000.0), Point(23, 1500.0, 2000.0), Point(24, 2000.0, 2000.0),
	};
	//#endif

	// create ajdacency_list object
	Map map(edge_array, edge_array + num_arcs, weights, num_nodes);
	map_ = map;

	// construct vertex property
	graph_traits<Map>::vertex_iterator vi, vend;
	int i;
	for (i = 0, tie(vi, vend) = vertices(map_); vi != vend; ++vi, ++i) {
		map_[*vi].id_ = point_arr[i].id_;
		map_[*vi].x_ = point_arr[i].x_;
		map_[*vi].y_ = point_arr[i].y_;
	}
}

MsvrMap::~MsvrMap()
{
}

Road
MsvrMap::getRoadByPos(double x, double y)
{
	Road r;
	graph_traits<Map>::vertex_iterator vi, vend;//准备循环地图所有的顶点

	r.id_ = -1;

	for (tie(vi, vend) = vertices(map_); vi != vend; ++vi) {
		graph_traits<Map>::adjacency_iterator ai, aend;
		for (tie(ai, aend) = adjacent_vertices(*vi, map_);//循环扫描某个点的所有 相邻的点
				ai != aend; ++ai) {
			if (std::min(map_[*vi].x_, map_[*ai].x_) - MSVR_ROAD_PAD <= x &&
					x <= std::max(map_[*vi].x_, map_[*ai].x_) + MSVR_ROAD_PAD &&
					std::min(map_[*vi].y_, map_[*ai].y_) - MSVR_ROAD_PAD <= y &&
					y <= std::max(map_[*vi].y_, map_[*ai].y_) + MSVR_ROAD_PAD) {

				graph_traits<Map>::edge_iterator ei, eend;
				for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
					if (source(*ei, map_) == *vi &&
							target(*ei, map_) == *ai) {
						r.id_ = map_[*ei].id_;
						r.type_ = map_[*ei].type_;
						return r;
					}
				}
			}
		}
	}

	return r;
}

int
MsvrMap::getRoadByNode(int src, int dst)
{
	graph_traits<Map>::edge_iterator ei, eend;

	for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
		int srcid = map_[source(*ei, map_)].id_;
		int dstid = map_[target(*ei, map_)].id_;

		if ((srcid == src && dstid == dst) ||
				(srcid == dst && dstid == src))
			return map_[*ei].id_;
	}

	return -1;
}

bool
MsvrMap::hasNextPath(vertex_descriptor& src, vertex_descriptor& dst)
{
	graph_traits<Map>::edge_iterator ei, eend;
	double dist1 = msvr_cal_dist(map_[src].x_, map_[src].y_,
			map_[dst].x_, map_[dst].y_);

	for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
		if (map_[source(*ei, map_)].id_ == map_[dst].id_ ||
				map_[target(*ei, map_)].id_ == map_[dst].id_) {

			double dist2 = std::numeric_limits<double>::max();
			if (map_[source(*ei, map_)].id_ == map_[dst].id_)
				dist2 = msvr_cal_dist(map_[src].x_, map_[src].y_,
						map_[target(*ei, map_)].x_, map_[target(*ei, map_)].y_);
			else
				dist2 = msvr_cal_dist(map_[src].x_, map_[src].y_,
						map_[source(*ei, map_)].x_, map_[source(*ei, map_)].y_);

			if (map_[*ei].type_ != std::numeric_limits<int>::max() && dist1 > dist2)
				return true;
		}
	}

	return false;
}

double
MsvrMap::meanSquare(const std::vector<int>& types)
{
	int sum = 0;
	for (std::vector<int>::const_iterator iter = types.begin();
			iter != types.end(); ++iter)
		sum += *iter;

	double mean = sum / types.size();

	double sum2 = 0.0;
	for (std::vector<int>::const_iterator iter = types.begin();
			iter != types.end(); ++iter) {

		double tmp1 = *iter - mean;
		double tmp2 = tmp1 * tmp1;
		sum2 += tmp2;
	}

	double mean_square = sqrt(sum2 / types.size());

	return mean_square;
}

int
MsvrMap::findMeanMin(const std::vector<std::vector<int> >& types, const std::vector<double>& means)
{
	int index = 0;
	int i = 0;
	int cnt = 0;
	double max = std::numeric_limits<double>::max();
	int imax = std::numeric_limits<int>::max();

	for (std::vector<std::vector<int> >::const_iterator ii = types.begin();
			ii != types.end(); ++ii, ++i) {
		int sum = 0;
		for (std::vector<int>::const_iterator j = ii->begin();
				j != ii->end(); ++j) {
			sum += *j;
		}

		if (sum < imax) {
			cnt = 1;
			imax = sum;
			index = i;
		} else if (sum == imax) {
			cnt = 2;
		}
	}

	if (cnt == 1)
		return index;

	i = 0;
	for (std::vector<double>::const_iterator iter = means.begin();
			iter != means.end(); ++iter, ++i) {

		if (*iter < max) {
			max = *iter;
			index = i;
		}
	}

	return index;
}

std::vector<int>
MsvrMap::getPaths(double x1, double y1, double x2, double y2)
{
	typedef graph_traits<Map>::vertex_descriptor vertex_descriptor;
	std::vector<int> res;
	std::pair<int, int> path;   // save src and dst point id

	path = getSrcAndDst(x1, y1, x2, y2);

	std::vector<std::vector<vertex_descriptor> > preds_vec;
	std::vector<std::vector<int> > types_vec;
	std::vector<std::vector<int> > paths_vec;
	std::vector<double> means_vec;

	vertex_descriptor src = vertex(path.first, map_);
	vertex_descriptor dst = vertex(path.second, map_);
	fprintf(stdout , "起点坐标:(%.0f,%.0f),终点坐标:(%.0f,%.0f)\n",x1 ,y1 , x2 , y2);
	//fprintf(stdout,"起点路口ID:终点路口ID:%d %d--------------------------msvr_map.cc292\n",path.first,path.second);

	//while (hasNextPath(src, dst)) {//判断src和dst是不是在同一个路上,src距离dst到底有多近
		std::vector<vertex_descriptor> p(num_vertices(map_));
		std::vector<double> d(num_vertices(map_));
		std::vector<int> paths;

		dijkstra_shortest_paths(map_, src,
				predecessor_map(&p[0]).weight_map(get(&Road::type_, map_)).distance_map(&d[0]));

		graph_traits<Map>::vertex_iterator src_iter, dst_iter;
		graph_traits<Map>::vertex_iterator vi, vend;

		for (tie(vi, vend) = vertices(map_); vi != vend; ++vi) {
			if (map_[*vi].id_ == path.first)
				src_iter = vi;
			if (map_[*vi].id_ == path.second)
				dst_iter = vi;
		}

		paths.push_back(map_[*dst_iter].id_);
		graph_traits<Map>::edge_iterator ei, eend;
		std::vector<int> types;
		for (vi = dst_iter; vi != src_iter; vi = p[*vi]) {

			paths.push_back(map_[p[*vi]].id_);

			for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
				if ((source(*ei, map_) == *vi && target(*ei, map_) == p[*vi]) ||
						(source(*ei, map_) == p[*vi] && target(*ei, map_) == *vi)) {

					types.push_back(map_[*ei].type_);
					map_[*ei].type_ = std::numeric_limits<int>::max();
				}
			}
		}

#if 0 
		for (std::vector<int>::iterator iter = paths.begin();
				iter != paths.end(); ++iter)
			std::cerr << *iter << " ";
		std::cerr << "\n";
#endif

		means_vec.push_back(meanSquare(types));

		/*std::reverse(types.begin(), types.end());*/
		std::reverse(paths.begin(), paths.end());

		preds_vec.push_back(p);
		types_vec.push_back(types);
		paths_vec.push_back(paths);
	//}

	int index = findMeanMin(types_vec, means_vec);
	res = paths_vec[index];

	//graph_traits<Map>::vertex_iterator src_iter, dst_iter;
	//graph_traits<Map>::vertex_iterator vi, vend;
	for (tie(vi, vend) = vertices(map_); vi != vend; ++vi) {
		if (map_[*vi].id_ == path.first)
			src_iter = vi;
		if (map_[*vi].id_ == path.second)
			dst_iter = vi;
	}

	//graph_traits<Map>::edge_iterator ei, eend;
	std::vector<std::vector<int> >::iterator types_iter = types_vec.begin();
	for (std::vector<std::vector<vertex_descriptor> >::iterator pre_iter = preds_vec.begin();
			pre_iter != preds_vec.end(); ++pre_iter, ++types_iter) {

		std::vector<int>::iterator type_iter = types_iter->begin();
		for (vi = dst_iter; vi != src_iter; vi = (*pre_iter)[*vi], ++type_iter) {
			for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
				if ((source(*ei, map_) == *vi && target(*ei, map_) == (*pre_iter)[*vi]) ||
						(source(*ei, map_) == (*pre_iter)[*vi] && target(*ei, map_) == *vi)) {

					map_[*ei].type_ = *type_iter;
				}
			}
		}
	}

	return res;
}


std::vector<int>
MsvrMap::getPaths_new(double x1, double y1, double x2, double y2 , std::vector<int>& paths_old)
{
	std::vector<int> res;
	int roads_num = paths_old.size();


	int roadid1 = getRoadByNode(paths_old[0], paths_old[1]);
	int roadid2 = getRoadByNode(paths_old[roads_num-2], paths_old[roads_num-1]);

	graph_traits<Map>::edge_iterator ei, eend;
	for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
		if(map_[*ei].id_ == roadid1 || map_[*ei].id_ == roadid2 ){
			//map_[*ei].type_ = std::numeric_limits<int>::max()/3;
			map_[*ei].type_ *= 1000;
		}
	}

	res = getPaths(x1,y1,x2,y2);

	for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
		if(map_[*ei].id_ == roadid1 || map_[*ei].id_ == roadid2 ){
			//map_[*ei].type_ = std::numeric_limits<int>::max()/3;
			map_[*ei].type_ /= 1000;
		}
	}

	return res;
}



std::pair<int, int>
MsvrMap::getSrcAndDst(double x1, double y1, double x2, double y2)
{
	typedef graph_traits<Map>::vertex_descriptor vertex_descriptor;
	std::pair<int, int> path;
	Road srcRoad, dstRoad;
//	double x, y;
	//int temp = 0;//标志变量,先算第一个点的, 再算第二个点

	srcRoad = getRoadByPos(x1, y1);
	dstRoad = getRoadByPos(x2, y2);

	printf("srcRoad dstRoad %d %d\n",srcRoad.id_,dstRoad.id_);

	graph_traits<Map>::edge_iterator ei, eend;
	for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
		if( map_[*ei].id_== srcRoad.id_){
			vertex_descriptor srcVer = source(*ei,map_);
			vertex_descriptor dstVer = target(*ei,map_);
			if( map_[srcVer].x_ == x1 && map_[srcVer].y_ == y1){
				path.first = map_[srcVer].id_;
			}else if(map_[dstVer].x_ == x1 && map_[dstVer].y_ == y1){
				path.first = map_[dstVer].id_;
			}else if(msvr_cal_dist(map_[srcVer].x_,map_[srcVer].y_, x2, y2) <=
					msvr_cal_dist(map_[dstVer].x_,map_[dstVer].y_, x2, y2)){
				path.first = map_[srcVer].id_;
			}else {
				path.first = map_[dstVer].id_;
			}
			break;
		}
		// get src node id
		//------------------------------------------------------------------modified
		//		if ( map_[*ei].id_ == srcRoad.id_) {
		//			vertex_descriptor srcVer = source(*ei, map_);
		//			vertex_descriptor dstVer = target(*ei, map_);
		//			double srclen = msvr_cal_dist(map_[srcVer].x_,
		//					map_[srcVer].y_, x1, y1);
		//			double dstlen = msvr_cal_dist(map_[dstVer].x_,
		//					map_[dstVer].y_, x1, y1);
		//
		//			/*if (srclen > dstlen)*/
		//			if (srclen < dstlen) {
		//				path.first = map_[srcVer].id_;
		//				x = map_[srcVer].x_;
		//				y = map_[srcVer].y_;
		//			} else {
		//				path.first = map_[dstVer].id_;
		//				x = map_[dstVer].x_;
		//				y = map_[dstVer].y_;
		//			}
		//			//temp = 1;
		//			//tie(ei,eend)=edges(map_);
		//			break;
		//		}
		//------------------------------------------------------------------modified
	}
	printf("paths first %d\n",path.first);
	for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
		if( map_[*ei].id_== dstRoad.id_){
			vertex_descriptor srcVer = source(*ei,map_);
			vertex_descriptor dstVer = target(*ei,map_);
			if( map_[srcVer].x_ == x2 && map_[srcVer].y_ == y2){
				path.second = map_[srcVer].id_;
			}else if(map_[dstVer].x_ == x2 && map_[dstVer].y_ == y2){
				path.second = map_[dstVer].id_;
			}else if(msvr_cal_dist(map_[srcVer].x_,map_[srcVer].y_, x1, y1) <=
					msvr_cal_dist(map_[dstVer].x_,map_[dstVer].y_, x1, y1)){
				path.second = map_[srcVer].id_;
			}else {
				path.second = map_[dstVer].id_;
			}
			break;
		}
		//------------------------------------------------------------------modefied
		//		// get dst node id
		//		if ( map_[*ei].id_ == dstRoad.id_) {
		//			vertex_descriptor srcVer = source(*ei, map_);
		//			vertex_descriptor dstVer = target(*ei, map_);
		//			if(x > 2000 || x < 0 || y > 2000 || y < 0)
		//			{
		//				fprintf(stderr , "getSrcAndDst x: %lf--y:%lf\n",x,y);
		//				exit(0);
		//			}
		//			double srclen = msvr_cal_dist(map_[srcVer].x_, map_[srcVer].y_, x, y);
		//			double dstlen = msvr_cal_dist(map_[dstVer].x_, map_[dstVer].y_, x, y);
		//
		//			// check src and dst node is in the same roadid
		//			// if so, save src and dst id respectly
		//			// if no so, check distance to save node id
		//			if (path.first == map_[srcVer].id_)
		//				path.second = map_[dstVer].id_;
		//			else if (path.first == map_[dstVer].id_)
		//				path.second = map_[srcVer].id_;
		//			else if (srclen > dstlen)
		//				path.second = map_[dstVer].id_;
		//				//path.second = map_[srcVer].id_;
		//			else
		//				path.second = map_[srcVer].id_;
		//				//path.second = map_[dstVer].id_;
		//			break;
		//		}
		//------------------------------------------------------------------modefied
	}
	printf("paths second %d\n",path.second);
	return path;
}

bool MsvrMap::nodeInRoad(double srcx, double srcy, int roadid){
	graph_traits<Map>::edge_iterator ei, eend;
	for (tie(ei, eend) = edges(map_); ei != eend; ++ei) {
		if(map_[*ei].id_ == roadid){
			vertex_descriptor srcVer = source(*ei,map_);
			vertex_descriptor dstVer = target(*ei,map_);
			if( srcx <= std::max(map_[srcVer].x_,map_[dstVer].x_) + MSVR_ROAD_PAD &&
				srcx >= std::min(map_[srcVer].x_,map_[dstVer].x_) - MSVR_ROAD_PAD &&
				srcy <= std::max(map_[srcVer].y_,map_[dstVer].y_) + MSVR_ROAD_PAD &&
				srcy >= std::min(map_[srcVer].y_,map_[dstVer].y_) - MSVR_ROAD_PAD
			)
				return true;
			else
				return false;
			break;
		}
	}
}
