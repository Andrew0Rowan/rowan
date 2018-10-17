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


/*----------------------------------------------------------------------------*/
/*! @file u_net_wlan.h
 *  $RCSfile: $
 *  $Revision: #1 $
 *  $Date: 2015/12/08 $
 *  $Author: bdbm01 $
 *
 *  @par Description:
 *         This header file contains wifi definitions, which are exported
 */
/*----------------------------------------------------------------------------*/
#ifndef _U_NET_WLAN_H
#define _U_NET_WLAN_H

/*----------------------------------------------------------------------------*/
/*! @defgroup groupMW_INET_WIFI WIFI
 *  @ingroup groupMW_INET
 *  @brief The Svl builder module is the super module of other svl builder engine
 *  %MW Application API would be used by Application, lot of these APIs would
 *   be wrapped from Exported API. Using these APIs, application could create scan engine
 *   or start scan from the exported API of svl builder.
 *
 *  @see groupMW_INET
 *  @{
 */
/*----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define WLAN_SUB_VERSION     0x00         /**< Wlan FW version.*/

#define NET_WLAN_OK                 0     /**< Success. */
#define NET_WLAN_ERROR              -1    /**< Undefined error.*/
#define NET_WLAN_INVALID_PARAM      -2    /**< The parameter was incompatible to this software or invalid.*/
#define NET_WLAN_DEV_NOT_READY      -3    /**< Wlan driver/device is not ready.       */
#define NET_WLAN_NOT_INIT           -4    /**< Wlan module is not init.        */
#define NET_WLAN_ALREADY_INIT       -6    /**< Wlan module is already init.       */
#define NET_WLAN_NOT_STARTED        -7    /**< Wlan module is not started.       */
#define NET_WLAN_ALREADY_STARTED    -8    /**< Wlan module is already started.       */
#define NET_WLAN_DRV_INTERFACE_ERR  -9    /**< Wlan module function pointer is incorrect.       */
#define NET_WLAN_SUPPLICANT_STARTING  -10    /**< Wlan supplicant is starting     */


#define NET_WLAN_WPS_FAILED       (-1)    /**< Process is terminated because some error occurred or WPS process was cancelled by a user. */
#define NET_WLAN_WPS_IDLE          (0)    /**< Idle.    */
#define NET_WLAN_WPS_SEARCHING     (1)    /**< Searching a registrar.   */
#define NET_WLAN_WPS_REGISTERING   (2)    /**< Registering.  */
#define NET_WLAN_WPS_REGISTERED    (3)    /**< Registered.   */


#define NET_802_11_MAX_LEN_SSID     32    /**< maximum SSID length.       */
#define NET_802_11_MAX_LEN_BSSID    30
#define NET_802_11_MAX_LEN_SCAN_SSID  64  /**< maximum scan ssid length, twice of ssid*/
#define WLAN_KEY_MAX_LENGTH     (64)

#define NET_WPA_DATA_LENGTH         1024  /**< NET_802_11_WPA_T DATA length.       */
#define NET_NFY_DATA_LENGTH         256   /**< NET_802_11_NFY_T DATA length.       */
#define DRIVER_VERSION_LENGTH       30

#if PLATFORM_IS_BDP
#define BDP_CMD_PATH    "/var/run/bdp_cmd"
#define	BDP_NFY_PATH    "/var/run/bdp_nfy"
#define WPA_PROG_PATH   "/var/run/wpaprog"
#endif

/** \defgroup CAPINFO Defines for Capability IE definitions
 *  @{
 */
#define NET_WLAN_IEEE80211_CAP_ESS      0x0001
#define NET_WLAN_IEEE80211_CAP_IBSS     0x0002 /* Set when in Ad-hoc mode */
#define NET_WLAN_IEEE80211_CAP_PRIVACY  0x0010

#define ACFG_MAX_ANTENNA       3
#define ETHADDR_LEN    6
/**
 * @brief Type of BSSID data array.
 */
typedef UINT8 NET_802_11_BSSID_T [6]; /**< Type of BSSID data array.       */

typedef enum
{
    OID_802_11_A,
    OID_802_11_B,
    OID_802_11_G,
    OID_802_11_N,
    OID_802_11_B_G,
    OID_802_11_G_N,
    OID_802_11_B_G_N,
    OID_802_11_A_N,
    OID_802_11_A_B_G,
    OID_802_11_A_G_N,
    OID_802_11_A_B_G_N
} NET_802_11_T;

typedef enum
{
    OID_802_11_BW20,
    OID_802_11_BW40
} NET_802_11_BW_T;

#if CONFIG_MW_CUSTOM_JSN

typedef enum
{
    OID_802_11_START = 0x00040000,
    /* set OIDs */
    OID_802_11_SCAN,            /* param : NET_802_11_SCAN_T */
    OID_802_11_ASSOCIATE,       /* param : NET_802_11_ASSOCIATE_T */
    OID_802_11_DISASSOCIATE,    /* param : NULL */
    OID_802_11_SET_MAC_ADDR,    /* param : UINT8 */
    OID_802_11_WPS_ASSOCIATE,   /* param : NET_802_11_WPS_ASSOC_MODE_T */
    OID_802_11_WPS_REG_CB,      /* param : NONE */
    OID_802_11_WPS_UNREG_CB,    /* param : NONE */
    OID_802_11_WPS_CANCEL,      /* param : NONE */
    OID_802_11_WPS_REFRESH_PIN, /* param : NONE */
    /* get OIDs */
    OID_802_11_GET_SCAN_RESULT, /* param : NET_802_11_SCAN_RESULT_T */
    OID_802_11_GET_ESS_LIST,    /* param : NET_802_11_ESS_LIST_T */
    OID_802_11_GET_CURR_BSS,    /* param : NET_802_11_BSS_INFO_T */
    OID_802_11_GET_SIG_STRENGTH,/* param : UINT8 */
    OID_802_11_GET_MAC_ADDR,    /* param : UINT8 */
    OID_802_11_GET_EEPROM,      /* param : UINT32 */
    OID_802_11_WPA_GET_STATUS,  /* param : INT32 */
    OID_802_11_WPS_GET_PIN,     /* param : NONE */
    OID_802_11_WPS_GET_STATUS,  /* param : NONE */

    OID_802_11_WLAN_CHECK_INIT,/* param: NONE*/
    OID_802_11_WLAN_DUMMY_START,/* param : NONE */
    OID_802_11_WLAN_INIT,		/* param : NONE */
    OID_802_11_WLAN_DEINIT,		/* param : NONE */
    OID_802_11_WLAN_START,		/* param : NONE */
    OID_802_11_WLAN_STOP,		/* param : NONE */
    OID_802_11_WLAN_TASK_CREATE,/* param : NONE */
    OID_802_11_WLAN_PLUG_IN,	/* param : NONE */
    OID_802_11_WLAN_PLUG_OUT,	/* param : NONE */

    OID_802_11_GET_RSSI

}WLAN_OID_T;


#else

#if 1 //CONFIG_WIFI_MW_NEW_SUPPLICANT
typedef enum
{
    OID_802_11_START = 0x00040000,
    /* set OIDs */
    OID_802_11_SCAN,            /* param : NET_802_11_SCAN_T */
    OID_802_11_ASSOCIATE,       /* param : NET_802_11_ASSOCIATE_T */
    OID_802_11_DISASSOCIATE,    /* param : NULL */
    OID_802_11_SET_MAC_ADDR,    /* param : UINT8 */
    OID_802_11_WPS_ASSOCIATE,   /* param : NET_802_11_WPS_ASSOC_MODE_T */
    OID_802_11_WPS_REG_CB,      /* param : NONE */
    OID_802_11_WPS_UNREG_CB,    /* param : NONE */
    OID_802_11_WPS_CANCEL,      /* param : NONE */
    OID_802_11_WPS_REFRESH_PIN, /* param : NONE */
    /* get OIDs */
    OID_802_11_GET_SCAN_RESULT, /* param : NET_802_11_SCAN_RESULT_T */
    OID_802_11_GET_ESS_LIST,    /* param : NET_802_11_ESS_LIST_T */
    OID_802_11_GET_CURR_BSS,    /* param : NET_802_11_BSS_INFO_T */
    OID_802_11_GET_SIG_STRENGTH,/* param : UINT8 */
    OID_802_11_GET_MAC_ADDR,    /* param : UINT8 */
    OID_802_11_GET_EEPROM,      /* param : UINT32 */
    OID_802_11_WPA_GET_STATUS,  /* param : INT32 */
    OID_802_11_WPS_GET_PIN,     /* param : NONE */
    OID_802_11_WPS_GET_STATUS,  /* param : NONE */

    OID_802_11_WLAN_CHECK_INIT,/* param: NONE*/
    OID_802_11_WLAN_DUMMY_START,/* param : NONE */
    OID_802_11_WLAN_INIT,		/* param : NONE */
    OID_802_11_WLAN_DEINIT,		/* param : NONE */
    OID_802_11_WLAN_START,		/* param : NONE */
    OID_802_11_WLAN_STOP,		/* param : NONE */
    OID_802_11_WLAN_TASK_CREATE,/* param : NONE */
    OID_802_11_WLAN_PLUG_IN,	/* param : NONE */
    OID_802_11_WLAN_PLUG_OUT,	/* param : NONE */

    OID_802_11_GET_RSSI

}WLAN_OID_T;
#endif

#endif

/*------------------------------------------------------------------*/
/*! @brief Type of wlan callback fuction that notifies upper layer.
 *  @param [in]  i4CallbackId - callback ID
 *  @param [in]  pParam - parameter
 *  @return   results
 *  @retval   WLAN_NFY_MSG_OK   -  successful
 *  @retval   Others         -  failed
 */
/*------------------------------------------------------------------*/
typedef INT32 (*x_net_wlan_notify_fct)(INT32 i4CallbackId,
                                       VOID *pParam);

/*--------------------------------------------------------------*/
/*                                                              */
/* Enumeration                                                  */
/*                                                              */
/*--------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*! @enum WLAN_NOTIFY_ID_T
 *  @brief NotifyID Defines for notification types
 *  @code
 *  typedef enum
 *  {
 *      WLAN_NOTIFY_START_IND,
 *      WLAN_NOTIFY_STOP_IND,
 *      WLAN_NOTIFY_INIT_IND,
 *      WLAN_NOTIFY_DEINIT_IND,
 *      WLAN_NOTIFY_SCAN,
 *      WLAN_NOTIFY_ASSOCIATE,
 *      WLAN_NOTIFY_WPS_ASSOC_SETTING,
 *      WLAN_NOTIFY_WPS_STATE,
 *      WLAN_NOTIFY_WPS_ASSOCIATE
 *  }   WLAN_NOTIFY_ID_T;
 *  @endcode
 *  @li@c  WLAN_NOTIFY_START_IND                            - WLAN module start \ref NotifyMsg "indication
 *  @li@c  WLAN_NOTIFY_STOP_IND                              - WLAN module stopinit \ref NotifyMsg "indication
 *  @li@c  WLAN_NOTIFY_INIT_IND                               - WLAN module init \ref NotifyMsg "indication
 *  @li@c  WLAN_NOTIFY_DEINIT_IND                            - WLAN module deinit \ref NotifyMsg "indication"
 *  @li@c  WLAN_NOTIFY_SCAN                                    - Scan result notification \ref NotifyMsg "message"
 *  @li@c  WLAN_NOTIFY_ASSOCIATE                            - Association result notification \ref NotifyMsg "message"
 *  @li@c  WLAN_NOTIFY_WPS_ASSOC_SETTING             - WPS association notification \ref NotifyMsg "message"
 *  @li@c  WLAN_NOTIFY_WPS_STATE                           - WPS processing \ref WPSStatus "state"
 *  @li@c  WLAN_NOTIFY_WPS_ASSOCIATE                    - WPS Association result notification \ref NotifyMsg "message"
 */
/*------------------------------------------------------------------*/
typedef enum
{
    WLAN_NOTIFY_START_IND,
    WLAN_NOTIFY_STOP_IND,
    WLAN_NOTIFY_INIT_IND,
    WLAN_NOTIFY_DEINIT_IND,
    WLAN_NOTIFY_SCAN,
    WLAN_NOTIFY_ASSOCIATE,
    WLAN_NOTIFY_WPS_ASSOC_SETTING,
    WLAN_NOTIFY_WPS_STATE,
    WLAN_NOTIFY_WPS_ASSOCIATE,
    WLAN_NOTIFY_DEV_PLUG_IN,
    WLAN_NOTIFY_DEV_PLUG_OUT,
    WLAN_NOTIFY_DISCONNECTED,
    WLAN_NOTIFY_P2P_DEVICE_FOUND,
    WLAN_NOTIFY_P2P_GROUP_STARTED,
    WLAN_NOTIFY_P2P_GROUP_REMOVED,
    WLAN_NOTIFY_P2P_GO_NEG_REQUEST,
    WLAN_NOTIFY_P2P_LEGACY_CONNECT,
    WLAN_NOTIFY_P2P_LEGACY_DISCONNECT,
    WLAN_NOTIFY_DRIVER_START_IND,
    WLAN_NOTIFY_DRIVER_START_FAILED ,
    WLAN_NOTIFY_PROV_DISC_SHOW_PIN,
    WLAN_NOTIFY_PROV_DISC_ENTER_PIN,
    WLAN_NOTIFY_P2P_PROV_DISC_PBC_REQ,
    WLAN_NOTIFY_P2P_INVITATION_RECEIVED,
    WLAN_NOTIFY_P2P_ASSOC_TAB,
    WLAN_NOTIFY_P2P_PROV_DISC_RESP,
    WLAN_NOTIFY_DRIVER_VERSION,
    WLAN_NOTIFY_WFD_IP_GET,
    //mtk94097 Porting
    WLAN_NOTIFY_P2P_DEVICE_LOST ,
    WLAN_NOTIFY_P2P_GROUP_FORMATION_SUCCESS ,
    WLAN_NOTIFY_P2P_GROUP_FORMATION_FAILURE ,
    WLAN_NOTIFY_P2P_SUPP_START_IND,
    WLAN_NOTIFY_P2P_SUPP_START_FAILED,
    WLAN_NOTIFY_WFD_PIN_KEYPAD ,
    WLAN_NOTIFY_WFD_PIN_DISPLAY ,
    WLAN_NOTIFY_WFD_PBC ,
    WLAN_NOTIFY_P2P_GO_NEG_SUCCESS ,
    WLAN_NOTIFY_P2P_GO_NEG_FAILURE ,
    WLAN_NOTIFY_P2P_DHCP_SUCCESS_DHCPv4 ,
    WLAN_NOTIFY_P2P_DHCP_SUCCESS_LINKLOCAL ,
    WLAN_NOTIFY_P2P_DHCP_FAILURE_LINKLOCAL ,
    WLAN_NOTIFY_P2P_INVITATION_RESULT ,
    WLAN_NOTIFY_AP_STA_CONNECTED ,
    WLAN_NOTIFY_AP_STA_DISCONNECTED,
    WLAN_NOTIFY_P2P_WPS_SEND_M2D, //pin code error
    WLAN_NOTIFY_P2P_WPS_RECEIVE_NACK, //pin code error
	WLAN_NOTIFY_P2P_GO_NEGO_FAIL_INTENT,
	WLAN_NOTIFY_P2P_OPR_CH,
	WLAN_NOTIFY_SUPPLICANT_TERMINATING,
	WLAN_NOTIFY_P2P_GO_NEGO_FAIL_RECV_RESP,
	WLAN_NOTIFY_P2P_GO_NEGO_FAIL_RECV_CONFIRM,
	WLAN_NOTIFY_P2P_WPS_FAIL,
	WLAN_NOTIFY_P2P_WPS_TIMEOUT,
	WLAN_NOTIFY_P2P_WPS_2MINS_TIMEOUT,
	WLAN_NOTIFY_P2P_WPS_PBC_SESSION_OVERLAP,
	WLAN_NOTIFY_WPS_SUCCESS,
	WLAN_NOTIFY_P2P_STOP_CONNECT,
	WLAN_NOTIFY_P2P_DEAUTH_AP,
	WLAN_NOTIFY_P2P_PROV_DISC_PBC_RESP,
	WLAN_NOTIFY_CANCEL_CONNECT,
	WLAN_NOTIFY_P2P_FREQ_CONFLICT,
	//mtk94097
	/* =============== TDLS Notifications ============== */
    WLAN_NOTIFY_TDLS_DISCOVERY_REQ,
    WLAN_NOTIFY_TDLS_DISCOVERY_RESP,
    WLAN_NOTIFY_TDLS_SETUP_REQ,
    WLAN_NOTIFY_TDLS_SETUP_RESP,
    WLAN_NOTIFY_TDLS_SETUP_CONFIRM,
    WLAN_NOTIFY_TDLS_PEER_CONNECTED,
    WLAN_NOTIFY_TDLS_PEER_DISCONNECTED,
    WLAN_NOTIFY_TDLS_TEARDOWN,
    WLAN_NOTIFY_WOW_MAGIC_PACKET_RECEIVED,
    WLAN_NOTIFY_WOW_WAKEUP_REASON_RECEIVED
}   WLAN_NOTIFY_ID_T;


/*------------------------------------------------------------------*/
/*! @brief NotifyMsg Defines for notification messages
 *  @code
 *  typedef enum
 *  {
 *      WLAN_NFY_MSG_OK                     = 0,
 *      WLAN_NFY_MSG_ALREADY_ASSOCIATED,
 *      WLAN_NFY_MSG_INIT_FAIL,
 *      WLAN_NFY_MSG_DEINIT_FAIL,
 *      WLAN_NFY_MSG_DONGLE_UNPLUG,
 *      WLAN_NFY_MSG_ASSOC_FAIL             = 10,
 *      WLAN_NFY_MSG_ASSOC_TIMEOUT,
 *      WLAN_NFY_MSG_AUTH_TIMEOUT,
 *      WLAN_NFY_MSG_PSK_INCORRECT,
 *      WLAN_NFY_MSG_NO_MATCHED_CONFIG,
 *      WLAN_NFY_MSG_NO_MATCHED_SSID,
 *      WLAN_NFY_MSG_WPS_CANCELLED,
 *      WLAN_NFY_MSG_WPS_PIN_MISMATCH,
 *      WLAN_NFY_MSG_WPS_SCAN_TIMEOUT,
 *      WLAN_NFY_MSG_WPS_PBC_OVERLAP,
 *      WLAN_NFY_MSG_WPS_PROTOCOL_TIMEOUT,
 *      WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID
 *  }   WLAN_NOTIFY_MSG_T;
 *  @endcode
 *  @li@c  WLAN_NFY_MSG_OK                                             - Success.
 *  @li@c  WLAN_NFY_MSG_ALREADY_ASSOCIATED                  - WLAN already associated.
 *  @li@c  WLAN_NFY_MSG_INIT_FAIL                                    - WLAN driver init fail.
 *  @li@c  WLAN_NFY_MSG_DEINIT_FAIL                                 - WLAN driver deinit fail.
 *  @li@c  WLAN_NFY_MSG_DONGLE_UNPLUG                           - WLAN dongle is unplugged.
 *  @li@c  WLAN_NFY_MSG_ASSOC_FAIL                                 - Undefined association fail.
 *  @li@c  WLAN_NFY_MSG_ASSOC_TIMEOUT                           - Association timeout.
 *  @li@c  WLAN_NFY_MSG_AUTH_TIMEOUT                             - WPA association fail, authentication timeout.
 *  @li@c  WLAN_NFY_MSG_PSK_INCORRECT                             - WPA association fail, incorrect Pre-share key.
 *  @li@c  WLAN_NFY_MSG_NO_MATCHED_CONFIG                     - WPA association fail, no matched config is found.
 *  @li@c  WLAN_NFY_MSG_NO_MATCHED_SSID                         - WPA association fail, no matched SSID is found.
 *  @li@c  WLAN_NFY_MSG_WPS_CANCELLED                            - WPS Registration was cancelled by a user.
 *  @li@c  WLAN_NFY_MSG_WPS_PIN_MISMATCH                       - WPS Association fail, PIN was mismatched to the registrar's PIN.
 *  @li@c  WLAN_NFY_MSG_WPS_SCAN_TIMEOUT                      - WPS Association fail, failed to detect a registrar in PIN/PBC mode.
 *  @li@c  WLAN_NFY_MSG_WPS_PBC_OVERLAP                         - WPS Association fail, due to PBC overlap.
 *  @li@c  WLAN_NFY_MSG_WPS_PROTOCOL_TIMEOUT               - WPS Association fail, protocol failed to complete after the credential(PIN/PBC) was authorized.
 *  @li@c  WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID               - WPS Association fail, credential received from the registrar is incompatible or invalid.
 */
/*------------------------------------------------------------------*/
typedef enum
{
    /* normal message */
    WLAN_NFY_MSG_OK                     = 0,
    WLAN_NFY_MSG_ALREADY_ASSOCIATED,

    /* wndrv abnormal message */
    WLAN_NFY_MSG_INIT_FAIL,
    WLAN_NFY_MSG_DEINIT_FAIL,
    WLAN_NFY_MSG_DONGLE_UNPLUG,
    //add for marvell
    //CONFIG_SUPPORT_MARVELL_WIFI
    WLAN_NFY_MSG_CONN_ESTABLISHING,
    /* wpa abnormal message */
    WLAN_NFY_MSG_ASSOC_FAIL             = 10,
    WLAN_NFY_MSG_ASSOC_TIMEOUT,
    WLAN_NFY_MSG_AUTH_TIMEOUT,
    WLAN_NFY_MSG_PSK_INCORRECT,
    WLAN_NFY_MSG_NO_MATCHED_CONFIG,
    WLAN_NFY_MSG_NO_MATCHED_SSID,
    /* wps abnormal message */
    WLAN_NFY_MSG_WPS_CANCELLED,
    WLAN_NFY_MSG_WPS_PIN_MISMATCH,
    WLAN_NFY_MSG_WPS_SCAN_TIMEOUT,
    WLAN_NFY_MSG_WPS_PBC_OVERLAP,
    WLAN_NFY_MSG_WPS_PROTOCOL_TIMEOUT,
    WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID,
    WLAN_NFY_MSG_DISCONNECTED,
    WLAN_NFY_MSG_WPS_CREDENTIAL_WEP,
    WLAN_NFY_MSG_ASSOC_START
}   WLAN_NOTIFY_MSG_T;


/*------------------------------------------------------------------*/
/*! @enum WLAN_WPS_STATE_T
 *  @brief WPSStatus Defines for WPS status codes
 *  @code
 *  typedef enum
 *  {
 *      WLAN_MW_WPS_IDLE            = 0,
 *      WLAN_MW_WPS_FAIL_RE_DO,
 *      WLAN_MW_WPS_DISCOVER,
 *      WLAN_MW_WPS_ASSOCIATING,
 *      WLAN_MW_WPS_WAIT_REQ_ID,
 *      WLAN_MW_WPS_WAIT_WSC_START,
 *      WLAN_MW_WPS_WAIT_WSC_M2,
 *      WLAN_MW_WPS_WAIT_WSC_M4,
 *      WLAN_MW_WPS_WAIT_WSC_M6,
 *      WLAN_MW_WPS_WAIT_WSC_M8,
 *      WLAN_MW_WPS_WAIT_EAP_FAIL,
 *      WLAN_MW_WPS_WAIT_WPA_SUPC,
 *      WLAN_MW_WPS_CONNECTED,
 *      WLAN_MW_WPS_FAIL,
 *      WLAN_MW_WPS_PERIOD_START,
 *      WLAN_MW_WPS_STATE_MAX
 *  }   WLAN_WPS_STATE_T;
 *  @endcode
 *  @li@c  WLAN_MW_WPS_IDLE                                      - WPS Idle state.
 *  @li@c  WLAN_MW_WPS_FAIL_RE_DO                            - WPS Fail and Redo state.
 *  @li@c  WLAN_MW_WPS_DISCOVER                               - WPS Discover AP state.
 *  @li@c  WLAN_MW_WPS_ASSOCIATING                          - WPS Associating AP state.
 *  @li@c  WLAN_MW_WPS_WAIT_REQ_ID                          - WPS Wait Req ID state.
 *  @li@c  WLAN_MW_WPS_WAIT_WSC_START                   - WPS Wait Start state.
 *  @li@c  WLAN_MW_WPS_WAIT_WSC_M2                        - WPS Wait M2 state.
 *  @li@c  WLAN_MW_WPS_WAIT_WSC_M4                        - WPS Wait M4 state.
 *  @li@c  WLAN_MW_WPS_WAIT_WSC_M6                        - WPS Wait M6 state.
 *  @li@c  WLAN_MW_WPS_WAIT_WSC_M8                        - WPS Wait M8 state.
 *  @li@c  WLAN_MW_WPS_WAIT_EAP_FAIL                       - WPS Wait EAP Fail state.
 *  @li@c  WLAN_MW_WPS_WAIT_WPA_SUPC                     - WPS Connecting state.
 *  @li@c  WLAN_MW_WPS_CONNECTED                             - WPS Connected state.
 *  @li@c  WLAN_MW_WPS_FAIL                                       - WPS Fail state.
 *  @li@c  WLAN_MW_WPS_PERIOD_START                         - WPS period state.
 *  @li@c  WLAN_MW_WPS_STATE_MAX                             - WPS state Max number.
 */
/*------------------------------------------------------------------*/
typedef enum
{
    WLAN_MW_WPS_IDLE            = 0,
    WLAN_MW_WPS_FAIL_RE_DO,
    WLAN_MW_WPS_DISCOVER,
    WLAN_MW_WPS_ASSOCIATING,
    WLAN_MW_WPS_WAIT_REQ_ID,
    WLAN_MW_WPS_WAIT_WSC_START,
    WLAN_MW_WPS_WAIT_WSC_M2,
    WLAN_MW_WPS_WAIT_WSC_M4,
    WLAN_MW_WPS_WAIT_WSC_M6,
    WLAN_MW_WPS_WAIT_WSC_M8,
    WLAN_MW_WPS_WAIT_EAP_FAIL,
    WLAN_MW_WPS_WAIT_WPA_SUPC,
    WLAN_MW_WPS_CONNECTED,
    WLAN_MW_WPS_FAIL,
    WLAN_MW_WPS_PERIOD_START,
    WLAN_MW_WPS_STATE_MAX
}   WLAN_WPS_STATE_T;


/*------------------------------------------------------------------*/
/*! @enum WLAN_WPA_STATUS_T
 *  @brief WPAStatus Defines for WPA status codes
 *  @code
 *  typedef enum
 *  {
 *  	WLAN_WPA_DISCONNECTED           = 0,
 *  	WLAN_WPA_INACTIVE,
 *  	WLAN_WPA_SCANNING,
 *  	WLAN_WPA_ASSOCIATING,
 *  	WLAN_WPA_ASSOCIATED,
 *  	WLAN_WPA_4WAY_HANDSHAKE,
 *  	WLAN_WPA_GROUP_HANDSHAKE,
 *  	WLAN_WPA_COMPLETED
 *  }   WLAN_WPA_STATUS_T;
 *  @endcode
 *  @li@c  WLAN_WPA_DISCONNECTED                         - Disconnected state.
 *  @li@c  WLAN_WPA_INACTIVE                                 - Inactive state (WPA disabled).
 *  @li@c  WLAN_WPA_SCANNING                                - Scanning for a network.
 *  @li@c  WLAN_WPA_ASSOCIATING                           - Trying to associate with a BSS/SSID.
 *  @li@c  WLAN_WPA_ASSOCIATED                            - Association completed.
 *  @li@c  WLAN_WPA_4WAY_HANDSHAKE                    - WPA 4-Way Key Handshake in progress.
 *  @li@c  WLAN_WPA_GROUP_HANDSHAKE                   - WPA Group Key Handshake in progress.
 *  @li@c  WLAN_WPA_COMPLETED                             - All authentication completed.
 */
/*------------------------------------------------------------------*/
typedef enum
{
    /* normal status */
    WLAN_WPA_DISCONNECTED           = 0,
    WLAN_WPA_INACTIVE,
    WLAN_WPA_SCANNING,
    WLAN_WPA_ASSOCIATING,
    WLAN_WPA_ASSOCIATED,
    WLAN_WPA_4WAY_HANDSHAKE,
    WLAN_WPA_GROUP_HANDSHAKE,
    WLAN_WPA_COMPLETED,
    WLAN_WPA_CONN_FAIL
}   WLAN_WPA_STATUS_T;



/*------------------------------------------------------------------*/
/*! @enum NET_802_11_ASSOC_CASE_T
 *  @brief AssocCase Defines for association cases
 *  @code
 *  typedef enum
 *  {
 *      IEEE_802_11_PWR_ON_ASSOC,
 *      IEEE_802_11_MANUAL_ASSOC,
 *      IEEE_802_11_SCAN_ASSOC,
 *      IEEE_802_11_WPS_ASSOC
 *  }   NET_802_11_ASSOC_CASE_T;
 *  @endcode
 *  @li@c  IEEE_802_11_PWR_ON_ASSOC                         - power on association.
 *  @li@c  IEEE_802_11_MANUAL_ASSOC                         - manual association.
 *  @li@c  IEEE_802_11_SCAN_ASSOC                             - scan and select SSID to associate.
 *  @li@c  IEEE_802_11_WPS_ASSOC                              - WPS association.
 */
/*------------------------------------------------------------------*/
typedef enum
{
    IEEE_802_11_PWR_ON_ASSOC,
    IEEE_802_11_MANUAL_ASSOC,
    IEEE_802_11_SCAN_ASSOC,
    IEEE_802_11_WPS_ASSOC
}   NET_802_11_ASSOC_CASE_T;



/*------------------------------------------------------------------*/
/*! @enum NET_802_11_AUTH_MODE_T
 *  @brief  802.11 auth mode
 *  @code
 *  typedef enum
 *  {
 *      IEEE_802_11_AUTH_MODE_NONE,
 *      IEEE_802_11_AUTH_MODE_OPEN,
 *      IEEE_802_11_AUTH_MODE_SHARED,
 *      IEEE_802_11_AUTH_MODE_WEPAUTO,
 *      IEEE_802_11_AUTH_MODE_WPA,
 *      IEEE_802_11_AUTH_MODE_WPA_PSK,
 *      IEEE_802_11_AUTH_MODE_WPA2,
 *      IEEE_802_11_AUTH_MODE_WPA2_PSK,
 *      IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK
 *  }   NET_802_11_AUTH_MODE_T;
 *  @endcode
 *  @li@c  IEEE_802_11_AUTH_MODE_NONE                                - no security.
 *  @li@c  IEEE_802_11_AUTH_MODE_OPEN                                - WEP open system.
 *  @li@c  IEEE_802_11_AUTH_MODE_SHARED                             - WEP shared key.
 *  @li@c  IEEE_802_11_AUTH_MODE_WEPAUTO                          - WEP open/share auto switch.
 *  @li@c  IEEE_802_11_AUTH_MODE_WPA                                 - WPA 802.1X enterprise mdoe.
 *  @li@c  IEEE_802_11_AUTH_MODE_WPA_PSK                           - WPA pre-share key mdoe.
 *  @li@c  IEEE_802_11_AUTH_MODE_WPA2                                - WPA2 802.1X enterprise mdoe.
 *  @li@c  IEEE_802_11_AUTH_MODE_WPA2_PSK                         - WPA2 pre-share key mdoe.
 *  @li@c  IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK         - Either WPA-PSK or WPA2-PSK mdoe.
 */
/*------------------------------------------------------------------*/
#if 1 //CONFIG_WIFI_MW_NEW_SUPPLICANT
typedef enum
{
    IEEE_802_11_AUTH_MODE_NONE,
    IEEE_802_11_AUTH_MODE_OPEN,
    IEEE_802_11_AUTH_MODE_SHARED,
    IEEE_802_11_AUTH_MODE_WEPAUTO,
    IEEE_802_11_AUTH_MODE_WPA,
    IEEE_802_11_AUTH_MODE_WPA_PSK,
    IEEE_802_11_AUTH_MODE_WPA_PSK_GTKIP,
    IEEE_802_11_AUTH_MODE_WPA_PSK_AES,
    IEEE_802_11_AUTH_MODE_WPA_PSK_AES_GTKIP,
    IEEE_802_11_AUTH_MODE_WPA_PSK_TKIP,
    IEEE_802_11_AUTH_MODE_WPA2,
    IEEE_802_11_AUTH_MODE_WPA2_PSK,
    IEEE_802_11_AUTH_MODE_WPA2_PSK_GTKIP,
    IEEE_802_11_AUTH_MODE_WPA2_PSK_AES,
    IEEE_802_11_AUTH_MODE_WPA2_PSK_AES_GTKIP,
    IEEE_802_11_AUTH_MODE_WPA2_PSK_TKIP,
    IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK,
    IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK_GTKIP,
    IEEE_802_11_AUTH_MODE_WPA_PSK_TKIP_GWEP104,
    IEEE_802_11_AUTH_MODE_WPA2_PSK_GWEP104,
    IEEE_802_11_AUTH_MODE_WPA_EAP,   // add for wpa_eap now DTV is not support
    IEEE_802_11_AUTH_MODE_WPA2_EAP,	// add for wpa_eap now DTV is not support
	IEEE_802_11_AUTH_MODE_WPAEAP_OR_WPA2EAP // add for wpa_eap now DTV is not support
}   NET_802_11_AUTH_MODE_T;

#else
typedef enum
{
    IEEE_802_11_AUTH_MODE_NONE,
    IEEE_802_11_AUTH_MODE_OPEN,
    IEEE_802_11_AUTH_MODE_SHARED,
    IEEE_802_11_AUTH_MODE_WEPAUTO,
    IEEE_802_11_AUTH_MODE_WPA,
    IEEE_802_11_AUTH_MODE_WPA_PSK,
    IEEE_802_11_AUTH_MODE_WPA2,
    IEEE_802_11_AUTH_MODE_WPA2_PSK,
    IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK,
    IEEE_802_11_AUTH_MODE_WPA_EAP,   // add for wpa_eap now DTV is not support
    IEEE_802_11_AUTH_MODE_WPA2_EAP,	// add for wpa_eap now DTV is not support
	IEEE_802_11_AUTH_MODE_WPAEAP_OR_WPA2EAP // add for wpa_eap now DTV is not support
}   NET_802_11_AUTH_MODE_T;
#endif
#define IEEE_802_11_AUTH_MODE_WEP IEEE_802_11_AUTH_MODE_WEPAUTO    /**< AuthMode       */



/*------------------------------------------------------------------*/
/*! @enum NET_802_11_AUTH_CIPHER_T
 *  @brief CipherMode Defines for cipher modes
 *  @code
 *  typedef enum
 *  {
 *      IEEE_802_11_AUTH_CIPHER_NONE        = (0x01 << 0),
 *      IEEE_802_11_AUTH_CIPHER_WEP40      = (0x01 << 1),
 *      IEEE_802_11_AUTH_CIPHER_WEP104     = (0x01 << 2),
 *      IEEE_802_11_AUTH_CIPHER_TKIP         = (0x01 << 3),
 *      IEEE_802_11_AUTH_CIPHER_AES          = (0x01 << 4),
 *      IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP = (IEEE_802_11_AUTH_CIPHER_AES | IEEE_802_11_AUTH_CIPHER_TKIP)
 *  }   NET_802_11_AUTH_CIPHER_T;
 *  @endcode
 *  @li@c  IEEE_802_11_AUTH_CIPHER_NONE                      - No cipher
 *  @li@c  IEEE_802_11_AUTH_CIPHER_WEP40                    - WEP40 cipher
 *  @li@c  IEEE_802_11_AUTH_CIPHER_WEP104                  - WEP104 cipher
 *  @li@c  IEEE_802_11_AUTH_CIPHER_TKIP                       - TKIP cipher
 *  @li@c  IEEE_802_11_AUTH_CIPHER_AES                        - AES cipher
 *  @li@c  IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP           - TKIP/AES cipher
 */
/*------------------------------------------------------------------*/
typedef enum
{
    IEEE_802_11_AUTH_CIPHER_NONE        = (0x01 << 0),
    IEEE_802_11_AUTH_CIPHER_WEP40       = (0x01 << 1),
    IEEE_802_11_AUTH_CIPHER_WEP104      = (0x01 << 2),
    IEEE_802_11_AUTH_CIPHER_TKIP        = (0x01 << 3),
    IEEE_802_11_AUTH_CIPHER_AES         = (0x01 << 4),
    IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP = (IEEE_802_11_AUTH_CIPHER_AES | IEEE_802_11_AUTH_CIPHER_TKIP)
}   NET_802_11_AUTH_CIPHER_T;



/*------------------------------------------------------------------*/
/*! @enum NET_802_11_WPS_ASSOC_MODE_T
 *  @brief WPSOpMode Defines for WPS operation mode
 *  @code
 *  typedef enum
 *  {
 *      IEEE_802_11_PIN_SETUP,
 *      IEEE_802_11_PBC_SETUP,
 *      IEEE_802_11_NFC_SETUP,
 *      IEEE_802_11_USB_SETUP
 *  }   NET_802_11_WPS_ASSOC_MODE_T;
 *  @endcode
 *  @li@c  IEEE_802_11_PIN_SETUP                             - WPS PIN.
 *  @li@c  IEEE_802_11_PBC_SETUP                             - WPS PBC.
 *  @li@c  IEEE_802_11_NFC_SETUP                             - WPS NFC.
 *  @li@c  IEEE_802_11_USB_SETUP                             - WPS USB.
 */
/*------------------------------------------------------------------*/
typedef enum
{
    IEEE_802_11_PIN_SETUP,
    IEEE_802_11_PBC_SETUP,
    IEEE_802_11_NFC_SETUP,
    IEEE_802_11_USB_SETUP
}   NET_802_11_WPS_ASSOC_MODE_T;


typedef enum
{
    FLAG_NONE               = (0x00),
    FLAG_REPLY_NOT_END      = (0x01 << 0)
}   WPA_FLAG_T;

/*--------------------------------------------------------------*/
/*                                                              */
/* Structure                                                    */
/*                                                              */
/*--------------------------------------------------------------*/




/*------------------------------------------------------------------*/
/*! @struct NET_802_11_KEY_T
 * @brief Structure of Key
 *  @code
 *  typedef struct
 *  {
 *      BOOL    b_IsAscii;
 *      CHAR    *pui1_PassPhrase;
 *  }   NET_802_11_KEY_T;
 *  @endcode
 *  @li@c  b_IsAscii                - TRUE for ASCII passphrase(key).
 *  @li@c  *pui1_PassPhrase         -Buffer pointer of the key content.
 *
 *                                  1.)8-63 ASCII or 64 hexadecimal words for valid WPA key
 *
 *                                  2.)5 ASCII or 10 hexadecimal words for valid 64-bit WEP key
 *
 *                                  3.)13 ASCII or 26 hexadecimal words for valid 128-bit WEP key
 */
/*------------------------------------------------------------------*/
typedef struct
{
    BOOL    b_IsAscii;
    CHAR    *pui1_PassPhrase;
    CHAR    key_body[WLAN_KEY_MAX_LENGTH + 1];         /* Benny'100521 add */
}   NET_802_11_KEY_T;



/*------------------------------------------------------------------*/
/*! @struct NET_802_11_SSID_T
 * @brief Structure of SSID
 *  @code
 *  typedef struct
 *  {
 *      UINT32  ui4_SsidLen;
 *      CHAR    ui1_aSsid[NET_802_11_MAX_LEN_SSID];
 *  }   NET_802_11_SSID_T;
 *  @endcode
 *  @li@c  ui4_SsidLen                                                     - SSID length in bytes.
 *  @li@c  ui1_aSsid[NET_802_11_MAX_LEN_SSID]                - SSID array with \ref SSIDLen "maximum length".
 */
/*------------------------------------------------------------------*/
typedef struct
{
    UINT32  ui4_SsidLen;
    CHAR    ui1_aSsid[NET_802_11_MAX_LEN_SSID + 1];
}   NET_802_11_SSID_T;





/*------------------------------------------------------------------*/
/*! @struct NET_802_11_SCAN_T
 * @brief Structure of Scan Mode
 *  @code
 *  typedef struct
 *  {
 *      NET_802_11_SSID_T           t_Ssid;
 *      BOOL                        b_ImplicitScan;
 *  }   NET_802_11_SCAN_T;
 *  @endcode
 *  @li@c  t_Ssid                                               - SSID in null-terminated string form.
 *  @li@c  b_ImplicitScan                                    - TRUE for Implicit scan or FLASE for explicit scan.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    NET_802_11_SSID_T           t_Ssid;
    BOOL                        b_ImplicitScan;
}   NET_802_11_SCAN_T;





/*------------------------------------------------------------------*/
/*! @struct NET_802_11_BSS_INFO_T
 * @brief Structure of BSS Information
 *  @code
 *  typedef struct
 *  {
 *      NET_802_11_BSSID_T                  t_Bssid;
 *      NET_802_11_SSID_T                    t_Ssid;
 *      NET_802_11_AUTH_MODE_T          e_AuthMode;
 *      NET_802_11_AUTH_CIPHER_T        e_AuthCipher;
 *      INT16                                        i2_Freq;
 *      INT16                                        i2_Channel;
 *      INT16                                        i2_Quality;
 *      INT16                                        i2_Noise;
 *      INT16                                        i2_Level;
 *      INT32                                        i4_MaxRate;
 *      UINT16                                      u2_Caps;
 *      UINT8                                       *ie;
 *      UINT32                                      u4_ieLength;
 *  }   NET_802_11_BSS_INFO_T;
 *  @endcode
 *  @li@c  t_Bssid                                           - BSSID in binary form. If it's unknown, set it as ff:ff:ff:ff:ff:ff
 *  @li@c  t_Ssid                                            - SSID in null-terminated string form.
 *  @li@c  e_AuthMode                                    - Authentication mode.
 *  @li@c  e_AuthCipher                                   - Cipher mode.
 *  @li@c  i2_Freq                                           - Primary radio frequency in unit of MHz
 *  @li@c  i2_Channel                                      - Primary radio channel number
 *  @li@c  i2_Quality                                        - Quality of signal
 *  @li@c  i2_Noise                                          - Noise level
 *  @li@c  i2_Level                                          - Normalized signal strength ranged from 1 to 100.
 *  @li@c  i4_MaxRate                                      - Max. supported radio rate of WiFi driver.
 *  @li@c  u2_Caps                                          - Capabilities included in the 802.11 management frames.
 *  @li@c  *ie                                                 - Buffer pointer of the information elements.
 *  @li@c  u4_ieLength                                     - Buffer length of the information elements.
 *  @li@is_wps_support						0 not support 1-support 2-unknown
 */
/*------------------------------------------------------------------*/
typedef struct
{
    NET_802_11_BSSID_T              t_Bssid;
    NET_802_11_SSID_T               t_Ssid;
    NET_802_11_AUTH_MODE_T          e_AuthMode;
    NET_802_11_AUTH_CIPHER_T        e_AuthCipher;
    NET_802_11_T                   e_802_11_type;
    NET_802_11_BW_T                 e_802_11_bw;
    INT16                           i2_Freq;
    INT16                           i2_Channel;
    INT16                           i2_Quality;
    INT16                           i2_Noise;
    INT16                           i2_Level;	
    INT16                           i2_Level_dB;
    INT32                           i4_MaxRate;
    UINT16                          u2_Caps;
#if 0 //CONFIG_MW_SUPPORT_MR
UINT32                          u4WpsSupport;
#else
	UINT8                           is_wps_support;
#endif
    UINT8                           *ie;
    UINT32                          u4_ieLength;
    UINT8 				u8_wpa_status;
}   NET_802_11_BSS_INFO_T;



/*------------------------------------------------------------------*/
/*! @struct NET_802_11_SCAN_RESULT_T
 * @brief Structure of Scan Result
 *  @code
 *  typedef struct
 *  {
 *      UINT32                                 ui4_NumberOfItems;
 *      NET_802_11_BSS_INFO_T       *p_BssInfo;
 *  }   NET_802_11_SCAN_RESULT_T;
 *  @endcode
 *  @li@c  ui4_NumberOfItems                             - Number of entries in the list.
 *  @li@c  *p_BssInfo                                        - Buffer in which BSS information is stored. The size of this buffer must
 *                                                             be enough to store num x sizeof(NET_802_11_BSS_INFO_T) bytes.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    UINT32                      ui4_NumberOfItems;
    NET_802_11_BSS_INFO_T       *p_BssInfo;

}   NET_802_11_SCAN_RESULT_T;



/*------------------------------------------------------------------*/
/*! @struct NET_802_11_ESS_INFO_T
 * @brief Structure of ESS Information
 *  @code
 *  typedef struct
 *  {
 *      NET_802_11_BSSID_T               t_Bssid;
 *      NET_802_11_SSID_T                t_Ssid;
 *      NET_802_11_AUTH_MODE_T      e_AuthMode;
 *      NET_802_11_AUTH_CIPHER_T    e_AuthCipher;
 *      INT16                                    i2_Level;
 *      INT32                                    i4_isConnected;
 *  }   NET_802_11_ESS_INFO_T;
 *  @endcode
 *  @li@c  t_Bssid                                           - BSSID in binary form.
 *  @li@c  t_Ssid                                            - SSID in null-terminated string form.
 *  @li@c  e_AuthMode                                    - Authentication mode.
 *  @li@c  e_AuthCipher                                   - Cipher mode.
 *  @li@c  i2_Level                                          - Normalized signal strength ranged from 1 to 100.
 *  @li@c  i4_isConnected                                 - Already connect with it
 */
/*------------------------------------------------------------------*/
typedef struct
{
    NET_802_11_BSSID_T          t_Bssid;
    NET_802_11_SSID_T           t_Ssid;
    NET_802_11_AUTH_MODE_T      e_AuthMode;
    NET_802_11_AUTH_CIPHER_T    e_AuthCipher;
    INT16                       i2_Level;
    INT32                       i4_isConnected;
	UINT8                       is_wps_support;
}   NET_802_11_ESS_INFO_T;



/*------------------------------------------------------------------*/
/*! @struct NET_802_11_ESS_LIST_T
 * @brief Structure of ESS List
 *  @code
 *  typedef struct
 *  {
 *      UINT32                                 ui4_NumberOfItems;
 *      NET_802_11_ESS_INFO_T       *p_EssInfo;
 *  }   NET_802_11_ESS_LIST_T;
 *  @endcode
 *  @li@c  ui4_NumberOfItems                             - Number of entries in the list.
 *  @li@c  *p_EssInfo                                        -Buffer in which ESS information is stored. The size of this buffer must
 *                                                            be enough to store num x sizeof(NET_802_11_ESS_INFO_T) bytes.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    UINT32                      ui4_NumberOfItems;
    NET_802_11_ESS_INFO_T       *p_EssInfo;

}   NET_802_11_ESS_LIST_T;



/*------------------------------------------------------------------*/
/*! @struct NET_802_11_ASSOCIATE_T
 * @brief Structure of Association
 *  @code
 *  typedef struct
 *  {
 *      NET_802_11_BSSID_T               t_Bssid;
 *      NET_802_11_SSID_T                t_Ssid;
 *      UINT8                                    ui1_Priority;
 *      NET_802_11_ASSOC_CASE_T     e_AssocCase;
 *      NET_802_11_AUTH_MODE_T      e_AuthMode;
 *      NET_802_11_AUTH_CIPHER_T    e_AuthCipher;
 *      NET_802_11_KEY_T        	        t_Key;
 *  }   NET_802_11_ASSOCIATE_T;
 *  @endcode
 *  @li@c  t_Bssid                                           - BSSID in binary form.
 *  @li@c  t_Ssid                                            - SSID in null-terminated string form.
 *  @li@c  ui1_Priority                                      - Priority of the selected SSID.
 *  @li@c  e_AssocCase                                   - Association condition.
 *  @li@c  e_AuthMode                                    - Authentication mode.
 *  @li@c  e_AuthCipher                                   - Cipher mode.
 *  @li@c  t_Key                                             - Key information.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    NET_802_11_BSSID_T          t_Bssid;
    NET_802_11_SSID_T           t_Ssid;
    UINT8                       ui1_Priority;
    NET_802_11_ASSOC_CASE_T     e_AssocCase;
    NET_802_11_AUTH_MODE_T      e_AuthMode;
    NET_802_11_AUTH_CIPHER_T    e_AuthCipher;
    NET_802_11_KEY_T        	t_Key;
}   NET_802_11_ASSOCIATE_T;


typedef struct
{
    NET_802_11_ASSOCIATE_T      t_Assocaite;
    BOOL                        fgWepShared;
    BOOL                        fgWepAutoMode;
}   NET_802_11_ASSOCIATE_WEP_T;


/*------------------------------------------------------------------*/
/*! @struct NET_802_11_EEPORM_T
 * @brief Structure of EEPORM Data
 *  @code
 *  typedef struct
 *  {
 *      UINT32  u4Addr;
 *      UINT8   *pData;
 *      UINT32  u4Len;
 *  }   NET_802_11_EEPORM_T;
 *  @endcode
 *  @li@c  u4Addr                                            - EEPORM address.
 *  @li@c  *pData                                           - Pointer to the data.
 *  @li@c  u4Len                                             - Size in bytes.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    UINT32  u4Addr;
    UINT8   *pData;
    UINT32  u4Len;
}   NET_802_11_EEPORM_T;

/*------------------------------------------------------------------*/
/*! @struct NET_802_11_RSSI_T
 *  @brief Structure of WIFI RSSI Data
 *  @code
 *  typedef struct
 *  {
 *      UINT32  u4Rssi_a;
 *      UINT32  u4Rssi_b;
 *      UINT32  u4Rssi_c;
 *  }   NET_802_11_RSSI_T;;
 *  @endcode
 *  @li@c  u4Rssi_a                                           - Antenna 1.
 *  @li@c  u4Rssi_b                                           - Antenna 2.
 *  @li@c  u4Rssi_b                                           - Antenna 3.
 */
/*------------------------------------------------------------------*/

typedef struct
{
    INT32  i4Rssi_a;
    INT32  i4Rssi_b;
    INT32  i4Rssi_c;
}   NET_802_11_WIFI_RSSI_T;


/*------------------------------------------------------------------*/
/*! @struct NET_802_11_SIGNAL_DATA_T
 *  @brief Structure of WIFI Signal Data
 *  @code
 *  typedef struct
 *  {
 *      INT32  Rssi;
 *      INT32  LinkSpeed;
 *      INT32  Noise;
 *      INT32  Freq;
 *  }   NET_802_11_SIGNAL_DATA_T;
 *  @endcode
 *  @li@c  Rssi                    - rssi of connection.
 *  @li@c  LinkSpeed           - linkspeed of connection.
 *  @li@c  Noise                   - noise of connection.
 *  @li@c  Freq                     - frequency of connection.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    INT32  Rssi;
    INT32  LinkSpeed;
    INT32  Noise;
    INT32  Freq;
}   NET_802_11_SIGNAL_DATA_T;



/*------------------------------------------------------------------*/
/*! @struct NET_802_11_WPA_T
 * @brief Structure of WPA Data
 *  @code
 *  typedef struct
 *  {
 *      CHAR     ifname[8];
 *      UINT32   ui4_type;
 *      UINT8    pData[256];
 *  }   NET_802_11_WPA_T;
 *  @endcode
 *  @li@c  ifname[8]                                       - Ctrl interface name
 *  @li@c  ui4_type                                         - WLAN_OID_T
 *  @li@c  pData[256]                                     - Data area.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    CHAR        ifname[8];
    UINT32      u4type;
    INT32       i4Ret;
    WPA_FLAG_T  eFlag;
    UINT8       pData[NET_WPA_DATA_LENGTH];
}   NET_802_11_WPA_T;


/*------------------------------------------------------------------*/
/*! @struct NET_802_11_NFY_T
 * @brief Structure of WPA Data
 *  @code
 *  typedef struct
 *  {
 *      INT32     i4NotifykId;
 *      INT32     pParam[256];
 *  }   NET_802_11_NFY_T;
 *  @endcode
 *  @li@c  i4NotifykId                                        - WLAN_NOTIFY_ID_T
 *  @li@c  pParam[256]                                     - Data area.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    INT32   i4NotifykId;
    INT32   i4Ret;
    INT32   pParam[NET_NFY_DATA_LENGTH];
    char    pDriverVer[DRIVER_VERSION_LENGTH];
}   NET_802_11_NFY_T;


/*----------------------------------------------------------------------------*/
/*! @} */                                           /* end of groupMW_INET_WIFI*/
/*----------------------------------------------------------------------------*/
/**
 * @brief Generic RSSI Types
 */
typedef struct
{
    UINT8      bc_avg_rssi;     /* average rssi */
    UINT8      bc_valid_mask;   /* bitmap of valid elements in rssi_ctrl/ext array */
    UINT8      bc_rssi_ctrl[ACFG_MAX_ANTENNA];
    UINT8      bc_rssi_ext[ACFG_MAX_ANTENNA];
    UINT8      data_avg_rssi;     /* average rssi */
    UINT8      data_valid_mask;   /* bitmap of valid elements in rssi_ctrl/ext array */
    UINT8      data_rssi_ctrl[ACFG_MAX_ANTENNA];
    UINT8      data_rssi_ext[ACFG_MAX_ANTENNA];
} NET_802_11_CHAIN_RSSI_T;


/*------------------------------------------------------------------*/
/*! @struct NET_802_11_KEY_T
 * @brief Structure of ATE parameters
 *  @code
 * typedef struct
 * {
 *    UINT8   antNum;
 *    UINT8   80211Standard;
 *    UINT8   channelNum;
 *    UINT8   PHYRate;
 *    UINT8   ModulationFlag;
 * }   NET_802_11_ATE_PARAMETER_T;
 *  @endcode
 *  @li@c  antNum  - the index of antenna.
 *  @li@c  80211Standard -the 802.11 standard.
                            0: CCK 802.11b, 1: OFDM 802.11g, 2: HT_MIX 802.11b/g/n, 3: Green Field 802.11n
 *  @li@c  channelNum -the channel number.
 *  @li@c  PHYRate -the PHY rate.
 *               1) 802.11b
 *                      0: 1Mbps, 1: 2Mbps, 2: 5.5Mbps, 3: 11Mbps, 8: 1Mbps, 9: 2Mbps, 10: 5.5Mbps, 11: 11Mbps
 *               2) 802.11g
 *                      0: 6Mbps, 1: 9Mbps, 2: 12Mbps, 3: 18Mbps, 4: 24Mbps, 5: 36Mbps, 6: 48Mbps, 7: 54Mbps
 *               3) 802.11b/g/n
 *                      0: 6.5Mbps, 1:13Mbps, 2:19.5Mbps, 3:26Mbps, 4:39Mbps, 5:52Mbps, 6:58.5Mbps, 7:65Mbps
 *                      8: 13Mbps, 9: 26Mbps, 10:39Mbps, 11:52Mbps, 12:78Mbps, 13:104Mbps, 14:117Mbps, 15:130Mbps, 32:6Mbps 
 *  @li@c  ModulationFlag -the flag of Modulation ON or OFF, 1: ON, 0: OFF.
 */
/*------------------------------------------------------------------*/
typedef struct
{
    UINT8   antNum;
    UINT8   WiFiStandard;
    UINT8   channelNum;
    UINT8   PHYRate;
    UINT8   ModulationFlag;
    UINT8   BandWidth;
}   NET_802_11_ATE_PARAMETER_T;


#define ACFG_CUSTDATA_LENGTH       20
/**
 * @brief get custdata
 */
typedef struct
{
    UINT8      custdata[ACFG_CUSTDATA_LENGTH];
}NET_802_11_ACFG_CUSTDAT_T;

#if PLATFORM_IS_TV
/**
 * @brief get p2p find device data ;
 *  P2P_CONNECT_MODE_PBC : p2p connect with pbc .
 *  P2P_CONNECT_MODE_PIN  : p2p connect with pin.
 */

/**
 * @brief This structure stores the network IP base info.
 */

typedef struct _NW_IP_INFO_T
{
	UINT32 ui4_address; 	/**< IP address */
	UINT32 ui4_netmask; 	/**< Subnet mask */
	UINT32 ui4_gw;		   /**< Default gateway */
	UINT32 ui4_1st_dns; 	/**< First DNS */
	UINT32 ui4_2nd_dns; 	/**< Second DNS */
} NW_IP_INFO_T;
#endif

typedef struct
{
    UINT8    ret_value;
    CHAR    _mac[18];
    CHAR    _addr[18];
    CHAR    _peer[18];
    CHAR    _name[33];
    UINT8    _length;
    CHAR    _pri_dev_type[13];
    BOOL    b_Is_GO;
    BOOL    b_Is_ForceGO;
    BOOL    b_Is_WFD;
    BOOL    b_Is_REJECT;
    UINT32  config_methods ;
    UINT32  dev_capab;
    UINT32  group_capab;
    UINT32  wfd_dev_info;
    UINT32  rtsp_port;
    UINT32  max_throughput;
    INT32   i4rssi;
    UINT32    devType;
    INT32   opchannel;
    CHAR	go_ssid[33];
    CHAR        primaryDevType[40];
    CHAR	go_passphrase[66];
    UINT8 devpasswdid;
    CHAR    pincode[9];
	BOOL b_update_ptable;
    BOOL b_is_peer_5G;
    BOOL b_is_SCCChannel;
    INT32   status_code;
    BOOL   b_Is_legacy_client;
	CHAR   _p2p_go_name[15] ;
	CHAR	p2p_client_ip_addr[16];
	CHAR	p2p_client_netmask[16];
	CHAR	p2p_go_ip_addr[16];
}   NET_802_11_P2P_DATA;

/**
 * @brief p2p peer info ;
 *  _device_name : the device name of P2P peer device .
 * _rtsp_port : port number for wifi display RTSP connection .
 */
typedef struct
{
    CHAR        _device_name[33];
    INT32       _rssi;
    UINT32      _rtsp_port;
}NET_802_11_P2P_PEER_INFO;

typedef struct
{
    CHAR _sa[18];
    CHAR _addr[18];
    CHAR _bssid[18];
    CHAR _network_name[65];
    UINT32 _status;
}NET_802_11_P2P_INVITATION_DATA;

/**
 * @brief wifi display data ;
 *  _port :port number that wifi display need .
 * _ip_addr : the ip address that wifi display need .
 */
typedef struct
{
    UINT32    _port;
    CHAR    _ip_addr[20];
}NET_802_11_WFD_DATA;

/**
 * @brief wifi display data ;
 *  _port :port number that wifi display need .
 * _ip_addr : the ip address that wifi display need .
 */

typedef struct
{
    UINT32    _port;
    CHAR    _ip_addr[20];
    CHAR    _mac[18];
}NET_802_11_IP_DATA;


#if PLATFORM_IS_TV
/**
 * @brief wifi display pin data ; it is be used when wfd connect with pin (keypad or display mode )
 *  _length :length of pin code  number that wifi display need .
 * _pin : pin code that up layer needed to show.
 */

typedef struct
{
    UINT8    _length;
    CHAR    _pin[10];
    CHAR    _mac[20];
}NET_802_11_WFD_PIN_DATA;
#endif


/**
 * @brief get p2p find device data ;
 *  P2P_CONNECT_MODE_PBC : p2p connect with pbc .
 *  P2P_CONNECT_MODE_PIN  : p2p connect with pin.
 */

typedef enum
{
    P2P_CONNECT_MODE_PBC,
    P2P_CONNECT_MODE_PIN_KEYPAD,
    P2P_CONNECT_MODE_PIN_DISPLAY,
    P2P_CONNECT_MODE_PIN,
    P2P_CONNECT_MODE_PBC_DIS
} P2P_CONNECT_MODE_T;

/*add for p2p persistent*/
typedef struct
{
    CHAR    subnet[20];
    CHAR    netmask[20];
    CHAR    range_start[20];
    CHAR    range_end[20];
    CHAR    routers[20];
    UINT32  default_lease_time;
    UINT32  max_lease_time;

}   NET_DHCP_CONFIG;


typedef struct
{
	UINT8    is_valid;		
	UINT8    my_p2p_rule;
	UINT8 peer_mac[6];
	UINT8 ap_mac[6];
    UINT8 DeviceName[33];
	UINT8 key_index;
	UINT16 auth_type;
	UINT16 encr_type;
	CHAR ssid[32];
	UINT16 ssid_len;
	CHAR key[64];
	UINT16 key_len;
}EEPROM_PER_ENTRY_T;

typedef struct
{ 
	int NumOfEntry;
	EEPROM_PER_ENTRY_T* eep_per_entry;
}PERSISTENT_ENTRY_T;

typedef struct
{
	UINT8    is_p2p;
	UINT8    is_wfd;
	UINT8 peer_mac[6];
	UINT8 dev_type[8];
	CHAR dev_name[32+1];
    INT32   i4rssi;
    UINT32  rtsp_port;
}P2P_ASSOC_PER_ENTRY_T;
typedef struct
{ 
	int NumOfEntry;
	P2P_ASSOC_PER_ENTRY_T* p2p_assoc_per_entry;
}P2P_ASSOC_ENTRY_T;

/* ======================== TDLS Data ======================== */

/* struct to identify the device want to connect */
typedef struct
{
    UINT8 macAddr[ETHADDR_LEN]; /* !< MAC address of the device */
} WLAN_TDLS_Device_Identifier_t;

/* struct to represent the information of the connection to a device */
typedef struct
{
    WLAN_TDLS_Device_Identifier_t   device; /* !< identifier to the peer device connected with TDLS */
    INT32                           rssi;   /* !< RSSI of TDLS */
    INT32                           rssiDiscoveryResp; /* !< RSSI which is taken in discovery */
} WLAN_TDLS_Connection_t;

/* This struct represents the information of TDLS **
** This contains the information of the connections to devices. **
** If numbers of connections is under MAX,
** the entries which doesn't represent the active connection should be filled with zero */
#define WLAN_TDLS_MAX_CONNECTIONS 20

typedef struct
{
    WLAN_TDLS_Connection_t connectionList[WLAN_TDLS_MAX_CONNECTIONS];
} WLAN_TDLS_Status_t;
#endif /* end of _U_NET_WLAN_H */



