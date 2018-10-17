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


#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>     /* for struct ifreq */
#include <net/if_arp.h> /* for ARPHRD_ETHER */
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

//#include "common.h"
//#include "x_os.h"
#include "u_common.h"
#include "u_net_common.h"
//#include "u_net_drv_if.h"
#include "u_net_ni_if.h"
#include "u_net_oid.h"   // for OID

//#include "utility.h"
//#include "request.h"
#include "netinfd.h"


extern INT32 x_net_ni_enable(CHAR *psz_name, INT8 u1EnableNi);

INT32 x_net_ni_enable(CHAR *psz_name, INT8 u1EnableNi)
{
  char *argp [1];
  if (u1EnableNi == 0)
  {
    argp [0] = (char *)"n";
  }
  else
  {
    argp [0] = (char *)"y";
  }
  if (netinfd_nienable((char *)psz_name, argp [0]) < 0)
  {
    return NET_FAILED;
  }
  return NET_OK;
}


INT32 x_net_ni_get(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param)
{
    switch (i4_cmd)
    {
    case OID_CMN_IF_MULTICASTADDRESS: /* param : MAC_ADDRESS_T */
        /* not support in get API, use OID_CMN_IF_MULTICASTADDRESSLIST */
        return NET_INV_ARG;
    case OID_CMN_IF_OPERSTATUS: /* param : UINT32 */
    case OID_CMN_IF_CONNECT_STATUS: /* param : UINT32 */
        {
          int sockfd;
          struct ifreq ifr;
          UINT32 u4Status;

          sockfd = socket(AF_INET, SOCK_DGRAM, 0);
          bzero(&ifr, sizeof(ifr));
          strncpy(ifr.ifr_name, ps_niName, 16);

          if (ioctl(sockfd, SIOCGIFFLAGS, &ifr)<0)
          {
              printf("ioctl flag failed\n");
              if (OID_CMN_IF_CONNECT_STATUS == i4_cmd)
              {
                u4Status = NI_DRV_EV_UNKNOW;
                memcpy((CHAR*) pv_param, (CHAR *)&u4Status, sizeof (UINT32));
              }
              else if (OID_CMN_IF_OPERSTATUS == i4_cmd)
              {
                u4Status = NET_IF_STATE_UNKNOWN;
                memcpy((CHAR*) pv_param, (CHAR *)&u4Status, sizeof (UINT32));
              }
              close (sockfd);
              return NET_FAILED;
          }

          if (ifr.ifr_flags & IFF_RUNNING) /* interface running and carrier ok */
          {
              if (OID_CMN_IF_CONNECT_STATUS == i4_cmd)
              {
                u4Status = NI_DRV_EV_ETHERNET_PLUGIN;
                memcpy((CHAR*) pv_param, (CHAR *)&u4Status, sizeof (UINT32));
              }
              else if (OID_CMN_IF_OPERSTATUS == i4_cmd)
              {
                u4Status = NET_IF_STATE_UP;
                memcpy((CHAR*) pv_param, (CHAR *)&u4Status, sizeof (UINT32));
              }
              printf("%s is plug!\n", ps_niName);
          }
          else
          {
              if (OID_CMN_IF_CONNECT_STATUS == i4_cmd)
              {
                u4Status = NI_DRV_EV_ETHERNET_UNPLUG;
                memcpy((CHAR*) pv_param, (CHAR *)&u4Status, sizeof (UINT32));
              }
              else if (OID_CMN_IF_OPERSTATUS == i4_cmd)
              {
                u4Status = NET_IF_STATE_DOWN;
                memcpy((CHAR*) pv_param, (CHAR *)&u4Status, sizeof (UINT32));
              }
              printf("%s is unplug!\n", ps_niName);
          }

          close (sockfd);
        }
        break;
    case OID_CMN_IF_ADMIN_STATUS: /* param : UINT32 */
        /* always return up */
        {
            UINT32 u4_statue = NET_IF_STATE_UP;
            memcpy((CHAR*) pv_param, (VOID *) &u4_statue, sizeof (UINT32));
        }
        break;
    case OID_CMN_IF_PHYADDRESS: /* param : NET_PHY_ADDRESS_T */
        {
            int sockfd;
            struct ifreq ifr;
            NET_PHY_ADDRESS_T *pPhyAddr = (NET_PHY_ADDRESS_T *)pv_param;
            MAC_ADDRESS_T *pMacAddr = &(pPhyAddr->t_MacAddress);

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            bzero(&ifr, sizeof(ifr));
            strncpy(ifr.ifr_name, ps_niName, 16);

            if (ioctl(sockfd, SIOCGIFHWADDR, &ifr)<0)
            {
                printf("Connect failed\n");
                close (sockfd);
                return NET_FAILED;
            }
            close (sockfd);
            memcpy(pMacAddr, ifr.ifr_hwaddr.sa_data, sizeof(MAC_ADDRESS_T));
        }
        break;
    case OID_CMN_IF_CURRNET_SPEED_INFO:
        {
            int sockfd;
            struct ifreq ifr;

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            bzero(&ifr, sizeof(ifr));
            strncpy(ifr.ifr_name, ps_niName, 16);

            if (ioctl(sockfd, SIOCDEVPRIVATE+7, &ifr)<0)
            {
                printf("Connect failed\n");
                close (sockfd);
                return NET_FAILED;
            }
            close (sockfd);
            memcpy(pv_param, ifr.ifr_data, sizeof(UINT32));
        }
        break;
    case OID_CMN_IF_INDEX:
    case OID_CMN_IF_DESCR: /* param : CHAR[256] */
    case OID_CMN_IF_TYPE:  /* param : UINT32 */
    case OID_CMN_IF_MTU:   /* param : UINT32 */
    case OID_CMN_IF_SPEED: /* param : UINT32 */
    case OID_CMN_IF_MULTICASTADDRESSLIST:  /* param : NET_MAC_ADDRESS_LIST_T */
    case OID_CMN_IF_LASTCHANGE: /* param : UINT32 */
    case OID_CMN_IF_NAME: /* param : CHAR[256] */
    case OID_CMN_IF_IP_ADDRESS: /* parm: UINT32 */
    case OID_CMN_IF_PROMISCUOUS_MODE: /* param : UINT32 */
    case OID_CMN_IF_IN_OCTETS:  /* param : UINT64 */
    case OID_CMN_IF_IN_UCASTPKT: /* param : UINT64 */
    case OID_CMN_IF_IN_DISCARDS: /* param : UINT64 */
    case OID_CMN_IF_IN_ERRORS: /* param : UINT64 */
    case OID_CMN_IF_IN_UNKNOWN_PROTOS: /* param : UINT64 */
    case OID_CMN_IF_OUT_OCTETS: /* param : UINT64 */
    case OID_CMN_IF_OUT_UCASTPKTS: /* param : UINT64 */
    case OID_CMN_IF_OUT_DISCARDS: /* param : UINT64 */
    case OID_CMN_IF_OUT_ERRORS: /* param : UINT64 */
    case OID_CMN_IF_IN_MULTICAST_PKTS: /* param : UINT64 */
    case OID_CMN_IF_IN_BROADCAST_PKTS: /* param : UINT64 */
    case OID_CMN_IF_OUT_MULTICAST_PKTS: /* param : UINT64 */
    case OID_CMN_IF_OUT_BROADCAST_PKTS: /* param : UINT64 */
    default:
        return NET_OK;
    }

    return NET_OK;
}

INT32 x_net_ni_set(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param)
{
    switch (i4_cmd)
    {
    case OID_CMN_IF_PHYADDRESS: /* param : NET_PHY_ADDRESS_T */
        {
            int sockfd;
            struct ifreq ifr;
            NET_PHY_ADDRESS_T *pPhyAddr = (NET_PHY_ADDRESS_T *)pv_param;
           // x_net_ni_enable(ps_niName, 0);

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            bzero(&ifr, sizeof(ifr));
            strncpy(ifr.ifr_name, ps_niName, 16);

            memcpy(&ifr.ifr_hwaddr.sa_data, &(pPhyAddr->t_MacAddress), sizeof(MAC_ADDRESS_T));
            ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

            if (ioctl(sockfd, SIOCSIFHWADDR, &ifr)<0)
            {
                printf("Connect failed\n");
                 if (x_net_ni_enable(ps_niName, 1) < 0)
                 {
                   close (sockfd);
                   return NET_FAILED;
                 }
                close (sockfd);
                return NET_FAILED;
            }

            if (x_net_ni_enable(ps_niName, 1) < 0)
            {
              printf("%s enable failed\n", ps_niName);
              close (sockfd);
              return NET_FAILED;
            }

            close (sockfd);
        }
        break;
    case OID_CMN_IF_INDEX:
    case OID_CMN_IF_DESCR: /* param : CHAR[256] */
    case OID_CMN_IF_TYPE:  /* param : UINT32 */
    case OID_CMN_IF_MTU:   /* param : UINT32 */
    case OID_CMN_IF_SPEED: /* param : UINT32 */
    case OID_CMN_IF_MULTICASTADDRESS: /* param : MAC_ADDRESS_T */
    case OID_CMN_IF_MULTICASTADDRESSLIST:
    case OID_CMN_IF_MULTICASTADDRESS_DEL:
    case OID_CMN_IF_ADMIN_STATUS: /* param : UINT32 */
        break;
        /**
         *  read-only
         *  case OID_CMN_IF_OPERSTATUS:
         *  break;
         */
    case OID_CMN_IF_LASTCHANGE: /* param : UINT32 */
    case OID_CMN_IF_NAME: /* param : CHAR[256] */
    case OID_CMN_IF_IN_OCTETS:  /* param : UINT64 */
    case OID_CMN_IF_IN_UCASTPKT: /* param : UINT64 */
    case OID_CMN_IF_IN_DISCARDS: /* param : UINT64 */
    case OID_CMN_IF_IN_ERRORS: /* param : UINT64 */
    case OID_CMN_IF_IN_UNKNOWN_PROTOS: /* param : UINT64 */
    case OID_CMN_IF_OUT_OCTETS: /* param : UINT64 */
    case OID_CMN_IF_OUT_UCASTPKTS: /* param : UINT64 */
    case OID_CMN_IF_OUT_DISCARDS: /* param : UINT64 */
    case OID_CMN_IF_OUT_ERRORS: /* param : UINT64 */
    case OID_CMN_IF_IN_MULTICAST_PKTS: /* param : UINT64 */
    case OID_CMN_IF_IN_BROADCAST_PKTS: /* param : UINT64 */
    case OID_CMN_IF_OUT_MULTICAST_PKTS: /* param : UINT64 */
    case OID_CMN_IF_OUT_BROADCAST_PKTS: /* param : UINT64 */
        return NET_INV_ARG;

    default:
        return NET_OK;
    }

    return NET_OK;
}

