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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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

// System header files

#include <signal.h>
#include <stdio.h>
#include <sched.h>

#include "u_bt_mw_types.h"

#include "c_mw_config.h"
#if ENABLE_A2DP_SINK
#include "bt_a2dp_alsa_playback.h"
#endif

#if !ENABLE_A2DP_ADEV && ENABLE_A2DP_SRC
#include "bt_a2dp_alsa_uploader.h"
#include "c_bt_mw_a2dp_src.h"
#endif


#include "rh_init_mtk_bt_service.h"
#include "_rpc_ipc_util.h"

#if defined(MTK_BT_COREDUMP_BACKTRACE)
#include <sys/prctl.h>
#include <execinfo.h>
#include <ucontext.h>

#if defined(MTK_BT_SYS_LOG)
__attribute__((visibility("default")))unsigned int ui4_enable_all_log = 0;
#endif

static struct sigaction old_int, old_ill, old_abrt, old_term, old_segv,
old_fpe, old_bus, old_pipe;
static void show_regs(struct sigcontext *regs)
{
    char comm[20] = { '\0' };
    int i = 0;

    prctl(PR_GET_NAME, comm);
    printf("=============================reg=============================\n");
    printf("Pid: %d, comm: %20s\n", (int)getpid(), comm);
    printf("pc : [<%08x>]   sp : %08x\n", regs->pc, regs->sp);

    for (i = 0; i < 31;)
    {
        if (i >= 27 && i < 29)
        {
            printf("r%d: %08x   r%d : %08x\n", i, regs->regs[i], i + 1, regs->regs[i + 1]);
            i += 2;
        }
        else if( i >= 29)
        {
            printf("fp: %08x   lr : %08x\n", regs->regs[i], regs->regs[i + 1]);
            i += 2;
        }
        else
        {
            printf("r%d: %08x  r%d : %08x  r%d : %08x\n", i, regs->regs[i], i + 1,
                   regs->regs[i + 1], i + 2, regs->regs[i + 2]);
            i += 3;
        }
    }
    printf("==========================================================\n");

}

static void dump_stack(void)
{
    void *trace[128];
    char **messages;
    int i, trace_size;
    printf("=============================backtrace=============================\n");

    trace_size = backtrace(trace, 128);
    printf("obtained %d stack frames:\n",trace_size);

    messages = backtrace_symbols(trace, trace_size);

    if (messages != NULL)
    {

        printf("backtrace:\n");


        for (i = 0; i < trace_size; i++)
            printf("[%d]%s\n", i,messages[i]);

        free(messages);
    }
    printf("=============================================================\n");
}

static void sighandler(int signo, siginfo_t *info, void *context)
{
    ucontext_t *uc = (ucontext_t *)context;

    switch (signo)
    {

    case SIGSEGV:
    case SIGABRT:
        printf("=========================SIGSEGV=SIGABRT=============================\n");
        printf("Invalid memory reference %d", signo);
        printf("\tpc: 0x%x, addr: 0x%x\n", (unsigned int)(void *)(uc->uc_mcontext.pc), (unsigned int)info->si_addr);
        show_regs(&uc->uc_mcontext);
        dump_stack();
        printf("Using default signal handler.\n");

        //while (1) pause();
        usleep(50*1000);
        sigaction(SIGSEGV, &old_segv, NULL);
        //exit(0);

    break;

    case SIGTERM:
        printf("==========================SIGTERM===================================\n");
        printf("Termination signal SIGTERM");
        printf("Using default signal handler.\n");

        sigaction(SIGTERM, &old_term, NULL);
        exit(0);
    break;


    case SIGPIPE:
        printf("==========================SIGPIPE===================================\n");
        printf("SIGPIPE signal SIGPIPE");
        printf("Using default signal handler.\n");

        sigaction(SIGPIPE, &old_pipe, NULL);

    break;

    default:
        printf("%d Unexpected signal", signo);

        show_regs(&uc->uc_mcontext);

        dump_stack();

        while (1) pause();

        break;
    }
}
#endif

int main(int argc, char **argv)
{
    printf("Mtk_bt_service server start:\n");
    struct sched_param param;
    int i4_ret = 0;
#if defined(MTK_BT_COREDUMP_BACKTRACE)
    struct sigaction sa;
    int ret;

#if defined(MTK_BT_SYS_LOG)
    /*init output log type*/
    if (0 == access("/data/log_all", 0))
    {
        printf("enable all ouput in btservice!!\n");
        ui4_enable_all_log = 1;
    }

#endif

    printf("sighandler\n");
    //system("ulimit -c unlimited");
    //system("echo core.%e.%p > /proc/sys/kernel/core_pattern");

    sa.sa_sigaction = &sighandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if ((ret = sigaction(SIGINT, &sa, &old_int)) != 0) return ret;
    if ((ret = sigaction(SIGILL, &sa, &old_ill)) != 0) return ret;
    if ((ret = sigaction(SIGABRT, &sa, &old_abrt)) != 0) return ret;
    if ((ret = sigaction(SIGTERM, &sa, &old_term)) != 0) return ret;
    if ((ret = sigaction(SIGFPE, &sa, &old_fpe)) != 0) return ret;
    if ((ret = sigaction(SIGBUS, &sa, &old_bus)) != 0) return ret;
    sa.sa_flags |= SA_ONSTACK;
    if ((ret = sigaction(SIGSEGV, &sa, &old_segv)) != 0) return ret;
    sa.sa_handler = SIG_IGN;
    if ((ret = sigaction(SIGPIPE, &sa, &old_pipe)) != 0) return ret;
#endif
    param.sched_priority = 95;
    printf("increase the priority of bluedroid.");

    i4_ret = sched_setscheduler(0, SCHED_RR, &param);
    printf("i4_ret:%d @ %s\n", i4_ret, __FUNCTION__);
    if (-1 == i4_ret)
    {
        printf("bluedroid sched_setscheduler error\n");
    }
    else
    {
        printf("set bluedroid priority thread done\n");
    }

    c_rpc_init_mtk_bt_service_server();
    bt_rpcu_tl_log_start();
    c_rpc_start_mtk_bt_service_server();

#if ENABLE_A2DP_SINK
    c_btm_a2dp_sink_register_player_cb(bt_a2dp_alsa_get_player());
#endif

#if !ENABLE_A2DP_ADEV && ENABLE_A2DP_SRC
    bt_mw_a2dp_src_register_uploader(bt_a2dp_alsa_get_uploader());
    bt_a2dp_alsa_register_output_callback(bt_mw_a2dp_src_send_stream_data);
#endif

    pause();

    return 0;
}
