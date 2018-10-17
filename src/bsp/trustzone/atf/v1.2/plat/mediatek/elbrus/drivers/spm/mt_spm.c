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
#include <arch.h>
#include <bakery_lock.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware, i.e.,
 * - spm_hotplug.c for cpu power control in cpu hotplug flow.
 * - spm_mcdi.c for cpu power control in cpu idle power saving state.
 * - spm_suspend.c for system power control in system suspend scenario.
 *
 * This file provide utility functions common to hotplug, mcdi(idle), suspend
 * power scenarios. A bakery lock (software lock) is incoporated to protect
 * certain critical sections to avoid kicking different SPM firmware
 * concurrently.
 */

DEFINE_BAKERY_LOCK(spm_lock);

void spm_lock_init(void)
{
	bakery_lock_init(&spm_lock);
}

void spm_lock_get(void)
{
	bakery_lock_get(&spm_lock);
}

void spm_lock_release(void)
{
	bakery_lock_release(&spm_lock);
}

#define SPM_FW_MAGIC    0x53504D32

static void *local_buf = (void *)SPM_FW_BASE;
static int dyna_load_pcm_done;
static char *dyna_load_pcm_path[] = {
	[DYNA_LOAD_PCM_SUSPEND] = "pcm_allinone.bin",
	[DYNA_LOAD_PCM_MAX] = "pcm_path_max",
};

struct dyna_load_pcm_t dyna_load_pcm[DYNA_LOAD_PCM_MAX];

#pragma pack(push)
#pragma pack(2)
struct spm_fw_header {
	unsigned int magic;
	unsigned int size;
	char name[58];
};
#pragma pack(pop)

char *spm_load_firmware(char *name)
{
	struct spm_fw_header *header;
	char *addr;

	addr = 0;
	header = (struct spm_fw_header *) local_buf;

	while (header->magic == SPM_FW_MAGIC) {
		if (!strcmp(header->name, name))
			break;

		header = (struct spm_fw_header *) (((char *) header) + sizeof(*header) + header->size);
	}

	if (header->magic == SPM_FW_MAGIC) {
		addr = (((char *) header) + sizeof(*header));
		return addr;
	}

	return NULL;
}

int spm_load_pcm_firmware(void)
{
	int err = 0;
	int i;
	int offset = 0;
	int spm_fw_count = 0;

	if (dyna_load_pcm_done)
		return err;

	for (i = DYNA_LOAD_PCM_SUSPEND; i < DYNA_LOAD_PCM_MAX; i++) {
		__uint16_t firmware_size = 0;
		int copy_size = 0;
		struct pcm_desc *pdesc = &(dyna_load_pcm[i].desc);
		char *ptr;

		ptr = spm_load_firmware(dyna_load_pcm_path[i]);
		if (ptr == NULL) {
			ERROR("Failed to load %s, err = %d.\n", dyna_load_pcm_path[i], err);
			err = -1;
			continue;
		}

		/* Do whatever it takes to load firmware into device. */
		/* start of binary size */
		offset = 0;
		copy_size = 2;
		memcpy(&firmware_size, ptr + offset, copy_size);

		/* start of binary */
		offset += copy_size;
		copy_size = firmware_size * 4;
		dyna_load_pcm[i].buf = ptr + offset;

		/* start of pcm_desc without pointer */
		offset += copy_size;
		copy_size = sizeof(struct pcm_desc) - offsetof(struct pcm_desc, size);
		memcpy((void *)&(dyna_load_pcm[i].desc.size), ptr + offset, copy_size);

		/* start of pcm_desc version */
		offset += copy_size;
		snprintf(dyna_load_pcm[i].version, PCM_FIRMWARE_VERSION_SIZE - 1,
				"%s", ptr + offset);
		pdesc->version = dyna_load_pcm[i].version;
		pdesc->base = (__uint32_t *) dyna_load_pcm[i].buf;
		pdesc->base_dma = (__uint64_t)dyna_load_pcm[i].buf;

		INFO("#@# %s(%d) use spmfw partition for %s - %s\n", __func__, __LINE__,
				dyna_load_pcm_path[i], pdesc->version);

		dyna_load_pcm[i].ready = 1;
		spm_fw_count++;
	}

	if (spm_fw_count == DYNA_LOAD_PCM_MAX) {
		dyna_load_pcm_done = 1;
	}

	return err;
}

__uint64_t spm_load_firmware_status()
{
	return dyna_load_pcm_done;
}

void spm_irq0_handler(__uint64_t x1)
{
	mmio_write_32(SPM_IRQ_MASK, mmio_read_32(SPM_IRQ_MASK) | ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, x1);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT0);
}

void spm_ap_mdsrc_req(__uint64_t x1)
{
	if (x1)
		mmio_write_32(AP_MDSRC_REQ, mmio_read_32(AP_MDSRC_REQ) | AP_MD1SRC_REQ_LSB);
	else
		mmio_write_32(AP_MDSRC_REQ, mmio_read_32(AP_MDSRC_REQ) & ~AP_MD1SRC_REQ_LSB);
}

void save_pwr_ctrl(struct pwr_ctrl *pwrctrl, unsigned int index, unsigned int val)
{
	switch (index)
	{
		case PWR_PCM_FLAGS:
			pwrctrl->pcm_flags = val;
			break;
		case PWR_PCM_FLAGS_CUST:
			pwrctrl->pcm_flags_cust = val;
			break;
		case PWR_PCM_FLAGS_CUST_SET:
			pwrctrl->pcm_flags_cust_set = val;
			break;
		case PWR_PCM_FLAGS_CUST_CLR:
			pwrctrl->pcm_flags_cust_clr = val;
			break;
		case PWR_PCM_RESERVE:
			pwrctrl->pcm_reserve = val;
			break;
		case PWR_TIMER_VAL:
			pwrctrl->timer_val = val;
			break;
		case PWR_TIMER_VAL_CUST:
			pwrctrl->timer_val_cust = val;
			break;
		case PWR_TIMER_VAL_RAMP_EN:
			pwrctrl->timer_val_ramp_en = val;
			break;
		case PWR_TIMER_VAL_RAMP_EN_SEC:
			pwrctrl->timer_val_ramp_en_sec = val;
			break;
		case PWR_WAKE_SRC:
			pwrctrl->wake_src = val;
			break;
		case PWR_WAKE_SRC_CUST:
			pwrctrl->wake_src_cust = val;
			break;
		case PWR_WAKE_SRC_MD32:
			pwrctrl->wake_src_md32 = val;
			break;
		case PWR_WDT_DISABLE:
			pwrctrl->wdt_disable = val;
			break;
		case PWR_DVFS_HALT_SRC_CHK:
			pwrctrl->dvfs_halt_src_chk = val;
			break;
		case PWR_SYSPWREQ_MASK:
			pwrctrl->syspwreq_mask = val;
			break;
		case PWR_REG_SRCCLKEN0_CTL:
			pwrctrl->spm_clk_con.bits.reg_srcclken0_ctl = val;
			break;
		case PWR_REG_SRCCLKEN1_CTL:
			pwrctrl->spm_clk_con.bits.reg_srcclken1_ctl = val;
			break;
		case PWR_REG_SPM_LOCK_INFRA_DCM:
			pwrctrl->spm_clk_con.bits.reg_spm_lock_infra_dcm = val;
			break;
		case PWR_REG_SRCCLKEN_MASK:
			pwrctrl->spm_clk_con.bits.reg_srcclken_mask = val;
			break;
		case PWR_REG_MD1_C32RM_EN:
			pwrctrl->spm_clk_con.bits.reg_md1_c32rm_en = val;
			break;
		case PWR_REG_MD2_C32RM_EN:
			pwrctrl->spm_clk_con.bits.reg_md2_c32rm_en = val;
			break;
		case PWR_REG_CLKSQ0_SEL_CTRL:
			pwrctrl->spm_clk_con.bits.reg_clksq0_sel_ctrl = val;
			break;
		case PWR_REG_CLKSQ1_SEL_CTRL:
			pwrctrl->spm_clk_con.bits.reg_clksq1_sel_ctrl = val;
			break;
		case PWR_REG_SRCCLKEN0_EN:
			pwrctrl->spm_clk_con.bits.reg_srcclken0_en = val;
			break;
		case PWR_REG_SRCCLKEN1_EN:
			pwrctrl->spm_clk_con.bits.reg_srcclken1_en = val;
			break;
		case PWR_REG_SYSCLK0_SRC_MASK_B:
			pwrctrl->spm_clk_con.bits.reg_sysclk0_src_mask_b = val;
			break;
		case PWR_REG_SYSCLK1_SRC_MASK_B:
			pwrctrl->spm_clk_con.bits.reg_sysclk1_src_mask_b = val;
			break;
		case PWR_REG_MPWFI_OP:
			pwrctrl->spm_ap_standby_con.bits.reg_mpwfi_op = val;
			break;
		case PWR_REG_MP0_CPUTOP_IDLE_MASK:
			pwrctrl->spm_ap_standby_con.bits.reg_mp0_cputop_idle_mask = val;
			break;
		case PWR_REG_MP1_CPUTOP_IDLE_MASK:
			pwrctrl->spm_ap_standby_con.bits.reg_mp1_cputop_idle_mask = val;
			break;
		case PWR_REG_DEBUGTOP_IDLE_MASK:
			pwrctrl->spm_ap_standby_con.bits.reg_debugtop_idle_mask = val;
			break;
		case PWR_REG_MP_TOP_IDLE_MASK:
			pwrctrl->spm_ap_standby_con.bits.reg_mp_top_idle_mask = val;
			break;
		case PWR_REG_MCUSYS_IDLE_MASK:
			pwrctrl->spm_ap_standby_con.bits.reg_mcusys_idle_mask = val;
			break;
		case PWR_REG_MD_DDR_EN_0_DBC_EN:
			pwrctrl->spm_ap_standby_con.bits.reg_md_ddr_en_0_dbc_en = val;
			break;
		case PWR_REG_MD_DDR_EN_1_DBC_EN:
			pwrctrl->spm_ap_standby_con.bits.reg_md_ddr_en_1_dbc_en = val;
			break;
		case PWR_REG_CONN_DDR_EN_DBC_EN:
			pwrctrl->spm_ap_standby_con.bits.reg_conn_ddr_en_dbc_en = val;
			break;
		case PWR_REG_MD32_MASK_B:
			pwrctrl->spm_ap_standby_con.bits.reg_md32_mask_b = val;
			break;
		case PWR_REG_MD_0_MASK_B:
			pwrctrl->spm_ap_standby_con.bits.reg_md_0_mask_b = val;
			break;
		case PWR_REG_MD_1_MASK_B:
			pwrctrl->spm_ap_standby_con.bits.reg_md_1_mask_b = val;
			break;
		case PWR_REG_SCP_MASK_B:
			pwrctrl->spm_ap_standby_con.bits.reg_scp_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI0_MASK_B:
			pwrctrl->spm_ap_standby_con.bits.reg_srcclkeni0_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI1_MASK_B:
			pwrctrl->spm_ap_standby_con.bits.reg_srcclkeni1_mask_b = val;
			break;
		case PWR_REG_MD_APSRC_1_SEL:
			pwrctrl->spm_ap_standby_con.bits.reg_md_apsrc_1_sel = val;
			break;
		case PWR_REG_MD_APSRC_0_SEL:
			pwrctrl->spm_ap_standby_con.bits.reg_md_apsrc_0_sel = val;
			break;
		case PWR_REG_CONN_MASK_B:
			pwrctrl->spm_ap_standby_con.bits.reg_conn_mask_b = val;
			break;
		case PWR_REG_CONN_APSRC_SEL:
			pwrctrl->spm_ap_standby_con.bits.reg_conn_apsrc_sel = val;
			break;
		case PWR_REG_SPM_APSRC_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_apsrc_req = val;
			break;
		case PWR_REG_SPM_F26M_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_f26m_req = val;
			break;
		case PWR_REG_SPM_INFRA_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_infra_req = val;
			break;
		case PWR_REG_SPM_DDREN_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_ddren_req = val;
			break;
		case PWR_REG_SPM_VRF18_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_vrf18_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL0_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_dvfs_level0_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL1_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_dvfs_level1_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL2_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_dvfs_level2_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL3_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_dvfs_level3_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL4_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_dvfs_level4_req = val;
			break;
		case PWR_REG_SPM_PMCU_MAILBOX_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_pmcu_mailbox_req = val;
			break;
		case PWR_REG_SPM_SW_MAILBOX_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_sw_mailbox_req = val;
			break;
		case PWR_REG_SPM_CKSEL2_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_cksel2_req = val;
			break;
		case PWR_REG_SPM_CKSEL3_REQ:
			pwrctrl->spm_src_req.bits.reg_spm_cksel3_req = val;
			break;
		case PWR_REG_CSYSPWREQ_MASK:
			pwrctrl->spm_src_mask.bits.reg_csyspwreq_mask = val;
			break;
		case PWR_REG_MD_SRCCLKENA_0_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md_srcclkena_0_infra_mask_b = val;
			break;
		case PWR_REG_MD_SRCCLKENA_1_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md_srcclkena_1_infra_mask_b = val;
			break;
		case PWR_REG_MD_APSRC_REQ_0_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md_apsrc_req_0_infra_mask_b = val;
			break;
		case PWR_REG_MD_APSRC_REQ_1_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md_apsrc_req_1_infra_mask_b = val;
			break;
		case PWR_REG_CONN_SRCCLKENA_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_conn_srcclkena_infra_mask_b = val;
			break;
		case PWR_REG_CONN_INFRA_REQ_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_conn_infra_req_mask_b = val;
			break;
		case PWR_REG_MD32_SRCCLKENA_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md32_srcclkena_infra_mask_b = val;
			break;
		case PWR_REG_MD32_INFRA_REQ_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md32_infra_req_mask_b = val;
			break;
		case PWR_REG_SCP_SRCCLKENA_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_scp_srcclkena_infra_mask_b = val;
			break;
		case PWR_REG_SCP_INFRA_REQ_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_scp_infra_req_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI0_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_srcclkeni0_infra_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI1_INFRA_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_srcclkeni1_infra_mask_b = val;
			break;
		case PWR_REG_CCIF0_MD_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif0_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF0_AP_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif0_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIF1_MD_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif1_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF1_AP_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif1_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIF2_MD_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif2_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF2_AP_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif2_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIF3_MD_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif3_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF3_AP_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccif3_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIFMD_MD1_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccifmd_md1_event_mask_b = val;
			break;
		case PWR_REG_CCIFMD_MD2_EVENT_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ccifmd_md2_event_mask_b = val;
			break;
		case PWR_REG_C2K_PS_RCCIF_WAKE_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_c2k_ps_rccif_wake_mask_b = val;
			break;
		case PWR_REG_C2K_L1_RCCIF_WAKE_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_c2k_l1_rccif_wake_mask_b = val;
			break;
		case PWR_REG_PS_C2K_RCCIF_WAKE_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_ps_c2k_rccif_wake_mask_b = val;
			break;
		case PWR_REG_L1_C2K_RCCIF_WAKE_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_l1_c2k_rccif_wake_mask_b = val;
			break;
		case PWR_REG_DQSSOC_REQ_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_dqssoc_req_mask_b = val;
			break;
		case PWR_REG_DISP2_REQ_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_disp2_req_mask_b = val;
			break;
		case PWR_REG_MD_DDR_EN_0_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md_ddr_en_0_mask_b = val;
			break;
		case PWR_REG_MD_DDR_EN_1_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_md_ddr_en_1_mask_b = val;
			break;
		case PWR_REG_CONN_DDR_EN_MASK_B:
			pwrctrl->spm_src_mask.bits.reg_conn_ddr_en_mask_b = val;
			break;
		case PWR_REG_DISP0_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_disp0_req_mask_b = val;
			break;
		case PWR_REG_DISP1_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_disp1_req_mask_b = val;
			break;
		case PWR_REG_DISP_OD_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_disp_od_req_mask_b = val;
			break;
		case PWR_REG_MFG_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_mfg_req_mask_b = val;
			break;
		case PWR_REG_VDEC0_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_vdec0_req_mask_b = val;
			break;
		case PWR_REG_GCE_VRF18_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_gce_vrf18_req_mask_b = val;
			break;
		case PWR_REG_GCE_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_gce_req_mask_b = val;
			break;
		case PWR_REG_LPDMA_REQ_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_lpdma_req_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI1_CKSEL2_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_srcclkeni1_cksel2_mask_b = val;
			break;
		case PWR_REG_CONN_SRCCLKENA_CKSEL2_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_conn_srcclkena_cksel2_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI0_CKSEL3_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_srcclkeni0_cksel3_mask_b = val;
			break;
		case PWR_REG_MD32_APSRC_REQ_DDREN_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_md32_apsrc_req_ddren_mask_b = val;
			break;
		case PWR_REG_SCP_APSRC_REQ_DDREN_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_scp_apsrc_req_ddren_mask_b = val;
			break;
		case PWR_REG_MD_VRF18_REQ_0_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_md_vrf18_req_0_mask_b = val;
			break;
		case PWR_REG_MD_VRF18_REQ_1_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_md_vrf18_req_1_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL0_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_next_dvfs_level0_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL1_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_next_dvfs_level1_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL2_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_next_dvfs_level2_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL3_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_next_dvfs_level3_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL4_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_next_dvfs_level4_mask_b = val;
			break;
		case PWR_REG_MSDC1_DVFS_HALT_MASK:
			pwrctrl->spm_src2_mask.bits.reg_msdc1_dvfs_halt_mask = val;
			break;
		case PWR_REG_MSDC2_DVFS_HALT_MASK:
			pwrctrl->spm_src2_mask.bits.reg_msdc2_dvfs_halt_mask = val;
			break;
		case PWR_REG_MSDC3_DVFS_HALT_MASK:
			pwrctrl->spm_src2_mask.bits.reg_msdc3_dvfs_halt_mask = val;
			break;
		case PWR_REG_SW2SPM_INT0_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_sw2spm_int0_mask_b = val;
			break;
		case PWR_REG_SW2SPM_INT1_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_sw2spm_int1_mask_b = val;
			break;
		case PWR_REG_SW2SPM_INT2_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_sw2spm_int2_mask_b = val;
			break;
		case PWR_REG_SW2SPM_INT3_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_sw2spm_int3_mask_b = val;
			break;
		case PWR_REG_PMCU2SPM_INT0_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_pmcu2spm_int0_mask_b = val;
			break;
		case PWR_REG_PMCU2SPM_INT1_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_pmcu2spm_int1_mask_b = val;
			break;
		case PWR_REG_PMCU2SPM_INT2_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_pmcu2spm_int2_mask_b = val;
			break;
		case PWR_REG_PMCU2SPM_INT3_MASK_B:
			pwrctrl->spm_src2_mask.bits.reg_pmcu2spm_int3_mask_b = val;
			break;
		case PWR_REG_WAKEUP_EVENT_MASK:
			pwrctrl->reg_wakeup_event_mask = val;
			break;
		case PWR_REG_EXT_WAKEUP_EVENT_MASK:
			pwrctrl->reg_ext_wakeup_event_mask = val;
			break;
		case PWR_MP0_CPU0_WFI_EN:
			pwrctrl->mp0_cpu0_wfi_en = val;
			break;
		case PWR_MP0_CPU1_WFI_EN:
			pwrctrl->mp0_cpu1_wfi_en = val;
			break;
		case PWR_MP0_CPU2_WFI_EN:
			pwrctrl->mp0_cpu2_wfi_en = val;
			break;
		case PWR_MP0_CPU3_WFI_EN:
			pwrctrl->mp0_cpu3_wfi_en = val;
			break;
		case PWR_MP1_CPU0_WFI_EN:
			pwrctrl->mp1_cpu0_wfi_en = val;
			break;
		case PWR_MP1_CPU1_WFI_EN:
			pwrctrl->mp1_cpu1_wfi_en = val;
			break;
		case PWR_MP1_CPU2_WFI_EN:
			pwrctrl->mp1_cpu2_wfi_en = val;
			break;
		case PWR_MP1_CPU3_WFI_EN:
			pwrctrl->mp1_cpu3_wfi_en = val;
			break;
		case PWR_DEBUG0_WFI_EN:
			pwrctrl->debug0_wfi_en = val;
			break;
		case PWR_DEBUG1_WFI_EN:
			pwrctrl->debug1_wfi_en = val;
			break;
		case PWR_DEBUG2_WFI_EN:
			pwrctrl->debug2_wfi_en = val;
			break;
		case PWR_DEBUG3_WFI_EN:
			pwrctrl->debug3_wfi_en = val;
			break;
		default:
			ERROR("%s: %d is undefined!!!\n", __func__, index);
	}
}

void spm_pwr_ctrl_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	switch(x1) {
		case SPM_PWR_CTRL_SUSPEND:
			save_pwr_ctrl(__spm_suspend.pwrctrl, x2, x3);
			break;
		case SPM_PWR_CTRL_DPIDLE:
			//save_pwr_ctrl(__spm_dpidle.pwrctrl, x2, x3);
			break;
		case SPM_PWR_CTRL_SODI:
			//save_pwr_ctrl(__spm_sodi.pwrctrl, x2, x3);
			break;
		case SPM_PWR_CTRL_SODI3:
			//save_pwr_ctrl(__spm_sodi3.pwrctrl, x2, x3);
			break;
		case SPM_PWR_CTRL_VCOREFS:
			//save_pwr_ctrl(__spm_vcorefs.pwrctrl, x2, x3);
			break;
	}
}

void spm_register_init(void)
{
	/* enable register control */
	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);

	/* init power control register */
	/* dram will set this register */
	/* mmio_write_32(SPM_POWER_ON_VAL0, 0); */
	mmio_write_32(SPM_POWER_ON_VAL1, POWER_ON_VAL1_DEF);
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	if ((mmio_read_32(PCM_FSM_STA) & 0x7fffff) != PCM_FSM_STA_DEF)
		ERROR("PCM reset failed\n");

	/* init PCM control register */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | RG_EN_IM_SLEEP_DVS_LSB);
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | REG_EVENT_LOCK_EN_LSB |
			REG_SPM_SRAM_ISOINT_B_LSB | RG_AHBMIF_APBEN_LSB);
	mmio_write_32(PCM_IM_PTR, 0);
	mmio_write_32(PCM_IM_LEN, 0);

	mmio_write_32(SPM_CLK_CON, mmio_read_32(SPM_CLK_CON) | (0x3 << 2) |
			REG_SPM_LOCK_INFRA_DCM_LSB | (1 << 6) |
			REG_CLKSQ1_SEL_CTRL_LSB | REG_SRCCLKEN0_EN_LSB |
			(0x20 << 23));

	/* clean wakeup event raw status */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, SPM_WAKEUP_EVENT_MASK_DEF);

	/* clean ISR status */
	mmio_write_32(SPM_IRQ_MASK, ISRM_ALL);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);

	/* init r7 with POWER_ON_VAL1 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	mmio_write_32(PCM_PWR_IO_EN, 0);
}

void spm_boot_init(void)
{
	spm_lock_init();
	spm_register_init();
	spm_load_pcm_firmware();
}
