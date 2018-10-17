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


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <linux/wireless.h>
#include <netinet/ip.h>       
#include <linux/if_packet.h>
#include <pthread.h>

#define IFNAME  "wlan0"
#define SNIFFER "radiotap0"
#define IOCTL_GET_DRIVER (SIOCIWFIRSTPRIV + 15)

static int sock_fd;
static int thread_flag;
static int g_channel = 1;
static char *ether_frame = NULL;
static int networkconfig_chn_scanning_period_ms = 400;

FILE *gfile = NULL;

struct pcap_pkthdr 
{
	int tv_sec;
	int tv_usec;
	int caplen;
	int len;
};

struct pcap_file_header 
{
	int   magic;
	short version_major;
	short version_minor;
	int   thiszone;   /* gmt to local correction */
	int   sigfigs;    /* accuracy of timestamps */
	int   snaplen;    /* max length saved portion of each pkt */
	int   linktype;   /* data link type (LINKTYPE_*) */
};

void c_net_dumpMemory8(char *buf, int len)
{
	printf("DUMP8 ADDRESS: %x, Length: %d\n", (int)buf, len);

	while (len > 0) 
	{
		if (len >= 16) 
		{
			printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				   buf[0], buf[1],buf[2], buf[3], buf[4], buf[5],buf[6], buf[7], 
				   buf[8], buf[9],buf[10], buf[11],buf[12],buf[13], buf[14], buf[15]);

			len -= 16;
			
			buf += 16;
		} 
		else 
		{
			switch (len) 
			{
				case 1:
				{
					printf("%02x\n", buf[0]);
				}
				break;
				
				case 2:
				{
					printf("%02x %02x\n", buf[0], buf[1]);
				}
				break;
				
				case 3:
				{
					printf("%02x %02x %02x\n",buf[0], buf[1], buf[2]);
				}
				break;
				
				case 4:
				{
					printf("%02x %02x %02x %02x\n",buf[0], buf[1], buf[2], buf[3]);
				}
				break;
				
				case 5:
				{
					printf("%02x %02x %02x %02x %02x\n",buf[0], buf[1], buf[2],buf[3],buf[4]);
				}
				break;
				
				case 6:
				{
					printf("%02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2],buf[3], buf[4], buf[5]);
				}
				break;
				
				case 7:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6]);
				}
				break;
				
				case 8:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6], buf[7]);
				}
				break;
				
				case 9:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4],
						   buf[5], buf[6], buf[7],buf[8]);
				}
				break;
				
				case 10:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], 
						   buf[5], buf[6], buf[7], buf[8],buf[9]);
				}
				break;
				
				case 11:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], 
						   buf[6], buf[7], buf[8], buf[9],buf[10]);
				}
				break;
				
				case 12:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], 
						   buf[6], buf[7], buf[8], buf[9],buf[10],buf[11]);
				}
				break;
				
				case 13:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], buf[5],buf[6], 
						   buf[7], buf[8], buf[9], buf[10],buf[11],buf[12]);
				}
				break;
				
				case 14:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6], buf[7],
						   buf[8], buf[9], buf[10], buf[11],buf[12], buf[13]);
				}
				break;
				
				case 15:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						   buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6], buf[7],
						   buf[8], buf[9], buf[10], buf[11],buf[12], buf[13], buf[14]);
				}
				break;
				
				default:
					break;
			}
			
			len = 0;
		}
	}

	printf("\n");
}

void c_net_frame_save(FILE * f, char * frame, int len)
{
	struct pcap_pkthdr	hdr;
	memset(&hdr, 0, sizeof(hdr));
	hdr.caplen = hdr.len = len;
	fwrite(&hdr, sizeof(hdr), 1, f);	
	fwrite(frame, len, 1, f);
}

void c_net_config_80211_frame_handler(char *frame, int len , int link_type, int with_fcs)
{
#if 0/*add just for debug log  */
	c_net_dumpMemory8(frame, len);	
#endif
	c_net_frame_save(gfile, frame, len);
}

int c_net_recv_80211_frame(void)
{
	int fd = sock_fd;
	int len, ret;
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = 0;
	tv.tv_usec = networkconfig_chn_scanning_period_ms*1000;

	ret = select(fd + 1, &rfds, NULL, NULL, &tv);
	if (ret <= 0) 
	{
		perror("c_net_recv_80211_frame() select:");
		return ret;
	}

	memset(ether_frame, 0, IP_MAXPACKET);
	len = recv(fd, ether_frame, IP_MAXPACKET, 0);
	if (len < 0) 
	{
		perror ("recv() failed:");//Something weird happened
		return -2;
	}

#if 1
	{
		/* mt6630: do not include 80211 FCS field(4 byte) */
		int with_fcs = 0;
		/* mt6630: link-type IEEE802_11_RADIO (802.11 plus radiotap header) */
		int link_type = 0;
		c_net_config_80211_frame_handler(ether_frame, len , link_type, with_fcs);
	}
#endif

	ret = 1;

	return ret;
}

int c_net_open_socket(void)
{
	int fd;
	struct ifreq ifr;
	int sockopt = 1;
	struct sockaddr_ll ll;
	
	/* create a raw socket that shall sniff */
	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(fd < 0) 
	{
		perror("create socket failed\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	
#if 0
	/* set interface to promiscuous mode */
	strncpy(ifr.ifr_name, IFNAME, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) 
	{
		perror("ioctl(SIOCGIFFLAGS)");
		goto exit;
	}
	ifr.ifr_flags |= IFF_PROMISC;
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) 
	{
		perror("ioctl(SIOCSIFFLAGS)");
		goto exit;
	}
#endif

#if 0
	/* allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) < 0) 
	{
		perror("setsockopt(SO_REUSEADDR)");
		goto exit;
	}
#endif

	/* bind to device */
	memset(&ll, 0, sizeof(ll));
	ll.sll_family = PF_PACKET;
	ll.sll_protocol = htons(ETH_P_ALL);
	//ll.sll_ifindex = if_nametoindex(IFNAME);
	ll.sll_ifindex = if_nametoindex(SNIFFER);
	if (bind(fd, (struct sockaddr *)&ll, sizeof(ll)) < 0) 
	{
		perror("bind[PF_PACKET] failed");
		goto exit;
	}

	return fd;
	
exit:
	close(fd);
	
	exit(EXIT_FAILURE);
}

void c_net_monitor_open(void)
{
	char data[512] = {0};
	struct iwreq wrq;
	struct ifreq ifr;
	int sock;
	int enable_mon = 0;
	int channel = 0;
	int channel_width = 0;
	int ext = 0;

	printf("++c_net_monitor_open++\n");
	/* 1 create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0 );
	if ( sock < 0 )
	{
		printf("c_net_monitor_open create socket failed %s\n" ,strerror(errno));
		return;
	}

	/* 2 set monitor parameters */
	enable_mon = 1;
	channel = 6;
	channel_width = 20;
	ext = 0;

	/* 3 set iwreq parameters to start monitor interface */
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, IFNAME, strlen(IFNAME));
	snprintf(data, sizeof(data), "MONITOR %d %d %d %d", 
			 enable_mon, channel, channel_width, ext);
	
	wrq.u.data.pointer = (void *) data;
	wrq.u.data.length = strlen(data)+1;
	wrq.u.data.flags = 0;

	printf("[DBG]wrq %s %d %d\n", wrq.u.data.pointer, wrq.u.data.length, wrq.u.data.flags);
	/* ioctl to driver monitor cmd */
	if (ioctl(sock, IOCTL_GET_DRIVER, &wrq) < 0)
	{
		printf("ioctl fail: %s(%d): %s\n", __FILE__, __LINE__, strerror(errno));
		goto exit;
	}

	/* ioctl to driver monitor interface up */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, SNIFFER, sizeof(ifr.ifr_name));
	ifr.ifr_flags = IFF_UP;
	if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
	{
		printf("ioctl fail: %s(%d): %s\n", __FILE__, __LINE__, strerror(errno));
		goto exit;
	}

	ether_frame = malloc(IP_MAXPACKET);
	if(ether_frame == NULL)
	{
		printf("malloc frame buffer fail!\n");
		goto exit;
	}

	sock_fd = c_net_open_socket();
	printf("c_net_open_socket sock_fd=%d\n", sock_fd);

exit:
	close(sock);
	printf("--c_net_monitor_open--\n");
}

void c_net_monitor_close(void)
{
	char data[512] = {0};
	struct iwreq wrq;
	int sock;
	int enable_mon = 0;
	int channel = 0;
	int channel_width = 0;
	int ext = 0;

	printf("++c_net_monitor_close++\n");	

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0 );
	if ( sock < 0 )
	{
		printf("c_net_monitor_close Create socket failed %s\n" ,strerror(errno));
		return;
	}

	/* set monitor parameters */
	enable_mon = 0;
	channel = 6;
	channel_width = 20;
	ext = 0;

	/* set iwreq parameters */
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, IFNAME, strlen (IFNAME));
	snprintf(data, sizeof(data), "MONITOR %d %d %d %d", enable_mon, channel, channel_width, ext);
	
	wrq.u.data.pointer = (void *) data;
	wrq.u.data.length = strlen(data)+1;
	wrq.u.data.flags = 0;

	printf("[wrq]%p %d %d\n", wrq.u.data.pointer, wrq.u.data.length, wrq.u.data.flags);
	/* ioctl to driver monitor cmd */
	if (ioctl(sock, IOCTL_GET_DRIVER, &wrq) < 0)
	{
		printf("ioctl fail: %s(%d): %s\n", __FILE__, __LINE__, strerror(errno));
		close(sock);
		printf("--c_net_monitor_close--\n");
		return;
	}

	close(sock);

	if(ether_frame != NULL) 
	{
		free(ether_frame);
		ether_frame = NULL;
	}
	
	close(sock_fd);

	printf("--c_net_monitor_close--\n");
}

void c_net_channel_switch(char primary_channel,char secondary_channel, char bssid[6])
{
	char data[512] = {0};
	struct iwreq wrq;
	struct ifreq ifr;
	int sock;
	int enable_mon = 0;
	int channel = 0;
	int channel_width = 0;
	int ext = 0;

	printf("++c_net_channel_switch++\n");
	/* 1 create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0 );
	if ( sock < 0 )
	{
		printf("c_net_channel_switch create socket failed %s\n" ,strerror(errno));
		return;
	}

	/* 2 set monitor parameters */
	enable_mon = 1;
	channel = primary_channel;
	channel_width = 20;
	ext = 0;

	/* 3 set iwreq parameters to start monitor interface */
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, IFNAME, strlen(IFNAME));
	snprintf(data, sizeof(data), "MONITOR %d %d %d %d", enable_mon, channel, channel_width, ext);
	
	wrq.u.data.pointer = (void *) data;
	wrq.u.data.length = strlen(data)+1;
	wrq.u.data.flags = 0;

	printf("[DBG]wrq %s %d %d\n", wrq.u.data.pointer, wrq.u.data.length, wrq.u.data.flags);
	/* ioctl to driver monitor cmd */
	if (ioctl(sock, IOCTL_GET_DRIVER, &wrq) < 0)
	{
		printf("ioctl fail: %s(%d): %s\n", __FILE__, __LINE__, strerror(errno));
		goto exit;
	}

exit:
	close(sock);
	printf("--c_net_channel_switch--\n");
}

void c_net_recv_packets(void)
{
	int ret;
	struct pcap_file_header file_hdr;

	memset(&file_hdr, 0, sizeof(file_hdr));
	file_hdr.magic = 0xa1b2c3d4;
	file_hdr.version_major = 2;
	file_hdr.version_minor = 4;
	file_hdr.snaplen = 0xFFFF;
	file_hdr.linktype = 0x7F;
	
	gfile = fopen("/tmp/mt66_snif.pcap", "w");
	fwrite(&file_hdr, sizeof(file_hdr), 1, gfile);
	
	printf("++c_net_recv_packets++\n");
	while(thread_flag)
	{
		ret = c_net_recv_80211_frame();
	}

	printf("--c_net_recv_packets--\n");
	fclose(gfile);
}

void c_net_create_packet_thread(void)
{
	pthread_t thread_id;
	pthread_attr_t attr;
	int ret;

	printf("++c_net_create_packet_thread++\n");
	
	thread_flag = 1;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&thread_id, &attr, (void*)c_net_recv_packets, NULL);
	if(ret != 0)
	{
		printf("[err] create thread error!\n");
		return;
	}
	//pthread_exit(NULL);
	printf("--c_net_create_packet_thread--\n");
}

#if 1 /** The main for test !**/
int main(int argc,char **argv)
{
	int goterr = 0;
	char cmd[10] = {0};
	
	printf("++main++\n");
	while(1) 
	{
		printf("support cmd:");
		memset(cmd, 0, 10);
		scanf("%s", cmd);
		printf("\n input cmd=%s\n", cmd);
		if(!strcmp(cmd, "open"))
		{
			c_net_monitor_open();
		}
		else if(!strcmp(cmd, "close")) 
		{
			c_net_monitor_close();
		}
		else if(!strcmp(cmd, "switch")) 
		{
			if(g_channel > 13)
			{
				g_channel = 1;
			}
			printf("g_channel = %d\n", g_channel);
			
			c_net_channel_switch(g_channel, 0, NULL);
			
			g_channel++;
		}
		else if(!strcmp(cmd, "pkt_on")) 
		{
			c_net_create_packet_thread();
		}
		else if(!strcmp(cmd, "pkt_off")) 
		{
			thread_flag = 0;
		}
		else if(!strcmp(cmd, "exit")) 
		{
			break;
		}
	}
	
	printf("--main--\n");
	
	return(goterr);
	
}
#else
int main(int argc,char **argv)
{
	int goterr = 0;

	printf("++main++\n");
	if(argc == 1) 
	{
		printf("No argument!\n");
	} 
	else 
	{
		if(!strcmp(argv[1], "open"))
		{
			c_net_monitor_open();
		}
		else if(!strcmp(argv[1], "close")) 
		{
			c_net_monitor_close();
		}
		else if(!strcmp(argv[1], "switch")) 
		{
			
		}
		else if(!strcmp(argv[1], "pkt_on"))
		{
			c_net_create_packet_thread();
		}
		else if(!strcmp(argv[1], "pkt_off"))
		{
			thread_flag = 0;
		}
	}

	printf("--main--\n");
	
	return(goterr);
}
#endif
