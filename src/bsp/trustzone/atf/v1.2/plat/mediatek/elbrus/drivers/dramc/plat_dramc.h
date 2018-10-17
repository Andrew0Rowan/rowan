#ifndef _DRAMC_H_
#define _DRAMC_H_

//#define SPM_BASE			0x108C0000
#define PWR_DPYB_PWR_CON               ((SPM_BASE+0x0324))
#define PWR_DRAMC_DPY_CLK_SW_CON	    ((SPM_BASE+0x0464))
#define PWR_SPM_SW_RSV_4	            ((SPM_BASE+0x0618))
#define PWR_DRAMC_DPY_CLK_SW_CON_SEL	((SPM_BASE+0x0460))

extern uint32_t sip_dram_smc_chb(unsigned int OnOff);

#endif
