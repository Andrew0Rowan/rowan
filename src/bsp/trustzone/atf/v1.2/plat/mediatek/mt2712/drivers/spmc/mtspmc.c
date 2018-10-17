/*
 * Copyright (c) 2015,  ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms,  with or without
 * modification,  are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,  this
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
 * AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO,  THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 * CONSEQUENTIAL DAMAGES (INCLUDING,  BUT NOT LIMITED TO,  PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  DATA,  OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <mtspmc.h>

unsigned int cpu_bitmask __section("tzfw_coherent_mem");
char big_on = 0;		/* at most 4 cores */
char little_on = 0x1;		/* [7:0] = core7~core0,  core 0 is power-on in defualt */

int mcsib_sw_workaround_main(void);
void spark2_setldo_AMUXSEL(int cputop_mpx, unsigned int cpu_corex, unsigned int amuxsel,unsigned int vret);
unsigned long read_cpuectlr(void);
void write_cpuectlr(unsigned long);
#if 0
void little_spmc_info(void)
{
	PRINTF_SPMC("============== cluster0 =================\n");
	PRINTF_SPMC("Little SW HW T0:0x%x \t0x%x\n", mmio_read_32(MP0_CPUTOP_SPMC_CTL),
		    mmio_read_32(MP0_CPUTOP_SPMC_CTL));
	PRINTF_SPMC("Little SW HW c0:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU0_SPMC_CTL),
		    mmio_read_32(CPUSYS0_CPU0_SPMC_CTL));
	PRINTF_SPMC("Little SW HW c1:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU1_SPMC_CTL),
		    mmio_read_32(CPUSYS0_CPU1_SPMC_CTL));
	PRINTF_SPMC("Little SW HW c2:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU2_SPMC_CTL),
		    mmio_read_32(CPUSYS0_CPU2_SPMC_CTL));
	PRINTF_SPMC("Little SW HW c3:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU3_SPMC_CTL),
		    mmio_read_32(CPUSYS0_CPU3_SPMC_CTL));
	PRINTF_SPMC("============== cluster1 =================\n");
	PRINTF_SPMC("Little SW HW T1:0x%x \t0x%x\n", mmio_read_32(MP1_CPUTOP_SPMC_CTL),
		    mmio_read_32(MP1_CPUTOP_SPMC_CTL));
	PRINTF_SPMC("Little SW HW c4:0x%x \t0x%x\n", mmio_read_32(CPUSYS1_CPU0_SPMC_CTL),
		    mmio_read_32(CPUSYS1_CPU0_SPMC_CTL));
	PRINTF_SPMC("Little SW HW c5:0x%x \t0x%x\n", mmio_read_32(CPUSYS1_CPU1_SPMC_CTL),
		    mmio_read_32(CPUSYS1_CPU1_SPMC_CTL));
	PRINTF_SPMC("=========================================\n");
}

void little_wfi_wfe_status(){
	unsigned int tmp;
	PRINTF_SPMC("CPU_IDLE_STA_0x%x=0x%x\n", CPU_IDLE_STA,mmio_read_32(CPU_IDLE_STA));

	tmp=mmio_read_32(MP0_CA7_MISC_CONFIG);
	PRINTF_SPMC("MP0_CA7_MISC_CONFIG%x=0x%x\n",MP0_CA7_MISC_CONFIG,tmp);
	PRINTF_SPMC("cor3~0_WFE=%d %d %d %d\n",(tmp&MP0_CPU3_STANDBYWFE)>>23,(tmp&MP0_CPU2_STANDBYWFE)>>22,(tmp&MP0_CPU1_STANDBYWFE)>>21,(tmp&MP0_CPU0_STANDBYWFE)>>20);
	mmio_write_32(CA15M_MON_SEL, (mmio_read_32(CA15M_MON_SEL) & 0xFF) | 0x1b);
	tmp=mmio_read_32(MP1_CA15M_MON_L);
	PRINTF_SPMC("MP1_CA7_MISC_CONFIG%x=0x%x\n",MP1_CA15M_MON_L,tmp);
	PRINTF_SPMC("cor5~4_WFE=%d %d\n",(tmp&MP1_CPU1_STANDBYWFE)>>5,(tmp&MP1_CPU0_STANDBYWFE)>>4);
}

static void big_spmc_info(void)
{
// if it print CPUSYS2_CPU0_SPMC_CTL and CPUSYS2_CPU1_SPMC_CTL when cluster on power off state.
//the cluster poser on flow will fail
PRINTF_SPMC("Big SPMC CTL Top2:0x%x\n",mmio_read_32(MP2_CPUTOP_SPMC_CTL));
PRINTF_SPMC("Big SPMC STA Top2:0x%x\n",mmio_read_32(MP2_CPUTOP_SPMC_STA));
/*
PRINTF_SPMC("Big SwSeq SPMC T2:0x%x C0:0x%x C1:0x%x\n",
				mmio_read_32(MP2_CPUTOP_SPMC_CTL),
				mmio_read_32(CPUSYS2_CPU0_SPMC_CTL),
				mmio_read_32(CPUSYS2_CPU1_SPMC_CTL));
PRINTF_SPMC("Big SwSeq SPMC T2:0x%x C0:0x%x C1:0x%x\n",
				mmio_read_32(MP2_CPUTOP_SPMC_STA),
				mmio_read_32(CPUSYS2_CPU0_SPMC_STA),
				mmio_read_32(CPUSYS2_CPU1_SPMC_STA));
PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
*/
}
#endif
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
GG
	Set bypass_cpu_spmc_mode = 0
	Wait 200ns
	FOR ( each cluster n)
	FOR (each core m)
			Set mp<n>_spmc_resetpwron_config_cpu<m> = 0
			Set mp<n>_spmc_pwr_rst_cpu<m> = 0
		ENDFOR
		Set mp<n>_spmc_resetpwron_config_cputop = 0
		Set mp<n>_spmc_pwr_rst_cputop = 0
		Set mp<n>_spmc_pwr_clk_dis_cputop = 0
	ENDFOR
*/

int spmc_init(void)
{
	int err = 0;

	cpu_bitmask = 1;

	/* TINFO="enable SPM register control" */
	mmio_write_32(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	PRINTF_SPMC("SPM_POWERON_CONFIG_SET_0x%x=0x%x\n", SPM_POWERON_CONFIG_SET, mmio_read_32(SPM_POWERON_CONFIG_SET));
#if CONFIG_SPMC_MODE != 0
	mmio_write_32(SPMC_BYPASS, 0x0);	/* de-assert Bypass SPMC  0: SPMC mode  1: Legacy mode */
	PRINTF_SPMC("BYPASS_SPMC_0x%x=0x%x\n", SPMC_BYPASS, mmio_read_32(SPMC_BYPASS));
	/* udelay(200); */
	PRINTF_SPMC("[%s]change to SPMC mode !!!\n", __func__);
#endif
/* MP0 SPMC power Ctrl signals */
	mmio_write_32(SPMC_MP0_CPU0_PWR_CON,  mmio_read_32(SPMC_MP0_CPU0_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPU0_PWR_CON,  mmio_read_32(SPMC_MP0_CPU0_PWR_CON) & ~SPMC_PWR_RST_B);/*Polarity don't need to inverse*/
	mmio_write_32(SPMC_MP0_CPU1_PWR_CON,  mmio_read_32(SPMC_MP0_CPU1_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPU1_PWR_CON,  mmio_read_32(SPMC_MP0_CPU1_PWR_CON) & ~SPMC_PWR_RST_B);/*Polarity don't need to inverse*/
	mmio_write_32(SPMC_MP0_CPU2_PWR_CON,  mmio_read_32(SPMC_MP0_CPU2_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPU2_PWR_CON,  mmio_read_32(SPMC_MP0_CPU2_PWR_CON) & ~SPMC_PWR_RST_B);/*Polarity don't need to inverse*/
	mmio_write_32(SPMC_MP0_CPU3_PWR_CON,  mmio_read_32(SPMC_MP0_CPU3_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPU3_PWR_CON,  mmio_read_32(SPMC_MP0_CPU3_PWR_CON) & ~SPMC_PWR_RST_B);/*Polarity don't need to inverse*/

	mmio_write_32(SPMC_MP0_CPUTOP_PWR_CON,  mmio_read_32(SPMC_MP0_CPUTOP_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPUTOP_PWR_CON,  mmio_read_32(SPMC_MP0_CPUTOP_PWR_CON) & ~SPMC_PWR_RST_B);
	mmio_write_32(SPMC_MP0_CPUTOP_CLK_DIS,  mmio_read_32(SPMC_MP0_CPUTOP_CLK_DIS) & ~SPMC_PWR_CLK_DIS);

#if 0
	mmio_write_32(SPMC_MP0_CPU1_PWR_CON,  mmio_read_32(SPMC_MP0_CPU1_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPU1_PWR_CON,  mmio_read_32(SPMC_MP0_CPU1_PWR_CON) & ~SPMC_PWR_RST_B);/*Polarity don't need to inverse*/
	mmio_write_32(SPMC_MP0_CPU2_PWR_CON,  mmio_read_32(SPMC_MP0_CPU2_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPU2_PWR_CON,  mmio_read_32(SPMC_MP0_CPU2_PWR_CON) & ~SPMC_PWR_RST_B);/*Polarity don't need to inverse*/
	mmio_write_32(SPMC_MP0_CPU3_PWR_CON,  mmio_read_32(SPMC_MP0_CPU3_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP0_CPU3_PWR_CON,  mmio_read_32(SPMC_MP0_CPU3_PWR_CON) & ~SPMC_PWR_RST_B);/*Polarity don't need to inverse*/
#endif

/* MP2 SPMC power Ctrl signals */
	mmio_write_32(SPMC_MP2_CPU0_PWR_CON, mmio_read_32(SPMC_MP2_CPU0_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP2_CPU0_PWR_CON, mmio_read_32(SPMC_MP2_CPU0_PWR_CON) & ~SPMC_PWR_RST_B);
	mmio_write_32(SPMC_MP2_CPU1_PWR_CON, mmio_read_32(SPMC_MP2_CPU1_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP2_CPU1_PWR_CON, mmio_read_32(SPMC_MP2_CPU1_PWR_CON) & ~SPMC_PWR_RST_B);

	mmio_write_32(SPMC_MP2_CPUTOP_PWR_CON,  mmio_read_32(SPMC_MP2_CPUTOP_PWR_CON) & ~SPMC_PWR_ON_2ND);
	mmio_write_32(SPMC_MP2_CPUTOP_PWR_CON,  mmio_read_32(SPMC_MP2_CPUTOP_PWR_CON) & ~SPMC_PWR_RST_B);
	mmio_write_32(SPMC_MP2_CPUTOP_CLK_DIS,  mmio_read_32(SPMC_MP2_CPUTOP_CLK_DIS) & ~SPMC_PWR_CLK_DIS);


/* setup_sw_cluster_default_state(0x10,STA_POWER_ON); */
/* setup_sw_core_default_state(0x0,STA_POWER_ON); */

/* MP0 not support
#if SPMC_SPARK2
		little_spark2_setldo(0);
		little_spark2_core_enable(0,1);
#endif
*/


	return err;
}


int spmc_cputop_mpx_onoff(int cputop_mpx, int state, int mode)
{
	int err = 0;
#if SPMC_DVT
	unsigned int MPx_AXI_CONFIG, MPx_SNOOP_CTRL;
#endif
	unsigned int MPx_CPUTOP_SPMC, MPx_CPUTOP_PWR_CON;
	unsigned int MPx_CPUTOP_PWR_STA_MASK, MPx_CPUx_STANDBYWFI;

	/* TINFO="enable SPM register control" */
	mmio_write_32(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	PRINTF_SPMC("######## %s cputop_mpx=%d state=%d mode=%d\n", __func__, cputop_mpx, state,
		    mode);

	if (cputop_mpx == CPUTOP_MP0) {
#if SPMC_DVT
		MPx_AXI_CONFIG = MP0_AXI_CONFIG;
		MPx_SNOOP_CTRL = MP0_SNOOP_CTRL;
#endif
		MPx_CPUTOP_SPMC = MP0_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = SPMC_MP0_CPUTOP_PWR_CON;
		/*MPx_CPUTOP_PWR_STA_MASK = MP0_CPUTOP_PWR_STA_MASK;*/
		MPx_CPUx_STANDBYWFI = MP0_CPUTOP_STANDBYWFI;
	} else if (cputop_mpx == CPUTOP_MP1) {
			ERROR("%s() MP%d not exists\n", __func__, cputop_mpx);
			assert(0);
	} else {
#if SPMC_DVT
		MPx_AXI_CONFIG = MP2_AXI_CONFIG;
		MPx_SNOOP_CTRL = MP2_SNOOP_CTRL;
#endif
		MPx_CPUTOP_SPMC = MP2_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = SPMC_MP2_CPUTOP_PWR_CON;
		/*MPx_CPUTOP_PWR_STA_MASK = MP2_CPUTOP_PWR_STA_MASK;*/
		MPx_CPUx_STANDBYWFI = MP2_CPUTOP_STANDBYWFI;
	}

	/*PRINTF_SPMC("MPx_CPUTOP_PWR_STA_MASK=%x\n", MPx_CPUTOP_PWR_STA_MASK);*/
	/*PRINTF_SPMC("MPx_AXI_CONFIG=%x\n", MPx_AXI_CONFIG);*/
	/*PRINTF_SPMC("mpx_mask=%x\n", mpx_mask);*/
	/*PRINTF_SPMC("MPx_SNOOP_CTRL=%x\n", MPx_SNOOP_CTRL);*/

MPx_CPUTOP_PWR_STA_MASK = (1U << 16); // TODO:

	if (state == STA_POWER_DOWN) {

#if SPMC_DVT			/* by callee in plat_affinst_off() */
		/* TINFO="Start to turn off MP0_CPUTOP" */
/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */
		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) & ~MPx_SNOOP_ENABLE);
		PRINTF_SPMC("MPx_SNOOP_CTRL_0x%x=0x%x\n", MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL));
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS, mmio_read_32(MPx_SNOOP_STATUS));
		/* mcsib_sw_workaround_main(); */
		/*      Program MP<n>_AXI_CONFIG acinactm to 1 */
		/*      Wait mp<n>_STANDBYWFIL2 to high */
		/* TINFO="Set acinactm = 1" */
		if (cputop_mpx == CPUTOP_MP2)
			mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) | MP2_AXI_CONFIG_acinactm);
		else
			mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) | MPx_AXI_CONFIG_acinactm);
		PRINTF_SPMC("MPx_AXI_CONFIG_0x%x=0x%x\n", MPx_AXI_CONFIG, mmio_read_32(MPx_AXI_CONFIG));
#endif		
		if (cputop_mpx == CPUTOP_MP2)
			mmio_write_32(MP2_AXI_CONFIG,
				      mmio_read_32(MP2_AXI_CONFIG) | MP2_AXI_CONFIG_acinactm);

		if (mode == MODE_AUTO_SHUT_OFF) {
			/* Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw_spark_en (mcucfg_reg) 1 */
			/* mmio_write_32(MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC) | sw_spark_en);*/
			/* Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw sw_no_wait_for_q_channel (mcucfg_reg) 0 */
			mmio_write_32(MPx_CPUTOP_SPMC,
				      mmio_read_32(MPx_CPUTOP_SPMC) & ~sw_no_wait_for_q_channel);
			/* Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw_coq_dis (mcucfg_reg) 0 */
			if (cputop_mpx == CPUTOP_MP2)
				mmio_write_32(PTP3_B_COQ, mmio_read_32(PTP3_B_COQ) & ~B_SW_COQ_DIS);
			else
				mmio_write_32(MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC) & ~sw_coq_dis);
		} else {	/* MODE_SPMC_HW or MODE_DORMANT */
			/* Set  cluster<n>.SPMC.MP<n>.sw_no_wait_for_q_channel (mcucfg_reg) 1 */
			mmio_write_32(MPx_CPUTOP_SPMC,
				      mmio_read_32(MPx_CPUTOP_SPMC) | sw_no_wait_for_q_channel);
			if (cputop_mpx == CPUTOP_MP2)
				mmio_write_32(PTP3_B_COQ, mmio_read_32(PTP3_B_COQ) | B_SW_COQ_DIS);
			else
				mmio_write_32(MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC) | sw_coq_dis);

			/* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
			while (!(mmio_read_32(SPM_SLEEP_TIMER_STA) & MPx_CPUx_STANDBYWFI));
			PRINTF_SPMC("SPM_SLEEP_TIMER_STA_0x%x=0x%x\n", SPM_SLEEP_TIMER_STA, mmio_read_32(SPM_SLEEP_TIMER_STA));

			/*Set  pwrdnreqn_mp<n>_adb to low (all together)
			 *Set  pwrdnreqn_mp<n>_l2c_afifo to low (all together)*/
			if (cputop_mpx == CPUTOP_MP0) {
				/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
				/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1,  mmio_read_32(INFRA_TOPAXI_PROTECTEN_1)|mpx_mask); */
				mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN_SET, IDX_PROTECT_MP0_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
				mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_SET, IDX_PROTECT_L2C0_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));

				/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
				while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN) & IDX_PROTECT_MP0_CACTIVE) != IDX_PROTECT_MP0_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
				while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1) & IDX_PROTECT_L2C0_CACTIVE) != IDX_PROTECT_L2C0_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));
			} else { /*CPUTOP_MP2*/
				mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN_SET, IDX_PROTECT_MP1_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
				mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_SET, IDX_PROTECT_L2C1_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));

				/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
				while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN) & IDX_PROTECT_MP1_CACTIVE) != IDX_PROTECT_MP1_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
				while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1) & IDX_PROTECT_L2C1_CACTIVE) != IDX_PROTECT_L2C1_CACTIVE);
				PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));
				/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */
			}
		}
		PRINTF_SPMC("MPx_CPUTOP_SPMC_0x%x=0x%x\n", MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC));
/* Set mp<n>_spmc_resetpwron_config_cputop to 0 */
/*		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) & ~PWR_ON_2ND);*/
#if 0
		if (mode == MODE_DORMANT) {	/*  Set mp<n>_spmc_sram_dormant_en 0 */
			mmio_write_32(SPMC_DORMANT_ENABLE,
				      mmio_read_32(SPMC_DORMANT_ENABLE) | (MP0_SPMC_SRAM_DORMANT_EN
									   << cputop_mpx));
		} else {	/* MODE_SPMC_HW or MODE_AUTO_SHUT_OFF *//*     Set mp<n>_spmc_sram_dormant_en 0 */
			mmio_write_32(SPMC_DORMANT_ENABLE,
				      mmio_read_32(SPMC_DORMANT_ENABLE) & ~(MP0_SPMC_SRAM_DORMANT_EN
									    << cputop_mpx));
		}
		PRINTF_SPMC("MPx_CPUTOP_SPMC_0x%x=0x%x\n", MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC));
#endif		
/*	Set  mp<n>_spmc_pwr_on_cputop 0 */
		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) & ~PWR_ON);
		if (mode == MODE_SPMC_HW) {	/* TINFO="Wait until PWR_STATUS = 0" */
			while (mmio_read_32(MPx_CPUTOP_PWR_CON) & MPx_CPUTOP_PWR_STA_MASK);
			PRINTF_SPMC("MPx_CPUTOP_PWR_CON_0x%x=0x%x\n", MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON));
		}
#if AO_REG_PATCH
		if (cputop_mpx == CPUTOP_MP2) {
			PRINTF_SPMC("Before : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) | ((0x1)<<1));
			PRINTF_SPMC("After : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
		}
#endif
		PRINTF_SPMC("MPx_CPUTOP_SPMC_0x%x=0x%x\n", MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC));

/* TODO: DUAL_VCORE_VCA15M_PWR_ISO(CPU_EXT_BUCK_ISO)  0x10006404[12..13]*/
		if (cputop_mpx == CPUTOP_MP2)
			mmio_write_32(CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO) | (0x1 << 13));
		else
			mmio_write_32(CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO) | (0x1 << 12));
			PRINTF_SPMC("CPU_EXT_BUCK_ISO_0x%x=0x%x\n", CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO));

		/* TINFO="Finish to turn off MP0_CPUTOP" */
	} else {
		/* TINFO="Start to turn on MP0_CPUTOP" */

#if AO_REG_PATCH
		if (cputop_mpx == CPUTOP_MP2) {
			PRINTF_SPMC("[Before]W/O BUCK_EXT_ISO : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) & (~(0x1)));
			PRINTF_SPMC("[After]W/O BUCK_EXT_ISO : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
		}
#endif
/* TODO: DUAL_VCORE_VCA15M_PWR_ISO(CPU_EXT_BUCK_ISO)  0x10006404[12..13]*/			
		if (cputop_mpx == CPUTOP_MP2)
			mmio_write_32(CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO) & ~(0x1 << 13));
		else
			mmio_write_32(CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO) & ~(0x1 << 12));
			PRINTF_SPMC("CPU_EXT_BUCK_ISO_0x%x=0x%x\n", CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO));
#if AO_REG_PATCH
		if (cputop_mpx == CPUTOP_MP2) {
			PRINTF_SPMC("Before : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) | (0x1));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) & (~(0x1 << 1)));
			PRINTF_SPMC("After : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
		}
#endif
		/* TINFO="Set PWR_ON = 1" */
		PRINTF_SPMC("MPx_CPUTOP_PWR_CON_0x%x=0x%x\n", MPx_CPUTOP_PWR_CON,mmio_read_32(MPx_CPUTOP_PWR_CON));
		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) | PWR_ON);
		PRINTF_SPMC("MPx_CPUTOP_PWR_CON_0x%x=0x%x\n", MPx_CPUTOP_PWR_CON,mmio_read_32(MPx_CPUTOP_PWR_CON));
		/* TINFO="Wait until PWR_STATUS = 1" */
		while (!(mmio_read_32(MPx_CPUTOP_PWR_CON) & MPx_CPUTOP_PWR_STA_MASK));
		PRINTF_SPMC("MPx_CPUTOP_PWR_CON_0x%x=0x%x\n", MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON));
	/*	PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
		PRINTF_SPMC("0x10200590<=0x58\n");
mmio_write_32(0x10200590, 0x58);
PRINTF_SPMC("0x10200594=>0x%x\n", mmio_read_32(0x10200594));
		PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));*/

/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */
		/* TINFO="Release bus protect" */
		/* TINFO="Release ADB pwrdnreqn for Cluster 0" */
	
		/*Set  pwrdnreqn_mp<n>_adb to high (all together)
		 *Set  pwrdnreqn_mp<n>_l2c_afifo to high (all together)*/
		if (cputop_mpx == CPUTOP_MP0) {
			/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
			/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1,  mmio_read_32(INFRA_TOPAXI_PROTECTEN_1)|mpx_mask); */
			mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN_CLR, IDX_PROTECT_MP0_CACTIVE);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
			mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_CLR, IDX_PROTECT_L2C0_CACTIVE);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));

			/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
			while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN) & IDX_PROTECT_MP0_CACTIVE) != 0);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
			while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1) & IDX_PROTECT_L2C0_CACTIVE) != 0);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));
		} else { /*CPUTOP_MP2*/
			mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN_CLR, IDX_PROTECT_MP1_CACTIVE);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
			mmio_write_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_CLR, IDX_PROTECT_L2C1_CACTIVE);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));

			/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
			while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN) & IDX_PROTECT_MP1_CACTIVE) != 0);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN));
			while ((mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1) & IDX_PROTECT_L2C1_CACTIVE) != 0);
			PRINTF_SPMC("MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_0x%x=0x%x\n", MCUSYS_PROTECTEN_TOPAXI_PROT_EN1, mmio_read_32(MCUSYS_PROTECTEN_TOPAXI_PROT_EN1));
		}


#if SPMC_DVT			/* by callee in plat_affinst_off() */
/*	Program MP<n>_AXI_CONFIG acinactm to 0*/
		if (cputop_mpx == CPUTOP_MP2)
			mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) & ~MP2_AXI_CONFIG_acinactm);
		else
			mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) & ~MPx_AXI_CONFIG_acinactm);
		PRINTF_SPMC("MPx_AXI_CONFIG_0x%x=0x%x\n", MPx_AXI_CONFIG,
			    mmio_read_32(MPx_AXI_CONFIG));
/*	Program MCSI-A Slave Control Register (MP0:slave0,  MP1:slave1) */
/*to enable snoop/DVM function for related slave port through ACE(0x10390000) or APB(0x10200000) interface*/
/*	Polling CCI Status register through ACE interface (0x1039000C[0]) returns to 0*/
/*		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);*/
/*	  PRINTF_SPMC("MPx_SNOOP_CTRL_0x%x=0x%x\n", MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL));*/
		/*mmio_write_32(0x10394000, mmio_read_32(0x10394000) | MPx_SNOOP_ENABLE);
		   mmio_write_32(0x10395000, mmio_read_32(0x10395000) | MPx_SNOOP_ENABLE); */
		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);
		PRINTF_SPMC("MPx_SNOOP_CTRL%x=0x%x\n", MPx_SNOOP_CTRL,
			    mmio_read_32(MPx_SNOOP_CTRL));
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		/* mcsib_sw_workaround_main(); */
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS,
			    mmio_read_32(MPx_SNOOP_STATUS));
#endif
/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */

		/* TINFO="Finish to turn on MP0_CPUTOP" */

	}
	return err;
}

int spmc_cpu_corex_onoff(int linear_id, int state, int mode)
{
	int err = 0;
	unsigned int CPUSYSx_CPUx_SPMC_CTL, MPx_CPUx_PWR_CON, MPx_CPUx_STANDBYWFI, MPx_CPUx_PWR_STA_MASK;
	/* TINFO="enable SPM register control" */
	mmio_write_32(SPM_POWERON_CONFIG_SET,  (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	PRINTF_SPMC(">>>>>>>> %s >>>>>>>>linear_id=%d state=%d mode=%d\n", __func__, linear_id, state, mode);
	MPx_CPUx_PWR_STA_MASK = (1U << 16);
	switch (linear_id) {
	case 0:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU0_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU0_STANDBYWFI;
			MPx_CPUx_PWR_CON	  = SPMC_MP0_CPU0_PWR_CON;
			break;
	case 1:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU1_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU1_STANDBYWFI;
			MPx_CPUx_PWR_CON	  = SPMC_MP0_CPU1_PWR_CON;
			break;
	case 2:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU2_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU2_STANDBYWFI;
			MPx_CPUx_PWR_CON	  = SPMC_MP0_CPU2_PWR_CON;
			break;
	case 3:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU3_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP0_CPU3_STANDBYWFI;
			MPx_CPUx_PWR_CON	  = SPMC_MP0_CPU3_PWR_CON;
			break;
	case 8:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU0_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP2_CPU0_STANDBYWFI;
			MPx_CPUx_PWR_CON	  = SPMC_MP2_CPU0_PWR_CON;
			break;
	case 9:
			CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU1_SPMC_CTL;
			MPx_CPUx_STANDBYWFI   = MP2_CPU1_STANDBYWFI;
			MPx_CPUx_PWR_CON	  = SPMC_MP2_CPU1_PWR_CON;
			break;

	default:
			ERROR("%s() CPU%d not exists\n",  __func__,  (int)linear_id);
			assert(0);
	}

	PRINTF_SPMC("SPM_SLEEP_TIMER_STA_0x%x=0x%x\n", SPM_SLEEP_TIMER_STA, mmio_read_32(SPM_SLEEP_TIMER_STA));

	if (state  ==  STA_POWER_DOWN) {
		if (!(cpu_bitmask & (1 << linear_id))) {
			PRINTF_SPMC("core%d already turn off !!!\n", linear_id);
			return 0;
			}
#if 0
		if(linear_id>=8)
			mmio_write_32(DBUGPWRDUP, mmio_read_32(DBUGPWRDUP) & ~(0x1<<(linear_id-8)));
#endif
		if (mode == MODE_AUTO_SHUT_OFF) {
			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL, mmio_read_32(CPUSYSx_CPUx_SPMC_CTL) & ~cpu_sw_no_wait_for_q_channel);
			PRINTF_SPMC("auto_off_CPUSYSx_CPUx_SPMC_CTL_0x%x=0x%x\n", CPUSYSx_CPUx_SPMC_CTL, mmio_read_32(CPUSYSx_CPUx_SPMC_CTL));
			set_cpu_retention_control(1);
		} else {
			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL, mmio_read_32(CPUSYSx_CPUx_SPMC_CTL) | cpu_sw_no_wait_for_q_channel);
			PRINTF_SPMC("HW_off_CPUSYSx_CPUx_SPMC_CTL_0x%x=0x%x\n", CPUSYSx_CPUx_SPMC_CTL, mmio_read_32(CPUSYSx_CPUx_SPMC_CTL));
			while (!(mmio_read_32(SPM_SLEEP_TIMER_STA) & MPx_CPUx_STANDBYWFI))
				;	
			PRINTF_SPMC("SPM_SLEEP_TIMER_STA_0x%x=0x%x\n", SPM_SLEEP_TIMER_STA, mmio_read_32(SPM_SLEEP_TIMER_STA));
		}
		mmio_write_32(MPx_CPUx_PWR_CON,  mmio_read_32(MPx_CPUx_PWR_CON) & ~SPMC_PWR_ON);
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON));
		if (mode == MODE_SPMC_HW) {
			while (mmio_read_32(MPx_CPUx_PWR_CON) & MPx_CPUx_PWR_STA_MASK)
				;
			PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON));
			}
		cpu_bitmask &= ~(1 << linear_id);
		PRINTF_SPMC("cpu_bitmask=0x%x\n", cpu_bitmask);
	} else {
		if(linear_id>=8){
			/*Ensure DBUGPWRDUP is held LOW*/
			mmio_write_32(DBUGPWRDUP, mmio_read_32(DBUGPWRDUP) & ~(0x1<<(linear_id-8)));
			while ( mmio_read_32(DBUGPWRDUP) & (0x1<<(linear_id-8)) );
			}
		mmio_write_32(MPx_CPUx_PWR_CON,  mmio_read_32(MPx_CPUx_PWR_CON) | SPMC_PWR_ON);
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON));
		while ((mmio_read_32(MPx_CPUx_PWR_CON) & MPx_CPUx_PWR_STA_MASK) != MPx_CPUx_PWR_STA_MASK)
			;
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON));
		if(linear_id>=8)
			mmio_write_32(DBUGPWRDUP, mmio_read_32(DBUGPWRDUP) | (0x1<<(linear_id-8)));
		cpu_bitmask |= (1 << linear_id);
		PRINTF_SPMC("cpu_bitmask=0x%x\n", cpu_bitmask);
	}
	return err;
}


