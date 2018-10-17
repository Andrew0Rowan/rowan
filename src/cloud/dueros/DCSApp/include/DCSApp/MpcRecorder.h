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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

//#include <alsa/asoundlib.h>
//#include <signal.h>
#include <thread>

#define MPC_ASR_DUMP 0

#if MPC_ASR_DUMP
#include <fstream>
#endif

#ifndef MPCRECORDER_H_
#define MPCRECORDER_H_

namespace duerOSDcsApp {
namespace application {
class MpcRecorder {
private:
    static MpcRecorder *instance;

public:
    MpcRecorder();
    ~MpcRecorder();

    typedef void (*callback)(char *data, unsigned long len, void *userdata);

    static MpcRecorder* create();
    static MpcRecorder* getInstance() { return instance; };

    void startRecording();
    void stopRecording();
    int dataIndicate(short *out_asr, int samples, int voice_angle, int is_vad);

    /**
     * @brief open a recorder.
     *
     * @param callback called when pcm data is ready.
     *
     * @return the error code, 0 on successful.
     */
    int recorderOpen(const char* alsa_rec_device,
             void(*callback)(char* data, unsigned long len, void *userdata),
             void *userdata);

    /**
     * @brief close a recorder.
     *
     * @param none.
     *
     * @return the error code, 0 on successful.
     */
    int recorderClose(void);
private:
    std::thread m_thread;
    callback m_callback;
    void *m_userdata;

#if MPC_ASR_DUMP
    std::fstream m_file;
#endif
};
}
}

typedef int (*data_process_cb)(short *out_asr, int samples, int voice_angle, int is_vad);
void four_mic_front_end_alg_client_init(data_process_cb mic_data_is_processed);
void four_mic_front_end_alg_client_clean(void);

#endif //MPCRECORDER_H_
