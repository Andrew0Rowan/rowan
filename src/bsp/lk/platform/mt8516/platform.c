/*
 * Copyright (c) 2016 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <platform.h>
#include <arch/arm64/mmu.h>
#include <arch/ops.h>
#include <assert.h>
#include <debug.h>
#include <dev/interrupt/arm_gic.h>
#include <dev/timer/arm_generic.h>
#include <dev/uart.h>
#include <err.h>
#include <kernel/vm.h>
#include <platform/mt8516.h>
#include <platform/emi.h>
#include <platform/memory.h>
#include <platform/mtk_timer.h>
#include <platform/mtk_wdt.h>
#include <platform/pll.h>
#if WITH_MTK_PMIC_WRAP_AND_PMIC
#include <platform/pmic_wrap.h>
#endif

u32 g_ddr_reserve_enable = 0;
u32 g_ddr_reserve_success = 0;

#if WITH_KERNEL_VM
#define L2C_MAPPING_IDX 0
#define PERIPHERAL_MAPPING_IDX 1
#define SRAM_MAPPING_IDX 2

/* initial memory mappings. parsed by start.S */
struct mmu_initial_mapping mmu_initial_mappings[] = {
    {
        .phys = MEMORY_BASE_PHYS,
        .virt = MEMORY_BASE_VIRT,
        .size = MEMORY_APERTURE_SIZE,
        .flags = 0,
        .name = "l2c"
    },
    {
        .phys = PERIPHERAL_BASE_PHYS,
        .virt = PERIPHERAL_BASE_VIRT,
        .size = PERIPHERAL_BASE_SIZE,
        .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
        .name = "peripherals"
    },
    /* reserved for internal sram */
    { 0 },
    /* null entry to terminate the list */
    { 0 }
};

static pmm_arena_t arena = {
    .name = "sdram",
    .base = SRAM_BASE_PHYS,
    .size = SRAM_BASE_SIZE,
    .flags = PMM_ARENA_FLAG_KMAP,
};

inline void dcsw_op_all(void)
{
    _dcsw_op_all();
}

/* only disable el1 dcache */
void dcache_disable(void)
{
    uint32_t sctlr;

    asm volatile("mrs %0, sctlr_el1" : "=r" (sctlr) : : "cc");

    /* if cache isn't enabled no need to disable */
    if (!(sctlr & (1 << 2)))
        return;

    asm volatile("msr sctlr_el1, %0" : : "r" (sctlr & ~(1 << 2)) : "cc");
    /* clean and invalid dcache */
    dcsw_op_all();
    /* invalid el1 tlb */
    _tlbivmalle1();
}

/* only enable el1 dcache */
static void dcache_enable(void)
{
    uint32_t sctlr;

    asm volatile("mrs %0, sctlr_el1" : "=r" (sctlr) : : "cc");
    asm volatile("msr sctlr_el1, %0" : : "r" (sctlr | (1 << 2)) : "cc");
    asm volatile("isb");
}

void *dram_map(paddr_t pa)
{
    paddr_t dram_phy = DRAM_BASE_PHY;
    uint32_t dram_sz = get_dram_size();

    if (pa >= dram_phy && pa <= (dram_phy + dram_sz - 1)) {
        return (void *)(DRAM_BASE_VIRT + (pa - dram_phy));
    }

    return NULL;
}
#endif /* WITH_KERNEL_VM */

void platform_early_init(void)
{
    uart_init_early();

    /* initialize the interrupt controller */
    arm_gic_init();

    /* initialize the timer block */
    //mtk_timer_init();

    mtk_wdt_init();
    arm_generic_timer_init(ARM_GENERIC_TIMER_PHYSICAL_INT, 13000000);

#if WITH_KERNEL_VM
    dcache_disable();
#endif
    mt_pll_init();

    /*
      * mt_init_mempll() and mt_pll_post_init() should be invoked
      * after pmic_ini
      */
    mt_pll_post_init();

#if WITH_MTK_PMIC_WRAP_AND_PMIC
    pwrap_init();
#endif
    /* dram calibration, mapping and test */

    /* check DDR-reserve mode */
    check_ddr_reserve_status();

    mt_mem_init();
	
#if WITH_KERNEL_VM
    dcache_enable();
    /* mapping internel sram to cacheable memory */
    arch_mmu_map(SRAM_BASE_VIRT, SRAM_BASE_PHYS,  SRAM_BASE_SIZE >> PAGE_SIZE_SHIFT, 0);
    /* add intrenal sram to mmu_initial_mappings for heap */
    mmu_initial_mappings[SRAM_MAPPING_IDX].phys = SRAM_BASE_PHYS;
    mmu_initial_mappings[SRAM_MAPPING_IDX].virt = SRAM_BASE_VIRT;
    mmu_initial_mappings[SRAM_MAPPING_IDX].size = SRAM_BASE_SIZE;
    mmu_initial_mappings[SRAM_MAPPING_IDX].flags = 0;
    mmu_initial_mappings[SRAM_MAPPING_IDX].name = "sram";

    pmm_add_arena(&arena);
#endif
}

void platform_init(void)
{

}

typedef void (*jump_func_type)(uint64_t bl31_addr, uint64_t bl33_addr, uint64_t arg1) __attribute__ ((__noreturn__));

void el3_mtk_sip(uint32_t smc_fid, uint64_t bl31_addr, uint64_t bl33_addr)
{
    jump_func_type jump_func;
    jump_func = (jump_func_type)bl31_addr;
    (*jump_func)(bl31_addr, bl33_addr, 0);
}
