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
#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <mcucfg.h>
#include <mmio.h>
#include <assert.h>
#include <debug.h>
#if 0
/*
* MCSI-B SW Workaround
*/
int mcsib_sw_workaround(void)
{
        unsigned int CntRead1, CntRead2,CntRead3;
        *(volatile unsigned int *)(0x0C507100) = 0x800001;
        CntRead1 = *(volatile unsigned int *)(0x0C507104);

        *(volatile unsigned int *)(0x0C507100) = 0x900001;
        CntRead2 = *(volatile unsigned int *)(0x0C507104);

        *(volatile unsigned int *)(0x0C507100) = 0xa00001;
        CntRead3 = *(volatile unsigned int *)(0x0C507104);

        if (((CntRead1 < 2) && (CntRead2 < 2) && (CntRead3 < 2)) || (CntRead1 < CntRead2) || (CntRead2 < CntRead3))
                return 0;
        else
                return 1;
}
#endif
void disable_scu(unsigned long mpidr)
{
	uintptr_t axi_config = 0;

	INFO("[ATF]: %s()\n", __FUNCTION__);

/*	mcsib_sw_workaround();*/
	switch (mpidr & MPIDR_CLUSTER_MASK) {
		case 0x000:
			axi_config = MP0_AXI_CONFIG;
			mmio_setbits_32((uintptr_t)axi_config, MP0_ACINACTM);
			break;
		case 0x100:
			axi_config = MP1_AXI_CONFIG;
			mmio_setbits_32((uintptr_t)axi_config, MP1_ACINACTM);
			break;
		case 0x200:
			axi_config = MP2_AXI_CONFIG;
			mmio_setbits_32((uintptr_t)axi_config, MP2_ACINACTM);
			break;
		default:
			ERROR("[ATF]: disable_scu: mpidr not exists\n");
			assert(0);
	}
}

void enable_scu(unsigned long mpidr)
{
	uintptr_t axi_config = 0;

	INFO("[ATF]: %s()\n", __FUNCTION__);

	switch (mpidr & MPIDR_CLUSTER_MASK) {
		case 0x000:
			axi_config = MP0_AXI_CONFIG;
			mmio_clrbits_32((uintptr_t)axi_config, MP0_ACINACTM);
			break;
		case 0x100:
			axi_config = MP1_AXI_CONFIG;
			mmio_clrbits_32((uintptr_t)axi_config, MP1_ACINACTM);
			break;
		case 0x200:
			axi_config = MP2_AXI_CONFIG;
			mmio_write_32(axi_config, mmio_read_32(axi_config)& ~(1<<0));
			break;
		default:
			ERROR("[ATF]: enable_scu: mpidr not exists\n");
			assert(0);
	}
}
