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


/*---------------------------------------------------------------------------*
 * Description:
 *    system startup
 *       IP stack -> Switch -> NI -> Drv
 *---------------------------------------------------------------------------*/
#ifndef X_NET_NI_IF_H
#define X_NET_NI_IF_H

#include "u_net_drv_if.h"
#include "u_net_ni_if.h"

/**
 * Layer 2 switch
 */
extern L2_SWITCH_T *x_net_switch_new    (CHAR *ps_name);
extern INT32        x_net_switch_add_default_ni  (L2_SWITCH_T *pt_Switch, INT32 i4_NiHandle);
extern INT32        x_net_switch_add_ni          (L2_SWITCH_T *pt_Switch, INT32 i4_NiHandle);
extern INT32        x_net_switch_add_learn_entry (L2_SWITCH_T *pt_Switch, UINT8 *pac_Mac, INT32 i4_NiHandle);
extern INT32        x_net_swtich_find_learn_entry(L2_SWITCH_T *pt_Switch, UINT8 *pac_Mac);
extern INT32        x_net_switch_transmit_packet(L2_SWITCH_T *pt_Switch, PKT_BUFF_T *pt_packet, INT32 i4_NiHandle);

/**
 * Network interface utility
 */
extern NET_NI_T    *x_net_ni_new(CHAR *ps_name, INT32 i4_ifType);
extern INT32        x_net_ni_install_drv_entry(NET_NI_T *pt_ni, NetworkDriverEntry pf_fn);
extern INT32        x_net_ni_start(NET_NI_T *pt_ni);
extern INT32        x_net_ni_stop(NET_NI_T *pt_ni);
extern INT32        x_net_ni_install_transmit_packet_callback(NET_NI_T *pt_ni, net_ni_transmit_pkt pf_fn);
extern INT32        x_net_ni_install_receive_packet_callback(NET_NI_T *pt_ni, net_ni_receive_pkt pf_fn);
extern INT32        x_net_ni_packet_indication(NET_DRV_IF_T *pt_drv, PKT_BUFF_T *pt_pkb);
extern INT32        x_net_ni_tx_completed(NET_DRV_IF_T *pt_drv, PKT_BUFF_T *pt_pkb);
extern INT32        x_net_ni_rx_packet(NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff);
extern INT32        x_net_ni_tx_packet(NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff);
extern NET_NI_T    *x_net_ni_get_by_name(CHAR *ps_name);
extern VOID         x_net_ni_list_reset(VOID);
extern INT32        x_net_ni_set_mac(CHAR *psz_name, CHAR *pc_mac);
extern INT32        x_net_ni_get_mac(CHAR *psz_name, CHAR *pac_mac);
extern INT32        x_net_ni_set_if_type(NET_NI_T *pt_ni, INT32 i4_type);
extern NET_NI_T    *x_net_ni_list_get(VOID);
extern INT32        x_net_ni_add_assocate_ni(NET_NI_T *pt_ni1, NET_NI_T *pt_ni2);
extern VOID         x_net_ping(INT32 i4_ip_version, UINT32 ui4_src_ip, UINT32 ui4_dest_ip, INT16 i2_try_cnt, INT32 i4_size, INT16 i2_interval);
extern VOID         x_net_ni_show_by_name(CHAR *psz_name);
extern VOID         x_net_ni_show_all(VOID);
extern VOID         x_net_ping4 (CHAR * ps_ip, INT32  i4_len, INT16  i2_wait, VOID (*notify_fn) (INT16 i2_rsp_time));
extern VOID         x_net_ni_debug(VOID);
extern INT32        x_net_ni_add_pkt_hook(CHAR *ps_niName, INT32 i4_priority, net_pkt_hook_fn fn);

/* if information get */
extern INT32        x_net_ni_get(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param);
extern INT32        x_net_ni_set(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param);

extern INT32        x_net_ni_reg_ev_notify(CHAR *ps_ni_name, NI_DRV_EV_T e_ev, ni_ev_notify_fnct pf_notify);
extern INT32        x_net_ni_unreg_ev_notify(CHAR *ps_ni_name, NI_DRV_EV_T e_ev, ni_ev_notify_fnct pf_notify);
extern INT32        x_net_ni_ev_send(NET_NI_T *pt_ni, NI_DRV_EV_T e_ev);

/**
 * x_net_if_join
 *
 * Add multicast IP address to interface
 *
 * @param ps_niName
 * @param ui4_ip, the IP address (network order)
 *
 * @return INT32
 * NET_OK, ok
 * NET_NI_NOT_EXIST, ni not exist
 */
extern INT32 x_net_if_join(const CHAR* ps_niName, UINT32 ui4_ip);

/**
 * x_net_if_leave
 *
 * leave a multicast group
 *
 * @param ps_niName
 * @param ui4_ip, ip address (network order)
 *
 * @return INT32
 */
extern INT32 x_net_if_leave(const CHAR* ps_niName, UINT32 ui4_ip);

/**
 * x_net_if_multi_list
 *
 * get multicast list
 *
 * @param ps_niName
 * @param paui4_ip, local buffer for multicast IP
 * @param i4_size, size of paui4_ip
 *
 * @return INT32
 * >0, the # set of multicast IP
 */
extern INT32 x_net_if_multi_list(const CHAR* ps_niName, UINT32 *paui4_ip, INT32 i4_size);

/**
 * x_net_get_mac_by_ip
 *
 * get MAC by IP address
 *
 * @param ps_ip, IP address string
 * @param puac_mac, output, need allocate by users. UCHAR mac[6]
 *
 * @return 0 OK, else failure.
 */
extern INT32 x_net_get_mac_by_ip (const CHAR *ps_ip, UCHAR *puac_mac);

#endif


