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
 * $RCSfile: aee_api.c,v $
 * $Revision: #1 $
 * $Date: 2016/03/07 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/1 $
 * $SWAuthor: Iolo Tsai $
 * $MD5HEX: ed41ed3dcc5552c4ee8b06801ef05511 $
 *
 * Description:
 *         This file contains the implementation of the exported APIs.
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_common.h"
#include "u_handle.h"
#include "aee.h"


/*-----------------------------------------------------------------------------
 * Name: u_aee_create
 *
 * Description: This API creates a new application handle and spawn a thread
 *              to run the application's main function.
 *
 * Inputs:  pt_desc  References the application's description (resources, etc).
 *          ps_name  References the application's name.
 *
 * Outputs: ph_app  Contains the application handle.
 *
 * Returns: AEER_OK              Routine successful.
 *          AEER_FAIL            The AEE cannot be created (undefined reason).
 *          AEER_OUT_OF_HANDLES  Out of handles.
 *          AEER_OUT_OF_MEMORY   Out of memory.
 *          AEER_INV_ARG         Invalid argument.
 *          AEER_INV_NAME        Invalid name.
 ----------------------------------------------------------------------------*/
INT32 u_aee_create (const AEE_APP_DESC_T*  pt_desc,
                    const CHAR*            ps_name,
                    HANDLE_T*              ph_app)
{
    INT32  i4_res;

    if ((ph_app == NULL)
        || (ps_name == NULL)
        || (pt_desc == NULL)
        || (pt_desc->pf_main == NULL)
		)
    {
        i4_res = AEER_INV_ARG;
    }
    else
    {
        i4_res = aee_create_op(pt_desc,
                               ps_name,
                               ph_app);
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: u_aee_get_handle
 *
 * Description: This API returns a handle to an application (different from the
 *              handle created when the AEE was created).
 *
 * Inputs:  ps_name  References the name of the AEE.
 *
 * Outputs: ph_app  Contains the application handle.
 *
 * Returns: AEER_OK              Routine successful.
 *          AEER_FAIL            No application with such name exists.
 *          AEER_OUT_OF_HANDLES  Out of handles.
 *          AEER_INV_ARG         Invalid argument.
 *          AEER_INV_NAME        Invalid name.
 ----------------------------------------------------------------------------*/
static INT32 _aee_get_handle (const CHAR*  ps_name,
                        HANDLE_T*    ph_app)
{
    INT32  i4_res;

    if (ps_name == NULL)
    {
        i4_res = AEER_INV_NAME;
    }
    else if (ph_app == NULL)
    {
        i4_res = AEER_INV_ARG;
    }
    else
    {
        i4_res = aee_get_handle_op(ps_name, ph_app);
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: u_aee_send_data
 *
 * Description: This API calls the application's receive_data function.
 *
 * Inputs:  h_app       Contains the application handle.
 *          ui4_type    Contains the type of data.
 *          pv_data     References the data to send.
 *          z_data_len  Contains the size (in bytes) of the pv_data buffer.
 *
 * Outputs: -
 *
 * Returns: AEER_OK          Routine successful.
 *          AEER_INV_HANDLE  Invalid handle.
 ----------------------------------------------------------------------------*/
INT32 u_aee_send_data (HANDLE_T  h_app,
                       UINT32    ui4_type,
                       VOID*     pv_data,
                       SIZE_T    z_data_len)
{
    INT32  i4_res;

    i4_res = aee_send_data_op(h_app,
                              ui4_type,
                              pv_data,
                              z_data_len);

    return i4_res;
}


/*-----------------------------------------------------------------------------
 * Name: u_aee_get_handle
 *
 * Description: This API returns a handle to an application (different from the
 *              handle created when the AEE was created).
 *
 * Inputs:  ps_name  References the name of the AEE.
 *
 * Outputs: ph_app  Contains the application handle.
 *
 * Returns: AEER_OK              Routine successful.
 *          AEER_FAIL            No application with such name exists.
 *          AEER_OUT_OF_HANDLES  Out of handles.
 *          AEER_AEE_NO_RIGHTS   Permission denied.
 ----------------------------------------------------------------------------*/
INT32 u_aee_get_handle (const CHAR*  ps_name, HANDLE_T*    ph_app)
{
    HANDLE_T  h_aux;
    INT32     i4_res = AEER_OK;
    switch (i4_res)
    {
        case AEER_OK:
        {
            i4_res = _aee_get_handle(ps_name, ph_app);

            if (i4_res == AEER_OK)
            {
                u_handle_link_to_aux(h_aux, *ph_app);
            }
        }
        break;

        case AEER_OUT_OF_RESOURCES:
        {
            i4_res = AEER_AEE_OUT_OF_RESOURCES;
        }
        break;

        default:
        {
            i4_res = AEER_AEE_NO_RIGHTS;
        }
        break;
    }

    return i4_res;
}
INT32 u_aee_free_handle (HANDLE_T*    ph_app)
{
	return u_handle_free(*ph_app, TRUE);
}

