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




#include <sys/prctl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include "u_dbg.h"

#define SYS_Printf printf

#define gettid() syscall(__NR_gettid)

/*process restart function need to set _EN_CORE_DUMP == 1*/
int _EN_CORE_DUMP = 1;

extern void dump_stack(void);
static struct sigaction old_int, old_ill, old_term, old_segv, old_fpe, old_pipe, old_abrt;

static void show_regs(struct sigcontext *regs)
{
#ifdef __aarch64__
	int i, top_reg;
	unsigned long long lr, sp;
    char comm[20] = { '\0' };
    prctl(PR_GET_NAME, comm);
    SYS_Printf("\n");
    SYS_Printf("Pid: %d(tid: %d), comm: %20s\n", getpid(), gettid(), comm);
    lr = regs->regs[30];
	sp = regs->sp;
	top_reg = 29;

	SYS_Printf("pc : [<%016llx>] lr : [<%016llx>] pstate: %016llx\n",
	       regs->pc, lr, regs->pstate);
	SYS_Printf("sp : %016llx\n", sp);
	for (i = top_reg; i >= 0; i--) {
		SYS_Printf("x%-2d: %016llx ", i, regs->regs[i]);
		if (i % 2 == 0)
			SYS_Printf("\n");
	}
#else
    char comm[20] = { '\0' };
    prctl(PR_GET_NAME, comm);
    SYS_Printf("\n");
    SYS_Printf("Pid: %d(tid: %d), comm: %20s\n", getpid(), gettid(), comm);
    SYS_Printf("pc : [<%08lx>]    lr : [<%08lx>]    psr: %08lx\n"
           "sp : %08lx  ip : %08lx  fp : %08lx\n",
        regs->arm_pc, regs->arm_lr, regs->arm_cpsr,
        regs->arm_sp, regs->arm_ip, regs->arm_fp);
    SYS_Printf("r10: %08lx  r9 : %08lx  r8 : %08lx\n",
        regs->arm_r10, regs->arm_r9,
        regs->arm_r8);
    SYS_Printf("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
        regs->arm_r7, regs->arm_r6,
        regs->arm_r5, regs->arm_r4);
    SYS_Printf("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
        regs->arm_r3, regs->arm_r2,
        regs->arm_r1, regs->arm_r0);
#endif
}


static void sighandler(int signo, siginfo_t *info, void *context)
{
    ucontext_t *uc = (ucontext_t *)context;

    switch (signo)
    {
    case SIGINT:
        SYS_Printf("\n%s\n\n", "Interrupt from keyboard");
        SYS_Printf("Using default signal handler.\n");

        tcdrain(1);
        sigaction(SIGINT, &old_int, NULL);
        break;

    case SIGILL:
        SYS_Printf("\n%s\n\n", "Illegal Instruction");
        //SYS_Printf("\tpc: %p, inst: 0x%08lx\n", (void *)(uc->uc_mcontext.arm_pc), *(unsigned long *)(uc->uc_mcontext.arm_pc));
        show_regs(&uc->uc_mcontext);
        dump_stack();
        tcdrain(1);
        if(_EN_CORE_DUMP)
        {
          SYS_Printf("Using default signal handler.\n");
           sigaction(SIGILL, &old_segv, NULL);
        }
        //else
        //{
        //  while (1) pause();
        //}
        break;

    case SIGFPE:
        SYS_Printf("\n%s\n\n", "Floating point exception");
        //SYS_Printf("\tpc: %p, inst: 0x%08lx\n", (void *)(uc->uc_mcontext.arm_pc), *(unsigned long *)(uc->uc_mcontext.arm_pc));
      	show_regs(&uc->uc_mcontext);
        dump_stack();
        tcdrain(1);
        if(_EN_CORE_DUMP)
        {
          SYS_Printf("Using default signal handler.\n");
          sigaction(SIGFPE, &old_segv, NULL);
        }
        //else
        //{
        //  while (1) pause();
        //}
        break;

    case SIGSEGV:
        SYS_Printf("\n%s\n\n", "[APPMAINPROG] Invalid memory reference!");
        //SYS_Printf("\tpc: %p, addr: %p\n", (void *)(uc->uc_mcontext.arm_pc), info->si_addr);
        show_regs(&uc->uc_mcontext);
        dump_stack();
        tcdrain(1);
        if(_EN_CORE_DUMP)
        {
          SYS_Printf("Using default signal handler.\n");
          sigaction(SIGSEGV, &old_segv, NULL);
        }
        //else
        //{
        //  while (1) pause();
        //}
        break;

    case SIGTERM:
        SYS_Printf("\n%s\n\n", "Termination signal");
        SYS_Printf("Using default signal handler.\n");
        tcdrain(1);
        sigaction(SIGTERM, &old_term, NULL);
        break;

    case SIGPIPE:
        SYS_Printf("\n%s\n\n", " SIGPIPE signal");
        SYS_Printf("Using default signal handler.\n");
        tcdrain(1);
        sigaction(SIGPIPE, &old_pipe, NULL);
        break;

    case SIGABRT:
        SYS_Printf("\n%s\n\n", " Abort signal");
        show_regs(&uc->uc_mcontext);
        dump_stack();
        tcdrain(1);

        if(_EN_CORE_DUMP)
        {
          SYS_Printf("Using default signal handler.\n");
          sigaction(SIGABRT, &old_segv, NULL);
        }
        //else
        //{
        //  while (1) pause();
        //}
        break;
	default :
		SYS_Printf("%s.  signo=%d\n",__FUNCTION__,signo);
        break;
    }
}


int dbg_init(void)
{
    struct sigaction sa;
    int ret;

    sa.sa_sigaction = &sighandler;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_SIGINFO;
    if ((ret = sigaction(SIGINT, &sa, &old_int)) != 0) return ret;
    if ((ret = sigaction(SIGILL, &sa, &old_ill)) != 0) return ret;
    if ((ret = sigaction(SIGTERM, &sa, &old_term)) != 0) return ret;
    if ((ret = sigaction(SIGFPE, &sa, &old_fpe)) != 0) return ret;
    if ((ret = sigaction(SIGABRT, &sa, &old_abrt)) != 0) return ret;
    sa.sa_flags |= SA_ONSTACK;
    if ((ret = sigaction(SIGSEGV, &sa, &old_segv)) != 0) return ret;

    sa.sa_handler = SIG_IGN;
    if ((ret = sigaction(SIGPIPE, &sa, &old_pipe)) != 0) return ret;

    return 0;
}

