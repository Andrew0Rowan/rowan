#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <plat_dramc.h>
#include <interrupt_mgmt.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <delay_timer.h>


uint32_t sip_dram_smc_chb(unsigned int OnOff)
{
	if (OnOff) {
		/*TINFO="===In DCS DRAM_CHB Turn on flow start==="*/
    /* TINFO="In DCS Start to turn on DPYB" */

    /* TINFO="In DCS enable SPM register control" */
    mmio_write_32(SPM_BASE, (0xB16  << 16) | (0x1 << 0));
    //*((volatile unsigned int *)((0xB16  << 16) | (0x1 << 0))) = (0xB16  << 16) | (0x1 << 0);
    //! *((UINT32P)(0x508c0000))      = (0xB16  << 16) | (0x1 << 0);
#if 0 //because keep the power
			    ///TODO: Step 1 {DDRPHY MTCMOS on}
			    /* TINFO="In DCS Set DPYB PWR_ON = 1" */
			    mmio_write_32(PWR_DPYB_PWR_CON, mmio_read_32(PWR_DPYB_PWR_CON) | (1 <<  2) );
			    //*(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       |=  (1 <<  2);                       // sc_dpy_ch1_pwr_on       = 1
			    //! *((UINT32P)(0x508c0324)) |=  (1 <<  2);                       // sc_dpy_ch1_pwr_on       = 1

			    /* TINFO="In DCS Set DPYB PWR_ON_2ND = 1" */
			    mmio_write_32(PWR_DPYB_PWR_CON, mmio_read_32(PWR_DPYB_PWR_CON) | (1 <<  3) );
			    //*(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       |=  (1 <<  3);                       // sc_dpy_ch1_pwr_on_2nd       = 1
			    //! *((UINT32P)(0x508c0324)) |=  (1 <<  3);                       // sc_dpy_ch1_pwr_on_2nd       = 1

			    ///TODO: Step 2 {wait 1us}
			    udelay(1);
			    //mcDELAY_US(1);
			    /* TINFO="In DCS Set DPYB PWR_CLK_DIS = 0" */
			    mmio_write_32(PWR_DPYB_PWR_CON, mmio_read_32(PWR_DPYB_PWR_CON) & ~(0x1 << 4) );
			    //*(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       &=  ~(0x1 << 4);                       // sc_dpy_ch1_pwr_clk_dis = 0
			    //! *((UINT32P)(0x508c0324)) &=  ~(0x1 << 4);                       // sc_dpy_ch1_pwr_clk_dis = 0
			    ///TODO: Step 3 {DDRPHY ISO_EN de-assert}
			    /* TINFO="In DCS Set DPYB PWR_ISO = 0" */
			    mmio_write_32(PWR_DPYB_PWR_CON, mmio_read_32(PWR_DPYB_PWR_CON) & ~(0x1 << 1) );
			    //*(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       &=  ~(0x1 << 1);                       // sc_dpy_ch1_pwr_iso     = 0
			    ///TODO: Step 5 {wait 30ns}
			    udelay(1);
			    //mcDELAY_US(1);
			    //! *((UINT32P)(0x508c0324)) &=  ~(0x1 << 1);                       // sc_dpy_ch1_pwr_iso     = 0
			    ///TODO: Step 6 {DDRPHY power-on reset de-assert}
			    /* TINFO="In DCS Set DPYB PWR_RST_B = 1" */
			    *(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       |=  (1 <<  0);                         // sc_dpy_ch1_rst_b        = 1
			    //! *((UINT32P)(0x508c0324)) |=  (1 <<  0);                         // sc_dpy_ch1_rst_b        = 1
#endif //because keep the power
		///TODO: Step 7 {set DQS gating retry enable}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 1));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= (1 << 1);                         // sc_dr_gate_retry_ch1_en = 1
		///TODO: Step 8-1 {ddrphy_shuffle BCLK_FR CG OFF}
		mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) | (1 << 1));
		//*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         |= (1 << 1);                         //! set SPM_SW_RSV_4[1]=1 to enable CHB bclk en
		///TODO: Step 8-2 {wait 2T 26MHz}
		udelay(1);
		//mcDELAY_US(1);
		///TODO: Step 8-3 {restore shuffle state machine}
		mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) | (1 << 3));
		//*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         |= (1 << 3);                         //! set SPM_SW_RSV_4[3]=1 to enable CHB restore en
		///TODO: Step 8-4 {wait 2T 26MHz}
		udelay(1);
		//mcDELAY_US(1);
		///TODO: Step 8-5 {restore shuffle state machine}
		mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) & ~(1 << 3));
		//*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         &= ~(1 << 3);                        //! set SPM_SW_RSV_4[3]=0 to disable CHB restore en
		///TODO: Step 8-6 {wait 2T 26MHz}
		udelay(1);
		//mcDELAY_US(1);
		//*MDM_TM_WAIT_US = 1;                                                // wait 150ns
		//while (*MDM_TM_WAIT_US>0);
		udelay(1);
		//mcDELAY_US(1);
		///TODO: Step 8-7 {ddrphy_shuffle BCLK_FR CG ON}
		mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) & ~(1 << 1));
		//*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         &= ~(1 << 1);                        //! set SPM_SW_RSV_4[1]=0 to disable CHB bclk en
		//! CHB from S1 to Sidle
		///TODO: Step 9 {VREF enable, MCK8X CG disable, MIDPI enable}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 17));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 17;                          // sc_dpy_vref_ch1_en      = 1
		//! *((UINT32P)(0x508c0464))   |= 1 << 17;                          // sc_dpy_vref_ch1_en      = 1
		///TODO: Step 10 {All PI enable & CG disable}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 15));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 15;                          // sc_dpy_dll_ck_ch1_en    = 1
		//! *((UINT32P)(0x508c0464))   |= 1 << 15;                          // sc_dpy_dll_ck_ch1_en    = 1
		///TODO: Step 11 {set TOP feedback MCK to full frequency}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 21));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 21;                          // sc_ddrphy_fb_ck_ch1_en  = 1
		//! *((UINT32P)(0x508c0464))   |= 1 << 21;                          // sc_ddrphy_fb_ck_ch1_en  = 1
		///TODO: Step 12 {1st DLL enable}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 13));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 13;                          // sc_dpy_dll_ch1_en       = 1
		//! *((UINT32P)(0x508c0464))   |= 1 << 13;                          // sc_dpy_dll_ch1_en       = 1
		///TODO: Step 13{wait 150ns}
		//*MDM_TM_WAIT_US = 1;                                                // wait 150ns
		//while (*MDM_TM_WAIT_US>0);
		udelay(1);
		//mcDELAY_US(1);
		///TODO: Step 14{2nd DLL enable}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 11));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 11;                          // sc_dpy_2nd_dll_ch1_en   = 1
		//! *((UINT32P)(0x508c0464))   |= 1 << 11;                          // sc_dpy_2nd_dll_ch1_en   = 1
		///TODO: Step 15{wait 150ns}
		//*MDM_TM_WAIT_US = 1;                                                // wait 150ns
		//while (*MDM_TM_WAIT_US>0);
		udelay(1);
		//mcDELAY_US(1);
		///TODO: Step 16{DDRPHY switch to DRAMC control}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & ~(1 << 5));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 5);                        // sc_dpy_ch1_mode_sw      = 0
		//! *((UINT32P)(0x508c0464))   &= ~(1 << 5);                        // sc_dpy_ch1_mode_sw      = 0
		///TODO: Step 17{mem_ck CG disable}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & ~(1 << 9));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 9);                        // sc_mem_ck_off[1]        = 0
		//! *((UINT32P)(0x508c0464))   &= ~(1 << 9);                        // sc_mem_ck_off[1]        = 0
		///TODO: Step 18{set TOP feedback MCK to divided frequency}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & ~(1 << 21));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 21);                       // sc_ddrphy_fb_ck_ch1_en  = 0
		//! *((UINT32P)(0x508c0464))   &= ~(1 << 21);                       // sc_ddrphy_fb_ck_ch1_en  = 0
		///TODO: Step 19{wait 150ns}
		//*MDM_TM_WAIT_US = 1;                                                // wait 150ns
		//while (*MDM_TM_WAIT_US>0);
		udelay(1);
		//mcDELAY_US(1);
		///TODO: Step 20{IO exit suspend}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & ~(1 << 7));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 7);                        // sc_dmsus_ch1_off        = 0
		//! *((UINT32P)(0x508c0464))   &= ~(1 << 7);                        // sc_dmsus_ch1_off        = 0
		///TODO: Step 21{release self-refresh request}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & ~(1 << 3));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 3);                        // emi_clk_off_req[1]      = 0
		//! *((UINT32P)(0x508c0464))   &= ~(1 << 3);                        // emi_clk_off_req[1]      = 0
		///TODO: Step 25{wait exit self-refresh ack}
		while ( ((mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) >> 27) & 0x1) != 0x0) {}   // wait emi_clk_off_ack[1] = 0
		//while ( ((*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON >> 27) & 0x1) != 0x0) {}   // wait emi_clk_off_ack[1] = 0
		//!   while ( ((*((UINT32P)(0x508c0464)) >> 27) & 0x1) != 0x0) {}   // wait emi_clk_off_ack[1] = 0
		///TODO: Step 26{set DQS gating retry disable}
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & ~(1 << 1));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 1);                        // sc_dr_gate_retry_ch1_en = 0
		//! *((UINT32P)(0x508c0464))   &= ~(1 << 1);                        // sc_dr_gate_retry_ch1_en = 0
		///TODO: Step 27{set DQSCK pre-calibration enable }
		mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) | (1 << 5));
		//*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         |= (1 << 5);                         //! set SPM_SW_RSV_4[5]=1 to enable  CHB DQSCK precal sync en
		///TODO: Step 28 {wait 2T 26MHz}
		udelay(1);
		//mcDELAY_US(1);
		///TODO: Step 29{restore shuffle state machine}
		mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) & ~(1 << 5));
		//*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         &= ~(1 << 5);                        //! set SPM_SW_RSV_4[5]=0 to disable CHB DQSCK precal sync en
		///TODO: CPU on Step 0{enable DQSOSC calibration}
		//! CHB enable DQSOSCENDIS
                //!20160818 mmio_write_32(DRAMC1_AO_BASE + 0x00c8, mmio_read_32(DRAMC1_AO_BASE + 0x00c8) & ~(1 << 28));
		//*((volatile unsigned int *)(PWR_MD32_DRAMC1_AO_BASE + 0x00c8  )) &= ~(1 << 28);      // enable DQSOSCEN
		//! set SPM SW RSV 4 [9] = 1 to control DRAMC CHB back up restore function
		mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) | (1<<9));
		//*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         |= (1<<9);
		//!  change CHB selection from HW mode
		mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON_SEL, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON_SEL) | ((1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23)));
		//*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON_SEL |= ((1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23)) ;
		//! *((UINT32P)(0x508c0460))       |= ((1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23)) ;
		//! switch SPM to sw mode :  sc_dr_gate_retry_ch1_en , emi_clk_off_req[1],  sc_dpy_ch1_mode_sw, sc_dmsus_ch1_off, sc_mem_ck_off[1],
		//! sc_dpy_2nd_dll_ch1_en, sc_dpy_dll_ch1_en, sc_dpy_dll_ck_ch1_en, sc_dpy_vref_ch1_en, sc_phypll_ch1_en, sc_ddrphy_fb_ck_ch1_en, seperate_phy_pwr_sel
		/*TINFO="===In DCS DRAM_CHB Turn on flow End==="*/
		//! //! turn on CHA and CHB setting issue
		//! *PWR_MD32_INFRA_DRAMC_REG_CONFIG   |= 0x00000001;
		/* TINFO="In DCS Finish to turn on DPYB" */
	} else {
		/*TINFO="===In DCS DRAM_CHB Turn off flow start==="*/
    //! //! turn off CHA and CHB setting issue
    //! *PWR_MD32_INFRA_DRAMC_REG_CONFIG   &= ~(0x00000001);

    /*TINFO="===In DCS Setting DDRPHY register for checking SPM sw mode path Start ==="*/
    mmio_write_32(DDRPHY0AO_BASE + (0x009d << 2), 0xffffffff);
    //*((volatile unsigned int *)(PWR_MD32_DDRPHY0AO_BASE + (0x009d << 2))) = 0xffffffff; // Set DDRPHY0 conf turn on CG/PLL/CLK path control by SPM control
    mmio_write_32(DDRPHY1AO_BASE + (0x009d << 2), 0xffffffff);
    //*((volatile unsigned int *)(PWR_MD32_DDRPHY1AO_BASE + (0x009d << 2))) = 0xffffffff; // Set DDRPHY1 conf turn on CG/PLL/CLK path control by SPM control
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19);
    //! *((UINT32P)(0x508c0464))   |= (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19);
    // sc_dpy_2nd_dll_ch1_en, sc_dpy_2nd_ch1_en,sc_dpy_2nd_ck_ch1_en,sc_dpy_vref_ch1_en, sc_phypll_ch1_en = 1

    //! set SPM SW RSV 4 [9] = 0 to control DRAMC CHB back up restore function
    mmio_write_32(PWR_SPM_SW_RSV_4, mmio_read_32(PWR_SPM_SW_RSV_4) & (~(1<<9)));
    //*(volatile unsigned int *)PWR_MD32_SPM_SW_RSV_4         &= ~(1<<9);
    //!  change CHB selection from SW mode
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON_SEL, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON_SEL) & (~((1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23))));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON_SEL &= ~((1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23)) ;
    //! *((UINT32P)(0x508c0460))       &= ~((1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11) | (1 << 13) | (1 << 15) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23)) ;
    //! switch SPM to sw mode :  sc_dr_gate_retry_ch1_en , emi_clk_off_req[1],  sc_dpy_ch1_mode_sw, sc_dmsus_ch1_off, sc_mem_ck_off[1],
    //! sc_dpy_2nd_dll_ch1_en, sc_dpy_dll_ch1_en, sc_dpy_dll_ck_ch1_en, sc_dpy_vref_ch1_en, sc_phypll_ch1_en, sc_ddrphy_fb_ck_ch1_en, seperate_phy_pwr_sel

    ///TODO: [CPU off]Step 0 -1 {disable DQSOSC calibration}
    //! CHB disable DQSOSCENDIS
    mmio_write_32(DRAMC1_AO_BASE + 0x00c8, mmio_read_32(DRAMC1_AO_BASE + 0x00c8) | (1 << 28));
    //*((volatile unsigned int *)(PWR_MD32_DRAMC1_AO_BASE + 0x00c8  )) |= (1 << 28);      // Disable DQSOSCEN

    ///TODO: [CPU off]Step 0 -2 {wait 12us}
    udelay(12);
    //mcDELAY_US(12);

    //!//! CHB from Sidle to S1
    //!while ( ((*PWR_MD32_DRAMC_DPY_CLK_SW_CON >> 4) & 0x1) != 0x0) {}    // wait  dqssoc_req        = 0
    //!//!   while ( ((*((UINT32P)(0x508c0464)) >> 4) & 0x1) != 0x0) {}    // wait  dqssoc_req        = 0
    ///TODO: Step 2 {set DQS gating retry enable}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 1));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 1;                           // sc_dr_gate_retry_ch1_en = 1
    //!*((UINT32P)(0x508c0464))    |= 1 << 1;                           // sc_dr_gate_retry_ch1_en = 1
    ///TODO: Step 3 {request self-refresh}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 3));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 3;                           // emi_clk_off_req[1]      = 1
    //! *((UINT32P)(0x508c0464))   |= 1 << 3;                           // emi_clk_off_req[1]      = 1
    ///TODO: Step 5 {wait self-refresh ack}
//sagy for no DRAM #if 0
		while ( ((mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) >> 27) & 0x1) != 0x1) {}   // wait emi_clk_off_ack[1] = 1
    //while ( ((*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON >> 27) & 0x1) != 0x1) {}   // wait emi_clk_off_ack[1] = 1
//sagy for no DRAM #endif
    //!   while ( ((*((UINT32P)(0x508c0464)) >> 27) & 0x1) != 0x1) {}   // wait emi_clk_off_ack[1] = 1
    ///TODO: Step 6 {DDRPHY switch to SPM control}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 5));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 5;                           // sc_dpy_ch1_mode_sw      = 1
    //! *((UINT32P)(0x508c0464))   |= 1 << 5;                           // sc_dpy_ch1_mode_sw      = 1
    ///TODO: Step 7 {IO enter suspend}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 7));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 7;                           // sc_dmsus_ch1_off        = 1
    //! *((UINT32P)(0x508c0464))   |= 1 << 7;                           // sc_dmsus_ch1_off        = 1
    ///TODO: Step 8 {mem_ck CG enable}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) | (1 << 9));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON |= 1 << 9;                           // sc_mem_ck_off[1]        = 1
    //! *((UINT32P)(0x508c0464))   |= 1 << 9;                           // sc_mem_ck_off[1]        = 1
    ///TODO: Step 9 {DLL disable}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & (~(1 << 11)));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 11);                       // sc_dpy_2nd_dll_ch1_en   = 0
    //! *((UINT32P)(0x508c0464))   &= ~(1 << 11);                       // sc_dpy_2nd_dll_ch1_en   = 0
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & (~(1 << 13)));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 13);                       // sc_dpy_dll_ch1_en       = 0
    //! *((UINT32P)(0x508c0464))   &= ~(1 << 13);                       // sc_dpy_dll_ch1_en       = 0
    ///TODO: Step 10 {All PI disable & CG enable}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & (~(1 << 15)));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 15);                       // sc_dpy_dll_ck_ch1_en    = 0
    //! *((UINT32P)(0x508c0464))   &= ~(1 << 15);                       // sc_dpy_dll_ck_ch1_en    = 0
    ///TODO: Step 11 {VREF disable, MCK8X CG enable, MIDPI disable}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & (~(1 << 17)));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 17);                       // sc_dpy_vref_ch1_en      = 0
    //! *((UINT32P)(0x508c0464))   &= ~(1 << 17);                       // sc_dpy_vref_ch1_en      = 0
    ///TODO: Step 12 {set DQS gating retry disable}
    mmio_write_32(PWR_DRAMC_DPY_CLK_SW_CON, mmio_read_32(PWR_DRAMC_DPY_CLK_SW_CON) & (~(1 << 1)));
    //*(volatile unsigned int *)PWR_MD32_DRAMC_DPY_CLK_SW_CON &= ~(1 << 1);                        // sc_dr_gate_retry_ch1_en = 0
    //!*((UINT32P)(0x508c0464))    &= ~(1 << 1);                        // sc_dr_gate_retry_ch1_en = 0
    /* TINFO="In DCS enable SPM register control" */
    mmio_write_32(SPM_BASE, (0xB16  << 16) | (0x1 << 0));
    //*((volatile unsigned int *)(PWR_MD32_SLEEP_BASE)) = (0xB16  << 16) | (0x1 << 0);
    //! *((UINT32P)(0x508c0000))      = (0xB16  << 16) | (0x1 << 0);
		#if 0 //because keep the power
			    /* TINFO="In DCS Start to turn off DPYB" */
			    ///TODO: Step 13 {DDRPHY ISO_EN assert}
			    /* TINFO="In DCS Set DPHYB PWR_ISO = 1" */
			    *(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       |= (0x1 << 1);                       // sc_dpy_ch1_pwr_iso     = 1
			    //! *((UINT32P)(0x508c0324)) |= (0x1 << 1);                       // sc_dpy_ch1_pwr_iso     = 1

			    ///TODO: Step 14 {wait 100ns}
			    mcDELAY_US(1);
			    /* TINFO="In DCS Set DPHYB PWR_CLK_DIS = 1" */
			    *(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       |= (0x1 << 4);                       // sc_dpy_ch1_pwr_clk_dis = 1
			    //! *((UINT32P)(0x508c0324)) |= (0x1 << 4);                       // sc_dpy_ch1_pwr_clk_dis = 1
			    ///TODO: Step 15 {DDRPHY power-on reset assert}
			    /* TINFO="In DCS Set DPHYB PWR_RST_B = 0" */
			    *(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       &= ~(1 <<  0);                       // sc_dpy_ch1_rst_b        = 0
			    //! *((UINT32P)(0x508c0324)) &= ~(1 <<  0);                       // sc_dpy_ch1_rst_b        = 0
			    ///TODO: Step 16 {DDRPHY MTCMOS off}
			    /* TINFO="In DCS Set DPHYB PWR_ON = 0" */
			    *(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       &= ~(1 <<  2);                       // sc_dpy_ch1_pwr_on       = 0
			    //! *((UINT32P)(0x508c0324)) &= ~(1 <<  2);                       // sc_dpy_ch1_pwr_on       = 0
			    /* TINFO="In DCS Set PWR_ON_2ND = 0" */
			    *(volatile unsigned int *)PWR_MD32_DPYB_PWR_CON       &= ~(1 <<  3);                       // sc_dpy_ch1_pwr_on_2nd       = 0
			    //! *((UINT32P)(0x508c0324)) &= ~(1 <<  3);                       // sc_dpy_ch1_pwr_on_2nd       = 0

			    /* TINFO="In DCS Finish to turn off DPYB" */
		#endif //because keep the power
	}

	return SIP_SVC_E_SUCCESS;

}

