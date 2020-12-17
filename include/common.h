#ifndef __COMMON_H__
#define __COMMOM_H__

#include "node.h"
#include "thread.h"

#define ENABLE_PHYSICAL true
#define BUF_STR_SIZE 100
#define DEFAULT_TIMERSTATUS true

extern SatNode sats[SAT_NUM];
extern HostNode hosts[HOST_NUM];
extern StaticTopo G;
extern ThreadPool pool;
extern ThreadPool route_pool;
extern ThreadPool link_route_pool;
extern ISDB Int_DB[SAT_NUM];

extern bool updateRoute;
extern int updateRoute_time;
// extern string sys_stat;
//extern bool timerStatus;


#endif