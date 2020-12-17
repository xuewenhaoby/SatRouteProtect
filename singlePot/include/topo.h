#ifndef __TOPO_H__
#define __TOPO_H__

#include <cmath>

#include "opspf.h"
#include "common.h"
#include "timer.h"


void InitStaticTopo();

void ReadIslFile(string file_name);

NodePos ReadLocFile(int id,int time);

bool getInfStat(int satId,int interfaceIndex);

int getOrbitId(int id);

int getOrbitIndex(int id);

int getForeSatelliteId(int id);

int getRearSatelliteId(int id);

int getSideSatelliteId(int id,bool isEast,bool isNorth);

void updateLink();

void setPos(NodePos pos);

void updatePos();

#endif
