#include <arch_helpers.h>
#include <arch.h>
#include <bakery_lock.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <platform_def.h>
#include <plat_private.h>
#include <scu.h>
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
	[DYNA_LOAD_PCM_SUSPEND] = "pcm_suspend.bin",
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

/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
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
		case PWR_PCM_FLAGS1:
			pwrctrl->pcm_flags1 = val;
			break;
		case PWR_PCM_FLAGS1_CUST:
			pwrctrl->pcm_flags1_cust = val;
			break;
		case PWR_PCM_FLAGS1_CUST_SET:
			pwrctrl->pcm_flags1_cust_set = val;
			break;
		case PWR_PCM_FLAGS1_CUST_CLR:
			pwrctrl->pcm_flags1_cust_clr = val;
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
		case PWR_OPP_LEVEL:
			pwrctrl->opp_level = val;
			break;
		case PWR_WDT_DISABLE:
			pwrctrl->wdt_disable = val;
			break;
		case PWR_SYSPWREQ_MASK:
			pwrctrl->syspwreq_mask = val;
			break;
		case PWR_REG_SRCCLKEN0_CTL:
			pwrctrl->reg_srcclken0_ctl = val;
			break;
		case PWR_REG_SRCCLKEN1_CTL:
			pwrctrl->reg_srcclken1_ctl = val;
			break;
		case PWR_REG_SPM_LOCK_INFRA_DCM:
			pwrctrl->reg_spm_lock_infra_dcm = val;
			break;
		case PWR_REG_SRCCLKEN_MASK:
			pwrctrl->reg_srcclken_mask = val;
			break;
		case PWR_REG_MD1_C32RM_EN:
			pwrctrl->reg_md1_c32rm_en = val;
			break;
		case PWR_REG_MD2_C32RM_EN:
			pwrctrl->reg_md2_c32rm_en = val;
			break;
		case PWR_REG_CLKSQ0_SEL_CTRL:
			pwrctrl->reg_clksq0_sel_ctrl = val;
			break;
		case PWR_REG_CLKSQ1_SEL_CTRL:
			pwrctrl->reg_clksq1_sel_ctrl = val;
			break;
		case PWR_REG_SRCCLKEN0_EN:
			pwrctrl->reg_srcclken0_en = val;
			break;
		case PWR_REG_SRCCLKEN1_EN:
			pwrctrl->reg_srcclken1_en = val;
			break;
		case PWR_REG_SYSCLK0_SRC_MASK_B:
			pwrctrl->reg_sysclk0_src_mask_b = val;
			break;
		case PWR_REG_SYSCLK1_SRC_MASK_B:
			pwrctrl->reg_sysclk1_src_mask_b = val;
			break;
		case PWR_REG_SPM_APSRC_REQ:
			pwrctrl->reg_spm_apsrc_req = val;
			break;
		case PWR_REG_SPM_F26M_REQ:
			pwrctrl->reg_spm_f26m_req = val;
			break;
		case PWR_REG_SPM_INFRA_REQ:
			pwrctrl->reg_spm_infra_req = val;
			break;
		case PWR_REG_SPM_DDREN_REQ:
			pwrctrl->reg_spm_ddren_req = val;
			break;
		case PWR_REG_SPM_VRF18_REQ:
			pwrctrl->reg_spm_vrf18_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL0_REQ:
			pwrctrl->reg_spm_dvfs_level0_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL1_REQ:
			pwrctrl->reg_spm_dvfs_level1_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL2_REQ:
			pwrctrl->reg_spm_dvfs_level2_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL3_REQ:
			pwrctrl->reg_spm_dvfs_level3_req = val;
			break;
		case PWR_REG_SPM_DVFS_LEVEL4_REQ:
			pwrctrl->reg_spm_dvfs_level4_req = val;
			break;
		case PWR_REG_SPM_SSPM_MAILBOX_REQ:
			pwrctrl->reg_spm_sspm_mailbox_req = val;
			break;
		case PWR_REG_SPM_SW_MAILBOX_REQ:
			pwrctrl->reg_spm_sw_mailbox_req = val;
			break;
		case PWR_REG_SPM_CKSEL2_REQ:
			pwrctrl->reg_spm_cksel2_req = val;
			break;
		case PWR_REG_SPM_CKSEL3_REQ:
			pwrctrl->reg_spm_cksel3_req = val;
			break;
		case PWR_REG_CSYSPWREQ_MASK:
			pwrctrl->reg_csyspwreq_mask = val;
			break;
		case PWR_REG_MD_SRCCLKENA_0_INFRA_MASK_B:
			pwrctrl->reg_md_srcclkena_0_infra_mask_b = val;
			break;
		case PWR_REG_MD_SRCCLKENA_1_INFRA_MASK_B:
			pwrctrl->reg_md_srcclkena_1_infra_mask_b = val;
			break;
		case PWR_REG_MD_APSRC_REQ_0_INFRA_MASK_B:
			pwrctrl->reg_md_apsrc_req_0_infra_mask_b = val;
			break;
		case PWR_REG_MD_APSRC_REQ_1_INFRA_MASK_B:
			pwrctrl->reg_md_apsrc_req_1_infra_mask_b = val;
			break;
		case PWR_REG_CONN_SRCCLKENA_INFRA_MASK_B:
			pwrctrl->reg_conn_srcclkena_infra_mask_b = val;
			break;
		case PWR_REG_CONN_INFRA_REQ_MASK_B:
			pwrctrl->reg_conn_infra_req_mask_b = val;
			break;
		case PWR_REG_SSPM_SRCCLKENA_INFRA_MASK_B:
			pwrctrl->reg_sspm_srcclkena_infra_mask_b = val;
			break;
		case PWR_REG_SSPM_INFRA_REQ_MASK_B:
			pwrctrl->reg_sspm_infra_req_mask_b = val;
			break;
		case PWR_REG_SCP_SRCCLKENA_INFRA_MASK_B:
			pwrctrl->reg_scp_srcclkena_infra_mask_b = val;
			break;
		case PWR_REG_SCP_INFRA_REQ_MASK_B:
			pwrctrl->reg_scp_infra_req_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI0_INFRA_MASK_B:
			pwrctrl->reg_srcclkeni0_infra_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI1_INFRA_MASK_B:
			pwrctrl->reg_srcclkeni1_infra_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI2_INFRA_MASK_B:
			pwrctrl->reg_srcclkeni2_infra_mask_b = val;
			break;
		case PWR_REG_CCIF0_MD_EVENT_MASK_B:
			pwrctrl->reg_ccif0_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF0_AP_EVENT_MASK_B:
			pwrctrl->reg_ccif0_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIF1_MD_EVENT_MASK_B:
			pwrctrl->reg_ccif1_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF1_AP_EVENT_MASK_B:
			pwrctrl->reg_ccif1_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIF2_MD_EVENT_MASK_B:
			pwrctrl->reg_ccif2_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF2_AP_EVENT_MASK_B:
			pwrctrl->reg_ccif2_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIF3_MD_EVENT_MASK_B:
			pwrctrl->reg_ccif3_md_event_mask_b = val;
			break;
		case PWR_REG_CCIF3_AP_EVENT_MASK_B:
			pwrctrl->reg_ccif3_ap_event_mask_b = val;
			break;
		case PWR_REG_CCIFMD_MD1_EVENT_MASK_B:
			pwrctrl->reg_ccifmd_md1_event_mask_b = val;
			break;
		case PWR_REG_CCIFMD_MD2_EVENT_MASK_B:
			pwrctrl->reg_ccifmd_md2_event_mask_b = val;
			break;
		case PWR_REG_C2K_PS_RCCIF_WAKE_MASK_B:
			pwrctrl->reg_c2k_ps_rccif_wake_mask_b = val;
			break;
		case PWR_REG_C2K_L1_RCCIF_WAKE_MASK_B:
			pwrctrl->reg_c2k_l1_rccif_wake_mask_b = val;
			break;
		case PWR_REG_PS_C2K_RCCIF_WAKE_MASK_B:
			pwrctrl->reg_ps_c2k_rccif_wake_mask_b = val;
			break;
		case PWR_REG_L1_C2K_RCCIF_WAKE_MASK_B:
			pwrctrl->reg_l1_c2k_rccif_wake_mask_b = val;
			break;
		case PWR_REG_DISP2_REQ_MASK_B:
			pwrctrl->reg_disp2_req_mask_b = val;
			break;
		case PWR_REG_MD_DDR_EN_0_MASK_B:
			pwrctrl->reg_md_ddr_en_0_mask_b = val;
			break;
		case PWR_REG_MD_DDR_EN_1_MASK_B:
			pwrctrl->reg_md_ddr_en_1_mask_b = val;
			break;
		case PWR_REG_CONN_DDR_EN_MASK_B:
			pwrctrl->reg_conn_ddr_en_mask_b = val;
			break;
		case PWR_REG_DISP0_REQ_MASK_B:
			pwrctrl->reg_disp0_req_mask_b = val;
			break;
		case PWR_REG_DISP1_REQ_MASK_B:
			pwrctrl->reg_disp1_req_mask_b = val;
			break;
		case PWR_REG_DISP_OD_REQ_MASK_B:
			pwrctrl->reg_disp_od_req_mask_b = val;
			break;
		case PWR_REG_MFG_REQ_MASK_B:
			pwrctrl->reg_mfg_req_mask_b = val;
			break;
		case PWR_REG_VDEC0_REQ_MASK_B:
			pwrctrl->reg_vdec0_req_mask_b = val;
			break;
		case PWR_REG_GCE_REQ_MASK_B:
			pwrctrl->reg_gce_req_mask_b = val;
			break;
		case PWR_REG_GCE_VRF18_REQ_MASK_B:
			pwrctrl->reg_gce_vrf18_req_mask_b = val;
			break;
		case PWR_REG_LPDMA_REQ_MASK_B:
			pwrctrl->reg_lpdma_req_mask_b = val;
			break;
		case PWR_REG_CONN_SRCCLKENA_CKSEL2_MASK_B:
			pwrctrl->reg_conn_srcclkena_cksel2_mask_b = val;
			break;
		case PWR_REG_SSPM_APSRC_REQ_DDREN_MASK_B:
			pwrctrl->reg_sspm_apsrc_req_ddren_mask_b = val;
			break;
		case PWR_REG_SCP_APSRC_REQ_DDREN_MASK_B:
			pwrctrl->reg_scp_apsrc_req_ddren_mask_b = val;
			break;
		case PWR_REG_MD_VRF18_REQ_0_MASK_B:
			pwrctrl->reg_md_vrf18_req_0_mask_b = val;
			break;
		case PWR_REG_MD_VRF18_REQ_1_MASK_B:
			pwrctrl->reg_md_vrf18_req_1_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL0_MASK_B:
			pwrctrl->reg_next_dvfs_level0_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL1_MASK_B:
			pwrctrl->reg_next_dvfs_level1_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL2_MASK_B:
			pwrctrl->reg_next_dvfs_level2_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL3_MASK_B:
			pwrctrl->reg_next_dvfs_level3_mask_b = val;
			break;
		case PWR_REG_NEXT_DVFS_LEVEL4_MASK_B:
			pwrctrl->reg_next_dvfs_level4_mask_b = val;
			break;
		case PWR_REG_SW2SPM_INT0_MASK_B:
			pwrctrl->reg_sw2spm_int0_mask_b = val;
			break;
		case PWR_REG_SW2SPM_INT1_MASK_B:
			pwrctrl->reg_sw2spm_int1_mask_b = val;
			break;
		case PWR_REG_SW2SPM_INT2_MASK_B:
			pwrctrl->reg_sw2spm_int2_mask_b = val;
			break;
		case PWR_REG_SW2SPM_INT3_MASK_B:
			pwrctrl->reg_sw2spm_int3_mask_b = val;
			break;
		case PWR_REG_SSPM2SPM_INT0_MASK_B:
			pwrctrl->reg_sspm2spm_int0_mask_b = val;
			break;
		case PWR_REG_SSPM2SPM_INT1_MASK_B:
			pwrctrl->reg_sspm2spm_int1_mask_b = val;
			break;
		case PWR_REG_SSPM2SPM_INT2_MASK_B:
			pwrctrl->reg_sspm2spm_int2_mask_b = val;
			break;
		case PWR_REG_SSPM2SPM_INT3_MASK_B:
			pwrctrl->reg_sspm2spm_int3_mask_b = val;
			break;
		case PWR_REG_DQSSOC_REQ_MASK_B:
			pwrctrl->reg_dqssoc_req_mask_b = val;
			break;
		case PWR_REG_MPWFI_OP:
			pwrctrl->reg_mpwfi_op = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV1_4_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv1_4_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV1_3_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv1_3_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV1_2_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv1_2_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV1_1_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv1_1_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV1_0_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv1_0_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV0_4_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv0_4_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV0_3_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv0_3_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV0_2_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv0_2_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV0_1_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv0_1_mask_b = val;
			break;
		case PWR_REG_SPM_RESOURCE_REQ_RSV0_0_MASK_B:
			pwrctrl->reg_spm_resource_req_rsv0_0_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI2_CKSEL3_MASK_B:
			pwrctrl->reg_srcclkeni2_cksel3_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI2_CKSEL2_MASK_B:
			pwrctrl->reg_srcclkeni2_cksel2_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI1_CKSEL3_MASK_B:
			pwrctrl->reg_srcclkeni1_cksel3_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI1_CKSEL2_MASK_B:
			pwrctrl->reg_srcclkeni1_cksel2_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI0_CKSEL3_MASK_B:
			pwrctrl->reg_srcclkeni0_cksel3_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI0_CKSEL2_MASK_B:
			pwrctrl->reg_srcclkeni0_cksel2_mask_b = val;
			break;
		case PWR_REG_MD_DDR_EN_0_DBC_EN:
			pwrctrl->reg_md_ddr_en_0_dbc_en = val;
			break;
		case PWR_REG_MD_DDR_EN_1_DBC_EN:
			pwrctrl->reg_md_ddr_en_1_dbc_en = val;
			break;
		case PWR_REG_CONN_DDR_EN_DBC_EN:
			pwrctrl->reg_conn_ddr_en_dbc_en = val;
			break;
		case PWR_REG_SSPM_MASK_B:
			pwrctrl->reg_sspm_mask_b = val;
			break;
		case PWR_REG_MD_0_MASK_B:
			pwrctrl->reg_md_0_mask_b = val;
			break;
		case PWR_REG_MD_1_MASK_B:
			pwrctrl->reg_md_1_mask_b = val;
			break;
		case PWR_REG_SCP_MASK_B:
			pwrctrl->reg_scp_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI0_MASK_B:
			pwrctrl->reg_srcclkeni0_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI1_MASK_B:
			pwrctrl->reg_srcclkeni1_mask_b = val;
			break;
		case PWR_REG_SRCCLKENI2_MASK_B:
			pwrctrl->reg_srcclkeni2_mask_b = val;
			break;
		case PWR_REG_MD_APSRC_1_SEL:
			pwrctrl->reg_md_apsrc_1_sel = val;
			break;
		case PWR_REG_MD_APSRC_0_SEL:
			pwrctrl->reg_md_apsrc_0_sel = val;
			break;
		case PWR_REG_CONN_MASK_B:
			pwrctrl->reg_conn_mask_b = val;
			break;
		case PWR_REG_CONN_APSRC_SEL:
			pwrctrl->reg_conn_apsrc_sel = val;
			break;
		case PWR_REG_MD_SRCCLKENA_0_VRF18_MASK_B:
			pwrctrl->reg_md_srcclkena_0_vrf18_mask_b = val;
			break;
		case PWR_REG_WAKEUP_EVENT_MASK:
			pwrctrl->reg_wakeup_event_mask = val;
			break;
		case PWR_REG_EXT_WAKEUP_EVENT_MASK:
			pwrctrl->reg_ext_wakeup_event_mask = val;
			break;
		case PWR_MCU0_WFI_EN:
			pwrctrl->mcu0_wfi_en = val;
			break;
		case PWR_MCU1_WFI_EN:
			pwrctrl->mcu1_wfi_en = val;
			break;
		case PWR_MCU2_WFI_EN:
			pwrctrl->mcu2_wfi_en = val;
			break;
		case PWR_MCU3_WFI_EN:
			pwrctrl->mcu3_wfi_en = val;
			break;
		case PWR_MCU4_WFI_EN:
			pwrctrl->mcu4_wfi_en = val;
			break;
		case PWR_MCU5_WFI_EN:
			pwrctrl->mcu5_wfi_en = val;
			break;
		case PWR_MCU6_WFI_EN:
			pwrctrl->mcu6_wfi_en = val;
			break;
		case PWR_MCU7_WFI_EN:
			pwrctrl->mcu7_wfi_en = val;
			break;
		case PWR_MCU8_WFI_EN:
			pwrctrl->mcu8_wfi_en = val;
			break;
		case PWR_MCU9_WFI_EN:
			pwrctrl->mcu9_wfi_en = val;
			break;
		case PWR_MCU10_WFI_EN:
			pwrctrl->mcu10_wfi_en = val;
			break;
		case PWR_MCU11_WFI_EN:
			pwrctrl->mcu11_wfi_en = val;
			break;
		case PWR_MCU12_WFI_EN:
			pwrctrl->mcu12_wfi_en = val;
			break;
		case PWR_MCU13_WFI_EN:
			pwrctrl->mcu13_wfi_en = val;
			break;
		case PWR_MCU14_WFI_EN:
			pwrctrl->mcu14_wfi_en = val;
			break;
		case PWR_MCU15_WFI_EN:
			pwrctrl->mcu15_wfi_en = val;
			break;
		case PWR_MCU16_WFI_EN:
			pwrctrl->mcu16_wfi_en = val;
			break;
		case PWR_MCU17_WFI_EN:
			pwrctrl->mcu17_wfi_en = val;
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
			save_pwr_ctrl(__spm_dpidle.pwrctrl, x2, x3);
			break;
		case SPM_PWR_CTRL_SODI:
			save_pwr_ctrl(__spm_sodi.pwrctrl, x2, x3);
			break;
		case SPM_PWR_CTRL_SODI3:
			save_pwr_ctrl(__spm_sodi.pwrctrl, x2, x3);
			break;
		case SPM_PWR_CTRL_VCOREFS:
			//save_pwr_ctrl(__spm_vcorefs.pwrctrl, x2, x3);
			break;
	}
}

void spm_dcs_s1_setting(__uint64_t x1, __uint64_t x2)
{
#if 0
	__uint32_t enable = x1;
	__uint32_t flags = x2;

	mmio_write_32(DRAMC_DPY_CLK_SW_CON5, mmio_read_32(DRAMC_DPY_CLK_SW_CON5) | flags);
	while ((mmio_read_32(DRAMC_DPY_CLK_SW_CON5) & (0xf << 4)) != (flags << 4))
		;

	if (!!enable)
		mmio_write_32(SPM_SPARE_CON_SET, flags << 24);
	else
		mmio_write_32(SPM_SPARE_CON_CLR, flags << 24);

	mmio_write_32(DRAMC_DPY_CLK_SW_CON5, mmio_read_32(DRAMC_DPY_CLK_SW_CON5) & ~flags);
	while ((mmio_read_32(DRAMC_DPY_CLK_SW_CON5) & (0xf << 4)) != 0)
		;
#endif
}

void spm_dummy_read(__uint64_t x1, __uint64_t x2)
{
	__uint32_t rank0_addr = x1;
	__uint32_t rank1_addr = x2;

	mmio_write_32(SPM_PASR_DPD_2, rank0_addr & 0xffffffff);
	mmio_write_32(SPM_PASR_DPD_3, rank1_addr & 0xffffffff);
	if ((x2 >> 32) & 0x1)
		mmio_write_32(SPM_RSV_CON2, mmio_read_32(SPM_RSV_CON2) | SPM_FLAG_MSB_FOR_DUMMY_READ_ADDR);
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

	mmio_write_32(DDR_EN_DBC_LEN, 0x82 | (0x82 << 10) | (0x82 << 20));
	mmio_write_32(SRCCLKENI2PWRAP_MASK_B, 0x11);
	mmio_write_32(SPM_PC_TRACE_CON, 0x8 | (1 << 31));
}

void spm_boot_init(void)
{
	spm_lock_init();
	spm_register_init();
	spm_load_pcm_firmware();
}

void spm_legacy_sleep_wfi(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	unsigned long mpidr;

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/*
	 * Enter standby state. dsb is good practice before using wfi
	 * to enter low power states.
	 */
	isb();
	dsb();

	/* Disable coherency if this cluster is to be turned off */

	plat_cci_disable();
	disable_scu(mpidr);

	wfi();

	/* Enable coherency if this cluster was off */
	enable_scu(mpidr);
	plat_cci_enable();
}
