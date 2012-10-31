#include <unistd.h>
#include <stdio.h>
#include <linux/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <signal.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "tunnel.h"

//----------------FOR LOG----------
#define LOG_TAG "Tunnel"
#undef LOG
#include <android/log.h>
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  

struct msg_to_kernel
{
    struct nlmsghdr hdr;
    struct in6_addr addr; 
};

struct u_tunnel_info
{
    struct nlmsghdr hdr;
    struct TunnelInfo tunnel_info;
};

struct u_logs
{
    struct nlmsghdr hdr;
    struct Log log;
};

static int skfd;
struct sockaddr_nl local;
struct u_tunnel_info u_tunnel;
struct u_logs u_log;

static void sig_int(int signo)
{
    struct sockaddr_nl kpeer;
    struct msg_to_kernel message;

    memset(&kpeer, 0, sizeof(kpeer));
    kpeer.nl_family = AF_NETLINK;
    kpeer.nl_pid    = 0;
    kpeer.nl_groups = 0;

    memset(&message, 0, sizeof(message));
    message.hdr.nlmsg_len = NLMSG_LENGTH(0);
    message.hdr.nlmsg_flags = 0;
    message.hdr.nlmsg_type = LINK_STOP;
    message.hdr.nlmsg_pid = getpid();

    close(skfd);
}

int get_tunnel_info(char* show_string) {
    int ret = 0;
    struct sockaddr_nl kpeer;
    int kpeerlen;
    struct msg_to_kernel message;
    int sendlen = 0;
    int rcvlen = 0;
    memset(&kpeer, 0, sizeof(kpeer));
    kpeer.nl_family = AF_NETLINK;
    kpeer.nl_pid = 0;
    kpeer.nl_groups = 0;

    memset(&message, 0, sizeof(message));
    message.hdr.nlmsg_len = NLMSG_LENGTH(0);
    message.hdr.nlmsg_flags = 0;
    message.hdr.nlmsg_type = LINK_REQUEST_INFO;
    message.hdr.nlmsg_pid = local.nl_pid;

    if((ret = sendto(skfd, &message, message.hdr.nlmsg_len, 0,(struct sockaddr*)&kpeer, sizeof(kpeer)))<0) {
        LOGD("send_request_tunnel_info failed");
        return -1;
    }
    
    kpeerlen = sizeof(struct sockaddr_nl);
    if((rcvlen = recvfrom(skfd, &u_tunnel, sizeof(struct u_tunnel_info), 0, (struct sockaddr*)&kpeer, &kpeerlen))<0) {
        LOGD("recv tunnel info length < 0");
        return -1;
    }
    char v6_addr_string[129];
    inet_ntop(AF_INET6, &(u_tunnel.tunnel_info.v6_addr), v6_addr_string, 128);

    snprintf(show_string, 512, "%s#%s#%d-%d#%d#%d", 
            inet_ntoa(u_tunnel.tunnel_info.v4_addr),
            v6_addr_string,
			u_tunnel.tunnel_info.sport,
			u_tunnel.tunnel_info.eport,
            u_tunnel.tunnel_info.in_traffic,
            u_tunnel.tunnel_info.out_traffic);

    return 1;
}

int close_tunnel()
{
    int ret = 0;
    struct sockaddr_nl kpeer;
    int kpeerlen;
    struct msg_to_kernel message;
    int sendlen = 0;
LOGD("CLOSE");
    memset(&kpeer, 0, sizeof(kpeer));
    kpeer.nl_family = AF_NETLINK;
    kpeer.nl_pid = 0;
    kpeer.nl_groups = 0;

    memset(&message, 0, sizeof(message));
    message.hdr.nlmsg_len = NLMSG_LENGTH(0);
    message.hdr.nlmsg_flags = 0;
    message.hdr.nlmsg_type = LINK_STOP;
    message.hdr.nlmsg_pid = local.nl_pid;

    if((ret = sendto(skfd, &message, message.hdr.nlmsg_len, 0,
            (struct sockaddr*)&kpeer, sizeof(kpeer)))<0) {
        LOGD("send close info failed");
        close(skfd);
        return -1;
    }
    close(skfd);
    return 1;
}

int init_tunnel(char* tc_addr, char* ti_addr)
{   
    int ret = 0;
    skfd = socket(PF_NETLINK, SOCK_RAW, NL_PORT);
    if(skfd < 0)
    {
        LOGD("can not create a netlink socket\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = getpid();
    local.nl_groups = 0;
    if(bind(skfd, (struct sockaddr*)&local, sizeof(local)) != 0)
    {
        LOGD("bind() error\n");
        return -1;
    }
    signal(SIGINT, sig_int);

    struct sockaddr_nl kpeer;
    int kpeerlen;
    struct msg_to_kernel message;
    int sendlen = 0;

    memset(&kpeer, 0, sizeof(kpeer));
    kpeer.nl_family = AF_NETLINK;
    kpeer.nl_pid = 0;
    kpeer.nl_groups = 0;

    memset(&message, 0, sizeof(message));
    message.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(message.addr));
    message.hdr.nlmsg_flags = 0;
    message.hdr.nlmsg_type = LINK_TC_ADDRESS;
    message.hdr.nlmsg_pid = local.nl_pid;
    inet_pton(AF_INET6, tc_addr, (void *)&message.addr);
    if((ret = sendto(skfd, &message, message.hdr.nlmsg_len, 0,
            (struct sockaddr*)&kpeer, sizeof(kpeer)))<0) {
        return -1;
    }

    memset(&message, 0, sizeof(message));
    message.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(message.addr));
    message.hdr.nlmsg_flags = 0;
    message.hdr.nlmsg_type = LINK_TI_ADDRESS;
    message.hdr.nlmsg_pid = local.nl_pid;
    inet_pton(AF_INET6, ti_addr, (void *)&message.addr);
    if((ret = sendto(skfd, &message, message.hdr.nlmsg_len, 0,
            (struct sockaddr*)&kpeer, sizeof(kpeer)))<0) {
        return -1;
    }

    return 1;
}
