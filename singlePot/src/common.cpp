#include "common.h"
#include <string.h>

ThreadPool pool;
ThreadPool route_pool;
ThreadPool link_route_pool; 
StaticTopo G;
OpspfInfData selfInf[6];
ISDB Int_DB[SAT_NUM];
OpspfLsuInfo lsuData;

Tmp_Route_Table route_table[SAT_NUM];


unsigned int sup_array[SAT_NUM][SAT_NUM];
NodePos pos;

bool recv_send_LSU =false;
int recv_send_LSU_index =-1;
int key_value = 201 ;
unsigned char key_encrypt_value[32] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',
'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

int attackTime =0 ;


bool attackChangeRouteTable = false;
bool normalTopoChange = false;

int satelliteId;
int time_diff;
bool hashHandle;
bool backupRoute;
