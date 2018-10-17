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

#include "DCSApp/MpcRecorder.h"
#include "LoggerUtils/DcsSdkLogger.h"
#include <stdlib.h>
#include <string>

namespace duerOSDcsApp {
namespace application {

void ExecSystemCmd(std::string cmd) {
    system(cmd.c_str());
}

MpcRecorder *MpcRecorder::instance = nullptr;

int mic_data_is_processed(short *out, int samples, int voice_angle, int vad_flag) {
    size_t count;
    count = MpcRecorder::getInstance()->dataIndicate(out, samples, voice_angle, vad_flag);

    return count;
}

MpcRecorder::MpcRecorder() {
}

MpcRecorder::~MpcRecorder() {
}

MpcRecorder* MpcRecorder::create() {
    if (nullptr == instance) {
        instance = new MpcRecorder();
    }

    return instance;
}

int MpcRecorder::recorderOpen(const char* alsa_rec_device,
                            void(*callback)(char* data, unsigned long len, void *userdata),
                            void *userdata) {
    m_callback = callback;
    m_userdata = userdata;

    return 0;
}

int MpcRecorder::recorderClose(void) {
    four_mic_front_end_alg_client_clean();
    return 0;
}

void MpcRecorder::startRecording() {
    std::string cmd("/usr/bin/algo-data-server");
    m_thread = std::thread(&ExecSystemCmd, cmd);

    m_thread.detach();
    four_mic_front_end_alg_client_init(mic_data_is_processed);
#if MPC_ASR_DUMP
    m_file.open("/data/sda1/d.pcm", std::ios::out|std::ios::binary|std::ios::trunc);
    if (!m_file.is_open()) {
        APP_INFO("could not open /data/sda1/d.pcm");
    }
#endif
}

void MpcRecorder::stopRecording() {
    four_mic_front_end_alg_client_clean();
    std::string cmd("killall -9 algo-data-server");
#if MPC_ASR_DUMP
    m_file.close();
#endif
}

int MpcRecorder::dataIndicate(short *out_asr, int samples, int voice_angle, int is_vad) {
    m_callback((char *)out_asr, samples * 2, m_userdata);

#if MPC_ASR_DUMP
    if (m_file.is_open()) {
        m_file.write((char *)out_asr, samples * 2);
    }
#endif

    return samples*2;
}

}
}

