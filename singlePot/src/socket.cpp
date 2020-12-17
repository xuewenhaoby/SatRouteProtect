#include <unistd.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <memory.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h> // sockaddr_ll
#include<arpa/inet.h>
#include<netinet/if_ether.h>
#include <net/if.h>
#include<iomanip>

#include <sys/ioctl.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <time.h>
#include <sys/time.h>

#include "timer.h"
#include "thread_pool.h"
#include "message.h"
#include "common.h"
#include "task.h"
#include "topo.h"
#include "opspf.h"
#include "cmd.h"
#include "route.h"
#include "encrypt.h"


using namespace std;

int main(int argc,char* argv[])
{
	if(argc!=4)
	{
		cout<<"The number of paramer is wrong!"<<endl;
		cout<<"Please try again"<<endl;
		return 0;
	}
	satelliteId = atoi(argv[1]);
	
	if(atoi(argv[2])==0)
	{
		hashHandle = false;
		backupRoute = false;
	}
	else if(atoi(argv[2])==1)
	{
		hashHandle = true;
		backupRoute = false;
	}
	else if(atoi(argv[2])==2)
	{
		hashHandle = false;
		backupRoute = true;
	}
	else 
	{
		hashHandle = false;
		backupRoute = false;
	}
	time_diff = atoi(argv[3]);

	initialize(satelliteId);
}

void initialize(int satelliteId)
{
	initializeAllInterface(satelliteId);
	InitStaticTopo();
	initializeAllInt_DB();
	routeInitialize();
	updateLink();
	pthread_t recv;
	pthread_create(&recv,NULL,recv_opspf, NULL);
	pthread_join(recv,NULL);	
}


void initializeAllInterface(int satelliteId)
{
	for(int i=0;i<SINF_NUM;i++)
    {
    	selfInf[i] =interface_init(satelliteId,i); 
    	selfInf[i].changeStat = OPSPF_PORT_NOCHANGE; 
    	selfInf[i].stat = true;   
    	selfInf[i].ttl = OPSPF_PORT_TTL;   
    	selfInf[i].lsuAck = false;
    }

    for (int i = 0; i < SAT_NUM; ++i)
    {
    	for (int j = 0; j < SINF_NUM; ++j)
    	{
			Int_DB[i].linkId[j]=-1;
			Int_DB[i].port_stat[j]=true;
			Int_DB[i].dst_satid[j]=-1;
			Int_DB[i].dst_portid[j]=-1;
    	}
    }
}

void initializeAllInt_DB()
{
	for (int i = 1; i <= SAT_NUM; ++i)
	{
		Int_DB_init(i);
	}
}

void Int_DB_init(int satelliteId)
{

	for(int i = 0; i < SINF_NUM; i++)
	{		

		//Int_DB initial 

	    if(Int_DB[satelliteId-1].linkId[i]!=-1)
	    {
		    if(G.isl[Int_DB[satelliteId-1].linkId[i]-1].endpoint[0].nodeId==satelliteId)
		    {  
		        Int_DB[satelliteId-1].dst_satid[i]=G.isl[Int_DB[satelliteId-1].linkId[i]-1].endpoint[1].nodeId;  
	            Int_DB[satelliteId-1].dst_portid[i]=G.isl[Int_DB[satelliteId-1].linkId[i]-1].endpoint[1].inf;
		    }
		    else
		    {
			Int_DB[satelliteId-1].dst_satid[i]=G.isl[Int_DB[satelliteId-1].linkId[i]-1].endpoint[0].nodeId;
			Int_DB[satelliteId-1].dst_portid[i]=G.isl[Int_DB[satelliteId-1].linkId[i]-1].endpoint[0].inf;
		    }
		   

	    }
	}
}

OpspfInfData interface_init(int satelliteId,int portId)
{
	int sd; 
	// int  one = 1;
	// const int *val = &one;
	int ret;
	char dev[BUF_STR_SIZE];
	sprintf(dev,"sat%dp%d",satelliteId,portId);
	struct ifreq interface;
	OpspfInfData inf;
	int if_index;
	unsigned char if_mac[6];

	sd = socket(PF_PACKET, SOCK_RAW, IPPROTO_OPSPF);
	if (sd < 0)
	{
		perror("socket() error");
		// If something wrong just exit
		exit(-1);
	}


	strncpy(interface.ifr_name,dev,sizeof(dev));

	ret = ioctl(sd,SIOCGIFINDEX,(char*)&interface);
	if(ret<0)
	{
		close(sd);
		//cout<<"Can not get index"<<endl;
	}

	ret = ioctl(sd,SIOCGIFHWADDR,(char*)&interface);
	if(ret<0)
	{
		close(sd);
		//cout<<"Can not get HWADDR"<<endl;
	}

	ret = ioctl(sd,SIOCGIFFLAGS,(char*)&interface);
	if(ret<0)
	{
		close(sd);
		//cout<<"Can not do ioctl"<<endl;
	}

	interface.ifr_flags |= IFF_PROMISC;
	ret = ioctl(sd,SIOCSIFFLAGS,(char*)&interface);
	if(ret<0)
	{
		close(sd);
		//cout<<"Can not set to promisc mode"<<endl;
	}

	memcpy(if_mac,interface.ifr_hwaddr.sa_data,sizeof(if_mac));
	memcpy(inf.if_mac ,if_mac,sizeof(if_mac));

	ret = ioctl(sd,SIOCGIFINDEX,(char*)&interface);
	if(ret<0)
	{
		close(sd);
		//cout<<"Can not get index"<<endl;
	}
	if_index = interface.ifr_ifindex;

	//struct sockaddr_in addr,netmask;
	ret=ioctl(sd,SIOCGIFADDR,(char*)&interface);
	if(ret<0)
	{
		//cout<<"Can not get IP"<<endl;
	}
	inf.ip = ((struct sockaddr_in *)&interface.ifr_addr)->sin_addr.s_addr;

	//cout<<inf.ip<<endl;

	ret=ioctl(sd,SIOCGIFNETMASK,(char*)&interface);
	if(ret<0)
	{
		//cout<<"Can not get netmask"<<endl;
	}
	 inf.netmask = ((struct sockaddr_in *)&interface.ifr_netmask)->sin_addr.s_addr;

	 inf.sock = sd;
	 inf.if_index = if_index;
	 inf.satelliteId = satelliteId;
	 inf.portId = portId;
	if(setsockopt(sd,SOL_SOCKET,SO_BINDTODEVICE,(char*)&interface,sizeof(struct ifreq))<0)
	{
		//perror("SO_BINDTODEVICE failed");
	}


	// if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(int)))
	// {
	// 	perror("setsockopt() error");
	// 	exit(-1);
	// }
	//else
		//printf("setsockopt() is OK.\n");

	return inf;
}



void Send_Hello_Packet()
{
	for (int i = 0; i < SINF_NUM; i++)
	{
		if(selfInf[i].stat)
		{
			if(0 == selfInf[i].ttl--)//Time exceeds limit,judge the link port is down
			{
				cout<<to_string(i)<<"port is down"<<endl;
				if(GetTime()>20)
				{
					attackTime++;

					string packetLog = "packetFile/packetLog.txt";
					ofstream fin(packetLog,ios::app);
					string packet =to_string(GetTime())+"s."+"HelloPacketPortDown-"+to_string(satelliteId)+"-"+to_string(i);
					fin<<packet<<endl;
					fin.close();
				}
				selfInf[i].changeStat = OPSPF_PORT_LINKDOWN;
				Int_DB[satelliteId-1].port_stat[i] = false;
				if(backupRoute)
				{
					cout<<"attackTime"+to_string(attackTime)<<endl;
					UpdateOpspfRoutingTable();
					if(attackTime>=2||attackTime==0)
					{
						pool.addTask(new NodeTask(new Message(
						MSG_NODE_SendLsuPacket)));
						attackTime = 0;
					}
					else if(attackTime==1)
					{
						Init_Lsu_Packet();
						backup_UpdateOpspfRoutingTable(lsuData.dstSatelliteId,lsuData.max_distance);
					}
				}
				else
				{
					attackTime = 2;
					UpdateOpspfRoutingTable();
					pool.addTask(new NodeTask(new Message(
						MSG_NODE_SendLsuPacket)));

				}
				


			}
			else
			{
				OpspfSendProtocolPacket(selfInf[i],OPSPF_HELLO_PACKET);
			}
		}
	
	}
}

void Init_Lsu_Packet()
{
	for (int i = 0; i < SINF_NUM; ++i)
	{
		if(selfInf[i].changeStat != OPSPF_PORT_NOCHANGE && selfInf[i].stat ==true)
		{
			//reset the ACK state
			for (int j = 0; j < SINF_NUM; ++j)
			{
				selfInf[j].lsuAck = false;
			}
			lsuData.changeStat = selfInf[i].changeStat ;
			lsuData.srcSatelliteId = satelliteId;
			lsuData.srcPortId = i;

			for(int k = 0; k < 2; k++)
			{
				if(satelliteId == G.isl[ Int_DB[satelliteId-1].linkId[i]-1].endpoint[k].nodeId)
				{
					lsuData.dstSatelliteId = G.isl[ Int_DB[satelliteId-1].linkId[i]-1 ].endpoint[(k+1)%2].nodeId;
					lsuData.dstPortId      = G.isl[ Int_DB[satelliteId-1].linkId[i]-1].endpoint[(k+1)%2].inf;
				}
			}	
		}
		
	}
	if(attackTime == 2)
	{
		lsuData.lsuType = 2;
	}
	else if(attackTime == 1)
	{
		lsuData.lsuType =1;
		lsuData.max_distance = distanceTodstSat(lsuData.dstSatelliteId);
	}

}

void Send_Lsu_Packet()
{
	Init_Lsu_Packet();
	for (int j = 0; j < SINF_NUM; ++j)//flood from each port 
	{
		if(selfInf[j].stat == true && selfInf[j].lsuAck == false && Int_DB[satelliteId-1].port_stat[j] == true)
		{
			lsuData.timeStamp = GetTime();
			OpspfSendProtocolPacket(selfInf[j],OPSPF_LSU_PACKET);
		}
	}

	for (int i = 0; i < SINF_NUM; ++i)
	{
		selfInf[i].changeStat = OPSPF_PORT_NOCHANGE;
	}
}

void Flood_Lsu_Packet(int interfaceIndex)
{
	

	if(recv_send_LSU ==true)
	{
		for (int j = 0; j < SINF_NUM; ++j)
		{
			selfInf[j].lsuAck = false;
		}

		for (int j = 0; j < SINF_NUM; ++j)
		{
			if(selfInf[j].stat == true && selfInf[j].lsuAck == false && j != interfaceIndex)
			{
				cout<<"Send LSU packet"+to_string(j)<<endl;
				OpspfSendProtocolPacket(selfInf[j],OPSPF_LSU_PACKET);
			}

		}
		sleep(OPSPF_LSU_TTL);
	}
	
}



void OpspfSendProtocolPacket(OpspfInfData inf,OpspfPacketType packetType)
{
	char buffer[PCKT_LEN] ;
	char text[256];
	char *key_hash =new char;
	sprintf(key_hash,"%d",key_value);
	string temp,hashstr;
	//int count = 0;
	struct ethhdr *eth_header = (struct ethhdr *)buffer;
	struct iphdr *ip = (struct iphdr *) (buffer+sizeof(struct ethhdr));
	OPSPF_Header* opspf_header =(OPSPF_Header*)(buffer +sizeof(struct ethhdr)+ sizeof(struct iphdr));

	memset(buffer, 0, PCKT_LEN);

	switch(packetType)
	{
		case OPSPF_HELLO_PACKET:
		{		
			opspf_header->packetType = OPSPF_HELLO_PACKET;
			opspf_header->pktlen = sizeof(OPSPF_Header)+sizeof(OpspfHelloInfo);
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr) ,opspf_header , sizeof(OPSPF_Header));

			OpspfHelloInfo* helloInfo = (OpspfHelloInfo*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
			helloInfo->satelliteId = inf.satelliteId;
			helloInfo->portId =inf.portId;
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr)+sizeof(OPSPF_Header),helloInfo, sizeof(OpspfHelloInfo));

			//HASH
			if(hashHandle)
			{
			//memcpy(text,helloInfo,sizeof(OpspfHelloInfo));
			memcpy(text,key_hash,sizeof(KEY_BUFFER_SIZE));
			temp = text;
			hashstr = sha256_hash(temp);
			HASH* hashInfo = (HASH*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(OpspfHelloInfo));
			strcpy(hashInfo->hash_value,hashstr.c_str()); 
			}

			break;
		}
		case OPSPF_LSU_PACKET:
		{
			opspf_header->packetType = OPSPF_LSU_PACKET;
			opspf_header->pktlen = sizeof(OPSPF_Header)+sizeof(OpspfLsuInfo);
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr) ,opspf_header , sizeof(OPSPF_Header));

			// OpspfLsuInfo* lsuInfo = (OpspfLsuInfo*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
			// //lsuData.lsuId ++;
			// *lsuInfo  = lsuData;

			//memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr)+sizeof(OPSPF_Header),lsuInfo , sizeof(OpspfLsuInfo));

			//HASH
			if(hashHandle)
			{
				
				int SM4encrypt = 0;
				
				unsigned char buffer_encrypt[32];
			 	unsigned char lsuInfoEncrypt[32];

				lsuInfoEncrypt[0] = lsuData.lsuType +'0';
				lsuInfoEncrypt[1] = lsuData.lsuId+'0';
				lsuInfoEncrypt[2] = lsuData.srcSatelliteId/10 + '0';
				lsuInfoEncrypt[3] = lsuData.srcSatelliteId%10 + '0';
				lsuInfoEncrypt[4] = lsuData.srcPortId +'0';
				lsuInfoEncrypt[5] = lsuData.dstSatelliteId/10 +'0';
				lsuInfoEncrypt[6] = lsuData.dstSatelliteId%10 +'0';
				lsuInfoEncrypt[7] = lsuData.dstPortId +'0';
				lsuInfoEncrypt[8] = lsuData.max_distance/10+'0';
				lsuInfoEncrypt[9] = lsuData.max_distance%10+'0';
				lsuInfoEncrypt[10] = lsuData.changeStat+'0';
				lsuInfoEncrypt[11] = lsuData.timeStamp;
				for(int i=11;i<31;i++)
				{
					lsuInfoEncrypt[i] = '0';
				}
				lsuInfoEncrypt[31] = '\0';
				SM4encrypt =  SM4Crypt(1, key_encrypt_value,lsuInfoEncrypt,buffer_encrypt);
				if(SM4encrypt==1)
				{
	
					memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr)+sizeof(OPSPF_Header),buffer_encrypt ,sizeof(buffer_encrypt));
				}


				memcpy(text,key_hash,sizeof(KEY_BUFFER_SIZE));
				memcpy(text+sizeof(KEY_BUFFER_SIZE),lsuInfoEncrypt,sizeof(lsuInfoEncrypt));
				temp = text;
				hashstr = sha256_hash(temp);
				HASH* hashInfo = (HASH*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(buffer_encrypt));
				strcpy(hashInfo->hash_value,hashstr.c_str());				 
			
			}
			else
			{
				OpspfLsuInfo* lsuInfo = (OpspfLsuInfo*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
				lsuData.lsuId ++;
				*lsuInfo  = lsuData;
				memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr)+sizeof(OPSPF_Header),lsuInfo , sizeof(OpspfLsuInfo));
			}

			break;
		}
		case OPSPF_LSACK_PACKET:
		{
			opspf_header->packetType = OPSPF_LSACK_PACKET;
			opspf_header->pktlen = sizeof(OPSPF_Header)+sizeof(OpspfLsuackInfo);
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr) ,opspf_header , sizeof(OPSPF_Header));

			OpspfLsuackInfo* lsuackInfo = (OpspfLsuackInfo*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
			lsuackInfo->ackId = 1;
			lsuackInfo->timeStamp = GetTime();
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr)+sizeof(OPSPF_Header),lsuackInfo , sizeof(OpspfLsuackInfo));

			//HASH
			if(hashHandle)
			{
				//memcpy(text,lsuackInfo,sizeof(OpspfLsuackInfo));
				memcpy(text,key_hash,sizeof(KEY_BUFFER_SIZE));
				temp = text;
				hashstr = sha256_hash(temp);
				HASH* hashInfo = (HASH*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(OpspfLsuackInfo));
				strcpy(hashInfo->hash_value,hashstr.c_str()); 
			}

			break;
		}
		default:
		{
			cout<<"Unknown protocol packet type to send!"<<endl;
			break;
		}
	}

	struct sockaddr_in sin, din;
	sin.sin_family = AF_INET;
	din.sin_family = AF_INET;
	sin.sin_addr.s_addr = inf.ip;
	din.sin_addr.s_addr = inf.ip;

 
	// Fabricate the IP header or we can use the
	// standard header structures but assign our own values.
	ip->ihl = COMMON_IPHDR_LEN;
	ip->version = IP_VERSION;//报头长度，4*32=128bit=16B
	ip->tos = DEFAULT_OPSPF_TOS; // 服务类型
	ip->tot_len = (sizeof(struct iphdr) + opspf_header->pktlen);
	//ip->id = htons(54321);//可以不写
	ip->ttl = DEFAULT_OPSPF_TTL; // hops生存周期
	ip->protocol = IPPROTO_OPSPF; // OPSPF
	ip->check = 0;

	ip->saddr = sin.sin_addr.s_addr;
	ip->daddr = din.sin_addr.s_addr;

	struct sockaddr_ll dstmac;
	memset(&dstmac,0,sizeof(dstmac));
	dstmac.sll_family = AF_PACKET;
	dstmac.sll_ifindex = inf.if_index;
	dstmac.sll_halen = htons(ETH_HLEN);
	memcpy(dstmac.sll_addr,inf.if_mac,dstmac.sll_halen);
	memcpy(eth_header->h_dest , inf.if_mac,ETH_ALEN);
 
	setuid(getpid());//如果不是root用户，需要获取权限	
	if (sendto(inf.sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&dstmac, sizeof(dstmac)) < 0)
	{
		//perror("sendto() error");
	}	
		//close(inf.sock);
}

void *recv_opspf(void *ptr)
{
	int sockfd;
	struct iphdr *ip;
	// struct ethhdr *eth_header;
	char buf[2048];
	ssize_t n;
	int interfaceIndex;

	char local_text[256];
	char key_hash[KEY_BUFFER_SIZE];

	sprintf(key_hash,"%d",key_value);

	struct timeval t1,t2;
    double timeuse;
    
	
	if ((sockfd = socket(PF_PACKET,  SOCK_RAW, htons(ETH_P_ALL)))== -1)
	{    
	    printf("socket error!\n");
	}
	StartTimer();

	for(int i=0;;i++)
	{
		n = recv(sockfd, buf, sizeof(buf), 0);

	    if (n == -1)
	    {
	        printf("recv error %d\n",errno);
	        break;
	    }
	    else if (n==0)
	        continue;
	    //struct ethhdr *eth_header = (struct ethhdr*)(buf);
	    ip = ( struct iphdr *)(buf+sizeof(ethhdr));
	    // if(ip->protocol != IPPROTO_OPSPF )
	    // {
	    // 	//Handle data packet
	    // 	continue;
	    // }
	    //analyseIP(ip);
	    if(ip->saddr == selfInf[0].ip || ip->saddr == selfInf[1].ip || ip->saddr == selfInf[2].ip || ip->saddr == selfInf[3].ip 
	        	|| ip->saddr == selfInf[4].ip || ip->saddr == selfInf[5].ip)//self packet
        {
        	continue;

 		}
 		else
 		{
    		unsigned char* tmp1 = (unsigned char*)&ip->daddr;
        	unsigned char* tmp2;
        	for (int i = 0; i < SINF_NUM; ++i)
	    	{
	    		tmp2 = (unsigned char*)&selfInf[i].ip;
	    		if(tmp1[0]==tmp2[0] && tmp1[1]==tmp2[1] && tmp1[2]==tmp2[2])// the same subnet ip 
	    		{
	    			interfaceIndex = i;
	    		}
	    	}
			      
	        if (ip->protocol == IPPROTO_OPSPF)
	        {

	            OPSPF_Header *opspfhdr = (OPSPF_Header *)(buf +sizeof(ethhdr)+sizeof(struct iphdr));
	            switch(opspfhdr->packetType)
	            {
	                case OPSPF_HELLO_PACKET:
	                  {
	                    OpspfHelloInfo *helloInfo =(OpspfHelloInfo*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
	                    
	                    //HASH started
	                    if(hashHandle)
	                    {
							HASH* hashInfo = (HASH*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(OpspfHelloInfo));
							//memcpy(local_text,helloInfo,sizeof(OpspfHelloInfo));
							memcpy(local_text,key_hash,sizeof(KEY_BUFFER_SIZE));
							string temp = local_text;
							string hashstr = sha256_hash(temp);
							string recvHash = hashInfo->hash_value;
							if(recvHash.compare(hashstr)==0)
							{
								Opspf_Handle_HelloPacket(helloInfo,interfaceIndex);
							}
	                    }
	   
	                    else
	                    {
	                    	Opspf_Handle_HelloPacket(helloInfo,interfaceIndex);
	                    }

	                	//HASH ended			                	
	                    break;
	                  }
	                case OPSPF_LSU_PACKET:
	                {	                   
	                    //HASH started
						gettimeofday(&t1,NULL);
	                    if(hashHandle)
	                    {
	                    	int SM4encrypt = 0;

							unsigned char buffer_encrypt[32];
							unsigned char miwen[33];
							char *p = ( char*)malloc(32);
							p = (char*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
							for (int i = 0; i <32; ++i)
							{
								miwen[i] = p[i];
							}
							miwen[33] = '\0';
							
							unsigned char tmp[32];
							for (int i = 0; i < 32; ++i)
							{
								tmp[i] = miwen[i];
							}

							OpspfLsuInfo* lsuInfo = (OpspfLsuInfo*)malloc(sizeof(OpspfLsuInfo));

							
							SM4encrypt =  SM4Crypt(2, key_encrypt_value,miwen,buffer_encrypt);
							if(SM4encrypt==1)
							{
								
								char jiemihou[12];
								for (int i = 0; i < 12; ++i)
								{
									jiemihou[i] = buffer_encrypt[i];
								}
								lsuInfo->lsuType =jiemihou[0]- '0';
								lsuInfo->lsuId = jiemihou[1]-'0';
								lsuInfo->srcSatelliteId = 10*(jiemihou[2]- '0') +jiemihou[3]-'0' ;
								lsuInfo->srcPortId = jiemihou[4]-'0';
								lsuInfo->dstSatelliteId = 10*(jiemihou[5]-'0')+jiemihou[6]-'0';
								lsuInfo->dstPortId =jiemihou[7]-'0';
								lsuInfo->max_distance = (jiemihou[8]-'0')*10+jiemihou[9]-'0';
								lsuInfo->changeStat= OpspfPortChangeStatType(jiemihou[10]-'0');
								lsuInfo->timeStamp = 0;
								
							}	
	              
	  					    HASH* hashInfo = (HASH*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(buffer_encrypt));
							memcpy(local_text,key_hash,sizeof(KEY_BUFFER_SIZE));
							memcpy(local_text+sizeof(KEY_BUFFER_SIZE),tmp,sizeof(tmp));
							string temp = local_text;
							string hashstr = sha256_hash(temp);
							string recvHash = hashInfo->hash_value;

							if(recvHash.compare(hashstr)==0)
							{
								
								Opspf_Handle_LsuPacket(lsuInfo,interfaceIndex);															
							}
							free(lsuInfo);

	                    }
	                    else 
	                    {
	                    	OpspfLsuInfo *lsuInfo =(OpspfLsuInfo*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
	                    	Opspf_Handle_LsuPacket(lsuInfo,interfaceIndex);
	                    }
                    	gettimeofday(&t2,NULL);
					    timeuse = t2.tv_sec - t1.tv_sec +(t2.tv_usec-t1.tv_usec)/1000000.0;
						
						cout<<" time: "<<timeuse<<"s"<<endl;
	                 
	                    break; 
	                }
	                case OPSPF_LSACK_PACKET:
	                {
	                	OpspfLsuackInfo *lsuackInfo =(OpspfLsuackInfo*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
	                    if(hashHandle)
	                    {
							HASH* hashInfo = (HASH*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(OpspfLsuackInfo));
							memcpy(local_text,key_hash,sizeof(KEY_BUFFER_SIZE));
							string temp = local_text;
							string hashstr = sha256_hash(temp);
							string recvHash = hashInfo->hash_value;
							if(recvHash.compare(hashstr)==0)
							{
								Opspf_Handle_LsuackPacket(lsuackInfo,interfaceIndex);
							}
	                    }
	                    
	                    else
	                    {
	                    	Opspf_Handle_LsuackPacket(lsuackInfo,interfaceIndex);
	                    }
	                    			                    			                   
	                    break; 
	                }

	                default:
	                    break;
	            }
	        }
	        else
	        {
	        	//Handle data packet
	            //printf("other protocol!\n");
	        }  
 		}	  
	}
	EndTimer();
	close(sockfd);
    return 0;
}


void analyseIP(struct iphdr *ip)
{
    unsigned char* p = (unsigned char*)&ip->saddr;
    printf("Source IP\t: %u.%u.%u.%u\n",p[0],p[1],p[2],p[3]);
    p = (unsigned char*)&ip->daddr;
    printf("Destination IP\t: %u.%u.%u.%u\n",p[0],p[1],p[2],p[3]);
}


void Opspf_Handle_HelloPacket(const OpspfHelloInfo* helloInfo,int interfaceIndex)
{
   cout<<"HelloPacket"+to_string(helloInfo->satelliteId)+ to_string(helloInfo->portId)<<endl;
   selfInf[interfaceIndex].ttl = OPSPF_PORT_TTL;
    if(Int_DB[satelliteId-1].port_stat[interfaceIndex] == false) 
    {
    	cout<<"relink"<<endl;

		string packetLog = "packetFile/packetLog.txt";
		ofstream fin(packetLog,ios::app);
		string packet = to_string(GetTime())+"s."+"HelloPacketRelink-"+to_string(satelliteId)+"-"+to_string(interfaceIndex);
		fin<<packet<<endl;
		fin.close();
    	if(backupRoute){attackTime = 0;}
		selfInf[interfaceIndex].changeStat = OPSPF_PORT_RELINK;
		Int_DB[satelliteId-1].port_stat[interfaceIndex] = true;
		UpdateOpspfRoutingTable();
		pool.addTask(new NodeTask(new Message(
		MSG_NODE_SendLsuPacket)));
  
    } 

}

void Opspf_Handle_LsuPacket(const OpspfLsuInfo* lsuInfo,int interfaceIndex)
{
	// int timeStamp = GetTime();
	// if(timeStamp - lsuInfo->timeStamp >3)return;
	//if(lsuInfo->lsuId<lsuData.lsuId)return;

    // struct timeval t1,t2;
    // double timeuse;
    // gettimeofday(&t1,NULL);  

    recv_send_LSU =true;
    recv_send_LSU_index =interfaceIndex;
    int srcSatelliteId = lsuInfo->srcSatelliteId;
    int srcPortId = lsuInfo->srcPortId;
    int dstSatelliteId = lsuInfo->dstSatelliteId;
    int dstPortId = lsuInfo->dstPortId;
    int stat =lsuInfo->changeStat;
	cout<<"LsuPacket"+to_string(srcSatelliteId)+ " "+to_string(srcPortId)+" dst"+to_string(dstSatelliteId)+" "+to_string(dstPortId)+to_string(stat)<<endl;

	if(srcSatelliteId>SAT_NUM||dstSatelliteId>SAT_NUM||srcSatelliteId<0||dstSatelliteId<0)
	{
		cout<<"Wrong LSU packet"<<endl;
		return ;
	}
	string packetLog = "packetFile/packetLog.txt";
	ofstream fin(packetLog,ios::app);
	string packet = to_string(GetTime())+"s."+"LsuPacket-"+to_string(satelliteId)+"-"+to_string(interfaceIndex);
	fin<<packet<<endl;
	fin.close();
    
    OpspfSendProtocolPacket(selfInf[interfaceIndex],OPSPF_LSACK_PACKET);

    bool flood = false;

    if(lsuInfo->changeStat == OPSPF_PORT_LINKDOWN )
    {

    		if(Int_DB[srcSatelliteId-1].port_stat[srcPortId] == true ||Int_DB[dstSatelliteId-1].port_stat[dstPortId] == true)
	    	{	
	    		flood = true;
	    		cout<<"linkdown_flood"<<endl;
				Int_DB[srcSatelliteId-1].port_stat[srcPortId] = false;
				Int_DB[dstSatelliteId-1].port_stat[dstPortId] = false;

	    	}
 

    }
    else if(lsuInfo->changeStat == OPSPF_PORT_RELINK)
    {
    		if(Int_DB[srcSatelliteId-1].port_stat[srcPortId] == false ||Int_DB[dstSatelliteId-1].port_stat[dstPortId] == false)
	    	{
	    		flood = true;
	    		cout<<"relink_flood"<<endl;
				Int_DB[srcSatelliteId-1].port_stat[srcPortId] = true;
				Int_DB[dstSatelliteId-1].port_stat[dstPortId] = true;
	    	}
    	

    }
	if(lsuInfo->srcSatelliteId == lsuData.srcSatelliteId && lsuInfo->dstSatelliteId == lsuData.dstSatelliteId && lsuInfo->changeStat == lsuData.changeStat)
	{
		flood = false;

		cout<<"same packet"<<endl;
	}


	if(lsuInfo->lsuType == 1)
	{	
		if(flood)
		{
			cout<<"forward"+to_string(dstSatelliteId)<<endl;
			int lsuId = lsuData.lsuId;
			lsuData = *lsuInfo;
			lsuData.lsuId = lsuId;
			backup_UpdateOpspfRoutingTable(dstSatelliteId, lsuInfo->max_distance);
		}

	}
	else if(lsuInfo->lsuType == 2)
	{
		if(flood)
		{
			cout<<"flood"<<endl;
			UpdateOpspfRoutingTable();
			int lsuId = lsuData.lsuId;
			lsuData = *lsuInfo;
			lsuData.lsuId = lsuId;
			for (int i = 0; i < SINF_NUM; ++i)
			{
				if(i!=interfaceIndex && selfInf[i].stat == true )
				{
					OpspfSendProtocolPacket(selfInf[i],OPSPF_LSU_PACKET);
				}

			}    	
		}
	}
	// gettimeofday(&t2,NULL);
 //    timeuse = t2.tv_sec - t1.tv_sec +(t2.tv_usec-t1.tv_usec)/1000000.0;
	
	// cout<<" time: "<<timeuse<<"s"<<endl;

}


void Opspf_Handle_LsuackPacket(const OpspfLsuackInfo* lsuackInfo,int interfaceIndex)
{
	cout<<"ACKPacket"+to_string(lsuackInfo->ackId)<<endl;
	string packetLog = "packetFile/packetLog.txt";
	ofstream fin(packetLog,ios::app);
	string packet = to_string(GetTime())+"s."+"LsuackPacket-"+to_string(satelliteId)+"-"+to_string(interfaceIndex);
	fin<<packet<<endl;
	fin.close();
	selfInf[interfaceIndex].lsuAck = true;
	selfInf[interfaceIndex].changeStat = OPSPF_PORT_NOCHANGE;

}