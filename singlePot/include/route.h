#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "opspf.h"
#include "common.h"

int FindMin(unsigned int distance[],bool mark[]);

void Dijkstra(
    unsigned int sup_array[][SAT_NUM],
    int src_id,
    unsigned int distance[],
    int path[]);

int FindPreNode(
    int path[],
    int src_id,
    int dst_id);



void UpdateOpspfRoutingTable();

void backup_UpdateOpspfRoutingTable(int dstSatId, int max_distance);

int distanceTodstSat(int dstSatId);

void routeInitialize();

#endif