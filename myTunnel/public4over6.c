#include <net/protocol.h>
#include <net/ip6_route.h>
#include <net/addrconf.h>
#include <net/arp.h>
#include <net/inet_ecn.h>
#include <net/dst.h>
#include <linux/etherdevice.h>

#include "public4over6.h"

#define DEBUG_PUBLIC4OVER6_
#define TC_IPV6_ADDRESS "20010da8020d00277a2bcbfffe1b6ce0"

#ifdef DEBUG_PUBLIC4OVER6_
#define CDBG(msg,args...) printk(KERN_DEBUG msg,##args)
#else
#define CDBG(msg,args...) {}
#endif

//============CZW==============
#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif
//============CZW==============

//=============================
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm-generic/fcntl.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <net/sock.h>
#include <linux/net.h>

//===============CZW============
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
//DECLARE_MUTEX(receive_sem);
//#else
DEFINE_SEMAPHORE(receive_sem);
//#endif

static struct sock *nlfd;

struct in_addr dhcp_v4_address;
struct in6_addr TC_V6_ADDRESS;
struct in6_addr TI_V6_ADDRESS;

struct
{
  __u32 pid;
  rwlock_t lock;
}user_proc;

static int send_to_user();


//------------------------Calculate Traffic--------------
int traffic_in;
int traffic_out;
//------------------------END----------------------------


//===============CZW==============

static char msgbuf[50];
struct file *fp;
loff_t pos;
mm_segment_t fs;

//=============================

#ifndef NF_IP6_LOCAL_OUT
#define NF_IP6_LOCAL_OUT 3
#endif

struct ectable 
{
   struct in6_addr remote6,local6;
};

char* inet_ntoa( struct in_addr in, char *buf, size_t *rlen);
int xmit( struct sk_buff *skb, struct in6_addr out_addr,struct net_device *dev);
char* inet_ntop_v6(struct in6_addr,char *dst);
void inet_pton_v6(char *,struct in6_addr *);
static int public4over6_change_mtu(struct net_device* dev,int new_mtu);
static void public4over6_setup(struct net_device *dev);

static struct net_device *netdev;


struct tunnel_private
{
    struct net_device_stats stat;
    struct net_device *dev;
    struct ectable ect;
};

 
int tunnel_open( struct net_device *dev)
{
    //MOD_INC_USE_COUNT;
    return 0;
}

int tunnel_close( struct net_device *dev)
{
    //MOD_DEC_USE_COUNT;
    return 0;
}


/* Generate a random Ethernet address (MAC) that is not multicast
 * and has the local assigned bit set.
 */
void generate_random_hw(struct net_device *dev)
{
 //printk(KERN_INFO TUNNEL_DEVICE_NAME"generate_random_hw\n" );
 unsigned char tmp;
 struct net_device *pdev=__dev_get_by_name(&init_net,"wlan0");
 memcpy(dev->dev_addr,pdev->dev_addr,6);
 tmp=dev->dev_addr[1];
 dev->dev_addr[1]=dev->dev_addr[4];
 dev->dev_addr[4]=tmp;
 /*
 if(is_zero_ether_addr(addr))
 {
    get_random_bytes(addr,ETH_ALEN);
    addr [0]&= 0xfe;	// clear multicast bit 
    addr [0]|= 0x02;	//set local assignment bit (IEEE802)
    //printk(KERN_INFO TUNNEL_DEVICE_NAME"generate_random_hw 2\n" );
  }
  */
}

/*
   Main process routine
   if we use the way of invoking xmit() to send the package,
   then we still need some changes.
*/
int public4over6_tunnel_xmit(struct sk_buff *skb, struct net_device *dev)
{
 
    struct iphdr *iph;
    struct in6_addr src_addr,out_addr;
    char buf[128];
    struct in_addr daddr,saddr;
    unsigned int head_room;
    struct ipv6hdr *ip6h;
//    struct flowi fl;//route key
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	struct flowi6 fl6;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct dst_entry *dst;	
    struct ectable *ect;
    struct net_device_stats *stats=&dev->stats;//for statistics
    iph = ip_hdr(skb);
    daddr.s_addr = iph->daddr;
    saddr.s_addr = iph->saddr;
    
    if (skb->protocol!= htons(ETH_P_IP)) //judgement of IP protocol
    {
    	CDBG("[public 4over6 tunnel]:this is not IPv4 protocol!\n");
	CDBG("[public 4over6 tunnel]:this is not IPv4 protocol this is %d!\n",ntohs(skb->protocol));
        goto tx_error;
    }
     
    //for debug information
    #ifdef DEBUG_PUBLIC4OVER6_
    inet_ntoa(saddr,buf,NULL);
    CDBG("[public 4over6 tunnel]:the source IPv4 address is %s!\n",buf);
    inet_ntoa(daddr,buf,NULL);
    CDBG("[public 4over6 tunnel]:the destination IPv4 address is %s!\n",buf);
    #endif
    
    ect=&(((struct tunnel_private*)netdev_priv(dev))->ect);
    //inet_pton_v6(local,&(ect->local6));
    //inet_pton_v6(remote,&(ect->remote6));
    if (ect==0)
    {
        CDBG("[public 4over6 tunnel]: Can not find data destination in ECT!\n");
        goto tx_error;
    }
    
//    src_addr=ect->local6;
//    out_addr=ect->remote6;

    src_addr = TI_V6_ADDRESS;
    out_addr = TC_V6_ADDRESS;
    char dstaddr[40];
    CDBG("[public 4over6 tunnel]:the source IPv6 address is %s!\n",inet_ntop_v6(src_addr, dstaddr));
    CDBG("[public 4over6 tunnel]:the destination IPv6 address is %s!\n",inet_ntop_v6(out_addr, dstaddr));

    head_room = sizeof(struct ipv6hdr);
//	CDBG("[public 4over6 tunnel]:before head is %d data is %d,X:%d\n",skb->head, skb->data, skb->data-skb->head);
//    CDBG("[public 4over6 tunnel]:before head_room: %d Real head_room: %d\n",head_room, skb_headroom(skb));

    if (skb_headroom(skb)<head_room || skb_cloned(skb)||skb_shared(skb))
    {
       struct sk_buff* new_skb=skb_realloc_headroom(skb,head_room+16);
       if (!new_skb)
       {
CDBG("[public 4over6 tunnel]: TX_ERROR\n");
          goto tx_error;
       }
       if (skb->sk)
       {
          skb_set_owner_w(new_skb,skb->sk);
       }
       dev_kfree_skb(skb);
       skb=new_skb;
     }
    
    skb_push(skb,sizeof(struct ipv6hdr));
    skb->transport_header=skb->network_header;
    skb_reset_network_header(skb);//skb->network_header=skb->data
     
     //free the old router entry
     //dst_release(skb->dst);
     //skb->dst = NULL;

     skb_dst_drop(skb);
     //Fill the IPv6 header
     ip6h = ipv6_hdr(skb);
     memset(ip6h,0,sizeof(struct ipv6hdr));
     ip6h->version = 6;
     ip6h->priority = 0;
     ip6h->payload_len = htons(skb->len-sizeof( struct ipv6hdr));
     ip6h->nexthdr=IPPROTO_IPIP;//IPv4 over IPv6 protocol
     ip6h->hop_limit = 64;     
     ip6h->saddr = src_addr;
     ip6h->daddr = out_addr;
     skb->protocol = htons(ETH_P_IPV6);

     #ifdef CONFIG_NETFILTER
     nf_conntrack_put(skb->nfct);
     skb->nfct = NULL;
//     #ifdef CONFIG_NETFILTER_DEBUG
//     skb->nf_debug = 0;
//     #endif
     #endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=        
//     fl.u.ip6.flowi6_proto = IPPROTO_IPV6;
     fl6.flowi6_proto = IPPROTO_IPV6;
//     fl.u.ip6.daddr = out_addr;
     fl6.daddr = out_addr;
//     dst = ip6_route_output(dev_net(dev),NULL, &fl.u.ip6);
     dst = ip6_route_output(dev_net(dev), NULL, &fl6);
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    
     skb_dst_set(skb,dst_clone(dst));
     nf_reset(skb);
 
     if (!skb_dst(skb))
     {
       CDBG("[public 4over6 tunnel]:Cannot find route information for packet!\n");
       goto tx_error;
     }
     else
     {
        stats->tx_packets++; 
        stats->tx_bytes+=skb->len;
////////////////////////////////////////////////////////////////////////
//        ((struct inet_sock*)(skb->sk))->pinet6 is a pointer to struct ipv6_pinfo(private data of IPv6)
//        If I don't add the following line,then when ftp telnet or http(and so on)coexist with dhcp,
//        then at the very time of dhcp packet is sent through tunnel_xmit(), ((struct inet_sock*)(skb->sk))->pinet6
//        would be a novalid pointer which will result a kernel panic.
//        But at present,I don't know why it is the situation.So I just add this line in order to stop kernel panic from
//        happening.
//        Maybe as I study and debug the Linux(Network)kernel further and further,I will firgure out the troublesome problem.
////////////////////////////////////////////////////////////////////////


//   CDBG("[public 4over6 tunnel]: head_room: %d Real head_room: %d\n",head_room, skb_headroom(skb));
//	CDBG("[public 4over6 tunnel]: head is %d data is %d X:%d\n",skb->head, skb->data, skb->data-skb->head);


//        if(skb->sk && ((struct inet_sock*)(skb->sk))->pinet6)
//            ((struct inet_sock*)(skb->sk))->pinet6=NULL;

//------------------------Calculate Traffic--------------
		traffic_out += skb->len;
//------------------------END----------------------------

        NF_HOOK(PF_INET6,NF_IP6_LOCAL_OUT,skb,NULL,skb_dst(skb)->dev,skb_dst(skb)->output);
        CDBG("[public 4over6 tunnel]:OK!!!\n");
     }
     return 0;       

tx_error : 
     dev_kfree_skb(skb);
     return 0;
}



void public4over6_err( struct sk_buff* skb,struct inet6_skb_parm* opt,int type,int code,int offset,__u32 info)
{
    CDBG("[public 4over6 tunnel]:IP6IP Error! (type=%d, code=%d, offset=%d).\n",type,code,offset);
}

//IOCTL function
int public4over6_ioctl(struct net_device *dev,struct ifreq *ifr,int cmd)
{
    tunnel_info_t tunnelinfo;
    tunnel_info_t *p_tunnelinfo=NULL;
    
    char *addr=dev->dev_addr;
    int err;
    struct ectable *ect;
    ect=&(((struct tunnel_private*)netdev_priv(dev))->ect);
    if(!capable(CAP_NET_ADMIN))
    {
       return -EPERM;
    }
    if(cmd==TUNNELMESSAGE)
    {       
      p_tunnelinfo=(tunnel_info_t*)ifr->ifr_data;
      if(p_tunnelinfo->type==TUNNEL_SET)
      {
	  err=copy_from_user(&tunnelinfo,p_tunnelinfo,sizeof(tunnel_info_t));
	  if(err)
             return err;
	  //memcpy(addr,p_tunnelinfo->hw,6);
          ipv6_addr_copy(&ect->remote6,&p_tunnelinfo->daddr);
	  ipv6_addr_copy(&ect->local6,&p_tunnelinfo->saddr);             
      } 
      else if(p_tunnelinfo->type==TUNNEL_INFO)
      {
         /*
         err=copy_to_user(p_tunnelinfo,&tunnelinfo,sizeof(tunnel_info_t));
	 if(err)
            return err;
         */
         ipv6_addr_copy(&p_tunnelinfo->daddr,&ect->remote6);
         ipv6_addr_copy(&p_tunnelinfo->saddr,&ect->local6);
      }
      //more ioctls.
      else if(p_tunnelinfo->type==TUNNEL_SET_MTU)
      {
         
         int mtu;
         err=copy_from_user(&tunnelinfo,p_tunnelinfo,sizeof(tunnel_info_t));
         if(err)
         {
            CDBG("[public 4over6 tunnel]:copy_from_user failed!\n");
            return err;
         }
         public4over6_change_mtu(dev,p_tunnelinfo->mtu);
         printk("[public 4over6 tunnel]:public4over6 mtu is set to %d!\n",mtu);
      }
      else
      {
	 CDBG("[public 4over6 tunnel]:command type error!%d\n",tunnelinfo.type);
      }
    } 
    else
    { 
       CDBG("[public 4over6 tunnel]:command error!\n");
    }
    return 0;
}


static int public4over6_change_mtu(struct net_device* dev,int new_mtu)
{
    if((new_mtu<68)||(new_mtu>1500))
    {
       return -EINVAL;
    }
    dev->mtu = new_mtu;
    return 0;
}


void inet_pton_v6(char* addr,struct in6_addr* dst)
{
    const char xdigits[]="0123456789abcdef";
    u_char* ptr=dst->s6_addr;
    int i,j;
    int index;
    int sum;
    for(i=0;i<16;i++)
    {
       sum=0;
       for (j=0;j<2;j++)
       {
	  index=0;
	  while (addr[i*2+j]!=xdigits[index]&&index<16) index++;
	  sum*=16;
	  sum+=index;
       }
       ptr[i]=(u_char)sum;
    }
}


char* inet_ntop_v6(struct in6_addr addr, char* dst)
{
    const char xdigits[] = "0123456789abcdef";
    int i;
    const u_char* ptr = addr.s6_addr;

    for(i = 0; i < 8;++i)
    {
       int non_zerop = 0;

       if (non_zerop || ( ptr[0] >>4))
       {
          *dst++ = xdigits[ptr[0] >> 4];
          non_zerop = 1;
       }
       if ( non_zerop || ( ptr[0] & 0x0F ) )
       {
          *dst++ = xdigits[ptr[0] & 0x0F];
          non_zerop = 1;
       }
       if ( non_zerop || ( ptr[1] >> 4 ) )
       {
          *dst++ = xdigits[ptr[1] >> 4];
          non_zerop = 1;
       }
       *dst++ = xdigits[ptr[1] & 0x0F];
       if ( i != 7 )
       {
          *dst++ = ':';
       }
       ptr += 2;
    }
    *dst++ = 0;
    return dst;
}

char* inet_ntoa( struct in_addr in, char* buf, size_t* rlen )
{
   int i;
   char* bp;

   /*
    * This implementation is fast because it avoids sprintf(),
    * division/modulo, and global static array lookups.
   */

   bp = buf;
   for ( i = 0; i < 4; i++ )
   {
      unsigned int o, n;
      o = ( ( unsigned char * ) &in )[i];
      n = o;
      if ( n >= 200 )
      {
         *bp++ = '2';
         n -= 200;
      }
      else if ( n >= 100 )
      {
         *bp++ = '1';
         n -= 100;
      }
      if ( o >= 10 )
      {
         int i;
         for ( i = 0; n >= 10; i++ )
         {
             n -= 10;
         }
         *bp++ = i + '0';
      }
      *bp++ = n + '0';
      *bp++ = '.';
    }
    *--bp = 0;
    if ( rlen )
    {
       *rlen = bp - buf;
    }

    return buf;
}


static inline void ipgre_ecn_decapsulate(struct iphdr *iph, struct sk_buff *skb)
{
    if (INET_ECN_is_ce(iph->tos)) {
	if (skb->protocol == htons(ETH_P_IP)) {
		IP_ECN_set_ce(ip_hdr(skb));
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
			IP6_ECN_set_ce(ipv6_hdr(skb));
	}
    }
}


#define IP6_TNL_F_RCV_DSCP_COPY 0x10
static inline void ip6ip_ecn_decapsulate(struct net_device* dev, struct ipv6hdr *ipv6h, struct sk_buff* skb )
{ 
    __u8 dsfield = ipv6_get_dsfield(ipv6h) & ~INET_ECN_MASK;

    if (dev->flags & IP6_TNL_F_RCV_DSCP_COPY)
	ipv4_change_dsfield(ip_hdr(skb), INET_ECN_MASK, dsfield);

    if (INET_ECN_is_ce(dsfield))
		IP_ECN_set_ce(ip_hdr(skb));

}


int public4over6_rcv( struct sk_buff *skb )
{

    struct ipv6hdr* ipv6th;
    char buff[255];
    int err;
    struct net_device *ndev=dev_get_by_name(dev_net(skb->dev),TUNNEL_DEVICE_NAME);  
    struct net_device_stats *stats=&ndev->stats;
    
    CDBG("[public 4over6 tunnel]:receive packet:receiving!\n");
    stats->rx_packets++;
    stats->rx_bytes+=skb->len;

    if (!pskb_may_pull( skb, sizeof( struct ipv6hdr )))
    {
       stats->rx_errors++;  
       CDBG("[public 4over6 tunnel]:receive packet: error decapsulating packet!\n");
       dev_put(ndev);
       goto rcv_error;
    }
  
    dev_put(ndev);
    ipv6th=ipv6_hdr(skb);
    
    #ifdef DEBUG_PUBLIC4OVER6_
    inet_ntop_v6(ipv6th->saddr, buff);
    CDBG("[public 4over6 tunnel]:receive packet:the source address is %s.\n" ,buff);
    inet_ntop_v6(ipv6th->daddr,buff);
    CDBG("[public 4over6 tunnel]:receive packet:the destination address is %s.\n" ,buff);
    #endif
    
    skb_reset_network_header(skb);
    skb->protocol = htons(ETH_P_IP);
    skb->pkt_type = PACKET_HOST; 
    memset( skb->cb, 0, sizeof( struct inet6_skb_parm));
    skb->dev =__dev_get_by_name( &init_net,TUNNEL_DEVICE_NAME);
    
    //for debug info only.
    CDBG("[public 4over6 tunnel]:skb->len is %d,skb->data_len is %d,skb->hdr_len is %d,skb->mac_len is %d!\n",skb->len,skb->data_len,skb->hdr_len,skb->mac_len);
    skb->hdr_len=14;

    //dst_release(skb->dst);
    //skb->dst = NULL;
    skb_dst_drop(skb);
    nf_reset(skb);
    #ifdef CONFIG_NETFILTER
    nf_conntrack_put(skb->nfct);
    skb->nfct = NULL;
//    #ifdef CONFIG_NETFILTER_DEBUG
//    skb->nf_debug = 0;
//    #endif
    #endif
   
   ip6ip_ecn_decapsulate(skb->dev,ipv6th, skb);
   err=netif_rx(skb);
   if (err)
   {
      CDBG("[public 4over6 tunnel]:decaped packet is dropped!\n");
      goto rcv_error;
   }
   else
      CDBG("[public 4over6 tunnel]:finish decapsulating packet!\n" );
   traffic_in += skb->len;
   return 0;
rcv_error:
   dev_kfree_skb(skb);
   return 0;
}



static struct inet6_protocol ip6ip_protocol = {
  .handler = public4over6_rcv,
  .err_handler = public4over6_err,
  .flags = INET6_PROTO_NOPOLICY |INET6_PROTO_FINAL,
 };


static const struct net_device_ops ip6ip_netdev_ops = {
	//.ndo_init		= tunnel_init,
	//.ndo_uninit		= ipgre_tunnel_uninit,
	.ndo_open		= tunnel_open,
	.ndo_stop		= tunnel_close,
	.ndo_start_xmit		= public4over6_tunnel_xmit,
	.ndo_do_ioctl		= public4over6_ioctl,
	.ndo_change_mtu		= public4over6_change_mtu,
};

//===============CZW============

static void kernel_receive(struct sk_buff *__skb)
{
    do {
        struct sk_buff *skb;
        if(down_trylock(&receive_sem))
            return;
        if((skb = skb_get(__skb)) != NULL) {
            
                struct nlmsghdr *nlh = NULL;
	            if(skb->len >= sizeof(struct nlmsghdr)) {
                    nlh = (struct nlmsghdr *)skb->data;
                    if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
                       && (skb->len >= nlh->nlmsg_len)) {
	                    switch(nlh->nlmsg_type) {
                        case LINK_REQUEST_INFO:
                            CDBG("[public 4over6 tunnel]:LINK_REQUEST_INFO");
                            CDBG("nlh_pid: %d user_pid %d",nlh->nlmsg_pid,user_proc.pid);
                            //write_lock_bh(&user_proc.pid);
                            user_proc.pid = nlh->nlmsg_pid;
                            //write_unlock_bh(&user_proc.pid);
                            send_to_user();
                            break;
                        case LINK_TC_ADDRESS:
                            CDBG("[public 4over6 tunnel]:BBAA----LINK_TC_ADDRESS");
                            user_proc.pid = nlh->nlmsg_pid;
                            TC_V6_ADDRESS = *(struct in6_addr *)NLMSG_DATA(nlh);
		            CDBG("[public 4over6 tunnel]:nlh_len: %d",nlh->nlmsg_len);
			    char sss[40];
                            inet_ntop_v6(TC_V6_ADDRESS, sss);
                            CDBG("[public 4over6 tunnel]:tc_v6_address: %s",sss);
                            break;
                        case LINK_TI_ADDRESS:  
                            CDBG("[public 4over6 tunnel]:AABB-----LINK_TI_ADDRESS");
                            user_proc.pid = nlh->nlmsg_pid;
                            TI_V6_ADDRESS = *(struct in6_addr *)NLMSG_DATA(nlh);
                            break;
                        case LINK_STOP:
                            if(nlh->nlmsg_pid == user_proc.pid)
                                user_proc.pid = 0;
                            break;
                        case LINK_DHCP_ADDRESS:
                            CDBG("[public 4over6 tunnel]:----AAA---LINK_DHCP_ADDRESS");
                            user_proc.pid = nlh->nlmsg_pid;
                            dhcp_v4_address = *(struct in_addr *)NLMSG_DATA(nlh);
                            char ssss[40];
                            inet_ntoa(dhcp_v4_address, ssss, sizeof(ssss));
                            CDBG("[public 4over6 tunnel]:tc_v6_address: %s",ssss);
                            break;
                        default:
                            CDBG("[public 4over6 tunnel]:netlink received wrong type!");
                            break;
			            }
		            }
	            }
            kfree_skb(skb);
        }
        up(&receive_sem);
    } while(nlfd && nlfd->sk_receive_queue.qlen);
}

static int send_to_user()
{
    int ret;
    int size;
    unsigned char* old_tail;
    struct sk_buff *skb;
    struct nlmsghdr *nlh;

CDBG("In send_to_user.Ready to send\n");
        struct TunnelInfo *tInfo;
        size = NLMSG_SPACE(sizeof(*tInfo));
        skb = alloc_skb(size, GFP_ATOMIC);
        old_tail = skb->tail;
        nlh = NLMSG_PUT(skb, 0, 0, LINK_K_MSG, size-sizeof(*nlh));
        tInfo = NLMSG_DATA(nlh);
        memset(tInfo, 0, sizeof(struct TunnelInfo));
        tInfo->v4_addr = dhcp_v4_address;
        inet_pton_v6("2001:da8:20d:27:7a2b:cbff:fe1b:6ce0",&(tInfo->v6_addr));/*TODO how to get tunnel v6 address*/
        tInfo->in_traffic = traffic_in;/*TODO how to get traffic*/
        tInfo->out_traffic = traffic_out;/*TODO how to get traffic*/
CDBG("Send to user,finish fill skb\n");

    nlh->nlmsg_len = skb->tail - old_tail;
    NETLINK_CB(skb).dst_group = 0;

    read_lock_bh(&user_proc.lock);
    ret = netlink_unicast(nlfd, skb, user_proc.pid, MSG_DONTWAIT);
    read_unlock_bh(&user_proc.lock);
CDBG("send to user,unicast finish.PID is %d\n",user_proc.pid);
    return ret;

nlmsg_failure:
    if(skb)
        kfree_skb(skb);
    return -1;
}

int init_netlink() 
{
    rwlock_init(&user_proc.lock);
    nlfd = netlink_kernel_create(&init_net, NL_PORT, 0, kernel_receive, NULL, THIS_MODULE);
    if(!nlfd)
    {
      printk("can not create a netlink socket\n");
      return -1;
    }
    return 1;
}

//===============CZW============

//Module initialize
static int __init public4over6_init(void)
{
  int err;
  netdev = alloc_netdev( sizeof(struct tunnel_private),TUNNEL_DEVICE_NAME,public4over6_setup);
    strcpy(netdev->name,TUNNEL_DEVICE_NAME);
    memset(netdev_priv(netdev), 0, sizeof(struct tunnel_private));
    if((err=register_netdev(netdev)))//register the device
    {
       CDBG("[public 4over6 tunnel]:Can't register the device %s,error number is %i.\n",TUNNEL_DEVICE_NAME,err);
       return -EIO;
    }
    else
    {
        
    }
    if(inet6_add_protocol( &ip6ip_protocol,IPPROTO_IPIP))//register the 4over6 protocol in IPv6 level.
    {
       CDBG("[public 4over6 tunnel]:registered the 4over6 protocol!\n" );
    }
    else 
    {
       CDBG("[public 4over6 tunnel]:can't register the 4over6 protocol!\n");
       unregister_netdev(netdev);
       return -1;  
    }

//------------------------CZW---------------------------
    if(-1 == init_netlink())
    {
       CDBG("[public 4over6 tunnel]:can't init netlink!\n");
    }
//------------------------CZW---------------------------

//------------------------Calculate Traffic--------------
	traffic_in = 0;
	traffic_out = 0;
//------------------------END----------------------------

    CDBG("[public 4over6 tunnel]:initialized.\n");
    return 0;
}

static void public4over6_setup(struct net_device *dev)
{   
    //dev->uninit = tunnel_uninit;
    dev->flags|= IFF_NOARP|IFF_BROADCAST;
    dev->netdev_ops = &ip6ip_netdev_ops;
    dev->addr_len = 6;//ethernet mac address
    //dev->neigh_setup = tunnel_neigh_setup_dev;
    dev->destructor = free_netdev;
    //dev->iflink = 0;
    dev->features|= NETIF_F_NETNS_LOCAL; 
    dev->type=ARPHRD_TUNNEL6;
    dev->hard_header_len=14; //  dev->hard_header_len = 14;
    dev->mtu= ETH_DATA_LEN-sizeof(struct ipv6hdr); //dev->mtu = 1500 - sizeof( struct ipv6hdr );
    generate_random_hw(dev);//generate a random hardware address.

}
 

void cleanup_module( void )
{
  sock_release(nlfd->sk_socket);
  unregister_netdev(netdev);
  if (inet6_del_protocol( &ip6ip_protocol,IPPROTO_IPIP)<0)
  {
     CDBG("[public 4over6 tunnel]:Can't register the 4over6 protocol!\n");
  }
}

module_init(public4over6_init);
MODULE_LICENSE("Dual BSD/GPL");


