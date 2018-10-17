#include <arch.h>
#include <platform_def.h>
#include <mmio.h>

void disable_scu(unsigned long mpidr) {
	if (mpidr & MPIDR_CLUSTER_MASK)
		mmio_write_32(MP1_AXI_CONFIG, mmio_read_32(MP1_AXI_CONFIG) | MP1_ACINACTM);
	else
		mmio_write_32(MP0_AXI_CONFIG, mmio_read_32(MP0_AXI_CONFIG) | MP0_ACINACTM);
}

void enable_scu(unsigned long mpidr) {
	if (mpidr & MPIDR_CLUSTER_MASK)
		mmio_write_32(MP1_AXI_CONFIG, mmio_read_32(MP1_AXI_CONFIG) & ~MP1_ACINACTM);
	else
		mmio_write_32(MP0_AXI_CONFIG, mmio_read_32(MP0_AXI_CONFIG) & ~MP0_ACINACTM);
}
