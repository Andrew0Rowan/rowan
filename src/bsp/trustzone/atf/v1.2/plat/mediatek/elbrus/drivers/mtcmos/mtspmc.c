/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mcucfg.h>
#include <mmio.h>
#include <mtcmos.h>
#include <mtspmc.h>
#include <plat_helpers.h>
#include <platform_def.h>
#include <spm.h>

#define IGNORE_PWR_ACK		1
#define DDR_RESERVE_ENABLE	1


/* APB Module infracfg_ao */
#ifdef MT6757_PRIMARY_CPU
#define INFRA_TOPAXI_PROTECTEN		(INFRACFG_AO_BASE + 0x220)
#define INFRA_TOPAXI_PROTECTSTA0	(INFRACFG_AO_BASE + 0x224)
#define INFRA_TOPAXI_PROTECTSTA1	(INFRACFG_AO_BASE + 0x228)

#define INFRA_TOPAXI_PROTECTEN_1	(INFRACFG_AO_BASE + 0x250)
#define INFRA_TOPAXI_PROTECTSTA0_1	(INFRACFG_AO_BASE + 0x254)
#define INFRA_TOPAXI_PROTECTSTA1_1	(INFRACFG_AO_BASE + 0x258)

#define IDX_PROTECT_MP0_CACTIVE		10
#define IDX_PROTECT_MP1_CACTIVE		11
#define IDX_PROTECT_ICC0_CACTIVE	12
#define IDX_PROTECT_ICD0_CACTIVE	13
#define IDX_PROTECT_ICC1_CACTIVE	14
#define IDX_PROTECT_ICD1_CACTIVE	15
#define IDX_PROTECT_L2C0_CACTIVE	26
#define IDX_PROTECT_L2C1_CACTIVE	27

#define MPx_SNOOP_CTRL			    (0x10390000)
#define MPx_SNOOP_STATUS			(0x1039000C)
#define MPx_SNOOP_ENABLE            (0x3)


#else
#define INFRA_TOPAXI_PROTECTEN_1_SET (INFRACFG_AO_BASE + 0x240)
#define INFRA_TOPAXI_PROTECTEN_1_CLR (INFRACFG_AO_BASE + 0x244)
#define INFRA_TOPAXI_PROTECTSTA1_1	(INFRACFG_AO_BASE + 0x258)

#define IDX_PROTECT_ICC0_CACTIVE	 5
#define IDX_PROTECT_ICD0_CACTIVE	 6
#define IDX_PROTECT_ICC1_CACTIVE	 7
#define IDX_PROTECT_ICD1_CACTIVE	 8
#define IDX_PROTECT_ICC2_CACTIVE	 9
#define IDX_PROTECT_ICD2_CACTIVE	10

#define IDX_PROTECT_MP0_CACTIVE		11
#define IDX_PROTECT_MP1_CACTIVE		12
#define IDX_PROTECT_MP2_CACTIVE		13

#define IDX_PROTECT_L2C0_CACTIVE	29
#define IDX_PROTECT_L2C1_CACTIVE	30
#define IDX_PROTECT_L2C2_CACTIVE	31

#endif
/* */
#define STA_POWER_DOWN		0
#define STA_POWER_ON		1

#define MODE_SPMC_HW        0
#define MODE_AUTO_SHUT_OFF  1

#define mcu_spm_read(addr)		spm_read(addr)
#define mcu_spm_write(addr, val)	spm_write(addr, val)

#define CPUTOP_MP0 0
#define CPUTOP_MP1 1
#define CPUTOP_MP2 2


char big_on = 0;//at most 4 cores
char little_on = 0x1; //[7:0] = core7~core0, core 0 is power-on in defualt


void big_spmc_info(){
    printf("Big SwSeq SPMC T:0x%X 0x%X C0:0x%X 0x%X C1:0x%X 0x%X\n",mmio_read_32(MP2_CPUTOP_SPMC_CTL),
                                                        		mmio_read_32(MP2_CPUTOP_SPMC_STA),
                                                                mmio_read_32(CPUSYS2_CPU0_SPMC_CTL),
                                                                mmio_read_32(CPUSYS2_CPU0_SPMC_STA),
                                                                mmio_read_32(CPUSYS2_CPU1_SPMC_CTL),
                                                                mmio_read_32(CPUSYS2_CPU1_SPMC_STA));
}
void little_spmc_info(){
    printf("Little SwSeq SPMC T0:0x%X C0:0x%X C1:0x%X C2:0x%X C3:0x%X\n",mmio_read_32(MP0_CPUTOP_SPMC_CTL),
                                                                mmio_read_32(CPUSYS0_CPU0_SPMC_CTL),
                                                                mmio_read_32(CPUSYS0_CPU1_SPMC_CTL),
                                                                mmio_read_32(CPUSYS0_CPU2_SPMC_CTL),
                                                                mmio_read_32(CPUSYS0_CPU3_SPMC_CTL));
    printf("Little SwSeq SPMC T1:0x%X C0:0x%X C1:0x%X C2:0x%X C3:0x%X\n",mmio_read_32(MP1_CPUTOP_SPMC_CTL),
                                                                mmio_read_32(CPUSYS1_CPU0_SPMC_CTL),
                                                                mmio_read_32(CPUSYS1_CPU1_SPMC_CTL),
                                                                mmio_read_32(CPUSYS1_CPU2_SPMC_CTL),
                                                                mmio_read_32(CPUSYS1_CPU3_SPMC_CTL));

}


void set_cpu_retention_control(int retention_value)
{
	uint64_t cpuectlr;

	cpuectlr = read_cpuectlr();
	cpuectlr = ((cpuectlr >> 3) << 3);
	cpuectlr |= retention_value;
	write_cpuectlr(cpuectlr);
}


/*
 * SPMC Mode
 */


/*
 	Program CA7L core 0 ~ core 3 of cluster <n> into standby wait for interrupt. Then follow core power off sequence to power down each core in cluster.
 	DFD RAM don't support dormant mode, must flush out to save it's information
 	Program MCSI-B Slave Control Register (MP0:slave0, MP1:slave1, MP2: slave2) bit0~1 to disable snoop/DVM function for related slave port through ACE or APB interface
 	Program MP<n>_AXI_CONFIG acinactm to 1 (0x1020002C for MP0, 0x1020022c for MP1, bit 4,MP2 0x1020220c bit0)
 	Set  cluster<n>.SPMC.MP<n>.sw_no_wait_for_q_channel (mcucfg_reg) 1
 	Wait mp<n>_STANDBYWFIL2 to high
 	Set   pwrdnreqn_icc_mp<n>_adb to low (all together)
 	Set   pwrdnreqn_icd_mp<n>_adb to low  (all together)
 	Set   pwrdnreqn_mp<n>_adb to low (all together)
 	Set  pwrdnreqn_mp<n>_l2c_afifo to low (all together)
 	Wait until pwrdnackn_icc_mp<n>_adb is low(all together)
 	Wait until pwrdnackn_icd_mp<n>_adb is low(all together)
 	Wait until pwrdnackn_mp<n>_adb is low(all together)
 	Wait until pwrdnackn_mp<n>_l2c_afifo to low (all together)
 	Set mp<n>_spmc_resetpwron_config_cputop to 0
 	Set mp<n>_spmc_sram_dormant_en 0
 	Set  mp<n>_spmc_pwr_on_cputop 0
 	Wait mp<n>_spmc_pwr_on_ack_cputop is 0 and power off sequence is finished
*/
int spmc_cputop_mpx_onoff(int cputop_mpx,int state,int mode)
{
	int err = 0;
	unsigned int mpx_mask;
	unsigned int MPx_AXI_CONFIG,MPx_CPUTOP_SPMC,MPx_CPUTOP_PWR_CON,MPx_CPUTOP_PWR_STA_MASK;
#ifdef MT6757_PRIMARY_CPU
	if(cputop_mpx==CPUTOP_MP0){
        mpx_mask=(1 << IDX_PROTECT_ICC0_CACTIVE) |
		(1 << IDX_PROTECT_ICD0_CACTIVE) |
		(1 << IDX_PROTECT_MP0_CACTIVE) |
		(1 << IDX_PROTECT_L2C0_CACTIVE);
		MPx_AXI_CONFIG = MP0_AXI_CONFIG;
		MPx_CPUTOP_SPMC = MP0_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP0_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP0_CPUTOP_PWR_STA_MASK;
		}
	else{
        mpx_mask=(1 << IDX_PROTECT_ICC1_CACTIVE) |
		(1 << IDX_PROTECT_ICD1_CACTIVE) |
		(1 << IDX_PROTECT_MP1_CACTIVE);
		MPx_AXI_CONFIG = MP1_AXI_CONFIG;
		MPx_CPUTOP_SPMC = MP1_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP1_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP1_CPUTOP_PWR_STA_MASK;
		}
#else
	if(cputop_mpx==CPUTOP_MP0){
        mpx_mask=(1 << IDX_PROTECT_ICC0_CACTIVE) |
		(1 << IDX_PROTECT_ICD0_CACTIVE) |
		(1 << IDX_PROTECT_MP0_CACTIVE) |
		(1 << IDX_PROTECT_L2C0_CACTIVE);
		MPx_AXI_CONFIG = MP0_AXI_CONFIG;
		MPx_CPUTOP_SPMC = MP0_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP0_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP0_CPUTOP_PWR_STA_MASK;
		}
	else if(cputop_mpx==CPUTOP_MP1){
        mpx_mask=(1 << IDX_PROTECT_ICC1_CACTIVE) |
		(1 << IDX_PROTECT_ICD1_CACTIVE) |
		(1 << IDX_PROTECT_MP1_CACTIVE);
		MPx_AXI_CONFIG = MP1_AXI_CONFIG;
		MPx_CPUTOP_SPMC = MP1_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP1_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP1_CPUTOP_PWR_STA_MASK;
		}
	else{
        mpx_mask=(1 << IDX_PROTECT_ICC2_CACTIVE) |
		(1 << IDX_PROTECT_ICD2_CACTIVE) |
		(1 << IDX_PROTECT_MP2_CACTIVE);
		MPx_AXI_CONFIG = MP2_AXI_CONFIG;
		MPx_CPUTOP_SPMC = MP2_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP2_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP2_CPUTOP_PWR_STA_MASK;
		}
#endif

	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
    INFO("MPx_CPUTOP_PWR_STA_MASK=%08x\n",MPx_CPUTOP_PWR_STA_MASK);


	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPUTOP" */
/* TODO:check MP0_SNOOP_CTRL	MP0_SNOOP_STATUS address&layout
	Program MCSI-A Slave Control Register (MP0:slave0, MP1:slave1) to disable snoop/DVM function for related slave port through ACE(0x10390000) or APB(0x10200000) interface
	Polling CCI Status register through ACE interface (0x1039000C[0]) returns to 0*/
		mmio_write_32(MPx_SNOOP_CTRL,mmio_read_32(MPx_SNOOP_CTRL)& ~MPx_SNOOP_ENABLE);
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 0)) == (1 << 0));
/* 	Program MP<n>_AXI_CONFIG acinactm to 1
	Wait mp<n>_STANDBYWFIL2 to high*/
		/* TINFO="Set STANDBYWFIL2 for Cluster 0" */
    if(cputop_mpx==CPUTOP_MP2)
        mmio_write_32(MPx_AXI_CONFIG,mmio_read_32(MPx_AXI_CONFIG)|MP2_AXI_CONFIG_acinactm);
	else
        mmio_write_32(MPx_AXI_CONFIG,mmio_read_32(MPx_AXI_CONFIG)|MPx_AXI_CONFIG_acinactm);
        if(mode==MODE_AUTO_SHUT_OFF){
			//Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw_spark_en (mcucfg_reg) 1
            mmio_write_32(MPx_CPUTOP_SPMC,mmio_read_32(MPx_CPUTOP_SPMC)| sw_spark_en);
            //Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw sw_no_wait_for_q_channel (mcucfg_reg) 0
            mmio_write_32(MPx_CPUTOP_SPMC,mmio_read_32(MPx_CPUTOP_SPMC)& ~sw_no_wait_for_q_channel);
			//Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw_coq_dis (mcucfg_reg) 0
			mmio_write_32(MPx_CPUTOP_SPMC,mmio_read_32(MPx_CPUTOP_SPMC)& ~sw_coq_dis);
        	}
		else{
            //Set  cluster<n>.SPMC.MP<n>.sw_no_wait_for_q_channel (mcucfg_reg) 1
            mmio_write_32(MPx_CPUTOP_SPMC,mmio_read_32(MPx_CPUTOP_SPMC)|sw_no_wait_for_q_channel);

		    /* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
		    //while ((mmio_read_32(MP0_CA7_MISC_CONFIG) & MPx_CA7_MISC_CONFIG_standbywfil2) != MPx_CA7_MISC_CONFIG_standbywfil2);
          #ifdef MT6757_PRIMARY_CPU
		    while ((mmio_read_32(CPU_IDLE_STA) & (MT6757_MPx_CPU_IDLE_STA_standbywfil2<<cputop_mpx)) != (MT6757_MPx_CPU_IDLE_STA_standbywfil2<<cputop_mpx));
          #else
		    while ((mmio_read_32(CPU_IDLE_STA) & (Elbrus_MPx_CPU_IDLE_STA_standbywfil2<<cputop_mpx)) != (Elbrus_MPx_CPU_IDLE_STA_standbywfil2<<cputop_mpx));
          #endif

          #ifdef MT6757_PRIMARY_CPU
		    /* TINFO="Set ADB pwrdnreqn for Cluster 0" */
            mmio_write_32(INFRA_TOPAXI_PROTECTEN_1,mmio_read_32(INFRA_TOPAXI_PROTECTEN_1)|mpx_mask);
          #else
            mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_SET,mmio_read_32(INFRA_TOPAXI_PROTECTEN_1_SET)|mpx_mask);
          #endif
		    /* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
		    while ((mmio_read_32(INFRA_TOPAXI_PROTECTSTA1_1) & mpx_mask) != mpx_mask);
			}

/* 	Set mp<n>_spmc_resetpwron_config_cputop to 0*/
		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
/* 	Set mp<n>_spmc_sram_dormant_en 0*/
		mmio_write_32(SPMC_DORMANT_ENABLE, mmio_read_32(SPMC_DORMANT_ENABLE) & ~(MP0_SPMC_SRAM_DORMANT_EN<<cputop_mpx));
/* 	Set  mp<n>_spmc_pwr_on_cputop 0*/
		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) & ~PWR_ON);

#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mmio_read_32(PWR_STATUS) & MPx_CPUTOP_PWR_STA_MASK)
		       || (mmio_read_32(PWR_STATUS_2ND) & MPx_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Finish to turn off MP0_CPUTOP" */
	} else {    /* STA_POWER_ON */
/*
 	Set  mp<n>_spmc_pwr_on_cputop 1
 	When mp<n>_spmc_pwr_on_ack_cputop is 1

 	Set   pwrdnreqn_icc_mp<n>_adb to high (all together)
 	Set   pwrdnreqn_icd_mp<n>_adb to high  (all together)
 	Set   pwrdnreqn_mp<n>_adb to high (all together)
 	Set  pwrdnreqn_mp<n>_l2c_afifo to high (all together)
 	Wait until pwrdnackn_icc_mp<n>_adb is high (all together)
 	Wait until pwrdnackn_icd_mp<n>_adb is high (all together)
 	Wait until pwrdnackn_mp<n>_adb is high (all together)
 	Wait until pwrdnackn_mp<n>_l2c_afifo to high (all together)
 	Program MP<n>_AXI_CONFIG acinactm to 0
 	Program MCSI-A Slave Control Register (MP0:slave0, MP1:slave1) to enable snoop/DVM function for related slave port through ACE(0x10390000) or APB(0x10200000) interface
 	Polling CCI Status register through ACE interface (0x1039000C[0]) returns to 0
 	Finish power on and reset sequences
*/
		/* TINFO="Start to turn on MP0_CPUTOP" */
		/* TINFO="Set PWR_ON = 1" */
		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) | PWR_ON);

#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mmio_read_32(PWR_STATUS) & MPx_CPUTOP_PWR_STA_MASK)){
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif

		/* TINFO="Release bus protect" */

#ifdef MT6757_PRIMARY_CPU
		/* TINFO="Release ADB pwrdnreqn for Cluster 0" */
        mmio_write_32(INFRA_TOPAXI_PROTECTEN_1,mmio_read_32(INFRA_TOPAXI_PROTECTEN_1) & ~mpx_mask);
#else
        mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_CLR,mmio_read_32(INFRA_TOPAXI_PROTECTEN_1_CLR) | mpx_mask);
#endif
		/* TINFO="Wait ADB ~pwrdnreqn for Cluster 0" */
		while ((mmio_read_32(INFRA_TOPAXI_PROTECTSTA1_1) & mpx_mask) != 0);

/* 	Program MP<n>_AXI_CONFIG acinactm to 0*/
    if(cputop_mpx==CPUTOP_MP2)
        mmio_write_32(MPx_AXI_CONFIG,mmio_read_32(MPx_AXI_CONFIG)& ~MP2_AXI_CONFIG_acinactm);
	else
        mmio_write_32(MPx_AXI_CONFIG,mmio_read_32(MPx_AXI_CONFIG)& ~MPx_AXI_CONFIG_acinactm);

/* 	Program MCSI-A Slave Control Register (MP0:slave0, MP1:slave1) to enable snoop/DVM function for related slave port through ACE(0x10390000) or APB(0x10200000) interface
	Polling CCI Status register through ACE interface (0x1039000C[0]) returns to 0*/
		mmio_write_32(MPx_SNOOP_CTRL,mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 0)) == (1 << 0));
		/* TINFO="Finish to turn on MP0_CPUTOP" */
	}
	return err;
}


/*
 	Core0 set cluster0.SPMC.core0.sw_no_wait_for_q_channel (mcucfg_reg) to 1
 	Core0 set GICR_WAKER.ProcessorSleep to 1 (should be in open source code)
 	Core0 wait GICR_WAKER.ChildernAsleep  to1 (should be in open source code)
 	Core0 program CA7L core 0 into standby wait for interrupt
 	SPM wait mp0_STANDBYWFI[0] to high
 	SPM set mp0_spmc_resetpwron_config_cpu0 set to 0
 	SPM set mp0_spmc_pwr_on_cpu0 to low
 	SPM wait mp0_spmc_pwr_ack_cpu0 to low and then the power off sequence is finished

*/
int spmc_cpu_corex_onoff(int cpu_corex,int state,int mode)
{
	int err = 0;
	unsigned int CPUSYSx_CPUx_SPMC_CTL,MPx_CPUx_PWR_CON,MPx_CPUx_STANDBYWFI,MPx_CPUx_PWR_STA_MASK;
	/* TINFO="enable SPM register control" */
	spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

#ifdef MT6757_PRIMARY_CPU
	switch (cpu_corex) {
		case 0:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU0_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU0_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU0_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU0_PWR_STA_MASK;
			break;
		case 1:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU1_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU1_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU1_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU1_PWR_STA_MASK;
			break;
		case 2:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU2_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU2_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU2_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU2_PWR_STA_MASK;
			break;
		case 3:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU3_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU3_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU3_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU3_PWR_STA_MASK;
			break;
		case 4:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU0_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU0_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU0_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU0_PWR_STA_MASK;
			break;
		case 5:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU1_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU1_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU1_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU1_PWR_STA_MASK;
			break;
		case 6:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU2_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU2_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU2_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU2_PWR_STA_MASK;
			break;
		case 7:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU3_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU3_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU3_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU3_PWR_STA_MASK;
			break;
		default:
			ERROR("%s() CPU%d not exists\n", __FUNCTION__, (int)cpu_corex);
			assert(0);
	}
#else
	switch (cpu_corex) {
		case 0:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU0_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU0_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU0_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU0_PWR_STA_MASK;
			break;
		case 1:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU1_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU1_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU1_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU1_PWR_STA_MASK;
			break;
		case 2:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU2_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU2_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU2_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU2_PWR_STA_MASK;
			break;
		case 3:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU3_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU3_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP0_CPU3_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP0_CPU3_PWR_STA_MASK;
			break;
		case 4:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU0_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU0_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU0_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU0_PWR_STA_MASK;
			break;
		case 5:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU1_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU1_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU1_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU1_PWR_STA_MASK;
			break;
		case 6:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU2_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU2_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU2_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU2_PWR_STA_MASK;
			break;
		case 7:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU3_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP1_CPU3_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP1_CPU3_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP1_CPU3_PWR_STA_MASK;
			break;
		case 8:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU0_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP2_CPU0_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP2_CPU0_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP2_CPU0_PWR_STA_MASK;
			break;
		case 9:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU1_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP2_CPU1_STANDBYWFI;
			MPx_CPUx_PWR_CON      = MP2_CPU1_PWR_CON;
			MPx_CPUx_PWR_STA_MASK = MP2_CPU1_PWR_STA_MASK;
			break;
		default:
			ERROR("%s() CPU%d not exists\n", __FUNCTION__, (int)cpu_corex);
			assert(0);
	}
#endif


	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPU0" */

        if(mode==MODE_AUTO_SHUT_OFF){
			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL,mmio_read_32(CPUSYSx_CPUx_SPMC_CTL)& ~cpu_sw_no_wait_for_q_channel);
			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL,mmio_read_32(CPUSYSx_CPUx_SPMC_CTL)| cpu_sw_spark_en);
			set_cpu_retention_control(1);
        	}
		else{
			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL,mmio_read_32(CPUSYSx_CPUx_SPMC_CTL)|cpu_sw_no_wait_for_q_channel);
			INFO("Wait MP0_CPU0_STANDBYWFI's WFI\n");
			while(!(mmio_read_32(CPU_IDLE_STA) & MPx_CPUx_STANDBYWFI));
			INFO("MP0_CPU0_STANDBYWFI is in WFI\n");
			}

        mmio_write_32(MPx_CPUx_PWR_CON,mmio_read_32(MPx_CPUx_PWR_CON)& ~PWR_ON_2ND);
		/* TINFO="Set PWR_ON = 0" */
		mmio_write_32(MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON) & ~PWR_ON);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 0 and CPU_PWR_STATUS_2ND = 0" */
		while ((mmio_read_32(PWR_STATUS) & MPx_CPUx_PWR_STA_MASK)
		       || (mmio_read_32(PWR_STATUS_2ND) & MPx_CPUx_PWR_STA_MASK)) {
				/*  */
		}

#endif
		/* TINFO="Finish to turn off MP0_CPU0" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPU0" */
		/* TINFO="Set PWR_ON = 1" */
		mmio_write_32(MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON) | PWR_ON);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 1 and CPU_PWR_STATUS_2ND = 1" */
		while ((mmio_read_32(PWR_STATUS) & MPx_CPUx_PWR_STA_MASK) != MPx_CPUx_PWR_STA_MASK){

				/*  */
		}
#endif
		/* TINFO="Finish to turn on MP0_CPU0" */
	}
	return err;
}

/*
 	Core0 set cluster0.SPMC.core0.sw_no_wait_for_q_channel (mcucfg_reg) to 0
 	Core0 set cluster0.SPMC.core0.sw_spark_en (mcucfg_reg) to 1
 	Core0 set core0.CPUECTLR[2:0]  to  non-000 to enable retention ckt (check CA53 TRM)
 	Core0 set GICR_WAKER.ProcessorSleep to 1 (should be in open source code)
 	Core0 wait GICR_WAKER.ChildernAsleep  to 1 (should be in open source code)
 	Core0 set mp0_spmc_resetpwron_config_cpu0 set to 0
 	Core0 set mp0_spmc_pwr_on_cpu0 to low
 	Core0 program CA7L core 0 into standby wait for interrupt
 	SPM or core<n> wait mp0_spmc_pwr_ack to low and then the power off sequence is finished

*/
int spmc_ctrl_mp0_cpu0_autooff(int state)
{
	int err = 0;

	/* TINFO="enable SPM register control" */
	spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPU0" */


        mmio_write_32(CPUSYS0_CPU0_SPMC_CTL,mmio_read_32(CPUSYS0_CPU0_SPMC_CTL)& ~cpu_sw_no_wait_for_q_channel);
        mmio_write_32(CPUSYS0_CPU0_SPMC_CTL,mmio_read_32(CPUSYS0_CPU0_SPMC_CTL)|cpu_sw_spark_en);
		set_cpu_retention_control(1);


        mmio_write_32(MP0_CPU0_PWR_CON,mmio_read_32(MP0_CPU0_PWR_CON)& ~PWR_ON_2ND);
		/* TINFO="Set PWR_ON = 0" */
		spm_write(MP0_CPU0_PWR_CON, spm_read(MP0_CPU0_PWR_CON) & ~PWR_ON);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 0 and CPU_PWR_STATUS_2ND = 0" */
		while ((spm_read(PWR_STATUS) & MP0_CPU0_PWR_STA_MASK)
		       || (spm_read(PWR_STATUS_2ND) & MP0_CPU0_PWR_STA_MASK)) {
				/*  */
		}

#endif
		/* TINFO="Finish to turn off MP0_CPU0" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPU0" */
		/* TINFO="Set PWR_ON = 1" */
		spm_write(MP0_CPU0_PWR_CON, spm_read(MP0_CPU0_PWR_CON) | PWR_ON);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 1 and CPU_PWR_STATUS_2ND = 1" */
		while ((spm_read(PWR_STATUS) & MP0_CPU0_PWR_STA_MASK) != MP0_CPU0_PWR_STA_MASK){

				/*  */
		}
#endif
		/* TINFO="Finish to turn on MP0_CPU0" */
	}
	return err;
}

#if SPMC_SW_MODE
/*
static unsigned int select_spmc_base(int select, unsigned int *nb_srampd)
{
    unsigned int addr_spmc;
    switch(select)
    {
        case 0x1:
            addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
            if(nb_srampd)
                *nb_srampd = 16;
            break;
        case 0x2:
            addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
            if(nb_srampd)
                *nb_srampd = 16;
            break;
        case 0x10:
            addr_spmc = PTP3_CPUTOP_SPMC;
            if(nb_srampd)
                *nb_srampd = 32;
            break;
        default:
            PRINTF_SPMC("Should be not here\n");
            assert(0);
    }
    return addr_spmc;
}
*/
/**
 * Before enable the SW sequenwce, all of the big cores must be turn offset.
 * SO the function cannot be called on a big core
 * HW-API:BigSPMCSwPwrSeqEn
 * select: 0x1:core0 0x2:core1 0x4:all 0x10:top
 *
 */
/*
   1. if Select invalid, exit/return error -1
    For Select
    2. Read  sw_fsm_override  & fsm_state_out
    3. if sw_fsm_override=0 & fsm_state_out=0
    1. Set all CPU SW state to OFF (no sequencing required)
    - sram_pd = 5'h00 for CPU & 6'h00 for TOP
    - sw_logic_*_pdb = 0
    - sw_sram_sleepb = 0
    - sw_iso = 1, sw_sram_isointb = 0
    - sw_hot_plug_reset = 1
    2. Write all cpu sw_fsm_override = 1
    3. Poll/wait for all cpu fsm_state_out = 1
    3. else return error -2 or -3
    "0: Ok
    -1: Invalid parameter
    -2: sw_fsm_override=1
    -3: fsm_state_out=1
    -4: Timeout occurred"

*/

int little_spmc_sw_pwr_seq_en(int select)
{

    unsigned int tmp, result, retry;
    unsigned int addr_spmc; // = select_spmc_base(select, 0);
	switch (select) {
		case 0:
			addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
			break;
		case 1:
			addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
			break;
		case 2:
			addr_spmc = CPUSYS0_CPU2_SPMC_CTL;
			break;
		case 3:
			addr_spmc = CPUSYS0_CPU3_SPMC_CTL;
			break;
		case 4:
			addr_spmc = CPUSYS1_CPU0_SPMC_CTL;
			break;
		case 5:
			addr_spmc = CPUSYS1_CPU1_SPMC_CTL;
			break;
		case 6:
			addr_spmc = CPUSYS1_CPU2_SPMC_CTL;
			break;
		case 7:
			addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
			break;
		case 10://LL cluster
			addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
			break;
		case 20://L cluster
			addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
			break;
		default:
			ERROR("%s() CPU%d not exists\n", __FUNCTION__, (int)select);
			assert(0);
	}

    PRINTF_SPMC("SwPsqE Sel:0x%X Reg:0x%X\n", select, addr_spmc);//big sw power sequence error FSM

    if((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK)==FSM_ON)// TODO: no register
    {
        //FSM_out is not zero
        PRINTF_SPMC("SwPsqE ESt1\n");//big sw power sequence error FSM
        return -3; //-2: fsm_state_out=1

    }

    if((mmio_read_32(addr_spmc) & sw_fsm_override))
    {
        //FSM_out is not zero
        PRINTF_SPMC("SwPsqE EFO1\n");
        return -2; //-1: sw_fsm_override=1
    }


    if(select == 0x10 || select == 0x20){
//??????????????????????????????????????????????????????????????????????????????????
	//This part was moved to kernel
#if 0
        //Enable buck first
        tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1<<0);
        mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release pwr_rst_b

        tmp = mmio_read_32(WDT_SWSYSRST) | 0x88000800;
        mmio_write_32(WDT_SWSYSRST, tmp);

        tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) & ~(0x3);
        mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);//ISOLATE

        tmp = (mmio_read_32(WDT_SWSYSRST) & ~(0x0800)) | 0x88000000;
        mmio_write_32(WDT_SWSYSRST, tmp);
#endif
//??????????????????????????????????????????????????????????????????????????????????
        //TOP
        tmp = mmio_read_32(addr_spmc) & ~((sw_pd) | sw_logic_pdb | sw_logic_pre1_pdb | sw_logic_pre2_pdb| sw_sram_sleepb | sw_sram_isointb);
        tmp |= (sw_iso | sw_hot_plug_reset);
        mmio_write_32(addr_spmc,tmp);

        tmp = mmio_read_32(addr_spmc) | sw_fsm_override;
        PRINTF_SPMC("W TOP_SPMC:%x\n",tmp);
        mmio_write_32(addr_spmc, tmp);
        tmp = mmio_read_32(addr_spmc);
        PRINTF_SPMC("RB TOP_SPMC:%x\n",tmp);
        result = 0;

        for(retry=10;retry>0;retry--){

            if((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK)==FSM_ON){// TODO: no register
                result = 1;
                break;
            }
            udelay(RETRY_TIME_USEC);
        }
        if(!result){
            PRINTF_SPMC("SwPsqE ETopTO\n");//TO=timeout
            return -4;//timeout
        }

/*         while(!((mmio_read_32(PTP3_CPUTOP_SPMC) & FSM_STATE_OUT_MASK)==FSM_ON));*/

    }
	else if(select <= 0x0 && select <= 0x7){

        //core 0
        tmp = mmio_read_32(addr_spmc) & ~(cpu_sw_pd |cpu_sw_pd | cpu_sw_logic_pre1_pdb | cpu_sw_logic_pre2_pdb| cpu_sw_sram_sleepb | cpu_sw_sram_isointb);
        tmp |= (cpu_sw_iso | cpu_sw_hot_plug_reset);
        mmio_write_32(addr_spmc, tmp);


        tmp = mmio_read_32(addr_spmc) | cpu_sw_fsm_override;
        mmio_write_32(addr_spmc, tmp);


/*         while(!((mmio_read_32(PTP3_CPU0_SPMC) & FSM_STATE_OUT_MASK) == FSM_ON));
		return 0;*/
        result = 0;
        for(retry=10;retry>0;retry--){

            if((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK) == FSM_ON){// TODO: no register
                result = 1;
                break;
            }
            udelay(RETRY_TIME_USEC);

        }
        if(!result){
            PRINTF_SPMC("SwPsqE ETO:%x\n",select);
            return -4;//timeout
        }
    }
    else
        return -1;

    return 0;
}

/**
 * HW-API: BigSPMCSwPwrOn
 * selectL 0x1: core 1, 0x2:core 2, 0xF:all cores , 0x10:TOP
 */
/*
"1. if Select invalid, exit/return error -1
2. if Select = CPU0, CPU1, or ALLCPU, read TOP SPMC Status. If not powered on, Power  on TOP first
For each Select:
3. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
4. If already power on, continue to next Select.
5. If sw_fsm_override=0
    1. if sw_pwr_on_override=0
         .1. Write sw_pwr_on_override_en = 1
     2. write sw_pwr_on=1
6. else
        Sequence:
        1. sram_pd gray sequencing from 0 -> 11000
        2. sw_logic_pre1_pdb = 1
        3. Poll/wait logic_pre1_pdbo_all_on_ack = 1
        4. sw_logic_pre2_pdb = 1
        5. Poll/wait logic_pre2_pdbo_all_on_ack = 1
        6. sw_logic_pdb = 1
        7. Poll/wait logic_pdbo_all_on_ack = 1
        8. sw_sram_sleepb = 1
        9. sw_iso = 0, sw_sram_isointb = 1
        10. sw_hot_plug_reset = 0"

*/
char sw_pd_cmd[32]={0x01,0x03,0x02,0x06,0x07,0x05,0x04,0x0c,0x0d,0x0f,
                    0x0e,0x0a,0x0b,0x09,0x08,0x18,0x19,0x1b,0x1a,0x1e,
                    0x1f,0x1d,0x1c,0x14,0x15,0x17,0x16,0x12,0x13,0x11,
                    0x10,0x30};

int little_sw_on_seq(int select){
    PRINTF_SPMC("%s select:%x\n",__FUNCTION__,select);
    unsigned int addr_spmc, tmp,i;
    unsigned int nb_srampd,SW_LOGIC_PDBO_ALL_ON_ACK,SW_PWR_ON_OVERRIDE_EN,SW_PWR_ON,SW_HOT_PLUG_RESET,SW_FSM_OVERRIDE,SRAMPD_OFFSET,SW_PD,SPM_MPx_CPUx_PWR_CON;
    unsigned int SW_LOGIC_PRE1_PDB,SW_LOGIC_PRE1_PDBO_ALL_ON_ACK,SW_LOGIC_PRE2_PDB,SW_LOGIC_PRE2_PDBO_ALL_ON_ACK,SW_LOGIC_PDB,SW_SRAM_ISOINTB;
    //dr_spmc = select_spmc_base(select, &nb_srampd);
    if((select & 0xF0)!=0){/* For cluster*/
			SW_LOGIC_PDBO_ALL_ON_ACK = logic_pdbo_all_on_ack;
			SW_FSM_OVERRIDE = sw_fsm_override;
			SW_PWR_ON_OVERRIDE_EN = sw_pwr_on_override_en;
			SW_PWR_ON = sw_pwr_on;
			SW_HOT_PLUG_RESET = sw_hot_plug_reset;
    		nb_srampd=32;
			SRAMPD_OFFSET=16;
			SW_PD = sw_pd;
			SW_LOGIC_PRE1_PDB = sw_logic_pre1_pdb;
			SW_LOGIC_PRE1_PDBO_ALL_ON_ACK = logic_pre1_pdbo_all_on_ack;
			SW_LOGIC_PRE2_PDB = sw_logic_pre2_pdb;
			SW_LOGIC_PRE2_PDBO_ALL_ON_ACK = logic_pre2_pdbo_all_on_ack;
			SW_LOGIC_PDB = sw_logic_pdb;
			SW_SRAM_ISOINTB	= sw_sram_isointb;
    	}
	else{/* For core */

    		SW_LOGIC_PDBO_ALL_ON_ACK = cpu_pdbo_all_on_ack;
			SW_FSM_OVERRIDE = cpu_sw_fsm_override;
			SW_PWR_ON_OVERRIDE_EN = cpu_sw_powr_on_override_en;
			SW_PWR_ON = cpu_sw_pwr_on;
			SW_HOT_PLUG_RESET = cpu_sw_hot_plug_reset;
    		nb_srampd=16;
			SRAMPD_OFFSET=11;
			SW_PD = cpu_sw_pd;
			SW_LOGIC_PRE1_PDB = cpu_sw_logic_pre1_pdb;
			SW_LOGIC_PRE1_PDBO_ALL_ON_ACK = cpu_pre1_pdbo_allon_ack;
			SW_LOGIC_PRE2_PDB = cpu_sw_logic_pre2_pdb;
			SW_LOGIC_PRE2_PDBO_ALL_ON_ACK = cpu_pre2_pdbo_allon_ack;
			SW_LOGIC_PDB = cpu_sw_logic_pdb;
			SW_SRAM_ISOINTB	= cpu_sw_sram_isointb;
		}

	switch (select) {
		case 0:
			addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP0_CPU0_PWR_CON;
			break;
		case 1:
			addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP0_CPU1_PWR_CON;
			break;
		case 2:
			addr_spmc = CPUSYS0_CPU2_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP0_CPU2_PWR_CON;
			break;
		case 3:
			addr_spmc = CPUSYS0_CPU3_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP0_CPU3_PWR_CON;
			break;
		case 4:
			addr_spmc = CPUSYS1_CPU0_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP1_CPU0_PWR_CON;
			break;
		case 5:
			addr_spmc = CPUSYS1_CPU1_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP1_CPU1_PWR_CON;
			break;
		case 6:
			addr_spmc = CPUSYS1_CPU2_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP1_CPU2_PWR_CON;
			break;
		case 7:
			addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP1_CPU3_PWR_CON;
			break;
		case 10://LL cluster
			addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP0_CPUTOP_PWR_CON;
			break;
		case 20://L cluster
			addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
			SPM_MPx_CPUx_PWR_CON=MP1_CPUTOP_PWR_CON;
			break;
		default:
			ERROR("%s() CPU%d not exists\n", __FUNCTION__, (int)select);
			assert(0);
	}

    if(!addr_spmc){
        PRINTF_SPMC("SwOn Inv");
        return -1;
    }
    if(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_ON_ACK){
        return 0; //turn on already
    }

    PRINTF_SPMC("SwOn ChkOR\n");

    if(!(mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE)){
        tmp = mmio_read_32(addr_spmc);
        if(!( tmp & SW_PWR_ON_OVERRIDE_EN)){
            tmp |= SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(addr_spmc,tmp);
        }
        tmp = mmio_read_32(addr_spmc) | SW_PWR_ON;
        mmio_write_32(addr_spmc,tmp);
    }

    PRINTF_SPMC("SwOn HPRst\n");
    tmp = mmio_read_32(addr_spmc) | SW_HOT_PLUG_RESET;
    mmio_write_32(addr_spmc,tmp);

    PRINTF_SPMC("SwOn OrEn\n");
    tmp |= SW_PWR_ON_OVERRIDE_EN;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn PwrOn\n");

    tmp = mmio_read_32(addr_spmc) | SW_PWR_ON;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn ReRst&ClkDis\n");


    if((select & 0xF0)!=0){//cluster
      tmp = mmio_read_32(SPM_MPx_CPUx_PWR_CON) & ~MP0_CPUTOP_PWR_CLK_DIS_LSB;//all bit0
      mmio_write_32(SPM_MPx_CPUx_PWR_CON, tmp);//Release CPU0_PWR_RST_B
    	}
    tmp = mmio_read_32(SPM_MPx_CPUx_PWR_CON) | MCU_PWR_RST_B_LSB;//all bit0
    mmio_write_32(SPM_MPx_CPUx_PWR_CON, tmp);//Release CPU0_PWR_RST_B


    //power on SRAM
    tmp = mmio_read_32(addr_spmc) & ~SW_PD;

    PRINTF_SPMC("SwOn SRAM\n");
    for(i=0;i<nb_srampd;i++)
    {
        tmp = tmp | (sw_pd_cmd[i]<<SRAMPD_OFFSET);
        mmio_write_32(addr_spmc,tmp);
    }
    PRINTF_SPMC("SwOn PRE1\n");
    tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PRE1_PDB;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn WaitPRE1Ack\n");
    while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PRE1_PDBO_ALL_ON_ACK));//wait for 1
    big_spmc_info();
    PRINTF_SPMC("SwOn PRE2\n");
    tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PRE2_PDB;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn WaitPRE2Ack\n");
    while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PRE2_PDBO_ALL_ON_ACK));//wait for 1
    big_spmc_info();
    PRINTF_SPMC("SwOn PDB\n");
    tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PDB;
    mmio_write_32(addr_spmc,tmp);
    big_spmc_info();
    PRINTF_SPMC("SwOn WaitPDBAck\n");
    while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_ON_ACK));//wait for 1
    big_spmc_info();
    PRINTF_SPMC("SwOn SLEEPB\n");
    if((select & 0xF0)!=0){//cluster
      tmp = mmio_read_32(addr_spmc) & ~sw_sram_sleepb;
      for(i=0;i<nb_srampd;i++){
          tmp = tmp | (sw_pd_cmd[i]<<7);
          mmio_write_32(addr_spmc,tmp);
      	}
    	}
	else{
        tmp = mmio_read_32(addr_spmc) | cpu_sw_sram_sleepb;
        mmio_write_32(addr_spmc,tmp);
		}
    big_spmc_info();
    PRINTF_SPMC("SwOn ISO\n");
    tmp = mmio_read_32(addr_spmc) & ~cpu_sw_iso;
    tmp |= SW_SRAM_ISOINTB;
    mmio_write_32(addr_spmc,tmp);
    big_spmc_info();
    PRINTF_SPMC("SwOn HPDeRST\n");
    tmp = mmio_read_32(addr_spmc) & ~SW_HOT_PLUG_RESET;
    mmio_write_32(addr_spmc,tmp);
    big_spmc_info();

    if(select == 0x10){
        PRINTF_SPMC("SwOn EnPLL\n");
        tmp = mmio_read_32(ARMPLL_CON0) | (1<<0);
        mmio_write_32(ARMPLL_CON0, tmp); //ARMPLL_CON0[0], Enable PLL

        udelay(20);
        PRINTF_SPMC("SwOn SelPLL\n");
        tmp = mmio_read_32(PLL_DIV_MUXL_SEL) | (1<<0);
        mmio_write_32(PLL_DIV_MUXL_SEL, tmp); //pll_div_mux1_sel = 01 = pll clock
        big_spmc_info();
        PRINTF_SPMC("SwOn INFRA_TOPAXI_PROTECTEN1\n");
        tmp = mmio_read_32(INFRA_TOPAXI_PROTECTEN1) & ~(1<<10);
        mmio_write_32(INFRA_TOPAXI_PROTECTEN1, tmp); //bit[10]:pwrdnreqn
        big_spmc_info();

        PRINTF_SPMC("SwOn WaitAck\n");
        while((mmio_read_32(INFRA_TOPAXI_PROTEXTSTA3) & (1<<10)));//bit[10]:pwrdnackn waiting ack,wait for 0
        big_spmc_info();
        PRINTF_SPMC("SwOn GotAck\n");
    }
    return 0;
}

int little_spmc_sw_pwr_on(int select){
    /*power on a core before power on the TOP, we power on TOP automatically*/
    extern void bl31_on_entrypoint(void);
    PRINTF_SPMC("SwPsqOn Sel:%d on:%x\n",select,big_on);

    if(big_on & select)
        return PSCI_E_SUCCESS;

    switch(select){
        case 0x1:
        case 0x2:
#if SPMC_DVT
            big_spmc_info();
#endif
            big_spmc_sw_pwr_seq_en(0x10);
#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_sw_on_seq(0x10);//power on TOP

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            mmio_write_32(0x10222208, 0xf<<16);/*set BIG poweup on AARCH64*/
            unsigned add_idx = select-1;
            mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_L(add_idx), (unsigned long)bl31_on_entrypoint);
            mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_H(add_idx), 0);
            PRINTF_SPMC("mt_on_2, entry H:%x L:%x\n", mmio_read_32(MP2_MISC_CONFIG_BOOT_ADDR_H(add_idx)),mmio_read_32(MP2_MISC_CONFIG_BOOT_ADDR_L(add_idx)));


#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_spmc_sw_pwr_seq_en(select);

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_sw_on_seq(select);

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_on = big_on | select;
#if SPMC_SPARK2
            if(select == 0x1)
                big_spark2_setldo(0, 0);
            big_spark2_core(select+7,1);
#endif
            break;
        case 0xf:
            big_spmc_sw_pwr_seq_en(0x10);
            big_spmc_info();
            big_sw_on_seq(0x10);//power on TOP
            big_spmc_info();
            big_sw_on_seq(0x1);
            big_spmc_info();
#if SPMC_SPARK2
            big_spark2_setldo(0, 0);
            big_spark2_core(8,1);
#endif
            big_sw_on_seq(0x2);
            big_spmc_info();
#if SPMC_SPARK2
            big_spark2_core(9,1);
#endif
            break;
        case 0x10:
            big_spmc_sw_pwr_seq_en(0x10);
            big_spmc_info();
            big_sw_on_seq(0x10);
            big_spmc_info();
            break;
        default:
            return -1;
    }
    return PSCI_E_SUCCESS;
}

/**
 * BigSPMCSwPwrOff
 * selectL 0x1: core 1, 0x2:core 2, 0xF:all cores , 0x10:TOP
 */
/*
"1. if Select invalid, exit/return error -1
        ForSelect: (Do CPU's before TOP)
        2. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
        3. If already power off, continue to next Select.
        4. If sw_fsm_override=0
                1. if sw_pwr_on_override=0
                    1. Write sw_pwr_on_override_en = 1
                2. write sw_pwr_on=0
        5. else
       Sequence:
                1. sw_iso=1, sw_sram_isointb=0
                2. sw_logic*_pdb=0, sw_sram_sleepb=0, sw_pd=6?h00
                3. Poll/wait for logic_*pdb_all_on_ack=0 & logic_pdbo_all_off_ack=1
                4. sw_hot_plug_reset=1"

*/
void little_sw_off_seq(int select){
    unsigned int addr_spmc, tmp;
    switch(select)
    {
        case 0x1:
            addr_spmc = PTP3_CPU0_SPMC;
            break;
        case 0x2:
            addr_spmc = PTP3_CPU1_SPMC;
            break;
        case 0x10:
            addr_spmc = PTP3_CPUTOP_SPMC;
            break;
        default:
            PRINTF_SPMC("Should be not here\n");
            assert(0);
    }
    if(!(mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK))
        return; /*turn off already*/
    if(!(mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE)){
        tmp = mmio_read_32(addr_spmc);
        if(!( tmp & SW_PWR_ON_OVERRIDE_EN)){
            tmp |= SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(addr_spmc,tmp);
        }
        tmp &= ~SW_PWR_ON;
        mmio_write_32(addr_spmc,tmp);
    }
    {
        if(select == 0x10){
            PRINTF_SPMC("Wait CL3's WFI\n");
            while(!(mmio_read_32(SPM_CPU_IDLE_STA2)&(1<<(20))));
            PRINTF_SPMC("CL3 is in WFI\n");
            tmp = mmio_read_32(PLL_DIV_MUXL_SEL) & ~(1<<0);
            mmio_write_32(PLL_DIV_MUXL_SEL, tmp); /*pll_div_mux1_sel = 26MHZ*/
            tmp = mmio_read_32(ARMPLL_CON0) & ~(1<<0);
            mmio_write_32(ARMPLL_CON0, tmp); /*ARMPLL_CON0[0], Disable PLL*/

        }else{
            PRINTF_SPMC("Wait CPU WFI mask:%x\n",select);
            while(!(mmio_read_32(SPM_CPU_IDLE_STA2)&(1<<(9+select))));
            PRINTF_SPMC("CPU is in WFI, mask:%x\n",select);
        }
        tmp = mmio_read_32(addr_spmc) | SW_ISO;
        tmp &= ~SW_SRAM_ISOINTB;
        mmio_write_32(addr_spmc,tmp);

        tmp = mmio_read_32(addr_spmc) & ~(SW_LOGIC_PRE1_PDB | SW_LOGIC_PRE2_PDB | SW_LOGIC_PDB | SW_SRAM_SLEEPB | (0x3f<<SRAMPD_OFFSET));
        mmio_write_32(addr_spmc,tmp);

        while((mmio_read_32(addr_spmc) & SW_LOGIC_PRE1_PDBO_ALL_ON_ACK));/*wait for 0*/
        while((mmio_read_32(addr_spmc) & SW_LOGIC_PRE2_PDBO_ALL_ON_ACK));/*wait for 0*/
        while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_OFF_ACK));/*wait for 1*/

        tmp = mmio_read_32(addr_spmc) | SW_HOT_PLUG_RESET;
        mmio_write_32(addr_spmc,tmp);

        if(select == 0x10){
/*??????????????????????????????????????????????????????????????????????????????????*/
            tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) & ~(1<<0);
            mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release pwr_rst_b

            tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1<<4);
            mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release

            tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) | (0x2);
            mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);//ISOLATE
/*??????????????????????????????????????????????????????????????????????????????????*/
        }

    }
}

int little_spmc_sw_pwr_off(int select){
    switch(select){
        case 0x1:
        case 0x2:
#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_sw_off_seq(select);

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_on &= ~(1<<(select-1));
            PRINTF_SPMC("%s big_on:%x\n",__FUNCTION__,big_on);
            if(!big_on){
                big_sw_off_seq(0x10);
#if SPMC_DVT
                udelay(SPMC_DVT_UDELAY);
                big_spmc_info();
#endif
            }
            break;
        case 0xf:
            big_sw_off_seq(0x1);
            big_sw_off_seq(0x2);
            break;
        case 0x10:
            big_sw_off_seq(0x1);
            big_sw_off_seq(0x2);
            big_sw_off_seq(0x10);
            break;
        default:
            return -1;
    }
    return 0;

}

/**
 * BigSPMCSwPwrCntrlDisable
 * select : 0x1: core0, 0x2:core1 0xF:all 0x10:TOP
 * return : 0: Ok -1: Invalid Select value -2: Invalid request.

 */
/*
"1. if Select invalid, exit/return error -1
2. For Select, read sw_fsm_override.
3. If sw_fsm_override = 1
    1. return error -2
4. else
    1. Write all sw_pwr_on_override_en = 0"

*/

int little_spmc_sw_pwr_cntrl_disable(int select)
{
    unsigned int tmp;
    switch(select){
        case 0x1:
            if(mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE){
                return -2;
            }
            else{
                tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
                mmio_write_32(PTP3_CPU0_SPMC, tmp);
            }
            break;
        case 0x2:
            if(mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE){
                return -2;
            }
            else{
                tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
                mmio_write_32(PTP3_CPU1_SPMC, tmp);
            }
            break;
        case 0xf:
            if((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
                (mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)){
                return -2;
            }
            tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU0_SPMC, tmp);
            tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU1_SPMC, tmp);
            break;
        case 0x10:
            if((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
                (mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)){
                return -2;
            }
            tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU0_SPMC, tmp);
            tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU1_SPMC, tmp);
            tmp = mmio_read_32(PTP3_CPUTOP_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPUTOP_SPMC, tmp);
        default:
            return -1;
    }
    return 0;
}





/* SWBig BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB */
static unsigned int select_spmc_base(int select, unsigned int *nb_srampd)
{
    unsigned int addr_spmc;
    switch(select)
    {
        case 0x1:
            addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
            if(nb_srampd)
                *nb_srampd = 16;
            break;
        case 0x2:
            addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
            if(nb_srampd)
                *nb_srampd = 16;
            break;
        case 0x10:
            addr_spmc = PTP3_CPUTOP_SPMC;
            if(nb_srampd)
                *nb_srampd = 32;
            break;
        default:
            PRINTF_SPMC("Should be not here\n");
            assert(0);
    }
    return addr_spmc;
}
/**
 * Before enable the SW sequenwce, all of the big cores must be turn offset.
 * SO the function cannot be called on a big core
 * HW-API:BigSPMCSwPwrSeqEn
 * select: 0x1:core0 0x2:core1 0x4:all 0x10:top
 *
 */
/*
   1. if Select invalid, exit/return error -1
    For Select
    2. Read  sw_fsm_override  & fsm_state_out
    3. if sw_fsm_override=0 & fsm_state_out=0
    1. Set all CPU SW state to OFF (no sequencing required)
    - sram_pd = 5'h00 for CPU & 6'h00 for TOP
    - sw_logic_*_pdb = 0
    - sw_sram_sleepb = 0
    - sw_iso = 1, sw_sram_isointb = 0
    - sw_hot_plug_reset = 1
    2. Write all cpu sw_fsm_override = 1
    3. Poll/wait for all cpu fsm_state_out = 1
    3. else return error -2 or -3
    "0: Ok
    -1: Invalid parameter
    -2: sw_fsm_override=1
    -3: fsm_state_out=1
    -4: Timeout occurred"

*/

int big_spmc_sw_pwr_seq_en(int select)
{

    unsigned int tmp, result, retry;
    unsigned int addr_spmc = select_spmc_base(select, 0);
    PRINTF_SPMC("SwPsqE Sel:0x%X Reg:0x%X\n", select, addr_spmc);//big sw power sequence error FSM

    if((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK)==FSM_ON)
    {
        //FSM_out is not zero
        PRINTF_SPMC("SwPsqE ESt1\n");//big sw power sequence error FSM
        return -3; //-2: fsm_state_out=1

    }

    if((mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE))
    {
        //FSM_out is not zero
        PRINTF_SPMC("SwPsqE EFO1\n");
        return -2; //-1: sw_fsm_override=1
    }


    if(select == 0x10){
/*??????????????????????????????????????????????????????????????????????????????????*/
        //Enable buck first
        tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1<<0);
        mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release pwr_rst_b

        tmp = mmio_read_32(WDT_SWSYSRST) | 0x88000800;
        mmio_write_32(WDT_SWSYSRST, tmp);

        tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) & ~(0x3);
        mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);//ISOLATE

        tmp = (mmio_read_32(WDT_SWSYSRST) & ~(0x0800)) | 0x88000000;
        mmio_write_32(WDT_SWSYSRST, tmp);
/*??????????????????????????????????????????????????????????????????????????????????*/
        //TOP
        tmp = mmio_read_32(addr_spmc) & ~((0x3f<<SRAMPD_OFFSET) | SW_LOGIC_PDB | SW_LOGIC_PRE1_PDB | SW_LOGIC_PRE2_PDB| SW_SRAM_SLEEPB | SW_SRAM_ISOINTB);
        tmp |= (SW_ISO | SW_HOT_PLUG_RESET);
        mmio_write_32(addr_spmc,tmp);

        tmp = mmio_read_32(addr_spmc) | SW_FSM_OVERRIDE;
        PRINTF_SPMC("W TOP_SPMC:%x\n",tmp);
        mmio_write_32(addr_spmc, tmp);
        tmp = mmio_read_32(addr_spmc);
        PRINTF_SPMC("RB TOP_SPMC:%x\n",tmp);
        result = 0;

        for(retry=10;retry>0;retry--){

            if((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK)==FSM_ON){
                result = 1;
                break;
            }
            udelay(RETRY_TIME_USEC);
        }
        if(!result){
            PRINTF_SPMC("SwPsqE ETopTO\n");/*TO=timeout*/
            return -4;//timeout
        }


    }else if(select == 0x1 || select == 0x2){

        //core 0
        tmp = mmio_read_32(addr_spmc) & ~((0x1f<<SRAMPD_OFFSET) |SW_LOGIC_PDB | SW_LOGIC_PRE1_PDB | SW_LOGIC_PRE2_PDB| SW_SRAM_SLEEPB | SW_SRAM_ISOINTB);
        tmp |= (SW_ISO | SW_HOT_PLUG_RESET);
        mmio_write_32(addr_spmc, tmp);


        tmp = mmio_read_32(addr_spmc) | SW_FSM_OVERRIDE;
        mmio_write_32(addr_spmc, tmp);


        result = 0;
        for(retry=10;retry>0;retry--){

            if((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK) == FSM_ON){
                result = 1;
                break;
            }
            udelay(RETRY_TIME_USEC);

        }
        if(!result){
            PRINTF_SPMC("SwPsqE ETO:%x\n",select);
            return -4;//timeout
        }
    }
    else
        return -1;

    return 0;
}

/**
 * HW-API: BigSPMCSwPwrOn
 * selectL 0x1: core 1, 0x2:core 2, 0xF:all cores , 0x10:TOP
 */
/*
"1. if Select invalid, exit/return error -1
2. if Select = CPU0, CPU1, or ALLCPU, read TOP SPMC Status. If not powered on, Power  on TOP first
For each Select:
3. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
4. If already power on, continue to next Select.
5. If sw_fsm_override=0
    1. if sw_pwr_on_override=0
         .1. Write sw_pwr_on_override_en = 1
     2. write sw_pwr_on=1
6. else
        Sequence:
        1. sram_pd gray sequencing from 0 -> 11000
        2. sw_logic_pre1_pdb = 1
        3. Poll/wait logic_pre1_pdbo_all_on_ack = 1
        4. sw_logic_pre2_pdb = 1
        5. Poll/wait logic_pre2_pdbo_all_on_ack = 1
        6. sw_logic_pdb = 1
        7. Poll/wait logic_pdbo_all_on_ack = 1
        8. sw_sram_sleepb = 1
        9. sw_iso = 0, sw_sram_isointb = 1
        10. sw_hot_plug_reset = 0"


char sw_pd_cmd[32]={0x01,0x03,0x02,0x06,0x07,0x05,0x04,0x0c,0x0d,0x0f,
                    0x0e,0x0a,0x0b,0x09,0x08,0x18,0x19,0x1b,0x1a,0x1e,
                    0x1f,0x1d,0x1c,0x14,0x15,0x17,0x16,0x12,0x13,0x11,
                    0x10,0x30};
*/
int big_sw_on_seq(int select){
    PRINTF_SPMC("%s select:%x\n",__FUNCTION__,select);
    unsigned int addr_spmc, tmp;
    unsigned int nb_srampd;
    addr_spmc = select_spmc_base(select, &nb_srampd);
    if(!addr_spmc){
        PRINTF_SPMC("SwOn Inv");
        return -1;
    }
    if(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_ON_ACK){
        return 0; //turn on already
    }

    PRINTF_SPMC("SwOn ChkOR\n");

    if(!(mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE)){
        tmp = mmio_read_32(addr_spmc);
        if(!( tmp & SW_PWR_ON_OVERRIDE_EN)){
            tmp |= SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(addr_spmc,tmp);
        }
        tmp = mmio_read_32(addr_spmc) | SW_PWR_ON;
        mmio_write_32(addr_spmc,tmp);
    }
/*??????????????????????????????????????????????????????????????????????????????????*/
    PRINTF_SPMC("SwOn HPRst\n");
    tmp = mmio_read_32(addr_spmc) | SW_HOT_PLUG_RESET;
    mmio_write_32(addr_spmc,tmp);

    PRINTF_SPMC("SwOn OrEn\n");
    tmp |= SW_PWR_ON_OVERRIDE_EN;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn PwrOn\n");

    tmp = mmio_read_32(addr_spmc) | SW_PWR_ON;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn ReRst&ClkDis\n");

    if(select == 0x1){
        tmp = mmio_read_32(SPM_MP2_CPU0_PWR_CON+((0)<<2)) | (1<<0);
        mmio_write_32(SPM_MP2_CPU0_PWR_CON+((0)<<2), tmp);//Release CPU0_PWR_RST_B
    }else if(select == 0x2){
    tmp = mmio_read_32(SPM_MP2_CPU0_PWR_CON+((1)<<2)) | (1<<0);
    mmio_write_32(SPM_MP2_CPU0_PWR_CON+((1)<<2), tmp);//Release CPU0_PWR_RST_B
    }
    else if(select == 0x10){
        tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) & ~(1<<4);
        mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release clk_dis
        tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1<<0);
        mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release pwr_rst_b
    }
    else
        return -1;
/*??????????????????????????????????????????????????????????????????????????????????*/

    /*power on SRAM*/
    int i;
    unsigned int orig_sram_pd = mmio_read_32(addr_spmc) & 0xFFFF03FF;

    PRINTF_SPMC("SwOn SRAM\n");
    for(i=0;i<nb_srampd;i++)
    {
        tmp = orig_sram_pd | (sw_pd_cmd[i]<<SRAMPD_OFFSET);
        mmio_write_32(addr_spmc,tmp);
    }
    PRINTF_SPMC("SwOn PRE1\n");
    tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PRE1_PDB;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn WaitPRE1Ack\n");
    while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PRE1_PDBO_ALL_ON_ACK));/*wait for 1*/
    big_spmc_info();
    PRINTF_SPMC("SwOn PRE2\n");
    tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PRE2_PDB;
    mmio_write_32(addr_spmc,tmp);
    PRINTF_SPMC("SwOn WaitPRE2Ack\n");
    while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PRE2_PDBO_ALL_ON_ACK));/*wait for 1*/
    big_spmc_info();
    PRINTF_SPMC("SwOn PDB\n");
    tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PDB;
    mmio_write_32(addr_spmc,tmp);
    big_spmc_info();
    PRINTF_SPMC("SwOn WaitPDBAck\n");
    while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_ON_ACK));/*wait for 1*/
    big_spmc_info();
    PRINTF_SPMC("SwOn SLEEPB\n");
    tmp = mmio_read_32(addr_spmc) | SW_SRAM_SLEEPB;
    mmio_write_32(addr_spmc,tmp);
    big_spmc_info();
    PRINTF_SPMC("SwOn ISO\n");
    tmp = mmio_read_32(addr_spmc) & ~SW_ISO;
    tmp |= SW_SRAM_ISOINTB;
    mmio_write_32(addr_spmc,tmp);
    big_spmc_info();
    PRINTF_SPMC("SwOn HPDeRST\n");
    tmp = mmio_read_32(addr_spmc) & ~SW_HOT_PLUG_RESET;
    mmio_write_32(addr_spmc,tmp);
    big_spmc_info();

    if(select == 0x10){
        PRINTF_SPMC("SwOn EnPLL\n");
        tmp = mmio_read_32(ARMPLL_CON0) | (1<<0);
        mmio_write_32(ARMPLL_CON0, tmp); /*ARMPLL_CON0[0], Enable PLL*/

        udelay(20);
        PRINTF_SPMC("SwOn SelPLL\n");
        tmp = mmio_read_32(PLL_DIV_MUXL_SEL) | (1<<0);
        mmio_write_32(PLL_DIV_MUXL_SEL, tmp); /*pll_div_mux1_sel = 01 = pll clock*/
        big_spmc_info();
        PRINTF_SPMC("SwOn INFRA_TOPAXI_PROTECTEN1\n");
        tmp = mmio_read_32(INFRA_TOPAXI_PROTECTEN1) & ~(1<<10);
        mmio_write_32(INFRA_TOPAXI_PROTECTEN1, tmp); /*bit[10]:pwrdnreqn*/
        big_spmc_info();

        PRINTF_SPMC("SwOn WaitAck\n");
        while((mmio_read_32(INFRA_TOPAXI_PROTEXTSTA3) & (1<<10)));/*bit[10]:pwrdnackn waiting ack,wait for 0*/
        big_spmc_info();
        PRINTF_SPMC("SwOn GotAck\n");
    }
    return 0;
}

int big_spmc_sw_pwr_on(int select){
    /*power on a core before power on the TOP, we power on TOP automatically*/
    extern void bl31_on_entrypoint(void);
    PRINTF_SPMC("SwPsqOn Sel:%d on:%x\n",select,big_on);

    if(big_on & select)
        return PSCI_E_SUCCESS;

    switch(select){
        case 0x1:
        case 0x2:
#if SPMC_DVT
            big_spmc_info();
#endif
            big_spmc_sw_pwr_seq_en(0x10);
#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_sw_on_seq(0x10);/*power on TOP*/

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            mmio_write_32(0x10222208, 0xf<<16);/*set BIG poweup on AARCH64*/
            unsigned add_idx = select-1;
            mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_L(add_idx), (unsigned long)bl31_on_entrypoint);
            mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_H(add_idx), 0);
            PRINTF_SPMC("mt_on_2, entry H:%x L:%x\n", mmio_read_32(MP2_MISC_CONFIG_BOOT_ADDR_H(add_idx)),mmio_read_32(MP2_MISC_CONFIG_BOOT_ADDR_L(add_idx)));
/*            PRINTF_SPMC("Write 0x10208008=%d\n",8+add_idx);
            mmio_write_32(0x10208008, 8+add_idx);//discussed with Scott*/

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_spmc_sw_pwr_seq_en(select);

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_sw_on_seq(select);

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_on = big_on | select;
#if SPMC_SPARK2
            if(select == 0x1)
                big_spark2_setldo(0, 0);
            big_spark2_core(select+7,1);
#endif
            break;
        case 0xf:
            big_spmc_sw_pwr_seq_en(0x10);
            big_spmc_info();
            big_sw_on_seq(0x10);/*power on TOP*/
            big_spmc_info();
            big_sw_on_seq(0x1);
            big_spmc_info();
#if SPMC_SPARK2
            big_spark2_setldo(0, 0);
            big_spark2_core(8,1);
#endif
            big_sw_on_seq(0x2);
            big_spmc_info();
#if SPMC_SPARK2
            big_spark2_core(9,1);
#endif
            break;
        case 0x10:
            big_spmc_sw_pwr_seq_en(0x10);
            big_spmc_info();
            big_sw_on_seq(0x10);
            big_spmc_info();
            break;
        default:
            return -1;
    }
    return PSCI_E_SUCCESS;
}

/**
 * BigSPMCSwPwrOff
 * selectL 0x1: core 1, 0x2:core 2, 0xF:all cores , 0x10:TOP
 */
/*
"1. if Select invalid, exit/return error -1
        ForSelect: (Do CPU's before TOP)
        2. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
        3. If already power off, continue to next Select.
        4. If sw_fsm_override=0
                1. if sw_pwr_on_override=0
                    1. Write sw_pwr_on_override_en = 1
                2. write sw_pwr_on=0
        5. else
       Sequence:
                1. sw_iso=1, sw_sram_isointb=0
                2. sw_logic*_pdb=0, sw_sram_sleepb=0, sw_pd=6?h00
                3. Poll/wait for logic_*pdb_all_on_ack=0 & logic_pdbo_all_off_ack=1
                4. sw_hot_plug_reset=1"

*/
void big_sw_off_seq(int select){
    unsigned int addr_spmc, tmp;
    switch(select)
    {
        case 0x1:
            addr_spmc = PTP3_CPU0_SPMC;
            break;
        case 0x2:
            addr_spmc = PTP3_CPU1_SPMC;
            break;
        case 0x10:
            addr_spmc = PTP3_CPUTOP_SPMC;
            break;
        default:
            PRINTF_SPMC("Should be not here\n");
            assert(0);
    }
    if(!(mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK))
        return; /*turn off already*/
    if(!(mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE)){
        tmp = mmio_read_32(addr_spmc);
        if(!( tmp & SW_PWR_ON_OVERRIDE_EN)){
            tmp |= SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(addr_spmc,tmp);
        }
        tmp &= ~SW_PWR_ON;
        mmio_write_32(addr_spmc,tmp);
    }
    {
        if(select == 0x10){
            PRINTF_SPMC("Wait CL3's WFI\n");
            while(!(mmio_read_32(SPM_CPU_IDLE_STA2)&(1<<(20))));
            PRINTF_SPMC("CL3 is in WFI\n");
            tmp = mmio_read_32(PLL_DIV_MUXL_SEL) & ~(1<<0);
            mmio_write_32(PLL_DIV_MUXL_SEL, tmp); /*pll_div_mux1_sel = 26MHZ*/
            tmp = mmio_read_32(ARMPLL_CON0) & ~(1<<0);
            mmio_write_32(ARMPLL_CON0, tmp); /*ARMPLL_CON0[0], Disable PLL*/

        }else{
            PRINTF_SPMC("Wait CPU WFI mask:%x\n",select);
            while(!(mmio_read_32(SPM_CPU_IDLE_STA2)&(1<<(9+select))));
            PRINTF_SPMC("CPU is in WFI, mask:%x\n",select);
        }
        tmp = mmio_read_32(addr_spmc) | SW_ISO;
        tmp &= ~SW_SRAM_ISOINTB;
        mmio_write_32(addr_spmc,tmp);

        tmp = mmio_read_32(addr_spmc) & ~(SW_LOGIC_PRE1_PDB | SW_LOGIC_PRE2_PDB | SW_LOGIC_PDB | SW_SRAM_SLEEPB | (0x3f<<SRAMPD_OFFSET));
        mmio_write_32(addr_spmc,tmp);

        while((mmio_read_32(addr_spmc) & SW_LOGIC_PRE1_PDBO_ALL_ON_ACK));/*wait for 0*/
        while((mmio_read_32(addr_spmc) & SW_LOGIC_PRE2_PDBO_ALL_ON_ACK));/*wait for 0*/
        while(!(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_OFF_ACK));/*wait for 1*/

        tmp = mmio_read_32(addr_spmc) | SW_HOT_PLUG_RESET;
        mmio_write_32(addr_spmc,tmp);

        if(select == 0x10){
/*??????????????????????????????????????????????????????????????????????????????????*/
            tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) & ~(1<<0);
            mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release pwr_rst_b

            tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1<<4);
            mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);//Release

            tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) | (0x2);
            mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);//ISOLATE
/*??????????????????????????????????????????????????????????????????????????????????*/
        }

    }
}

int big_spmc_sw_pwr_off(int select){
    switch(select){
        case 0x1:
        case 0x2:
#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_sw_off_seq(select);

#if SPMC_DVT
            udelay(SPMC_DVT_UDELAY);
            big_spmc_info();
#endif

            big_on &= ~(1<<(select-1));
            PRINTF_SPMC("%s big_on:%x\n",__FUNCTION__,big_on);
            if(!big_on){
                big_sw_off_seq(0x10);
#if SPMC_DVT
                udelay(SPMC_DVT_UDELAY);
                big_spmc_info();
#endif
            }
            break;
        case 0xf:
            big_sw_off_seq(0x1);
            big_sw_off_seq(0x2);
            break;
        case 0x10:
            big_sw_off_seq(0x1);
            big_sw_off_seq(0x2);
            big_sw_off_seq(0x10);
            break;
        default:
            return -1;
    }
    return 0;

}

/**
 * BigSPMCSwPwrCntrlDisable
 * select : 0x1: core0, 0x2:core1 0xF:all 0x10:TOP
 * return : 0: Ok -1: Invalid Select value -2: Invalid request.

 */
/*
"1. if Select invalid, exit/return error -1
2. For Select, read sw_fsm_override.
3. If sw_fsm_override = 1
    1. return error -2
4. else
    1. Write all sw_pwr_on_override_en = 0"

*/

int big_spmc_sw_pwr_cntrl_disable(int select)
{
    unsigned int tmp;
    switch(select){
        case 0x1:
            if(mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE){
                return -2;
            }
            else{
                tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
                mmio_write_32(PTP3_CPU0_SPMC, tmp);
            }
            break;
        case 0x2:
            if(mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE){
                return -2;
            }
            else{
                tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
                mmio_write_32(PTP3_CPU1_SPMC, tmp);
            }
            break;
        case 0xf:
            if((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
                (mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)){
                return -2;
            }
            tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU0_SPMC, tmp);
            tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU1_SPMC, tmp);
            break;
        case 0x10:
            if((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
                (mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)){
                return -2;
            }
            tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU0_SPMC, tmp);
            tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPU1_SPMC, tmp);
            tmp = mmio_read_32(PTP3_CPUTOP_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
            mmio_write_32(PTP3_CPUTOP_SPMC, tmp);
        default:
            return -1;
    }
    return 0;
}
#endif /* SPMC_SW_MODE */
#if SPMC_SPARK2
/*
"1. Convert mVolts to 6 bit value vretcntrl (exit/return -1 if out of range)
2. Set sparkvretcntrl
3. Write for all CPUs, sw_spark_en = 1"

*/
/*switch 0:off 1:on*/
void big_spark2_setldo(unsigned int cpu0_amuxsel, unsigned int cpu1_amuxsel){
    unsigned int tmp;
    unsigned int sparkvretcntrl = 0x3f;
    PRINTF_SPMC("%s sparkvretcntrl=%x",__FUNCTION__,sparkvretcntrl);
    if(cpu0_amuxsel>7 || cpu1_amuxsel>7){
        return;
    }
    tmp =  cpu1_amuxsel<<9 | cpu0_amuxsel<<6 | sparkvretcntrl;
    mmio_write_32(SPARK2LDO,tmp);
}

int big_spark2_core(unsigned int core, unsigned int sw){
    unsigned int tmp;
    if(sw>1 || core <8 || core>9)
        return -1;
    PRINTF_SPMC("%s core:%d sw:%d\n",__FUNCTION__,core,sw);
    if(core==9){
        tmp = ((mmio_read_32(PTP3_CPU1_SPMC)>>1)<<1) | sw;
        PRINTF_SPMC("Write %x = %x\n",PTP3_CPU1_SPMC,tmp);
        mmio_write_32(PTP3_CPU1_SPMC, tmp);
    }else
    {
        tmp = ((mmio_read_32(PTP3_CPU0_SPMC)>>1)<<1) | sw;
        PRINTF_SPMC("Write %x = %x\n",PTP3_CPU0_SPMC,tmp);
        mmio_write_32(PTP3_CPU0_SPMC, tmp);
    }
    return 0;
}

int little_spark2_setldo(unsigned int core){
    if(core>7)
        return -1;
    unsigned long long base_vret;
    unsigned int offset, tmp, sparkvretcntrl = 0x3f;
    PRINTF_SPMC("%s sparkvretcntrl=%x",__FUNCTION__,sparkvretcntrl);
    if(core<4){
        offset = core;
        base_vret = CPUSYS0_SPARKVRETCNTRL;
    }
    else
    {
        offset = core-4;
        base_vret = CPUSYS1_SPARKVRETCNTRL;
    }
    tmp = (mmio_read_32(base_vret) & ~((0x3f) << (offset<<3))) | (sparkvretcntrl << (offset<<3));
    mmio_write_32(base_vret, tmp);
    return 0;
}

int little_spark2_core(unsigned int core, unsigned int sw){
    if(core>7 || sw >1)
        return -1;
    unsigned int offset, tmp;
    unsigned long long base_ctrl;
    if(core<4){
        offset = core;
        base_ctrl = CPUSYS0_SPARKEN;
    }
    else
    {
        offset = core-4;
        base_ctrl = CPUSYS1_SPARKEN;
    }

    tmp = (mmio_read_32(base_ctrl) & ~(1<<offset)) | (sw<<offset);
    mmio_write_32(base_ctrl, tmp);
    return 0;
}
#endif
