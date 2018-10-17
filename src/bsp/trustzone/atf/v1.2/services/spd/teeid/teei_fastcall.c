/*
 * Copyright (c) 2015
 * microtrust
 * All rights reserved
 * author: steven meng
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>
#include <teei_private.h>

#include <bl_common.h>
#include <fiq_smp_call.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <interrupt_mgmt.h>
#include "plat_private.h"
#include "plat_teei.h"
void tf_printf(const char *fmt, ...);
void mt_log_secure_os_print(int c);
#define MICROTRUST_LOG 0
extern unsigned int get_irq_target(unsigned int irq);
extern void aee_wdt_dump();

/*
 * Notes:
 *   * 32 bit SMC take and return only 32 bit results; remaining bits are undef.
 *     * Never use remaining values for anything.
 *   * PM callbacks are dummy.
 *     * Implement resume and migrate callbacks.
 *   * We update secure system registers at every return. We could optimize this.
 * To be consireded:
 *   * Initialization checks: Check non-null context
 *   * On-demand intialization
 *   * State checking: Chech teei does not return incorrect way
 *     (fastcall vs normal SMC / interrupt)
 *   * Disable FIQs, if occuring when teei is not ok to handle them.
 */

// MSM areas
struct msm_area_t msm_area;

uint64_t pre_mpidr;
uint32_t uart_apc_num;
uint32_t spi_apc_num;

 unsigned int TEEI_STATE;
 uint32_t spsr_el3_t;
 uint32_t scr_el3_t;
 uint64_t elr_el3_t;

static uint32_t non_irq_num = 0x00;

// Context for each core. gp registers not used by SPD.
teei_context *secure_context = msm_area.secure_context;

uint64_t teei_irq_handler( uint32_t id,
          uint32_t flags,
          void *handle,
          void *cookie);

 uint64_t teei_fiq_handler( uint32_t id,
          uint32_t flags,
          void *handle,
          void *cookie);

static void teei_migrate_context_core(teei_context *teei_ctx, uint64_t from_mpidr, uint64_t to_mpidr)
{
	uint32_t from_linear_id = platform_get_core_pos(from_mpidr);
	uint32_t to_linear_id = platform_get_core_pos(to_mpidr);

	memcpy(&secure_context[to_linear_id] , &secure_context[from_linear_id], sizeof(secure_context[from_linear_id]));
	//secure_context[to_linear_id]  = secure_context[from_linear_id];
	//secure_context[to_linear_id] .mpidr = to_mpidr;
	teei_ctx = &secure_context[to_linear_id];
	cm_set_context_by_mpidr(to_mpidr, &teei_ctx->cpu_ctx, SECURE);
}

static void teei_set_sec_dev()
{
			/*add spi secure attribute,
			* this time the ree fp driver
			*  have been inited.*/
			//FIXME : Make it be configured
		//	set_module_apc(spi_apc_num, 0, 1);
}
static void teei_prepare_switch_entry( uint32_t secure_state)
{

	if (secure_state == SECURE){
    /*add your device context here ,
     * maybe include save nt device state  and restore
     * t devcie state  --steven*/
     //set_module_apc(uart_apc_num, 0, 1); // set uart secure
     /*gic context*/
    migrate_gic_context(SECURE);
    cpu_context_t *s_context = (cpu_context_t *) cm_get_context_by_mpidr(read_mpidr(), SECURE);

	// Entry to teei
	el3_state_t *el3sysregs = get_el3state_ctx(s_context);
	 //write_ctx_reg(el3sysregs, CTX_SPSR_EL3, 0x1d1);
	 write_ctx_reg(el3sysregs, CTX_SCR_EL3, SCR_EL3_S);

	} else {
	  /*add your device context here ,
     * maybe include save t device state  and restore
     * nt devcie state  --steven*/
     //set_module_apc(uart_apc_num, 0, 0); // set uart share

     /*gic context*/
     migrate_gic_context(NON_SECURE);
     cpu_context_t *ns_context = (cpu_context_t *) cm_get_context_by_mpidr(read_mpidr(), NON_SECURE);
	// Entry to nwd
	el3_state_t *el3sysregs = get_el3state_ctx(ns_context);
	// write_ctx_reg(el3sysregs, CTX_SPSR_EL3, nwdEntrySpsr);
		//	 write_ctx_reg(el3sysregs, CTX_SCR_EL3, SCR_EL3_NS_FIQ);

		uint64_t scr_el3 = read_ctx_reg(el3sysregs, CTX_SCR_EL3) | SCR_FIQ_BIT;
		write_ctx_reg(el3sysregs, CTX_SCR_EL3, scr_el3);
	}

}
static void teei_passing_param(uint64_t x1, uint64_t x2, uint64_t x3)
{
	cpu_context_t *s_context = (cpu_context_t *) cm_get_context_by_mpidr(read_mpidr(), SECURE);

	gp_regs_t *s_gpregs = get_gpregs_ctx(s_context);

	write_ctx_reg(s_gpregs, CTX_GPREG_X1, x1 );
	write_ctx_reg(s_gpregs, CTX_GPREG_X2, x2 );
	write_ctx_reg(s_gpregs, CTX_GPREG_X3, x3 );

}

static void ns_teei_passing_param(uint64_t x1, uint64_t x2, uint64_t x3)
{
	cpu_context_t *ns_context = (cpu_context_t *) cm_get_context_by_mpidr(read_mpidr(), NON_SECURE);

	gp_regs_t *ns_gpregs = get_gpregs_ctx(ns_context);

	write_ctx_reg(ns_gpregs, CTX_GPREG_X1, x1 );
	write_ctx_reg(ns_gpregs, CTX_GPREG_X2, x2 );
	write_ctx_reg(ns_gpregs, CTX_GPREG_X3, x3 );
}

 void teei_register_fiq_handler()
{
	uint32_t flags = 0;
	set_interrupt_rm_flag(flags, NON_SECURE);

	uint32_t rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
		teei_fiq_handler,
		flags);

	if (rc!=0) {
		DBG_PRINTF( "======  FIQ register failed.\n\r");
	}
	else {
//		DBG_PRINTF( "======  FIQ register success! \n\r");
	}
}

static void teei_trigger_soft_intr(uint32_t id)
{
	trigger_soft_intr(id);
}


// ************************************************************************************
// fastcall handler

static uint64_t teei_fastcall_handler(uint32_t smc_fid,
        uint64_t x1,
        uint64_t x2,
        uint64_t x3,
        uint64_t x4,
        void *cookie,
        void *handle,
        uint64_t flags)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	teei_context *teei_ctx = &secure_context[linear_id];
	int caller_security_state = flags&1;
	static uint32_t count;
	//set_uart_flag();
 //	unsigned int gicd_base = get_plat_config()->gicd_base;

if (caller_security_state==SECURE) {
#if MICROTRUST_LOG
		DBG_PRINTF("ft, t to nt id[%x] cid[%ld]\n\r", maskSWdRegister(smc_fid),linear_id);
#endif

		assert(handle == cm_get_context_by_mpidr(mpidr, SECURE));
		switch(maskSWdRegister(smc_fid)) {
			 case T_BOOT_NT_OS:  {
					// Return values from fastcall already in cpu_context!
					TEEI_STATE = TEEI_BOOT;
					teei_register_fiq_handler();
					prepare_gic_for_nsec_boot();
					teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
				}
			case T_ACK_N_OS_READY:  {
					non_irq_num = NSEC_BOOT_INTR;
					teei_trigger_soft_intr(NSEC_CALL_INTR);
					TEEI_STATE = TEEI_SERVICE_READY;
					// Return values from fastcall already in cpu_context!
					teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
			}
			case T_ACK_N_FAST_CALL:{
					non_irq_num = NSEC_BOOT_INTR;
					teei_trigger_soft_intr(NSEC_CALL_INTR);
					// Return values from fastcall already in cpu_context!
					teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
			}
			case T_ACK_N_INIT_FC_BUF: {
				    if (count == 1) {
					TEEI_STATE = TEEI_BUF_READY;
					enable_group(1);
					}
					count = 1;
					// Return values from fastcall already in cpu_context!
					teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
			}
			case T_GET_BOOT_PARAMS: {
				x1 =  TEEI_BOOT_PARAMS;
				x2 =  TEEI_SECURE_PARAMS;
				teei_passing_param(x1, x2, x3);
				SMC_RET1(handle, T_GET_BOOT_PARAMS);
			}
			case T_WDT_FIQ_DUMP: {
				mt_fiq_smp_call_function(aee_wdt_dump, 0, 0);
				aee_wdt_dump();
				//DBG_PRINTF( "teei_fastcall_handler T_WDT_FIQ_DUMP ! \n\r" );
				SMC_RET1(handle, T_WDT_FIQ_DUMP);
			}
			case T_FIQ_SGI_DUMP: {
				cm_el1_sysregs_context_restore(NON_SECURE);
				//cm_fpregs_context_restore(NON_SECURE);
				cm_set_next_eret_context(NON_SECURE);
				fiq_icc_isr();
				break;
			}
			case T_VUART_LOG_CALL: {

	               // set_uart_flag();
        	       // DBG_PRINTF("vuart[%c][%x]\n\r",x1,x2);

               		// clear_uart_flag();
			mt_log_secure_os_print(0xff & x1);
			 SMC_RET1(handle, T_GET_BOOT_PARAMS);}
			default:  {
				DBG_PRINTF( "teei_fastcall_handler T SMC_UNK %x\n\r", smc_fid );
				SMC_RET1(handle, SMC_UNK);
				break;
			  }
		}
  } else {
	    //set_module_apc(uart_apc_num, 0, 1); // set uart secure
#if MICROTRUST_LOG
	   	DBG_PRINTF("ft,nt to t,id[%x]cid[%ld]\n\r", smc_fid,linear_id);
#endif
	   assert(handle == cm_get_context_by_mpidr(mpidr, NON_SECURE));
		switch (smc_fid) {
			case N_INIT_T_BOOT_STAGE1:
			case N_INIT_T_BOOT_STAGE1_32:
			case N_SWITCH_TO_T_OS_STAGE2:
			case N_SWITCH_TO_T_OS_STAGE2_32:
			case N_INIT_T_FC_BUF:
			case N_INIT_T_FC_BUF_32:
			case N_INVOKE_T_FAST_CALL:
			case N_INVOKE_T_FAST_CALL_32: {
							cm_el1_sysregs_context_save(NON_SECURE);
							//cm_fpregs_context_save(NON_SECURE);
							      if (( smc_fid == N_INIT_T_BOOT_STAGE1 )||(smc_fid == N_INIT_T_BOOT_STAGE1_32))
								teei_set_sec_dev();
							      if (( smc_fid == N_INIT_T_FC_BUF || smc_fid == N_INIT_T_BOOT_STAGE1) ||(smc_fid == N_INIT_T_FC_BUF_32 || smc_fid == N_INIT_T_BOOT_STAGE1_32) )
								teei_passing_param(x1, x2, x3);
							teei_prepare_switch_entry(SECURE);
							      if ((smc_fid == N_INVOKE_T_FAST_CALL) ||(smc_fid == N_INVOKE_T_FAST_CALL_32))
								teei_trigger_soft_intr(SEC_APP_INTR);
							if (TEEI_STATE == TEEI_BOOT)
								prepare_gic_for_sec_boot();
							teei_synchronous_sp_entry(teei_ctx);
							cm_el1_sysregs_context_restore(NON_SECURE);
							//cm_fpregs_context_restore(NON_SECURE);
							cm_set_next_eret_context(NON_SECURE);
						    teei_prepare_switch_entry(NON_SECURE);
							break;
					}
			case N_SWITCH_CORE:
			case N_SWITCH_CORE_32:
					{
#if MICROTRUST_LOG
		DBG_PRINTF( "SC       [%ld] [%ld]\n\r",platform_get_core_pos(x2),platform_get_core_pos(x1));
#endif

						//	uint32_t cur_mpidr =  read_mpidr();
							teei_context *teei_ctx;
							teei_migrate_context_core(teei_ctx, x2, x1);
							SMC_RET1(handle, N_SWITCH_CORE);
					}

			case N_GET_NON_IRQ_NUM:
			case N_GET_NON_IRQ_NUM_32:
						      {
#if MICROTRUST_LOG
					DBG_PRINTF( "NON IRQ NUM [%d]",non_irq_num);
#endif
					ns_teei_passing_param(non_irq_num, x2, x3);
					non_irq_num = 0;
					SMC_RET1(handle, N_GET_NON_IRQ_NUM);
				}

		      default: {
					DBG_PRINTF( "teei_fastcall_handler NT SMC_UNK %x\n\r", smc_fid );
					SMC_RET1(handle, SMC_UNK);
					break;
				}
	}
}
  //clear_uart_flag();
  return 0;
}

// ************************************************************************************
// Standard call handler
extern unsigned int NSEC_UART;


static uint64_t teei_standard_call_handler(uint32_t smc_fid,
        uint64_t x1,
        uint64_t x2,
        uint64_t x3,
        uint64_t x4,
        void *cookie,
        void *handle,
        uint64_t flags)
{

  uint64_t mpidr = read_mpidr();
  uint32_t linear_id = platform_get_core_pos(mpidr);
  teei_context *teei_ctx = &secure_context[linear_id];
  int caller_security_state = flags&1;
   	//unsigned int gicd_base;
	//gicd_base = get_plat_config()->gicd_base;

  //set_uart_flag();

 if (caller_security_state==SECURE) {
#if MICROTRUST_LOG
		DBG_PRINTF("st, t to nt id[%x]cid[%ld]\n\r", maskSWdRegister(smc_fid),linear_id);
#endif
		assert(handle == cm_get_context_by_mpidr(mpidr, SECURE));
		switch(maskSWdRegister(smc_fid)) {
				case    T_SCHED_NT: {
					non_irq_num =  NSEC_SCHED_INTR;
					 teei_trigger_soft_intr(NSEC_CALL_INTR);
					 // Return values from fastcall already in cpu_context!
					 teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
				}
				case    T_SCHED_NT_IRQ: {
					teei_trigger_soft_intr(NSEC_INTSCHED_INTR);
					//  Return values from fastcall already in cpu_context!
					teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
				}
				case	T_SCHED_NT_LOG: {
					non_irq_num =  NSEC_LOG_INTR;
					teei_trigger_soft_intr(NSEC_INTSCHED_INTR);
					teei_trigger_soft_intr(NSEC_CALL_INTR);
					//  Return values from fastcall already in cpu_context!
					teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call


				}
				case T_NOTIFY_N_ERR: {
					non_irq_num =  NSEC_ERR_INTR;
					teei_trigger_soft_intr(NSEC_CALL_INTR);
			        // DBG_PRINTF( "[Microtrust]:  =======  Secure world error !!!! \n\r");
					//  Return values from fastcall already in cpu_context!
					teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
				}
				case    T_INVOKE_N_LOAD_IMG:
				case    T_ACK_N_KERNEL_OK:
				case	T_ACK_N_NQ:
				case	T_ACK_N_INVOKE_DRV:
				case    T_INVOKE_N_DRV:
				case    T_ACK_N_BOOT_OK:
				 {
					if (maskSWdRegister(smc_fid) != T_ACK_N_BOOT_OK &&  \
						maskSWdRegister(smc_fid) != T_ACK_N_KERNEL_OK && \
						maskSWdRegister(smc_fid) != T_ACK_N_INVOKE_DRV && \
						maskSWdRegister(smc_fid) != T_INVOKE_N_DRV){
						non_irq_num =  NSEC_APP_INTR;
						teei_trigger_soft_intr(NSEC_CALL_INTR);
					 } else if (maskSWdRegister(smc_fid) == T_ACK_N_INVOKE_DRV){
						non_irq_num =  NSEC_DRV_INTR;
						teei_trigger_soft_intr(NSEC_CALL_INTR);
					 } else if (maskSWdRegister(smc_fid) == T_INVOKE_N_DRV){
						non_irq_num =  NSEC_RDRV_INTR;
						teei_trigger_soft_intr(NSEC_CALL_INTR);
					 } else {
						non_irq_num =  NSEC_BOOT_INTR;
					    teei_trigger_soft_intr(NSEC_CALL_INTR);
				     }
					if (maskSWdRegister(smc_fid) == T_ACK_N_KERNEL_OK ) {
							TEEI_STATE = TEEI_KERNEL_READY;
						}
					if (maskSWdRegister(smc_fid) == T_ACK_N_BOOT_OK ) {
							TEEI_STATE = TEEI_ALL_READY;
						}
					// Return values from fastcall already in cpu_context!
					 teei_synchronous_sp_exit(teei_ctx, 0, 1);   // it will return to teei_synchronous_sp_entry call
				}
			default:  {
						//DBG_PRINTF( "teei_fastcall_handler T SMC_UNK %x\n\r", smc_fid );
						SMC_RET1(handle, SMC_UNK);
						break;
				}
		}
  } else {
	    //set_module_apc(uart_apc_num, 0, 1); // set uart secure
#if MICROTRUST_LOG
	    DBG_PRINTF("st,nt to t, id [%x]cid[%ld]\n\r", smc_fid,linear_id);
#endif
	    assert(handle == cm_get_context_by_mpidr(mpidr, NON_SECURE));
		switch (smc_fid) {
			case  N_ACK_T_LOAD_IMG:
			case  N_ACK_T_LOAD_IMG_32:
			case  NT_SCHED_T:
			case  NT_SCHED_T_32:
			case  N_INVOKE_T_NQ:
			case  N_INVOKE_T_NQ_32:
			case  N_INVOKE_T_DRV:
			case  N_INVOKE_T_DRV_32:
			case  N_ACK_T_INVOKE_DRV:
			case  N_ACK_T_INVOKE_DRV_32:
			case  N_INVOKE_T_LOAD_TEE:
			case  N_INVOKE_T_LOAD_TEE_32:
					 {
									cm_el1_sysregs_context_save(NON_SECURE);
									//cm_fpregs_context_save(NON_SECURE);
									teei_prepare_switch_entry(SECURE);
					if ( (smc_fid != NT_SCHED_T && smc_fid != N_ACK_T_LOAD_IMG \
								&& smc_fid != N_INVOKE_T_DRV && smc_fid != N_ACK_T_INVOKE_DRV) \
							&&(smc_fid != NT_SCHED_T_32 && smc_fid != N_ACK_T_LOAD_IMG_32 \
								&& smc_fid != N_INVOKE_T_DRV_32 && smc_fid != N_ACK_T_INVOKE_DRV_32) )
											teei_trigger_soft_intr(SEC_APP_INTR);
									if (TEEI_STATE == TEEI_BOOT)
											prepare_gic_for_sec_boot();
					if ((smc_fid == N_INVOKE_T_DRV) ||(smc_fid == N_INVOKE_T_DRV_32))
	    									teei_trigger_soft_intr(SEC_DRV_INTR);
					if ((smc_fid == N_ACK_T_INVOKE_DRV) ||(smc_fid == N_ACK_T_INVOKE_DRV_32))
	    									teei_trigger_soft_intr(SEC_RDRV_INTR);
									teei_synchronous_sp_entry(teei_ctx);
									cm_el1_sysregs_context_restore(NON_SECURE);
								    //cm_fpregs_context_restore(NON_SECURE);
									cm_set_next_eret_context(NON_SECURE);
									teei_prepare_switch_entry(NON_SECURE);
									break;
					  }
		         default: {
								DBG_PRINTF( "teei_fastcall_handler NT SMC_UNK %x\n\r", smc_fid );
								SMC_RET1(handle, SMC_UNK);
								break;
				       }
			}
  }
  //clear_uart_flag();
  return 0;
}

uint64_t teei_fiq_el3_handler(void)
{
	/* ack & get interrupt id */
	uint32_t irq = plat_ic_acknowledge_interrupt() & 0x3ff;

	/* Need to save non-secure context because later aee_wdt_dump() would
	 * (1) access non-secure context data structure
	 * (2) restore to REE from non-secure context */
	cm_el1_sysregs_context_save(NON_SECURE);
	// cm_fpregs_context_save(NON_SECURE);

	switch (irq) {
	case FIQ_SMP_CALL_SGI:
		//DBG_PRINTF("teei_fiq_handler : FIQ_SMP_CALL_SGI \n ");
		fiq_icc_isr();
		break;
	case WDT_IRQ_BIT_ID:
		//DBG_PRINTF("teei_fiq_handler : Catch WDT FIQ !!! \n ");
		mt_fiq_smp_call_function(aee_wdt_dump, 0, 0);
		aee_wdt_dump();
		break;
	default:
		//DBG_PRINTF("teei_fiq_handler : UNK FIQ NUM \n ");
		break;
	}

	plat_ic_end_of_interrupt(irq);
	return 0;
}

/*fiq is handled by S-EL1*/
uint64_t teei_fiq_handler( uint32_t id,
			   uint32_t flags,
			   void *handle,
			   void *cookie)
{
	uint32_t linear_id = platform_get_core_pos(read_mpidr());
	uint32_t irq;
	//teei_context *teei_ctx = &secure_context[linear_id];
	int caller_security_state = flags&1;

	//set_module_apc(uart_apc_num, 0, 1); // set uart secure
	//set_uart_flag();

	if (caller_security_state==SECURE) {
		SMC_RET1(handle, SMC_UNK);
	}

	/* get pending irq id , no ack */
	irq = plat_ic_get_pending_interrupt_id();

	DBG_PRINTF("fiq id [%x]cid[%ld]\n\r", irq,linear_id);

	if (irq == FIQ_SMP_CALL_SGI ||
	    irq == WDT_IRQ_BIT_ID) {
		/* irq which need to be handlered in EL3 */
		teei_fiq_el3_handler();
	}

	SMC_RET0(handle);
}

/* Register teei fastcalls service */
DECLARE_RT_SVC(
  teei_fastcall,
  OEN_TOS_START,
  OEN_TOS_END,
  SMC_TYPE_FAST,
  teei_fastcall_setup,
  teei_fastcall_handler
);

/* Register teei SMC service */
// Note: OEN_XXX constants do not apply to normal SMCs (only to fastcalls).
DECLARE_RT_SVC(
  teei_standard_call,
  OEN_TOS_START,
  OEN_TOS_END,
  SMC_TYPE_STD,
  NULL,
  teei_standard_call_handler
);

#if TEEI_OEM_ROUTE_ENABLE
/* Register teei OEM SMC handler service */
DECLARE_RT_SVC(
  teei_oem_fastcall,
  OEN_OEM_START,
  OEN_OEM_END,
  SMC_TYPE_FAST,
  NULL,
  teei_fastcall_handler
);
#endif

#if TEEI_SIP_ROUTE_ENABLE
/* Register teei SIP SMC handler service */
DECLARE_RT_SVC(
  teei_sip_fastcall,
  OEN_SIP_START,
  OEN_SIP_END,
  SMC_TYPE_FAST,
  NULL,
  teei_fastcall_handler
);
#endif

#if TEEI_DUMMY_SIP_ROUTE_ENABLE
/* Register teei SIP SMC handler service, unfortunately because of a typo in our
 * older versions we must serve the 0x81000000 fastcall range for backward compat */
DECLARE_RT_SVC(
  teei_dummy_sip_fastcall,
  OEN_CPU_START,
  OEN_CPU_END,
  SMC_TYPE_FAST,
  teei_dummy_setup,
  teei_fastcall_handler
);
#endif
