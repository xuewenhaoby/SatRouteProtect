#include "topo.h"
#include "common.h"
#include "cmd.h"
#include "task.h"
#include "message.h"
#include "timer.h"
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


using namespace std;

void TopoInitialize()
{
	cout << "Start TopoInitialize..." <<endl;
	ClearLog();
	ClearTopo();
	time_t time0 = time(NULL);
	for(int i=0;i<SAT_NUM;i++){
		for(int j=0;j<SINF_NUM;j++){
			Int_DB[i].linkId[j]=-1;
			Int_DB[i].port_stat[j]=false;
			Int_DB[i].dst_satid[j]=-1;
			Int_DB[i].dst_portid[j]=-1;
		}
	}
	InitStaticTopo();
	for (int i = 0; i < SAT_NUM; ++i)
	{
		sats[i].setId(i+1);
		pool.addTask(new NodeTask(new Message(
			&sats[i],MSG_NODE_Initialize)));
	}
	pool.wait();

	for(int i = 0; i < SAT_NUM; i++){
	pool.addTask(new NodeTask(new Message(
		&sats[i],MSG_LINK_Initialize)));
	}

	pool.wait();

	for(int i = 0; i < SAT_NUM; i++){
	pool.addTask(new NodeTask(new Message(
		&sats[i],MSG_ROUTE_Initialize)));
	}

	for (int i = 0; i < HOST_NUM; ++i)
	{
		hosts[i].setId(i+1);
		pool.addTask(new NodeTask(new Message(
			&hosts[i],MSG_NODE_Initialize)));
	}
	pool.wait();

	for(int i = 0; i < HOST_NUM; i++){
	pool.addTask(new NodeTask(new Message(
		&hosts[i],MSG_LINK_Initialize)));
	}

	pool.wait();

	AttackInitialize();

	//SocketInitialize(GetTime());

	time_t time1 = time(NULL); 
	
	cout << "TopoInitialize finished:" << time1-time0 << endl;

}

void TopoFinalize()
{
	cout << "Start TopoFinalize..." << endl;
	time_t time0 = time(NULL);

	for(int i = 0; i < SAT_NUM; i++){
		pool.addTask(new NodeTask(new Message(
			&sats[i],MSG_NODE_Finalize)));
	}

	pool.wait();

	for (int i = 0; i < SLINK_NUM; ++i)
	{
		pool.addTask(new NodeTask(new Message(
			&sats[i],MSG_LINK_Finalize)));
	}

	pool.wait();

	for (int i = 0; i < HOST_NUM; ++i)
	{
		pool.addTask(new NodeTask(new Message(
			&hosts[i],MSG_NODE_Finalize)));
	}

	pool.wait();

	time_t time1 = time(NULL);
	cout << "TopoFinalize finished:" << time1-time0 << endl;

	cout << "Start clear thread pool..." << endl;
	
	pool.stopAll();
	route_pool.stopAll();

    cout << "Clear thread pool finished." << endl;
}

NodePos ReadLocFile(int id,int time)
{
	ifstream fileIn(MOBILITY_FILE, ios_base::binary);

	int t[2];
	int startTime[2];
	Coord location[2];
	t[0] = time;
	t[1] = t[0] + 1;
	int v = id-1;
	for(int i = 0; i < 2; i++)
	{
		startTime[i] = v * DATA_SIZE + t[i] % DATA_SIZE;
		fileIn.seekg(sizeof(Coord) * startTime[i], ios_base::beg);
		fileIn.read((char *)(location + i), sizeof(Coord));
	}
	NodePos pos;
	memcpy(& pos.loc, location, sizeof(Coord));
	pos.isNorth = (location[0].lat < location[1].lat);

	fileIn.close();
	return pos;
}

void ReadIslFile(string file_name)
{
	memset(&G,0,sizeof(StaticTopo));
	ifstream in;
	in.open(file_name);
	string s;
	while (getline(in, s)){
		int p0 = s.find(",", 0);
		string sub = s.substr(1, p0);
		int id = atoi(sub.c_str());
		G.isl[id - 1].linkId = id;

		int * value = (int*) G.isl[id - 1].endpoint;
		for (int i = 0; i < 2; i++){
			int p1 = s.find(":",p0);
			sub = s.substr(p0 + 1, p1 - p0 - 1);
			*value = atoi(sub.c_str());

			int p2 = s.find("|", p1);
			sub = s.substr(p1 + 1, p2 - p1 - 1);
			*(value+1) = atoi(sub.c_str());
			
			int p3 = s.find(",", p2);
			sub = s.substr(p2 + 1, p3 - p2 - 1);
			*(value+2) = atoi(sub.c_str());

			p0 = p3;
			value += 3;
		}
		value = NULL;

		int pos2 = s.find("]");
		sub = s.substr(p0 + 1, pos2 - p0 - 1);
		G.isl[id - 1].weight = atoi(sub.c_str());

		G.isl[id - 1].subnetIp = G.isl[id - 1].endpoint[0].ip & 0xffffff00;
	}
	in.close();
}



void InitStaticTopo()
{
	// Set G.isl
	ReadIslFile(ISL_FILE);
	// Initilaize default value
	for(int i = 0; i < SAT_NUM; i++){
		// Initialize G.arcs
		for(int j = 0; j < SAT_NUM; j++){
			if(i == j){
				G.arcs[i][j].weight = 0;
				G.arcs[i][j].linkId = LINKID_SELFLOOP;
			}else{
				G.arcs[i][j].weight = MAX_ARC_WEIGHT;
				G.arcs[i][j].linkId = LINKID_NOLINK;
			}
		}
		
		// Initialize infData
		for(int j = 0; j < SINF_NUM; j++){
			InfData tmp = {-1,false};
			sats[i].setInfData(tmp,j);
		}
	}
	// Set value
	for(int i = 0; i < SLINK_NUM; i++){
		Isl* isl = & G.isl[i];
		int v[2] = {isl->endpoint[0].nodeId-1, isl->endpoint[1].nodeId-1};
		// Set G.arcs
		for(int j = 0; j < 2; j++){
			G.arcs[ v[j] ][ v[(j+1)%2] ].linkId = isl->linkId;
			G.arcs[ v[j] ][ v[(j+1)%2] ].weight = isl->weight;
		}
		// Set infData
		for(int j = 0; j < 2; j++){
			int inf = isl->endpoint[j].inf;
			(sats[v[j]].acqInfData(inf))->linkId = isl->linkId;
		}
	}
}

NodePos RandomPos()
{
	srand((unsigned)time(NULL));
	Coord loc = {RANDOM(-90,90),RANDOM(-180,180)};
	NodePos pos = {loc,false};
	return pos;	
}

void AttackInitialize()
{
	string attack_cmd;
	attack_cmd = "cd attack && ./attack 0 ";
	system(attack_cmd.c_str());
}

void SocketInitialize(int time_diff)
{
	string s;
	
	//s = "cd singlePot && ip netns exec sat"+to_string(i)+" ./socket "+to_string(i) + " "+to_string(1) + " "+to_string(time_diff);
	s= "cd singlePot && bash test.sh "+to_string(0)+" "+to_string(time_diff);
	system(s.c_str());
	// pid_t pid;
	// if((pid =fork())<0)
	// {
	// 	printf("fork erroe\n");
	// 	exit(1);
	// }
	// else if(pid == 0)
	// {
	// 	system(s.c_str());
	// }
	
}

void ClearTopo()
{
	string  s = " del/./del";
	system(s.c_str());
}

