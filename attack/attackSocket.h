#ifndef __ATTACKSOCKET_H__
#define __ATTACKSOCKET_H__

#include <cstring>
#include <fstream>
#include <iostream>
#include <arpa/inet.h>
#include <sys/time.h>

using namespace std;
#define COMMON_IPHDR_LEN 5
#define IP_VERSION 4
#define DEFAULT_OPSPF_TOS 0xc0
#define DEFAULT_OPSPF_TTL 1

#ifndef  IPPROTO_OPSPF
#define  IPPROTO_OPSPF 200
#endif
#define  OPSPF_VERSION 2

#define OPSPF_HELLO_INTERVAL 1
#define OPSPF_LSU_INTERVAL 30
#define OPSPF_PORT_TTL 4
#define OPSPF_LSU_TTL 3
#define OPSPF_LSU_TIME 3


#define BUF_STR_SIZE 100
#define PCKT_LEN 1500
#define KEY_BUFFER_SIZE 10

struct timeval tv = {.tv_sec = 0, .tv_usec = 0};


typedef int NodeAddress;

enum OpspfPacketType
{
    OPSPF_HELLO_PACKET =1,
    OPSPF_LSU_PACKET   =2,
    OPSPF_LSACK_PACKET =3

};


enum OpspfPortChangeStatType
{
    OPSPF_PORT_LINKDOWN =0,
    OPSPF_PORT_RELINK =1,
    OPSPF_PORT_NOCHANGE =2
};

enum AttackMode
{
    SEND_FAKE_HELLO_PACKET =1,
    SEND_FAKE_LSU_PACKET   =2,
    DROP_HELLO_PACKET =3,
    MODIFY_LSU_PACKET =4,
    DELAY_LSU_PACKET = 5,
};

typedef struct OpspfInterfaceData
{
    int type;
    in_addr_t ip;
    in_addr_t netmask;
    in_addr_t gw_addr;
    int sock;
    int ttl;
    bool lsuAck;
    int satelliteId;
    int portId;
    int if_index;
    unsigned char if_mac[6];
    int linkId;
    bool stat;
    OpspfPortChangeStatType changeStat;
}OpspfInfData;



typedef struct _opspfhdr
{
    OpspfPacketType packetType;
    u_int16_t pktlen;
    int dst_addr;

}OPSPF_Header;

typedef struct opspf_hello_info
{
    int satelliteId;  
    int portId;
}OpspfHelloInfo;

typedef struct opspf_lsu_info
{
    int lsuType;
    int lsuId;
    int srcSatelliteId;
    int srcPortId;
    int dstSatelliteId;
    int dstPortId;
    int max_distance;
    OpspfPortChangeStatType changeStat;
    int timeStamp;
}OpspfLsuInfo;

typedef struct opspf_lsack_info
{
    int ackId;
    int timeStamp;
}OpspfLsuackInfo;


typedef struct hash_data
{
    char hash_value[64];
}HASH;

void initialize(int satelliteId);

OpspfInfData interface_init(int satelliteId,int portId);

void OpspfSendProtocolPacket(OpspfInfData inf,OpspfPacketType packetType);

void *recv_opspf(void *ptr);

void analyseIP(struct iphdr *ip);

void Opspf_Handle_HelloPacket(const OpspfHelloInfo* helloInfo);

void Opspf_Modify_LsuPacket(const OpspfLsuInfo* lsuInfo);

void Opspf_Delay_LsuPacket(const OpspfLsuInfo* lsuInfo);

//void Opspf_Modify_LsuackPacket(const OpspfLsuackInfo* lsuackInfo);

void Send_FakeHello_Packet();

void Send_FakeLsu_Packet();

//void Send_FakeLsack_Packet();

int GetTime();

string sha256_hash(const string str);


#endif

