#include "common.h"
#include "message.h"
#include "timer.h"
#include "task.h"
#include "cmd.h"

void *func1(void *arg)
{
	return 0;
}

void *HandleNodeEvent(void *arg)
{
	Message *msg = (Message *)arg;
	Node *node = (Node *)msg->getNode();
	
	switch(msg->getEventType())
	{
		case MSG_NODE_Initialize:
		{
			node->nodeInitialize();
			break;
		}
		case MSG_NODE_Finalize:
		{
			node->nodeFinalize();
			break;
		}
		case MSG_LINK_Initialize:
		{
			node->linkInitialize();
			break;
		}
		case MSG_ROUTE_Initialize:
		{
			node->routeInitialize();
		}
		case MSG_LINK_Finalize:
		{
			node->linkFinalize();			
			break;
		}
		case MSG_NODE_UpdateLink:
		{
			node->updateLink();
			break;
		}
		case MSG_NODE_Test:
		{
			if(typeid(*node) == typeid(SatNode)){
				cout << "SatNode!" << endl; 
			}
			else if(typeid(*node) == typeid(HostNode)){
				cout << "HostNode!" << endl; 
			}
			break;
		}
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
			// Update-sats-pos should be done first.
			for(int i = 0; i < SAT_NUM; i++){
				sats[i].updatePos();
			}
			// Other tasks should be assigned after update-sats-pos.
			for(int i = 0; i < SAT_NUM; i++){
				pool.addTask(new NodeTask(new Message(
					&sats[i],MSG_NODE_UpdateLink)));
			}
				

			break;
		}  
		default:{
			break;
		}
	}
	MESSAGE_FREE(msg);  

	return 0;
}

void *thread_route(void *argument_route)
{
    struct  arg_route *argument=(struct arg_route *)argument_route;
    route_write("sat",argument->src_id,argument->src_portid,argument->dst_addr,argument->gw_addr);
    
    return 0;
}

void *thread_link_route(void *argument_route)
{
    struct  arg_route *argument=(struct arg_route *)argument_route;
    link_route_write("sat",argument->src_id,argument->src_portid,argument->dst_id,Int_DB,argument->gw_addr);
    return 0;
}