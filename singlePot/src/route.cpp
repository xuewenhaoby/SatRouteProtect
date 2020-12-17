#include "opspf.h"
#include "common.h"
#include "route.h"
#include "timer.h"
#include "topo.h"
#include "thread_pool.h"
#include "cmd.h"


#include <iostream>


int distanceTodstSat(int dstSatId)
{
	int temp_link;
	int path[SAT_NUM];
	unsigned int distance[SAT_NUM];
	for(int i=0; i<SAT_NUM; i++)
    {
    	for(int j=0;j<SAT_NUM;j++)
    	{
    		temp_link = G.arcs[i][j].linkId;
    		int k =0;
    		if(temp_link != -1 && temp_link!=0) //exists link between satellite nodes
    		{
    			if(G.isl[temp_link-1].endpoint[0].nodeId==i+1)
    			{
    				k=G.isl[temp_link-1].endpoint[0].inf;
    			}
    			else
    			{
					k=G.isl[temp_link-1].endpoint[1].inf;
				}
    			if(Int_DB[i].port_stat[k]== true)
    				sup_array[i][j]=G.arcs[i][j].weight;
    			else
    				sup_array[i][j]=MAX_ARC_WEIGHT;

    		}
    		else 
    			sup_array[i][j]=MAX_ARC_WEIGHT;
    	}
     }
     Dijkstra(sup_array,satelliteId,distance,path);
     return distance[dstSatId-1];
}

void UpdateOpspfRoutingTable()
{
	int path[SAT_NUM],next_hop[SAT_NUM];
	unsigned int distance[SAT_NUM];
	int src_portid,dst_id,linkId;
	//int dst_linkid,gw_id;
	int temp_link;
    bool Route_Link[SLINK_NUM];
    NodeAddress gw_addr,dst_addr;
    if(GetTime()<20)return;
    for(int i=0; i < SLINK_NUM; i++)
    {
    	Route_Link[i] = false;
    }
    for(int i=0; i<SAT_NUM; i++)
    {
    	for(int j=0;j<SAT_NUM;j++)
    	{
    		temp_link = G.arcs[i][j].linkId;
    		
    		int k =0;
    		if(temp_link != -1 && temp_link!=0) //exists link between satellite nodes
    		{
    			if(G.isl[temp_link-1].endpoint[0].nodeId==i+1)
    			{
    				k=G.isl[temp_link-1].endpoint[0].inf;
    			}
    			else
    			{
					k=G.isl[temp_link-1].endpoint[1].inf;
				}
    			if(Int_DB[i].port_stat[k]== true)
    				sup_array[i][j]=G.arcs[i][j].weight;

    			else
    				sup_array[i][j]=MAX_ARC_WEIGHT;

    		}
    		else 
    			sup_array[i][j]=MAX_ARC_WEIGHT;
    	}
     }
    Dijkstra(sup_array,satelliteId,distance,path);
    //cout<<"dijkstra"<<endl;

    for (int i = 0; i < SAT_NUM; ++i)
    {
    	next_hop[i]=FindPreNode(path,satelliteId,i+1);
    	if(i != satelliteId-1 && next_hop[i] != -1)
    	{

    		//gw_id = next_hop[i]+1;
    		dst_id = i+1;
    		dst_addr = dst_id*256*256*256;
    		linkId=G.arcs[satelliteId-1][next_hop[i]].linkId;
            //cout<<"dstid "<< i+1<<"next_hop"<<next_hop[i]+1<<endl;
    		if(linkId != -1)
    		{
    			if(G.isl[linkId-1].endpoint[0].nodeId==satelliteId)
                {
                     src_portid=G.isl[linkId-1].endpoint[0].inf;
                     //gw_portid=topo.isl[linkid-1].endpoint[1].inf;
                     gw_addr = G.isl[linkId-1].endpoint[1].ip;
                }
                else
                {
                     src_portid=G.isl[linkId-1].endpoint[1].inf;
                     //gw_portid=topo.isl[linkid-1].endpoint[0].inf;
                     gw_addr = G.isl[linkId-1].endpoint[0].ip;
                }

                if(route_table[i].src_portid!=src_portid )
                {
                    route_table[i].src_id = satelliteId;
                    route_table[i].gw_addr=gw_addr;
                    route_table[i].dst_addr=dst_addr;
                    route_table[i].src_portid = src_portid;
                    route_table[i].linkid = linkId;
                    route_table[i].dst_id = dst_id;
                    if(normalTopoChange == false)
                    {
                          route_write(&route_table[i],Route_Link);
                    }
                  
                }
    		}
    	}
    }
    normalTopoChange = false;
}

void backup_UpdateOpspfRoutingTable(int dstSatId, int max_distance)
{
    //  int dst_linkid;
    if(GetTime()<20)return;
	int path[SAT_NUM],next_hop[SAT_NUM];
	unsigned int distance[SAT_NUM];
	int src_portid,dst_id,linkId;
	int temp_link;
    //int gw_id;
	bool Route_Link[SLINK_NUM];
    NodeAddress gw_addr,dst_addr;
    for (int i = 0; i < SLINK_NUM; ++i)
    {
        Route_Link[i] = false;
    }
    for(int i=0; i<SAT_NUM; i++)
    {
    	for(int j=0;j<SAT_NUM;j++)
    	{
    		temp_link = G.arcs[i][j].linkId;
    		int k =0;
    		if(temp_link != -1 && temp_link!=0) //exists link between satellite nodes
    		{
    			if(G.isl[temp_link-1].endpoint[0].nodeId==i+1)
    			{
    				k=G.isl[temp_link-1].endpoint[0].inf;
    			}
    			else
    			{
					k=G.isl[temp_link-1].endpoint[1].inf;
				}
    			if(Int_DB[i].port_stat[k]== true)
    				sup_array[i][j]=G.arcs[i][j].weight;
    			else
    				sup_array[i][j]=MAX_ARC_WEIGHT;

    		}
    		else 
    			sup_array[i][j]=MAX_ARC_WEIGHT;
    	}
     }
    Dijkstra(sup_array,satelliteId,distance,path);
    cout<<"dijkstra"<<endl;
    for (int i = 0; i < SAT_NUM; ++i)
    {
    	next_hop[i]=FindPreNode(path,satelliteId,i+1);
    	if(i != satelliteId-1 && next_hop[i] != -1)
    	{

    		//gw_id = next_hop[i]+1;
            dst_id = i+1;
            dst_addr = dst_id*256*256*256;
            linkId=G.arcs[satelliteId-1][next_hop[i]].linkId;

    		if(linkId != -1)
    		{
    			if(G.isl[linkId-1].endpoint[0].nodeId==satelliteId)
                {
                     src_portid=G.isl[linkId-1].endpoint[0].inf;
                     //gw_portid=topo.isl[linkid-1].endpoint[1].inf;
                     gw_addr = G.isl[linkId-1].endpoint[1].ip;
                }
                else
                {
                     src_portid=G.isl[linkId-1].endpoint[1].inf;
                     gw_addr = G.isl[linkId-1].endpoint[0].ip;
                }
                if(route_table[i].src_portid!=src_portid)
                {
                    route_table[i].src_id = satelliteId;
                    route_table[i].gw_addr=gw_addr;
                    route_table[i].dst_addr=dst_addr;
                    route_table[i].src_portid = src_portid;
                    route_table[i].linkid = linkId;
                    route_table[i].dst_id = dst_id;
                    route_write(&route_table[i],Route_Link);
                }
                // route_table[i].src_id = satelliteId;
                // route_table[i].gw_addr=gw_addr;
                // route_table[i].dst_addr=dst_addr;
                // route_table[i].src_portid = src_portid;
                // route_table[i].linkid = linkId;
                // route_table[i].dst_id = dst_id;
                // route_write(&route_table[i],Route_Link);
    		}
    	}
    }
    int next_send_node_id = next_hop[dstSatId-1]+1;
    send_LSU_to_nextNode(next_send_node_id);    
}

void send_LSU_to_nextNode(int next_send_node_id)
{
	int next_send_link_id = G.arcs[satelliteId-1][next_send_node_id-1].linkId;
	int next_port_id = -1;

	if(next_send_link_id != -1 && next_send_link_id !=0) //exists link between satellite nodes
	{
		if(G.isl[next_send_link_id-1].endpoint[0].nodeId==satelliteId)
		{
			next_port_id=G.isl[next_send_link_id-1].endpoint[0].inf;
		}
		else
		{
			next_port_id=G.isl[next_send_link_id-1].endpoint[1].inf;
		}
	}
	if(selfInf[next_port_id].stat ==true)
	{
		lsuData.timeStamp = GetTime();
		OpspfSendProtocolPacket(selfInf[next_port_id],OPSPF_LSU_PACKET);
		//cout<<"send lsuV2 to sat"+to_string(dstSatId)+"through"+to_string(next_send_node_id)+"from" +to_string(next_port_id)<<endl;
			
	}
	for (int i = 0; i < SINF_NUM; ++i)
	{
		selfInf[i].changeStat = OPSPF_PORT_NOCHANGE;
	}
}

int FindMin(
    unsigned int distance[],
    bool mark[]
)
{
    int n=0;
    unsigned int min=MAX_ARC_WEIGHT;
    for(int k=0;k<SAT_NUM;k++)
    {
        if(!mark[k] && distance[k]<min)
        {
            min = distance[k];
            n = k;
        }
    }
    if(min == MAX_ARC_WEIGHT)
        return -1;

    return n;
}

void Dijkstra(
    unsigned int sup_array[][SAT_NUM],
    int src_id,
    unsigned int distance[],
    int path[])
{
    int temp =src_id-1;
    bool mark[SAT_NUM];
    //initialize mark , add it to the set path[]
    for(int i=0;i < SAT_NUM;i++)
    {
        mark[i]=false;
        distance[i]=sup_array[temp][i];
        if(distance[i]!=MAX_ARC_WEIGHT)
            path[i]=temp;
        else
            path[i]=-1;
    }
    //mark the choosed point temp and update its distance
    mark[temp]=true;
    distance[temp]=0;

    for(int i=0;i < SAT_NUM;i++)
    {
        //find the closed point and mark it
        if((temp = FindMin(distance, mark)) == -1)
			return;
        mark[temp]=true;

        for(int j=0;j<SAT_NUM;j++)
        {
            //not marked and temp is the closed point, addit to the set and update the shortest path
            if(!mark[j] && (sup_array[temp][j]+distance[temp]< distance[j]))
            {
                distance[j]= sup_array[temp][j]+distance[temp];
                path[j]=temp;
               	//cout<<"temp:"+to_string(temp+1) +" distance"+to_string(j+1)+" :"+to_string(distance[j])+" path:"+to_string(path[j]+1)<<endl;
            }
            
        }
    }
}

int FindPreNode(
    int path[],
    int src_id,
    int dst_id)
{
    int temp =dst_id-1;
    if(path[temp]==src_id-1)
       return temp;
    else if(path[temp] == -1)
       return -1;
    else 
       return FindPreNode(path,src_id,path[temp]+1);
}

void routeInitialize()
{
    for (int i = 0; i < SAT_NUM; ++i)
    {
            route_table[i].src_id = -1;
            route_table[i].gw_addr= -1;
            route_table[i].src_portid = -1;
            route_table[i].linkid = -1;
            route_table[i].dst_id = -1;    
    }
}
