#include "common.h"

SatNode sats[SAT_NUM];
HostNode hosts[HOST_NUM];
StaticTopo G;
ThreadPool pool;
ThreadPool route_pool;
ThreadPool link_route_pool;
ISDB Int_DB[SAT_NUM];

bool timerStatus = DEFAULT_TIMERSTATUS;  
int updateRoute_time  = 0;