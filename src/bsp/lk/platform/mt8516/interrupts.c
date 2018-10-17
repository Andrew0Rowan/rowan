/*
 * Copyright (c) 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#if ARCH_ARM
#include <arch/arm.h>
#endif
#if ARCH_ARM64
#include <arch/arm64.h>
#endif
#include <reg.h>
#include <debug.h>
#include <kernel/thread.h>
#include <platform/mt_irq.h>
#include <platform/interrupts.h>
#include <platform/mt_reg_base.h>

#define write_r(a, v) writel(v, a) /* need to fix it */

/* set for mt_gic */
void mt_irq_set_polarity(unsigned int irq, unsigned int polarity)
{
    unsigned int offset;
    unsigned int reg_index;
    unsigned int value;

    /* peripheral device's IRQ line is using GIC's SPI, and line ID >= GIC_PRIVATE_SIGNALS */
    if (irq < GIC_PRIVATE_SIGNALS) {
        dprintf(SPEW, "The Interrupt ID < 32, please check!\n");
        return;
    }

    offset = (irq - GIC_PRIVATE_SIGNALS) & 0x1F;
    reg_index = (irq - GIC_PRIVATE_SIGNALS) >> 5;
    if (polarity == 0) {
        value = readl(INT_POL_CTL0 + (reg_index * 4));
        value |= (1 << offset); /* always invert the incoming IRQ's polarity */
        write_r((INT_POL_CTL0 + (reg_index * 4)), value);
    } else {
        value = readl(INT_POL_CTL0 + (reg_index * 4));
        value &= ~(0x1 << offset);
        write_r(INT_POL_CTL0 + (reg_index * 4), value);

    }
}

/* set for arm gic */
void mt_irq_set_sens(unsigned int irq, unsigned int sens)
{
    unsigned int config;

    if (sens == EDGE_SENSITIVE) {
        config = readl(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4);
        config |= (0x2 << (irq % 16) * 2);
        write_r(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4, config);
    } else {
        config = readl(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4);
        config &= ~(0x2 << (irq % 16) * 2);
        write_r( GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4, config);
    }
    DSB;
}
