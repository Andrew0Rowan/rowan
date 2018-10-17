/* Copyright Statement:                                                        
 *                                                                             
 * This software/firmware and related documentation ("MediaTek Software") are  
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any     
 * reproduction, modification, use or disclosure of MediaTek Software, and     
 * information contained herein, in whole or in part, shall be strictly        
 * prohibited.                                                                 
 *                                                                             
 * MediaTek Inc. (C) 2014. All rights reserved.                                
 *                                                                             
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S 
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */



#include <linux/file.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <net/net_namespace.h>
#include <linux/syscalls.h>
#include <linux/time.h>

#include "x_module.h"

#define NETLINK_TEST 21

static struct nf_hook_ops nfho;
struct sock *nl_sk = NULL;
EXPORT_SYMBOL_GPL(nl_sk);

	
typedef struct {
	int icmp_off;
	int drop_ip;

}OWN;



static int icmp_off = 0;
static unsigned int drop_ip = 0;


/*From input nl sk->skb, get icmp_off & drop_ip value. */
void input (struct sk_buff* skb)
{ 
	struct nlmsghdr* nlh = NULL;

	nlh = (struct nlmsghdr*)skb->data;
	icmp_off = ((OWN *)NLMSG_DATA(nlh))->icmp_off;
	drop_ip = ((OWN *)NLMSG_DATA(nlh))->drop_ip;

	return;
}


/*create a netlink socket, with "input" as recv-entry*/
static int test_netlink(void) {
  nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, 0, input,NULL, THIS_MODULE);

  if (!nl_sk) {
    printk(KERN_ERR "net_link: Cannot create netlink socket.\n");
    return -EIO;
  }
  printk("net_link: create socket ok.\n");
  return 0;
}

static const int session_size = 1000;
static int IsSessionStart = 0;

static inline int _SessionCmp(int seq_id, int session_id)
{
	if(seq_id < session_id *session_size)
		return -1;
	if(seq_id >= (session_id+1)*session_size)
		return 1;
	return 0;
}


void _handle_wfd(char* pData)
{
	short seq_id = *((short*)pData);
	static int session_id = 0;
	static int pkt_num = 0;
	static int sum_time = {0};
	struct timeval this_time = {0};


	/*We all start from session-0, maybe redundant*/
	if(!IsSessionStart)
	{
		if(!_SessionCmp(seq_id,0))
			IsSessionStart = 1;
		else
			return;
	}
	
#ifdef DEV_DEBUG
	printk(KERN_ERR "<UDP_dbg>dst 3,4 bytes = %d\n", seq_id);
#endif
	/*If pkt in session, do stastics*/
	if(!_SessionCmp(seq_id, session_id))
	{
		do_gettimeofday(&this_time);
		sum_time += (this_time.tv_sec%100)*1000000 + this_time.tv_usec;		
	}
	else /*Jump to new session*/
	{
		printk(KERN_ERR "<wfd - statstic>Session_%d--Pkt_%d--Time_%d--Avrg_%d \n", session_id, pkt_num, sum_time, sum_time/pkt_num);
		session_id = seq_id / session_size;
		pkt_num = 0;
		sum_time = 0;		
	}
	return;
}



/*netfilter hook*/
unsigned int hook_func(unsigned int hooknum,
                       struct sk_buff *skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn)(struct sk_buff *))
{
   struct iphdr     *iph ;

   iph = ip_hdr(skb);
   switch(iph->protocol)
    {
    #if 0
     case IPPROTO_ICMP:{
          struct icmphdr _icmph;
          struct icmphdr* ich;

         ich = skb_header_pointer(skb, iph->ihl*4, sizeof(_icmph), &_icmph);
         printk("icmp type %u\n", ich->type);
         if(icmp_off == 1)
          {
            printk("now we drop icmp from %d.%d.%d.%d\n", NIPQUAD(iph->saddr));
            return NF_DROP;
          }
         break;
       }
     case IPPROTO_TCP:{
         struct tcphdr* th = NULL;
         struct tcphdr _tcph;
         th = skb_header_pointer(skb, iph->ihl*4, sizeof(_tcph), &_tcph);
         if(th == NULL)
          {
            printk("get tcp header error\n");
            return NF_DROP;
          }
         //unsigned int sip = ntohs(th->source);
         //printk("saddr:%d.%d.%d.%d,sport:%u\n", NIPQUAD(iph->saddr),ntohs(th->source));
         //printk("daddr:%d.%d.%d.%d,dport:%u\n", NIPQUAD(iph->daddr),ntohs(th->dest));
         if(iph->saddr ==drop_ip)
          {
            printk("now we drop tcp from %d.%d.%d.%d\n", NIPQUAD(iph->saddr));
             return NF_DROP;
          }
         break;
       }
	 #endif
	case IPPROTO_UDP:{
		struct udphdr *uh = NULL;
		short * data = NULL;


		uh = (struct udphdr *)((char*)iph + iph->ihl*4);
		data  = (short *)((char*)uh + sizeof(struct udphdr));
		
		//printk(KERN_ERR "dst port = %d, src port = %d\n", ntohs(uh->dest), ntohs(uh->source));
		if(ntohs(uh->dest) == 1010 || ntohs(uh->source) == 1010)
			_handle_wfd((char*)data);


		
	/*	{
			struct file* fp = NULL;
			static loff_t pos = 0;
			mm_segment_t old_fs;

 			if(!fileInit)
 			{
				fp = filp_open("/var/net_dbg_udp", O_CREAT, 0);
				filp_close(fp, NULL);
				fileInit = 1;
			}
				
			fp = filp_open("/var/net_dbg_udp", O_RDWR, 0);
			old_fs = get_fs();
			set_fs(KERNEL_DS);
			vfs_write(fp, buff, 25, &pos);
			set_fs(old_fs);
			
			pos += 25;
			filp_close(fp, NULL);

		}*/
	}
	
     default:
         break;
    } 
  return NF_ACCEPT;
}


/*Hook in Local-in point*/
static int __init netdbg_init(void)
{
       printk("insmod hook test!\n");
       test_netlink();
       nfho.hook      = hook_func;
       nfho.hooknum   = NF_INET_LOCAL_IN;
       nfho.pf        = PF_INET;
       nfho.priority  = NF_IP_PRI_FIRST;
   
       nf_register_hook(&nfho);

       return 0;
}

static void __exit netdbg_exit(void)
{
    printk("rmmod hook test!\n");
    nf_unregister_hook(&nfho);
    if (nl_sk != NULL){
      sock_release(nl_sk->sk_socket);
  }
}




EXPORT_SYMBOL(netdbg_init);
EXPORT_SYMBOL(netdbg_exit);
DECLARE_MODULE(netdbg);

