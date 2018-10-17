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



#ifndef U_HTTP_FIRMWARE_UPGRADE_H
#define U_HTTP_FIRMWARE_UPGRADE_H

#include "u_common.h"
#include "u_handle.h"
#include "u_dlna.h"

typedef enum
{
    /* Informational.  */
    HTTP_RSP_STATUS_100                        = 100,
    HTTP_RSP_STATUS_CONTINUE                   = 100,
    HTTP_RSP_STATUS_SWITCHING_PROTOCOLS        = 101,

    /* Successful.  */
    HTTP_RSP_STATUS_200_OK                     = 200,
    HTTP_RSP_STATUS_CREATED                    = 201,
    HTTP_RSP_STATUS_ACCEPTED                   = 202,
    HTTP_RSP_STATUS_NO_AUTHORATATIVE_INFO      = 203,
    HTTP_RSP_STATUS_NO_CONTENT                 = 204,
    HTTP_RSP_STATUS_RESET_CONTENT              = 205,
    HTTP_RSP_STATUS_PARTIAL_CONTENT            = 206,

    /* Redirection.  */
    HTTP_RSP_STATUS_MULTIPLE_CHOICES            = 300,
    HTTP_RSP_STATUS_MOVED_PERMANENTLY           = 301,
    HTTP_RSP_STATUS_FOUND                       = 302,
    HTTP_RSP_STATUS_SEE_OTHER                   = 303,

    HTTP_RSP_STATUS_NOT_MODIFIED                = 304,
    HTTP_RSP_STATUS_USE_PROXY                   = 305,
    HTTP_RSP_STATUS_TEMPORARY_REDIRECT          = 307,

    /* Client error.  */
    HTTP_RSP_STATUS_BAD_REQUEST                   = 400,
    HTTP_RSP_STATUS_UNAUTHORIZED                  = 401,
    HTTP_RSP_STATUS_PAYMENT_REQUIRED              = 402,
    HTTP_RSP_STATUS_FORBIDDEN                     = 403,
    HTTP_RSP_STATUS_NOT_FOUND                     = 404,
    HTTP_RSP_STATUS_METHOD_NOT_ALLOWED            = 405,
    HTTP_RSP_STATUS_NOT_ACCEPTABLE                = 406,
    HTTP_RSP_STATUS_PROXY_AUTH_REQUIRED           = 407,
    HTTP_RSP_STATUS_REQUEST_TIMEOUT               = 408,
    HTTP_RSP_STATUS_CONFLICT                      = 409,
    HTTP_RSP_STATUS_GONE                          = 410,
    HTTP_RSP_STATUS_LENGTH_REQUIRED               = 411,
    HTTP_RSP_STATUS_PRECONDITION_FAILED           = 412,
    HTTP_RSP_STATUS_REQ_ENTITY_TOO_LARGE          = 413,
    HTTP_RSP_STATUS_REQUEST_URI_TOO_LONG          = 414,
    HTTP_RSP_STATUS_UNSUPPORTED_MEDIA_TYPE        = 415,
    HTTP_RSP_STATUS_REQUEST_RANGE_NOT_SATISFIABLE = 416,
    HTTP_RSP_STATUS_EXPECTATION_FAILED            = 417,

    /* Server error. */
    HTTP_RSP_STATUS_INTERNAL_SERVER_ERROR         = 500,
    HTTP_RSP_STATUS_NOT_IMPLEMENTED               = 501,
    HTTP_RSP_STATUS_BAD_GATEWAY                   = 502,
    HTTP_RSP_STATUS_SERVICE_UNAVAILABLE           = 503,
    HTTP_RSP_STATUS_GATEWAY_TIMEOUT               = 504,
    HTTP_RSP_STATUS_VERSION_NOT_SUPPORTED         = 505,

    HTTP_RSP_STATUS_CODE_MAX                      = 600,
    HTTP_RSP_STATUS_CODE_UNKNOWN                  = -1
} HTTP_RESPONSE_STATUS_CODE_T;

typedef enum 
{
    CONTENT_TYPE_APPLICATION = 1,
    CONTENT_TYPE_AUDIO,
    CONTENT_TYPE_EXAMPLE,
    CONTENT_TYPE_IMAGE,
    CONTENT_TYPE_MESSAGE,
    CONTENT_TYPE_MODEL,
    CONTENT_TYPE_MULTIPART,
    CONTENT_TYPE_TEXT,
    CONTENT_TYPE_VIDEO
} CONTENT_TYPE_T;

typedef enum
{
    HTTP_STATE_DATA_BLOCK       =  0,
    HTTP_STATE_FILE_WRITING,
    HTTP_STATE_DATA_LAST_BLOCK,
    HTTP_STATE_FILE_EOF,
    HTTP_STATE_XML_RESPONSE,
    HTTP_STATE_DOWNLOADING,  
    HTTP_STATE_RECEIVE_DONE,

    HTTP_STATE_SESSION_ERROR     = -65535,
    HTTP_STATE_SESSION_TIME_OUT,  
    HTTP_STATE_RESOLVE_URL_ERROR,
    HTTP_STATE_DOWNLOAD_DATA_ERROR,
    HTTP_STATE_WRITE_INTO_FILE_ERROR,
    HTTP_STATE_SERVER_DOWNLOAD_ERROR
} HTTP_DOWNLOAD_NOTIFY_STATE_T;

typedef enum 
{
	HTTP_RET_OK = 0,
    HTTP_RET_OSAI_SEND_MSG_FAILED = -65535,
    HTTP_RET_OSAI_SEMA_LOCK_FAILED,
    HTTP_RET_WRONG_XML_FMT,
    HTTP_RET_CANNOT_CONNECT,
    HTTP_RET_RANGE_INVALID,
    HTTP_RET_INVALID_URL
} HTTP_RET_CODE_T;

typedef enum
{
   HTTP_DN_WR_MEM = 0,
   HTTP_DN_WR_FILE = 1
}HTTP_DN_WR_TYPE;

typedef struct _HTTP_DN_MEM
{
   CHAR *mem;
   UINT32 mem_len;
}HTTP_DN_MEM;

typedef struct _HTTP_DN_FILE
{
   CHAR *ps_file;
   HANDLE_T h_handle;
}HTTP_DN_FILE;
   
typedef struct _HTTP_DN_POOL
{
   HTTP_DN_WR_TYPE wr_type;
   union
   {
     HTTP_DN_MEM mem;
     HTTP_DN_FILE file;
   };
} HTTP_DN_POOL;

typedef struct http_fw_download_ctx_s HTTP_CTX_T;

typedef INT32 (*x_http_fw_dn_read_nfy) (HTTP_CTX_T *pt_ctx, CHAR *pac_buf, INT32 i4_download_size, HTTP_DOWNLOAD_NOTIFY_STATE_T e_state);

struct http_fw_download_ctx_s
{
    VOID                            *pt_hm;            /* master buffer */
    VOID                            *pt_hc;            /* http client  */
    VOID                            *pt_session;       /* connection session */
    CONTENT_TYPE_T                  e_dload_cnt_type;  /* download CONTENT_TYPE_T */
    HTTP_RET_CODE_T                 e_ret_code;        /* return code */
    INT32                           i4_method;         /* http method */
    HTTP_RESPONSE_STATUS_CODE_T     e_rsp_status;      /* http response status code */
                                                 
    CHAR                            *ps_url;           /* server's URL */
    CHAR                            *ps_body;          /* request body */

    x_http_fw_dn_read_nfy           pf_nfy;            /* read nfy */
    HTTP_DN_POOL                   *pool;
    INT32                           i4_buf_len;        /* buffer size */
    INT32                           i4_buf_use_size;   /* #of data has download */

    UINT32                          ui4_received_size; /* received size */
    UINT32                          ui4_content_size;  /* content size */
    INT32                           i4_abort;          /* abort context flag */
    INT32                           i4_done;

    CHAR                            s_host[256];       /* host fqdn or ip address */
    VOID                     *pv_rsp;
    HANDLE_T                        h_timer;           /* delay close timer */
    DLNA_DEVICE_TYPE_T e_device;                /* the  DLNA device type */ 

} ;

#endif
