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

#include <platform/mt8516.h>

/* I/O mapping */
#define IO_PHYS             PERIPHERAL_BASE_VIRT
#define IO_SIZE             PERIPHERAL_BASE_SIZE

/* IO register definitions */
#define EMI_BASE            (IO_PHYS + 0x00205000)
#define GPIO_BASE           (IO_PHYS + 0x00005000)
#define GIC_CPU_BASE        (IO_PHYS + 0x00222000)
#define GIC_DIST_BASE       (IO_PHYS + 0x00221000)
#define MCUSYS_CFGREG_BASE  (IO_PHYS + 0x00200000)
#define INT_POL_CTL0        (MCUSYS_CFGREG_BASE + 0x620)

#define UART1_BASE          (IO_PHYS + 0x01005000)
#define UART2_BASE          (IO_PHYS + 0x01006000)
#define UART3_BASE          (IO_PHYS + 0x01007000)
#define UART4_BASE          (IO_PHYS + 0x01007500)

#define NFI_BASE            (IO_PHYS + 0x01001000)
#define NFIECC_BASE         (IO_PHYS + 0x01002000)

#define MSDC0_BASE          (IO_PHYS + 0x01120000)
#define MSDC_CLK_GATE_CTRL  (IO_PHYS + 0x54)
#define MSDC_CLK_UNGATE_CTRL (IO_PHYS + 0x84)
#define MSDC0_CLK_SW_CG     (0x1 << 17)
#define MSDC_CLK_MUX_REG    (IO_PHYS + 0x0)
#define MSDC0_CLK_MUX_SEL	    (0x7 << 11)
#define MSDC_GATE_INFRA_SW_CG    (IO_PHYS + 0x9C)
#define MSDC_UNGATE_INFRA_SW_CG    (IO_PHYS + 0x6C)
#define MSDC0_INFRA_SW_CG   (0x1 << 28)

#define USB0_BASE           (IO_PHYS + 0x01100000)
#define USBSIF_BASE         (IO_PHYS + 0x01110000)
#define USB_BASE            (USB0_BASE)
#define INFRACFG_AO_BASE    (IO_PHYS + 0x00001000)
#define RGU_BASE            (IO_PHYS + 0x7000)

#define TOPRGU_BASE 				(IO_PHYS + 0x10007000)
