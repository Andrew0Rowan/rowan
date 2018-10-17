/******************************************************************************
 *
 *  Copyright (C) 2010 MediaTek Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#include "bt_target.h"

#if defined(MTK_B3DS_SUPPORT) && (MTK_B3DS_SUPPORT == TRUE)

#include <string.h>
#include "bt_types.h"
#include "bt_utils.h"
#include "b3ds_api.h"
#include "b3ds_int.h"
#include "sdp_api.h"
#include "sdpdefs.h"
#include "l2c_api.h"
#include "hcidefs.h"
#include "btm_api.h"
#include "bta_sys.h"

tB3DS_CB  b3ds_cb;

#define B3DS_FRAME_PERIOD_CALCULATE_FRAME 6
#define B3DS_FRAME_PERIOD_CALCULATE_CYCLE ( B3DS_FRAME_PERIOD_CALCULATE_FRAME * (B3DS_SET_CLK_NUM_CAP_TO_FILTER+1) )

// LeTV 3D frequency interval: 59.8 ~ 60.2
#define B3DS_LETV_60HZ_MAX 16722
#define B3DS_LETV_60HZ_MIN 16611

void b3ds_set_csb_data(UINT32 clock, UINT16 offset);

UINT32 b3ds_register_with_sdp (UINT16 uuid, char *p_name)
{
    UINT32  sdp_handle;
    UINT16  browse_list = UUID_SERVCLASS_PUBLIC_BROWSE_GROUP;
    tSDP_PROTOCOL_ELEM  protoList;
    B3DS_TRACE_DEBUG("b3ds_register_with_sdp");

    /* Create a record */
    sdp_handle = SDP_CreateRecord ();

    if (sdp_handle == 0)
    {
        B3DS_TRACE_ERROR ("b3ds_register_with_sdp: could not create SDP record");
        return 0;
    }
    APPL_TRACE_DEBUG("b3ds_register_with_sdp handle: %x", sdp_handle);

    /* Service Class ID List */
    SDP_AddServiceClassIdList (sdp_handle, 1, &uuid);

    /* For Service Search */
    protoList.protocol_uuid = UUID_PROTOCOL_L2CAP;
    protoList.num_params = 0;
    SDP_AddProtocolList(sdp_handle, 1, &protoList);

    /* Service Name */
    SDP_AddAttribute (sdp_handle, ATTR_ID_SERVICE_NAME, TEXT_STR_DESC_TYPE,
                        (UINT8) (strlen(p_name) + 1), (UINT8 *)p_name);

    /* Profile descriptor list */
    SDP_AddProfileDescriptorList (sdp_handle, UUID_SERVCLASS_3D_SYNCHRONIZATION, B3DS_PROFILE_VERSION);

    /* Make the service browsable */
    SDP_AddUuidSequence (sdp_handle,  ATTR_ID_BROWSE_GROUP_LIST, 1, &browse_list);

    bta_sys_add_uuid(uuid);

    return sdp_handle;
}

void b3ds_proc_cls_data_cb(BD_ADDR bd_addr, BT_HDR *p_buf)
{
    UINT8   *p = (UINT8 *)(p_buf + 1) + p_buf->offset;

    B3DS_TRACE_DEBUG( "b3ds_proc_cls_data_cb: Received command with code 0x%02x, bd_addr:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x", (UINT8) *p, bd_addr[0], bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);

    switch ((UINT8) *p)
    {
        case B3DS_COMM_CHNL_OPCODE_CON_ANNO_MSG:
            B3DS_TRACE_DEBUG( "b3ds_proc_cls_data_cb: len %d, option %d, battery %d", p_buf->len, (UINT8) *(p+1), (UINT8) *(p+2));
            b3ds_cb.b3ds_con_anno_msg_cb(bd_addr, (UINT8) *(p+1), (UINT8) *(p+2));
            break;

        default:
            B3DS_TRACE_WARNING("b3ds_proc_cls_data_cb: Undefined Opcode: %0x02x\n", (UINT8) *p);
            break;
    }

    osi_free(p_buf);
}

void b3ds_proc_cls_discover_cb (BD_ADDR bd_addr, UINT8 type , UINT32 extend)
{
    B3DS_TRACE_DEBUG("b3ds_proc_cls_discover_cb");
}

void b3ds_proc_cls_congestion_status_cb(BD_ADDR bd_addr, BOOLEAN congested)
{
    B3DS_TRACE_DEBUG("b3ds_proc_cls_congestion_status_cb");
}

UINT32 b3ds_register_with_l2cap()
{
    tL2CAP_UCD_CB_INFO cb_info;
    #ifndef B3DS_SERVICE_NAME
    #define B3DS_SERVICE_NAME "3D Synchronization"
    #endif
    #ifndef B3DS_SECURITY_LEVEL
    #define B3DS_SECURITY_LEVEL         BTM_SEC_NONE
    #endif
    if ( !BTM_SetSecurityLevel(FALSE, B3DS_SERVICE_NAME, BTM_SEC_SERVICE_3D_SYNCHRONIZATION, B3DS_SECURITY_LEVEL, BT_PSM_B3DS, 0, 0))
    {
        B3DS_TRACE_ERROR("b3ds_register_with_l2cap: security register failed");
        return FALSE;
    }

    cb_info.pL2CA_UCD_Discover_Cb = b3ds_proc_cls_discover_cb;
    cb_info.pL2CA_UCD_Data_Cb = b3ds_proc_cls_data_cb;
    cb_info.pL2CA_UCD_Congestion_Status_Cb = b3ds_proc_cls_congestion_status_cb;

    L2CA_UcdRegister(BT_PSM_B3DS, &cb_info);
    return TRUE;
}

void b3ds_process_timeout(UNUSED_ATTR void *data)
{
    B3DS_TRACE_DEBUG("b3ds_process_timeout");

    b3ds_cb.broadcast.period_mode = B3DS_PERIOD_2D_MODE;
    b3ds_set_csb_data(0, 0);
}

void b3ds_set_broadcast_data(uint16_t delay, uint8_t dual_view, uint16_t right_open_offset,
	uint16_t right_close_offset, uint16_t left_open_offset, uint16_t left_close_offset)
{
    B3DS_TRACE_DEBUG("b3ds_set_broadcast_data:delay:%d, video_mode:%d, left_open:0x%04X,close:0x%04X,right_open:0x%04X,close:0x%04X\n",
        delay, dual_view, left_open_offset, left_close_offset, right_open_offset, right_close_offset);

	b3ds_cb.broadcast.delay = delay;
	b3ds_cb.broadcast.vedio_mode = dual_view;
	b3ds_cb.broadcast.user_set_msg.right_open_offset = right_open_offset;
	b3ds_cb.broadcast.user_set_msg.right_close_offset = right_close_offset;
	b3ds_cb.broadcast.user_set_msg.left_open_offset = left_open_offset;
	b3ds_cb.broadcast.user_set_msg.left_close_offset = left_close_offset;
	b3ds_cb.user_set_bd_data = 1;
}

void b3ds_set_csb_data(UINT32 clock, UINT16 offset)
{
    UINT16 pre_frame_sync_period;
    UINT16 delay_to_clock;
    B3DS_TRACE_DEBUG("b3ds_set_csb_data, clock:0x%04X(%d) offset:0x%04X(%d)d delay:%d period:%d", clock, clock, offset, offset, b3ds_cb.broadcast.delay, b3ds_cb.broadcast.period_mode);

#if defined(__B3DS_BCM_SOLUTION__)

    if(b3ds_cb.user_set_bd_data == 0 || (clock == 0 && offset == 0) || b3ds_cb.current_frame_period < B3DS_LETV_60HZ_MIN || b3ds_cb.current_frame_period > B3DS_LETV_60HZ_MAX) /* No set 3DG data, set 2D mode value*/
    {
        B3DS_TRACE_DEBUG("b3ds: set 2D value");
        b3ds_cb.broadcast.msg.left_open_offset = 0xffff;
        b3ds_cb.broadcast.msg.left_close_offset = 0xffff;
        b3ds_cb.broadcast.msg.right_open_offset = 0xffff;
        b3ds_cb.broadcast.msg.right_close_offset = 0xffff;
        b3ds_cb.broadcast.delay = 0;
        b3ds_cb.broadcast.vedio_mode = 0;
    }
    else
    {
        b3ds_cb.broadcast.msg.left_open_offset = b3ds_cb.broadcast.user_set_msg.left_open_offset;
        b3ds_cb.broadcast.msg.left_close_offset = b3ds_cb.broadcast.user_set_msg.left_close_offset;
        b3ds_cb.broadcast.msg.right_open_offset = b3ds_cb.broadcast.user_set_msg.right_open_offset;
        b3ds_cb.broadcast.msg.right_close_offset = b3ds_cb.broadcast.user_set_msg.right_close_offset;
    }
/* The marked code only for LeTV
    UINT16 offset_leftOpen, offset_leftClose, offset_rightOpen, offset_rightClose, offset_delay;
    float tmpVal;
    UINT16 delay = 0;
    UINT16 duty_cycle = 100;
    offset_leftOpen = 0;

    tmpVal = (float)((b3ds_cb.current_frame_period * duty_cycle/(2*100.0)) - 1);
    offset_leftClose = round(tmpVal);

    tmpVal = (float)(b3ds_cb.current_frame_period / 2.0);
    offset_rightOpen = round(tmpVal);

    offset_rightClose = offset_rightOpen + offset_leftClose;

    b3ds_cb.broadcast.msg.left_open_offset = offset_leftOpen;
    b3ds_cb.broadcast.msg.left_close_offset = offset_leftClose;
    b3ds_cb.broadcast.msg.right_open_offset = offset_rightOpen;
    b3ds_cb.broadcast.msg.right_close_offset = offset_rightClose;
*/
    b3ds_cb.broadcast.msg.frame_sync_period = b3ds_cb.current_frame_period;
    b3ds_cb.broadcast.msg.frame_sync_period_fraction = 0;
#else
    // use default setting
    if (b3ds_cb.user_set_bd_data == 0)
    {
        B3DS_TRACE_DEBUG("b3ds_set_csb_data: set default setting");
        b3ds_cb.broadcast.msg.left_open_offset = b3ds_cb.broadcast.shutter_offset[b3ds_cb.broadcast.period_mode].left_open;
        b3ds_cb.broadcast.msg.left_close_offset = b3ds_cb.broadcast.shutter_offset[b3ds_cb.broadcast.period_mode].left_close;
        b3ds_cb.broadcast.msg.right_open_offset = b3ds_cb.broadcast.shutter_offset[b3ds_cb.broadcast.period_mode].right_open;
        b3ds_cb.broadcast.msg.right_close_offset = b3ds_cb.broadcast.shutter_offset[b3ds_cb.broadcast.period_mode].right_close;
    }
    else
    {
        B3DS_TRACE_DEBUG("b3ds_set_csb_data: set user setting");
        b3ds_cb.broadcast.msg.left_open_offset = b3ds_cb.broadcast.user_set_msg.left_open_offset;
        b3ds_cb.broadcast.msg.left_close_offset = b3ds_cb.broadcast.user_set_msg.left_close_offset;
        b3ds_cb.broadcast.msg.right_open_offset = b3ds_cb.broadcast.user_set_msg.right_open_offset;
        b3ds_cb.broadcast.msg.right_close_offset = b3ds_cb.broadcast.user_set_msg.right_close_offset;
    }

    b3ds_cb.broadcast.msg.frame_sync_period = b3ds_cb.broadcast.cal_frame_sync_period[b3ds_cb.broadcast.period_mode].period;
    b3ds_cb.broadcast.msg.frame_sync_period_fraction = b3ds_cb.broadcast.cal_frame_sync_period[b3ds_cb.broadcast.period_mode].period_fraction;
#endif

    if (b3ds_cb.broadcast.period_mode == B3DS_PERIOD_2D_MODE)
    {
        B3DS_TRACE_DEBUG("b3ds_set_csb_data: B3DS_PERIOD_2D_MODE");
        b3ds_cb.broadcast.msg.frame_sync_instant = 0;
        b3ds_cb.broadcast.msg.frame_sync_instant_phase = 0;
    }
    else if(clock)
    {
        b3ds_cb.broadcast.msg.frame_sync_instant = clock >> 1;
        b3ds_cb.broadcast.msg.frame_sync_instant_phase = offset % 625;
    }

    // clayton add , for delay change
    if (b3ds_cb.broadcast.delay != 0) {
        B3DS_TRACE_DEBUG("b3ds_set_csb_data: original instant:%d, phase:%d", b3ds_cb.broadcast.msg.frame_sync_instant, b3ds_cb.broadcast.msg.frame_sync_instant_phase);

        delay_to_clock = b3ds_cb.broadcast.delay / 625;
        b3ds_cb.broadcast.msg.frame_sync_instant += delay_to_clock;
        b3ds_cb.broadcast.msg.frame_sync_instant_phase += (b3ds_cb.broadcast.delay - (delay_to_clock * 625));

        if (b3ds_cb.broadcast.msg.frame_sync_instant_phase >= 625) {
            b3ds_cb.broadcast.msg.frame_sync_instant_phase -= 625;
            b3ds_cb.broadcast.msg.frame_sync_instant += 1;
        }
        B3DS_TRACE_DEBUG("b3ds_set_csb_data: new instant:%d, phase:%d", b3ds_cb.broadcast.msg.frame_sync_instant, b3ds_cb.broadcast.msg.frame_sync_instant_phase);
    }

#if defined(__B3DS_BCM_SOLUTION__)
    if (b3ds_cb.current_frame_period < B3DS_LETV_60HZ_MIN || b3ds_cb.current_frame_period > B3DS_LETV_60HZ_MAX)) {
        B3DS_TRACE_DEBUG("b3ds_set_csb_data: period is out interval, value:%d", b3ds_cb.current_frame_period);
        b3ds_cb.broadcast.msg.frame_sync_period = 0;
        b3ds_cb.broadcast.msg.frame_sync_period_fraction = 0;
    }
#endif

    // Set video mode
    if(b3ds_cb.broadcast.vedio_mode)
        b3ds_cb.broadcast.msg.frame_sync_instant |= 0x80000000;

    pre_frame_sync_period = b3ds_cb.broadcast.msg.frame_sync_period;

    B3DS_TRACE_DEBUG("b3ds_set_csb_data:video_mode:%d,sync_instant:0x%04X,instant_phase:0x%04X,left_open:0x%04X,close:0x%04X,right_open:0x%04X,close:0x%04X, pre_period:0x%04X, sync_period:0x%04X, fraction:0x%04X\n",
        b3ds_cb.broadcast.vedio_mode, b3ds_cb.broadcast.msg.frame_sync_instant, b3ds_cb.broadcast.msg.frame_sync_instant_phase,
        b3ds_cb.broadcast.msg.left_open_offset, b3ds_cb.broadcast.msg.left_close_offset, b3ds_cb.broadcast.msg.right_open_offset,
        b3ds_cb.broadcast.msg.right_close_offset, pre_frame_sync_period, b3ds_cb.broadcast.msg.frame_sync_period, b3ds_cb.broadcast.msg.frame_sync_period_fraction);

    BTM_SetCsbData(B3DS_LT_ADDR, B3DS_CSB_DATA_NO_FRAGMENTATION, 17, (UINT8 *)&b3ds_cb.broadcast.msg);
}

void b3ds_cal_frame_period(UINT32 synInstant, UINT16 synPhase)
{
    static UINT16 frameCount = 0;
    static UINT32 synInstantValue0 = 0;
    UINT32 durationInBtNativeClock;
    UINT32 durationInMSec;
    UINT16 avgFramePeriod;
    float avgFrame;
    UINT16 fraction;
    UINT8 i;

    B3DS_TRACE_DEBUG("b3ds_cal_frame_period: frameCount %d", frameCount);

    if (synInstant == 0 && synPhase == 0)
    {
    	B3DS_TRACE_DEBUG("b3ds_cal_frame_period: clock and offset is zero. 2D mode now!");

        // reset when becoming to 2D mode
        frameCount = 0;
        synInstantValue0 = 0;
        b3ds_cb.current_frame_period = 0;
        return;
    }

    if (frameCount == 0)
    {
        synInstantValue0 = synInstant;
    }
    else if (frameCount == B3DS_FRAME_PERIOD_CALCULATE_FRAME)
    {
        if (synInstantValue0 > synInstant)
            durationInBtNativeClock = (0x0FFFFFFF - synInstantValue0 + synInstant);
        else
            durationInBtNativeClock = synInstant - synInstantValue0;

        durationInMSec = durationInBtNativeClock*3125/10; //  + (synPhase - synPhaseValue0)>>8;

        avgFramePeriod = durationInMSec / B3DS_FRAME_PERIOD_CALCULATE_CYCLE; // clayton: get average frame
        avgFrame = (float) durationInMSec / B3DS_FRAME_PERIOD_CALCULATE_CYCLE;
        fraction = ((float)avgFrame - avgFramePeriod)*256;

        B3DS_TRACE_DEBUG("b3ds_cal_avg_frame_period: duration:%d, cycle:%d, avg_period:%d, avg_frame:%f, dis:%d", durationInMSec, B3DS_FRAME_PERIOD_CALCULATE_CYCLE, avgFramePeriod, avgFrame, fraction);

        b3ds_cb.current_frame_period = avgFramePeriod;
		b3ds_cb.frame_period_fraction = fraction;
        b3ds_cb.b3ds_send_frame_period_cb(avgFramePeriod); // clayton: set avg frame period mode

        for(i=B3DS_PERIOD_MAX;i!=B3DS_PERIOD_2D_MODE;i--)
            if(avgFramePeriod < b3ds_cal_frame_sync_period[i].max && avgFramePeriod > b3ds_cal_frame_sync_period[i].min)
                break;

        // Not matched but valid
        if(i == B3DS_PERIOD_2D_MODE && avgFramePeriod < B3DS_PERIOD_3D_24_HZ)
        {
            b3ds_cb.broadcast.period_mode = B3DS_PERIOD_DYNAMIC_CALCULATED;
            b3ds_cb.broadcast.cal_frame_sync_period[B3DS_PERIOD_DYNAMIC_CALCULATED].period = 0;
            b3ds_cb.broadcast.cal_frame_sync_period[B3DS_PERIOD_DYNAMIC_CALCULATED].period_fraction = 0;
            b3ds_cb.broadcast.shutter_offset[B3DS_PERIOD_DYNAMIC_CALCULATED].left_open = 0xffff;
            b3ds_cb.broadcast.shutter_offset[B3DS_PERIOD_DYNAMIC_CALCULATED].left_close = 0xffff;
            b3ds_cb.broadcast.shutter_offset[B3DS_PERIOD_DYNAMIC_CALCULATED].right_open = 0xffff;
            b3ds_cb.broadcast.shutter_offset[B3DS_PERIOD_DYNAMIC_CALCULATED].right_close = 0xffff;
        }
        // Matched or invalid
        else
            b3ds_cb.broadcast.period_mode = i;

        B3DS_TRACE_DEBUG("b3ds_cal_avg_frame_period: period_mode %d, avgFramePeriod %d", b3ds_cb.broadcast.period_mode, avgFramePeriod);

        // Reset
        frameCount = 0;
        synInstantValue0 = synInstant;
        // synPhaseValue0 = synPhase;
    }
    frameCount++;
}

#endif

