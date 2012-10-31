#include <arpa/inet.h>

#define LINK_K_MSG              0
#define LINK_REQUEST_INFO       1
#define LINK_REQUEST_LOG        2
#define LINK_TC_ADDRESS         3
#define LINK_TI_ADDRESS         4
#define LINK_STOP               5
#define LINK_ACK                6
//#define LINK_DHCP_ADDRESS	7

#define NL_PORT			19
/*
 * Use Netlink to communicate with kernel
 */
struct TunnelInfo {
    struct in_addr v4_addr;
    struct in6_addr v6_addr;
	unsigned short sport;
	unsigned short eport;
    unsigned int in_traffic; //byte
    unsigned int out_traffic; //byte
};

struct Log {
    char level;
    char tag;
    char message[122];
};

int get_tunnel_info(char* show_string);
int get_logs(char* loglist_string);
int init_tunnel(char* tc_addr,char* ti_addr);
int close_tunnel();
