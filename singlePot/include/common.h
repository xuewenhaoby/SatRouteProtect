#ifndef __COMMON_H__
#define __COMMOM_H__

#include "thread_pool.h"
#include "opspf.h"
extern ThreadPool pool; 
extern ThreadPool route_pool;
extern ThreadPool link_route_pool;
extern StaticTopo G;
extern OpspfInfData selfInf[6];
extern ISDB Int_DB[SAT_NUM];
extern OpspfLsuInfo lsuData;
extern Tmp_Route_Table route_table[SAT_NUM];
extern unsigned int sup_array[SAT_NUM][SAT_NUM];


extern NodePos pos;
extern bool recv_send_LSU;
extern int recv_send_LSU_index;
extern int key_value;
extern unsigned char key_encrypt_value[32];
extern int attackTime;

extern bool attackChangeRouteTable;
extern bool normalTopoChange;

extern int time_diff;
extern int satelliteId;
extern bool backupRoute;
extern bool hashHandle;

#endif