#ifndef ns_bundle_h
#define ns_bundle_h

//for debugging
#include <iostream>
#include<fstream>


#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"
#include "timer-handler.h"
#include "queue.h"
#include <priqueue.h>
#include "random.h"

#include <config.h>
#include <lib/bsd-list.h>
#include <map>
#include <vector>
#include <cmath>
#include <mobilenode.h>

typedef unsigned int u_int32_t;
typedef signed int int32_t;
typedef int32_t nsaddr_t;

#define TYPE_DATA   1
#define TYPE_ACK    2
#define TYPE_HELLO  3
#define TYPE_DISRU  4 //断路
#define TYPE_NAK    5 //重传请求

#define CURRENT_TIME    Scheduler::instance().clock()
#define MAX_BUFFER_TIME			10//【【【【【【
#define HELLO_INTERVAL          0.5               // 1000 ms ，在BundleAgent中有此变量
#define ALLOWED_HELLO_LOSS      3               // packets

#define NB_THRESHOLD	10
#define HELLO_TTL_S		1
#define HELLO_TTL_L		1

#define PI          3.1416
#define BLOCK 12
//#define V 0.02	//速度
//#define L_UP 300	//每条路长1010s
//#define L_RIGHT 300
#define DERATE 0.25
#define DERATE_T 0.1
#define CON_DERATE 0.985

#define DISTENCE_INFLU 0.2 //防止数据越发越远
#define SPEED_INFLU 0.2
#define NB_INFLU 0.2


struct hdr_bundle {
    int bundle_id; ///< 用来标识一个bundle ，for fragments
    int ack_bundle_id;
    bool is_fragment; //标识是否是分片
    struct {
        double  utiValue[BLOCK]; //综合效用值
        double  locX;
        double  locY;
        double  speed;
    }node_info;
    nsaddr_t src; //起点
    nsaddr_t dst; //终点
    nsaddr_t prev; //上一跳DTN节点

  double originating_timestamp;
  double initial_timestamp;
  double lifetime;
  int type;

  int custody_transfer;
  int return_receipt;

  //第几段
  int fragment;
  //总分片数
  int nfragments;
  int bundle_size;
//  int neighbor_table[100];
//  double dp_table[100];

  // Header access methods
  static int offset_; // required by PacketHeaderManager
  inline static int& offset() { return offset_; }
  inline static hdr_bundle* access(const Packet* p) {
    return (hdr_bundle*) p->access(offset_);
  }
};

class BundleAgent;

class Utival_cal_Timer : public TimerHandler {
 public:
	Utival_cal_Timer(BundleAgent* a): TimerHandler() {
    a_=a;
  }
 protected:
  virtual void expire(Event *e);
  BundleAgent* a_;
};

class MyHelloTimer : public TimerHandler {
 public:
  MyHelloTimer(BundleAgent* a): TimerHandler() {
    a_=a;
  }
 protected:
  virtual void expire(Event *e);
  BundleAgent* a_;
};

class BundleTimer : public TimerHandler {
 public:
  BundleTimer(BundleAgent* a): TimerHandler() {
    a_=a;
  }
 protected:
  virtual void expire(Event *e);
  BundleAgent* a_;
};



class Util_nodeInfo
{
public:
	Util_nodeInfo(){
		for(int i=0; i<BLOCK; i++){
			m_utilValue[i] = 0.0;
			m_synValue[i] = 0.0;
		}

		f1.open("out.txt");
//		Node *thisnode; //
//		thisnode = Node::get_node_by_address(index);//index);
//		speed = ((MobileNode *)thisnode)->speed();
//		((MobileNode *)thisnode)->getVelo(&locX, &locY, &locZ);
	}
	void uvalue_block_cal();
	void synCal(int ori_orient);
	//void increaseCal(Util_nodeInfo &matrix,double increase, int orient);
	void decayCal(double increase, int orient);
	void decayCal2();
	int orient_block_cal(double locX, double locY);

	double getUtilValue(int num){
		return m_utilValue[num];
	}
	void setUtilValue(int num, double dValue){
		m_utilValue[num] = dValue;
	}
	double getSynValue(int num){
		return m_synValue[num];
	}
	void setSynValue(int num, double dValue){
		m_synValue[num] = dValue;
	}
	double getLocX(){ return locX;}
	void setLocX(double tlocX){ locX = tlocX;}
	double getLocY(){ return locY;}
	void setLocY(double tlocY){ locY = tlocY;}

    double getOrientX(){ return orientX;}
	void setOrientX(double tX){ orientX = tX;}
    double getOrientY(){ return orientY;}
	void setOrientY(double tY){ orientY = tY;}
    double getOrientZ(){ return orientZ;}
	void setOrientZ(double tZ){ orientZ = tZ;}

	double getSpeed(){ return speed;}
	void setSpeed(double tspeed){ speed = tspeed;}
public:
    double  orientX;
    double  orientY;
    double  orientZ;
    double  locX;
    double  locY;
    double  locZ;
    double  speed; //节点速度
    nsaddr_t index; //this node
	double m_utilValue[BLOCK];
	double m_synValue[BLOCK];


	ofstream f1;

};

/*
   DTN Neighbor Cache Entry
*/
class DTN_Neighbor {
        friend class BundleAgent;
        friend class dtn_rt_entry;
 public:
        DTN_Neighbor(nsaddr_t a) { nb_addr = a; }

 protected:
        LIST_ENTRY(DTN_Neighbor) nb_link;
        nsaddr_t        nb_addr;
        double          locX;
        double          locY;
		int				orient;
        double          syn_utilValue[BLOCK];
        double          nb_expire;      // ALLOWED_HELLO_LOSS * HELLO_INTERVAL
};

LIST_HEAD(dtn_ncache, DTN_Neighbor);

/*
 * 重写packetQueue，提供一个方便搜索的接口
 */
class BundlePacketQueue : public PacketQueue {
public:
	int delete_by_SrcDst(nsaddr_t src, nsaddr_t dst) {
		int delete_count = 0;
		for (Packet* p = head_; p != 0; ) {
			hdr_bundle *bh = hdr_bundle::access(p);
			if(bh->src == src && bh->dst == dst){
				Packet* temp = p;
				p = p->next_;
				this->remove(temp);
				delete_count ++;
			}else
				p = p->next_;
		}
		return (delete_count);
	}

	int delete_by_id(int bundle_id) {
		int delete_count = 0;
		for (Packet* p = head_; p != 0; ) {
			hdr_bundle *bh = hdr_bundle::access(p);
			if(bh->bundle_id == bundle_id){
				Packet* temp = p;
				p = p->next_;
				this->remove(temp);
				delete_count ++;
			}else
				p = p->next_;
		}
		return (delete_count);
	}

	Packet* lookup_by_id(int bundle_id) {
		for (Packet* p = head_; p != 0; p = p->next_) {
			hdr_bundle *bh = hdr_bundle::access(p);
			if (bh->bundle_id == bundle_id)
				return (p);
		}
		return (0);
	}
};

class BundleAgent : public Agent {
 public:
  BundleAgent();
  virtual int command(int argc, const char*const* argv);
  virtual void recv(Packet*, Handler*);

  void recvData(Packet *p);
  void recvACK(Packet *p);
  void recvHello(Packet *p);
  void recvDisruption(Packet *p);
  void recvNAK(Packet *p);

  void sendHello();
  void sendCustAck(int Bundle_id, int dst);
  void sendRetReceipt(Packet *);
  Packet* copyBundle(Packet*);
  void sendBundle(Packet*, int, int, int);
  void checkStorage();
  void removeExpiredBundles();
  void dropOldest();
  void dropMostSpread();
  void dropLeastSpread();
  void dropRandom();

  int deleteForwarded;
  int helloInterval;

  int sentBundles;

	int receivedBundles;

  int bundleStorageSize;


    int duplicateFrag; //重复收到的分片计数

public:



    Packet* process_for_reassembly(nsaddr_t bundle_id);
	bool check_bundle(Packet *p); //不是过路数据返回true

    //std::map<int, std::map<int,Packet>>* fragMap() const { return fragMap_; }
    //void set_duplicateFrag(int l)  { duplicateFrag_ = l; }
	void nb_insert(Packet * p);
	DTN_Neighbor* nb_lookup(nsaddr_t id);
	void nb_delete(nsaddr_t id);
	void nb_purge();
	int nb_sum();
	void forward(Packet *p);
	void sendNAK(nsaddr_t prev, nsaddr_t bundle_id);
	void rt_update(int locX, int locY);
	void delete_bundleInStorage(BundlePacketQueue* storage, int eid);
public:
    struct packet_info{
        double recv_time;
        Packet *p; //
    }packet_info_n;
    std::map<int, std::map<int,packet_info> > fragMap; // 利用bundle id 检索fragment表；int代表第几个碎片
    std::vector<int> recivedBundle;//int::bundle_id
    BundlePacketQueue* myBundleBuffer;//bundle发送队列
	BundlePacketQueue* myBundleStorage;//bundle缓存
    BundlePacketQueue* midBundleBuffer;//中间bundle缓存
    BundlePacketQueue* controlPktBuffer;

	Util_nodeInfo m_uti_nodeInfo;

	dtn_ncache nbhead;                 // Neighbor Cache

    int bundleStore_size;
    int midBuffer_size;

    Utival_cal_Timer utival_cal_Timer_;



  MyHelloTimer helloTimer_;
  BundleTimer bundleTimer_;

  PriQueue *ifqueue; //ifqueue:priority queue ,Interface Queue
  int dropsRR;
  int repsRR;
};

#endif // ns_bundle_h
