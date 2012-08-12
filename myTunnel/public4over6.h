#ifndef _TUNNEL_46_H_
#define _TUNNEL_46_H_

#define TUNNEL_SET 1
#define TUNNEL_INFO 2
#define TUNNEL_SET_MTU 3

#define TUNNEL_DEVICE_NAME "public4over6"
#define TUNNELMESSAGE SIOCDEVPRIVATE

//a. 1 == Request/Send Status Information
//b. 2 == Request/Send Logs
//c. 3 == Send TC Address
//d. 4 == Start Kernel Module
//e. 5 == Stop Kernel Module
//f. 6 == Information Has Received
#define LINK_K_MSG		0
#define LINK_REQUEST_INFO	1
#define LINK_TC_ADDRESS   	3
#define LINK_TI_ADDRESS		4
#define LINK_STOP		5
#define LINK_DHCP_ADDRESS	7

#define NL_PORT      19

struct TunnelInfo {
    struct in_addr v4_addr;
    struct in6_addr v6_addr;
    unsigned int in_traffic; //byte
    unsigned int out_traffic; //byte
};

struct Log {
    char level;
    char tag;
    char message[122];
};

typedef struct tunnel_info
{
    struct in6_addr saddr,daddr;
    char hw[6];
    int type;
    int mtu;
} tunnel_info_t;


#endif


