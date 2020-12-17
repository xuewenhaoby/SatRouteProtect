#ifndef __CMD_H__
#define __CMD_H__

#include "opspf.h"
#include "common.h"

void link_route_write(Tmp_Route_Table *argument);

void route_write(Tmp_Route_Table *route_table,bool Route_Link[SLINK_NUM]) ;

string uint2strIP(uint addr);

void IpStr(char *buf,int addr,int maskSize);

void IpStr(char *buf,int addr);

void BIpStr(char *buf,int addr,int maskSize);

#endif