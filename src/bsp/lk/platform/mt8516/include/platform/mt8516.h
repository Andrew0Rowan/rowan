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
#pragma once
#include <debug.h>

/* L2C */
#define MEMORY_BASE_PHYS     (0x200000)
#define MEMORY_BASE_VIRT     (KERNEL_BASE)
#define MEMORY_APERTURE_SIZE (0x40000UL)

/* Internal SRAM */
#define SRAM_BASE_PHYS (0x100000)
#define SRAM_BASE_SIZE (0x10000L)
#define SRAM_BASE_VIRT (MEMORY_BASE_VIRT + 0x100000UL)


/* map all of 0-1GB into kernel space in one shot */
#define PERIPHERAL_BASE_PHYS (0x10000000)
#define PERIPHERAL_BASE_SIZE (0x0d000000UL)
#define VER_BASE            (0x08000000)

#if WITH_KERNEL_VM
#define PERIPHERAL_BASE_VIRT (SRAM_BASE_VIRT + 0x300000UL)
#else
#define PERIPHERAL_BASE_VIRT PERIPHERAL_BASE_PHYS
#endif

#define DRAM_BASE_PHY  (0x40000000U)
#if WITH_KERNEL_VM
#define DRAM_BASE_VIRT (PERIPHERAL_BASE_VIRT + PERIPHERAL_BASE_SIZE)
#else
#define DRAM_BASE_VIRT DRAM_BASE_PHY
#endif

/* individual peripherals in this mapping */
#define CPUPRIV_BASE_VIRT   (PERIPHERAL_BASE_VIRT + 0x08000000)
#define CPUPRIV_BASE_PHYS   (PERIPHERAL_BASE_PHYS + 0x08000000)
#define CPUPRIV_SIZE        (0x00020000)

/* interrupts */
#define ARM_GENERIC_TIMER_VIRTUAL_INT 27
#define ARM_GENERIC_TIMER_PHYSICAL_INT 30

#define MAX_INT 221

/*AP HW */
#define APSW_VER            (VER_BASE + 0x0C)
#define CKSYS_BASE        	(PERIPHERAL_BASE_VIRT + 0x00000000)

/* mt_mempll_init and dram calibration */
#define EMI_BASE            (PERIPHERAL_BASE_VIRT + 0x00205000)
#define SPM_BASE            (PERIPHERAL_BASE_VIRT + 0x6000)
#define SRAMROM_BASE        (PERIPHERAL_BASE_VIRT + 0x202000)
