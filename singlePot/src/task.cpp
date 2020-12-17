#include <typeinfo>
#include <unistd.h>

#include "message.h"
#include "timer.h"
#include "topo.h"
#include "opspf.h"
#include "cmd.h"
#include "route.h"
#include "common.h"
#include "task.h"



void *func1(void *arg)
{
	return 0;
}

void *HandleNodeEvent(void *arg)
{
	Message *msg = (Message *)arg;
	//Node *node = (Node *)msg->getNode();
	
	switch(msg->getEventType())
	{
		case MSG_NODE_SendHelloPacket:
		{
			Send_Hello_Packet();
			break;
		}
		case MSG_NODE_SendLsuPacket:
		{
			Send_Lsu_Packet();
			break;
		}
		case MSG_NODE_FloodLsuPacket:
		{
			Flood_Lsu_Packet(recv_send_LSU_index);
			break;
		}
		case MSG_ROUTE_Update:
		{
			UpdateOpspfRoutingTable();
			break;
		}
		case MSG_NODE_UpdateLink:
		{

			updateLink();
			break;
		}
		// case MSG_NODE_Test:
		// {
		// 	if(typeid(*node) == typeid(SatNode)){
		// 		cout << "SatNode!" << endl; 
		// 	}
		// 	else if(typeid(*node) == typeid(HostNode)){
		// 		cout << "HostNode!" << endl; 
		// 	}
		// 	break;
		// }
		default:{
			break;
		}
	}
	MESSAGE_FREE(msg);

	return 0;
}

void *HandleTimerEvent(void *arg)
{
	Message *msg = (Message *)arg;

	switch(msg->getEventType())
	{
		case MSG_TIME_Timer:
		{
		pool.addTask(new NodeTask(new Message(MSG_NODE_UpdateLink)));

		pool.addTask(new NodeTask(new Message(
			 		MSG_NODE_SendHelloPacket)));
			break;
		}
		default:{
			break;
		}
	}
	MESSAGE_FREE(msg);

	return 0;
}

// void *thread_route(void *argument_route)
// {
//     struct  Tmp_Route_Table *argument=(struct Tmp_Route_Table *)argument_route;
//     route_write(argument);
//     //struct  arg_route *argument=(struct arg_route *)argument_route;
//     //route_write("sat",argument->src_id,argument->src_portid,argument->dst_addr,argument->gw_addr);
//     return 0;
// }

// void *thread_link_route(void *argument_route)
// {
//     struct  Tmp_Route_Table *argument=(struct Tmp_Route_Table *)argument_route;
//     link_route_write(argument);
//     return 0;
// }