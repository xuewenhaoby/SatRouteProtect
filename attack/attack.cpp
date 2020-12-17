#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

//#include "topo.h"
#include "attack.h"
#include "shell.h"

using namespace std;

int satelliteId,satelliteId_opposite,attackMode;
string br,attackNode,attackAction;

//set the satellite 2 as an attack node
int main(int argc,char* argv[])
{

	if(argc != 2)
	{
		cout<<"Enter parameter again"<<endl;
		return 0;
		
	}
	bool OVS_established = atoi(argv[1]);
	// attackNum = atoi(argv[2]);
	// attackMode = atoi(argv[3]);
	// satelliteId_opposite = satelliteId-1;
	if(!OVS_established)
	{
		for (int i = 1; i <=4; ++i)
		{
			attackLinkChange(i);
			changeOVS_flow_v1(br);
		}
		return 0;
	}
	// else
	// {
	// 	// for (int i = 1; i < 4; ++i)
	// 	// {
	// 	// 	del_allFlowTable(i);
	// 	// 	changeOVS_flow_v1(br);
	// 	// }
	// }


	string input;	
	cout<<"Please input the satellite number you want to attack"<<endl;
	cout<<"You could emulate the attack scene on these satellites: 2 / 4 / 6 / 7"<<endl;
	while(1)
	{
		cin>>input;
		satelliteId = atoi(input.c_str());
		if(satelliteId!=2 && satelliteId!=4 && satelliteId!=6 && satelliteId!=7)
		{
			cout<<"Wrong satellite number,try again"<<endl;
		}
		else
			break;

	}

	cout<<"Please input the attack mode number you want to emulate:"<<endl;
	cout<<"SEND_FAKE_HELLO_PACKET --- 1"<<endl;
    cout<<"SEND_FAKE_LSU_PACKET   --- 2"<<endl;
    cout<<"DROP_HELLO_PACKET      --- 3"<<endl;
    cout<<"MODIFY_LSU_PACKET      --- 4"<<endl;
    cout<<"DELAY_LSU_PACKET       --- 5"<<endl;

    while(1)
    {
       cin>>input;
	   attackMode = atoi(input.c_str());

	    if(attackMode<=0 || attackMode>5)
	    {
	    	cout<<"Wrong number,try again"<<endl;
	    }
	    else
	    	break;
    }
    del_allFlowTable(satelliteId);
	switch(attackMode)
	{
		
		br = cmd(SAT_BR_NAME, satelliteId);

		case SEND_FAKE_HELLO_PACKET:
		{
			changeOVS_flow_v1(br);
			break;
		}
		case SEND_FAKE_LSU_PACKET :
		{
			changeOVS_flow_v1(br);
			break;
		}
		case DROP_HELLO_PACKET:
		{
			string satnode = cmd(SAT_NAME,satelliteId);
			string satport = cmd(SAT_NAME,satelliteId,"p0");
			string dropcmd= "ip netns exec "+satnode+" ip link set "+satport+" down";
			system(dropcmd.c_str());
			//changeOVS_flow_v3(br);
			break;
		}
		case MODIFY_LSU_PACKET:
		{
			changeOVS_flow_v2(br);
			break;
		}
		case DELAY_LSU_PACKET:
		{
			changeOVS_flow_v2(br);
			break;
		}
		// case MODIFY_LSACK_PACKET :
		// {
		// 	changeOVS_flow_v2(br);
		// 	break;
		// }
		// case SEND_FAKE_LSACK_PACKET :
		// {
		// 	changeOVS_flow_v3(br);
		// 	break;
		// }
		default:
		{
			break;
		}
			
	}

	attackNode = cmd(ATTACK_NAME,satelliteId);
	attackAction = cmdBlank(satelliteId,attackMode);
	string cmd = "bash invoke_socket.sh " + attackAction;
	run_q(cmd);
	return 0;
}

void attackLinkChange(int attackNum)
{
	string satellite_addr,satellite_opposite_addr,satellite_broad_addr,attack_addr;
	string satellite_port,satellite_opposite_port;
	switch(attackNum)
	{
		case 1:
		{
			satelliteId = 2;
			satelliteId_opposite = 1;
			satellite_addr  = "190.0.1.2/24";
			satellite_opposite_addr = "190.0.1.3/24";
			attack_addr = "190.0.1.100/24";
			satellite_broad_addr = "190.0.1.255";
			satellite_port = "p0";
			satellite_opposite_port = "p0";
			break;
		}
		case 2:
		{
			satelliteId = 4;
			satelliteId_opposite = 3;
			satellite_addr  = "190.0.8.2/24";
			satellite_opposite_addr = "190.0.8.1/24";
			attack_addr = "190.0.8.100/24";
			satellite_broad_addr = "190.0.8.255";
			satellite_port = "p0";
			satellite_opposite_port = "p1";
			break;
		}
		case 3:
		{
			satelliteId = 6;
			satelliteId_opposite = 5;
			satellite_addr  = "190.0.14.2/24";
			satellite_opposite_addr = "190.0.14.1/24";
			attack_addr = "190.0.14.100/24";
			satellite_broad_addr = "190.0.14.255";
			satellite_port = "p0";
			satellite_opposite_port = "p1";
			break;
		}
		case 4:
		{
			satelliteId = 7;
			satelliteId_opposite = 6;
			satellite_addr  = "190.0.17.2/24";
			satellite_opposite_addr = "190.0.17.1/24";
			attack_addr = "190.0.17.100/24";
			satellite_broad_addr = "190.0.17.255";
			satellite_port = "p0";
			satellite_opposite_port = "p1";
			break;

		}
	}

	string ns = cmd(SAT_NAME,satelliteId);
	string ns_opposite = cmd(SAT_NAME,satelliteId_opposite);
	br = cmd(SAT_BR_NAME, satelliteId);
	run_q(ns_do(ns,veth_del(cmd(SAT_NAME,satelliteId,satellite_port))));
	run_q(ns_do(ns_opposite,veth_del(cmd(SAT_NAME,satelliteId_opposite,satellite_opposite_port))));
	//add attack netns 
	string ns_attack = cmd(ATTACK_NAME,satelliteId);
	run_q(ns_add(ns_attack));
	run_q(ns_do(ns_attack,dev_set_stat("lo",true)));
	//add OVS

	run(ovs_add_br(br));
	run(ovs_set_mode(br));

	string opposite_tap1 = ns_opposite + satellite_opposite_port, opposite_tap2 = br+"p1";
	run_q(link_add(opposite_tap1,opposite_tap2));
	run_q(ns_add_port(ns_opposite,opposite_tap1));
	run_q(ns_do(ns_opposite,dev_set_stat(opposite_tap1,true)));
	run(dev_set_stat(opposite_tap2,true));
	run_q(ns_do(ns_opposite,dev_addr_add(opposite_tap1,satellite_opposite_addr,satellite_broad_addr)));

	string ns_tap1 = ns + satellite_port, ns_tap2 = br+"p2";
	run_q(link_add(ns_tap1,ns_tap2));
	run_q(ns_add_port(ns,ns_tap1));
	run_q(ns_do(ns,dev_set_stat(ns_tap1,true)));
	run(dev_set_stat(ns_tap2,true));
	run_q(ns_do(ns,dev_addr_add(ns_tap1,satellite_addr,satellite_broad_addr)));

	string attack_tap1 = ns_attack + "p100", attack_tap2 = br+"p3";
	run_q(link_add(attack_tap1,attack_tap2));
	run_q(ns_add_port(ns_attack,attack_tap1));
	run_q(ns_do(ns_attack,dev_set_stat(attack_tap1,true)));
	run(dev_set_stat(attack_tap2,true));
	run_q(ns_do(ns_attack,dev_addr_add(attack_tap1,attack_addr,satellite_broad_addr)));
	
	string port1 = cmd(br,"p1");
	run(ovs_add_port(br,port1,1));
	
	string port2 = cmd(br,"p2");
	run(ovs_add_port(br,port2,2));

	string port3 = cmd(br,"p3");
	run(ovs_add_port(br,port3,3));
}

void changeOVS_flow_v1(string br)
{
	run(ovs_add_flow(br,"in_port=2,priority=10,actions=output:1"));
	run(ovs_add_flow(br,"in_port=1,priority=10,actions=output:2"));
	run(ovs_add_flow(br,"in_port=3,priority=10,actions=output:1,2"));
}

void changeOVS_flow_v2(string br)
{
	run(ovs_add_flow(br,"in_port=2,priority=10,actions=output:3,1"));
	run(ovs_add_flow(br,"in_port=1,priority=10,actions=output:2"));
	run(ovs_add_flow(br,"in_port=3,priority=10,actions=output:1"));
}

void changeOVS_flow_v3(string br)
{
	run(ovs_add_flow(br,"in_port=2,priority=10,actions=output:3"));
	run(ovs_add_flow(br,"in_port=1,priority=10,actions=output:3"));
	run(ovs_add_flow(br,"in_port=3,priority=10,actions=drop"));
}

// void changeOVS_flow_v4(string br)
// {
// 	run(ovs_add_flow(br,"in_port=2,priority=10,actions=output:3"));
// 	run(ovs_add_flow(br,"in_port=1,priority=10,actions=output:2"));
// 	run(ovs_add_flow(br,"in_port=3,priority=10,actions=output:1"));
// }


void del_allFlowTable(int attackNum)
{
	br = cmd(SAT_BR_NAME, satelliteId);
	run(ovs_del_flow(br, "in_port=1"));
	run(ovs_del_flow(br, "in_port=2"));
	run(ovs_del_flow(br, "in_port=3"));
}
