#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <lpdma_drv.h>
#include <interrupt_mgmt.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>

/* return current DCS mode
 * 0 : Normal mode
 * 1 : Lowpower mode
 */

uint32_t sip_lpdma_get_mode(void)
{
    unsigned int val = (mmio_read_32(LPDMA_CONA) & (1 << 6)) >> 6;
    return val;
}

static int is_lpdma_reg_write_forbidden(unsigned int offset)
{
	if ((offset >= LPDMA_START) && (offset <= LPDMA_END))
		return 0;
	else
		return 1;
}

uint64_t sip_lpdma_write(unsigned int offset, unsigned int reg_value)
{
	unsigned int addr;
	if(is_lpdma_reg_write_forbidden(offset))
		return SIP_SVC_E_INVALID_Range;

	addr = LPDMA_BASE + offset;
	mmio_write_32(addr, reg_value);
	dsb();
	return SIP_SVC_E_SUCCESS;
}

uint32_t sip_lpdma_read(unsigned int offset)
{
	unsigned int addr;

	if(is_lpdma_reg_write_forbidden(offset))
		return SIP_SVC_E_INVALID_Range;

	addr = LPDMA_BASE + offset;
	return mmio_read_32(addr);
}
