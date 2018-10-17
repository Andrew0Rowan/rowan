#include <arch_helpers.h>
#include <arch.h>
#include <l2c.h>
#include <mmio.h>
#include <plat_private.h>
#include <platform_def.h>
#include <spinlock.h>

void config_L2_size()
{
	unsigned int cache_cfg0;

	/* MT6757: mp0 L2$ 512KB;*/
	cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) & (0xF << L2C_SIZE_CFG_OFF);
	cache_cfg0 = (cache_cfg0 << 1) | (0x1 << L2C_SIZE_CFG_OFF);
	cache_cfg0 = cache_cfg0 & ~(0x3 << 10);
	cache_cfg0 = (mmio_read_32(MP0_CA7L_CACHE_CONFIG) & ~(0xF << L2C_SIZE_CFG_OFF)) | cache_cfg0;
	mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
	cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) & ~(0x1 << L2C_SHARE_ENABLE);
	mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);

	/* mp1 L2$ 512KB */
	cache_cfg0 = mmio_read_32(MP1_CA7L_CACHE_CONFIG) & (0xF << L2C_SIZE_CFG_OFF);
        cache_cfg0 = (cache_cfg0 << 1) | (0x1 << L2C_SIZE_CFG_OFF);
	cache_cfg0 = cache_cfg0 & ~(0x3 << 10);
	cache_cfg0 = (mmio_read_32(MP1_CA7L_CACHE_CONFIG) & ~(0xF << L2C_SIZE_CFG_OFF)) | cache_cfg0;
	mmio_write_32(MP1_CA7L_CACHE_CONFIG, cache_cfg0);
	cache_cfg0 = mmio_read_32(MP1_CA7L_CACHE_CONFIG) & ~(0x1 << L2C_SHARE_ENABLE);
	mmio_write_32(MP1_CA7L_CACHE_CONFIG, cache_cfg0);
}
