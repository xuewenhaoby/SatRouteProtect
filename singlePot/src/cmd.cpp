#include "cmd.h"

#include <iostream>

void link_route_write(Tmp_Route_Table *route_table)
{
	int temp_link;
	int dst_id = route_table->dst_id;
	int src_portid = route_table->src_portid;
	NodeAddress gw_addr = route_table->gw_addr;
	string routeFile = "routeFile/routeTable.txt";
	ofstream fin(routeFile,ios::app);

	for(int p=0;p<SINF_NUM;p++)
	{

		if( Int_DB[dst_id-1].dst_satid[p]!=satelliteId )
		{
			temp_link=Int_DB[dst_id-1].linkId[p];

			// if(Int_DB[dst_id-1].port_stat[p]== true)
			// {
				NodeAddress temp_link_subnetIp =  G.isl[temp_link-1].subnetIp;
				string dstlink_netStr = uint2strIP(temp_link_subnetIp);
				string cmd_del = "ip netns exec sat"+to_string(satelliteId)+" ip route del " +dstlink_netStr+"/24";

				string gw_netStr = uint2strIP(gw_addr);
				string cmd = "ip netns exec sat"+to_string(satelliteId)+"  ip route add "+dstlink_netStr+ "/24 via "+ gw_netStr + " dev sat"+to_string(satelliteId)+"p"+to_string(src_portid);
				string COMMAND = cmd_del +"-"+ cmd;	
				fin<<cmd_del<<endl;
				fin<<cmd<<endl;
				
			//}
			
		}
	}
	fin.close();

}

void route_write(Tmp_Route_Table *route_table,bool Route_Link[SLINK_NUM]) 
{
	string routeFile = "routeFile/sat"+to_string(satelliteId)+".txt";
	ofstream fin(routeFile,ios::app);
	// char buf[100];
	// sprintf(buf,"%d %d %d %d %d",satelliteId,route_table->dst_id,route_table->src_portid,route_table->gw_addr,route_table->dst_addr);
	// fin<<buf<<endl;
	char name[5];
	sprintf(name,"sat");
	int src_id = satelliteId;
	int dst_id = route_table->dst_id;
	int src_portid = route_table->src_portid;
	NodeAddress gw_addr = route_table->gw_addr;
	NodeAddress dst_addr = route_table->dst_addr;
	int temp_link;	
	char dstlink_netStr[BUF_STR_SIZE],gw_netStr[BUF_STR_SIZE];
	IpStr(dstlink_netStr,dst_addr,24);
	IpStr(gw_netStr,gw_addr);
	char cmd[BUF_STR_SIZE],cmd_del[BUF_STR_SIZE];
	sprintf(cmd_del,"ip netns exec %s%d ip route del %s",name,src_id,dstlink_netStr);
	//system(cmd_del);

	sprintf(cmd,"ip netns exec %s%d ip route add %s via %s dev %s%dp%d",
		name,src_id,dstlink_netStr,gw_netStr,name,src_id,src_portid);
	//system(cmd);

	fin<<cmd_del<<endl;  
	fin<<cmd<<endl;
	for(int p=0;p<SINF_NUM;p++)
	{

		if( Int_DB[dst_id-1].dst_satid[p]!=satelliteId )
		{
			temp_link=Int_DB[dst_id-1].linkId[p];
			NodeAddress temp_link_subnetIp =  G.isl[temp_link-1].subnetIp;
			if(temp_link_subnetIp != 0 && Route_Link[temp_link] == false)
			{
				// sprintf(buf,"%d %d %d %d %d",satelliteId,route_table->dst_id,route_table->src_portid,route_table->gw_addr,temp_link_subnetIp);
				// fin<<buf<<endl;
				Route_Link[temp_link] = true;
				IpStr(dstlink_netStr,temp_link_subnetIp,24);				
				sprintf(cmd_del,"ip netns exec %s%d ip route del %s",name,src_id,dstlink_netStr);
				sprintf(cmd,"ip netns exec %s%d ip route add %s via %s dev %s%dp%d",
						name,src_id,dstlink_netStr,gw_netStr,name,src_id,src_portid);
				fin<<cmd_del<<endl;
				fin<<cmd<<endl;
			}

					
		}
	}

	fin.close();
}

string uint2strIP(uint addr)
{
	string res = "";
	uint x = 0xff << 24 ;
	for (int i = 3; i >=0 ; i--)
	{
		res = res + to_string((addr&x)>>(8*i)) + ".";
		x >>= 8;
	}
	res.pop_back();

	return res;
}

void IpStr(char *buf,int addr,int maskSize)
{
	int a1 = (addr & 0xff000000) >> 24;
	int a2 = (addr & 0x00ff0000) >> 16;
	int a3 = (addr & 0x0000ff00) >> 8;
	int a4 = (addr & 0x000000ff);
	sprintf(buf,"%d.%d.%d.%d/%d",a1,a2,a3,a4,maskSize);
}

void IpStr(char *buf,int addr)
{
	int a1 = (addr & 0xff000000) >> 24;
	int a2 = (addr & 0x00ff0000) >> 16;
	int a3 = (addr & 0x0000ff00) >> 8;
	int a4 = (addr & 0x000000ff);
	sprintf(buf,"%d.%d.%d.%d",a1,a2,a3,a4);
}


void BIpStr(char *buf,int addr,int maskSize)
{
	int tmp = 0;
	for(int i = 0; i < 32-maskSize;i++){
		tmp <<= 1;
		tmp |= 0x00000001;
	}
	addr |= tmp;
	int a1 = (addr & 0xff000000) >> 24;
	int a2 = (addr & 0x00ff0000) >> 16;
	int a3 = (addr & 0x0000ff00) >> 8;
	int a4 = (addr & 0x000000ff);
	sprintf(buf,"%d.%d.%d.%d",a1,a2,a3,a4);
}