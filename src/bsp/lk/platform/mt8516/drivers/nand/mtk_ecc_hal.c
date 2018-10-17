/*
 * Copyright (c) 2017 MediaTek Inc.
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
#include <arch/ops.h>
#include <sys/types.h>
#include <reg.h>
#include <malloc.h>
#include <errno.h>
#include <kernel/mutex.h>
#include <kernel/event.h>
#include <kernel/vm.h>
#include <platform/mt_irq.h>
#include <platform/interrupts.h>
#include <platform/mt_reg_base.h>
#include <string.h>
#include "mtk_nand_common.h"
#include "mtk_ecc_hal.h"

#define ECC_IDLE_MASK		NAND_BIT(0)
#define ECC_IRQ_EN		NAND_BIT(0)
#define ECC_OP_ENABLE		(1)
#define ECC_OP_DISABLE		(0)

#define ECC_ENCCON		(0x00)
#define ECC_ENCCNFG		(0x04)
#define		ECC_CNFG_4BIT		(0)
#define		ECC_CNFG_6BIT		(1)
#define		ECC_CNFG_8BIT		(2)
#define		ECC_CNFG_10BIT		(3)
#define		ECC_CNFG_12BIT		(4)
#define		ECC_CNFG_14BIT		(5)
#define		ECC_CNFG_16BIT		(6)
#define		ECC_CNFG_18BIT		(7)
#define		ECC_CNFG_20BIT		(8)
#define		ECC_CNFG_22BIT		(9)
#define		ECC_CNFG_24BIT		(0xa)
#define		ECC_CNFG_28BIT		(0xb)
#define		ECC_CNFG_32BIT		(0xc)
#define		ECC_CNFG_36BIT		(0xd)
#define		ECC_CNFG_40BIT		(0xe)
#define		ECC_CNFG_44BIT		(0xf)
#define		ECC_CNFG_48BIT		(0x10)
#define		ECC_CNFG_52BIT		(0x11)
#define		ECC_CNFG_56BIT		(0x12)
#define		ECC_CNFG_60BIT		(0x13)
#define		ECC_CNFG_68BIT		(0x14)
#define		ECC_CNFG_72BIT		(0x15)
#define		ECC_CNFG_80BIT		(0x16)
#define		ECC_MODE_SHIFT		(5)
#define		ECC_MS_SHIFT		(16)
#define ECC_ENCDIADDR		(0x08)
#define ECC_ENCIDLE		(0x0c)
#define	ECC_ENCSTA		(0x7c)
#define		ENC_IDLE	NAND_BIT(0)
#define ECC_ENCIRQ_EN		(0x80)
#define ECC_ENCIRQ_STA		(0x84)
#define		PG_IRQ_SEL		NAND_BIT(1)
#define	ECC_PIO_DIRDY	(0x90)
#define		PIO_DI_RDY		(0x01)
#define	ECC_PIO_DI		(0x94)
#define ECC_DECCON		(0x100)
#define ECC_DECCNFG		(0x104)
#define		DEC_EMPTY_EN		NAND_BIT(31)
#define		DEC_CON_SHIFT	(12)
#define ECC_DECDIADDR		(0x108)
#define ECC_DECIDLE		(0x10c)
#define ECC_DECENUM(x)		(0x114 + (x) * sizeof(u32))
#define		ERR_MASK		(0x7f)
#define ECC_DECDONE		(0x124)
#define ECC_DECIRQ_EN		(0x200)
#define ECC_DECIRQ_STA		(0x204)
#define ECC_DECFSM		(0x208)
#define		FSM_MASK	(0x3f3fff0f)
#define		FSM_IDLE	(0x01011101)
#define ECC_BYPASS		(0x20c)
#define		ECC_BYPASS_EN		NAND_BIT(0)
#define ECC_ENCPAR(x)		(0x10 + (x) * sizeof(u32))
#define ECC_ENCPAR_EXT(x)	(0x300 + (x) * sizeof(u32))

#define	ECC_DECEL(x)		(0x128 + (x) * sizeof(u32))
#define ECC_DECEL_EXT(x)	(0x400 + (x) * sizeof(u32))

#define ECC_TIMEOUT		(500000)

#define ECC_IDLE_REG(op)	((op) == ECC_ENCODE ? ECC_ENCIDLE : ECC_DECIDLE)
#define ECC_CTL_REG(op)		((op) == ECC_ENCODE ? ECC_ENCCON : ECC_DECCON)
#define ECC_IRQ_REG(op)		((op) == ECC_ENCODE ? \
					ECC_ENCIRQ_EN : ECC_DECIRQ_EN)

#define writew(v, a) (*REG16(a) = (v))
#define readw(a) (*REG16(a))

struct mtk_ecc {
	mutex_t lock;
	event_t irq_event;

	u64 regs;

	u32 sectors;
};

static inline void mtk_ecc_wait_ioready(struct mtk_ecc *ecc)
{
	if (!check_with_timeout((readl(ecc->regs +  ECC_PIO_DIRDY) & PIO_DI_RDY), ECC_TIMEOUT))
		dprintf(CRITICAL, "ecc io not ready\n");
}

static void mtk_ecc_runtime_config(struct mtk_ecc *ecc, struct mtk_ecc_config *config)
{
	u32 ecc_bit = ECC_CNFG_4BIT, dec_sz, enc_sz;
	u32 reg;

	switch (config->strength) {
	case 4:
		ecc_bit = ECC_CNFG_4BIT;
		break;
	case 6:
		ecc_bit = ECC_CNFG_6BIT;
		break;
	case 8:
		ecc_bit = ECC_CNFG_8BIT;
		break;
	case 10:
		ecc_bit = ECC_CNFG_10BIT;
		break;
	case 12:
		ecc_bit = ECC_CNFG_12BIT;
		break;
	case 14:
		ecc_bit = ECC_CNFG_14BIT;
		break;
	case 16:
		ecc_bit = ECC_CNFG_16BIT;
		break;
	case 18:
		ecc_bit = ECC_CNFG_18BIT;
		break;
	case 20:
		ecc_bit = ECC_CNFG_20BIT;
		break;
	case 22:
		ecc_bit = ECC_CNFG_22BIT;
		break;
	case 24:
		ecc_bit = ECC_CNFG_24BIT;
		break;
	case 28:
		ecc_bit = ECC_CNFG_28BIT;
		break;
	case 32:
		ecc_bit = ECC_CNFG_32BIT;
		break;
	case 36:
		ecc_bit = ECC_CNFG_36BIT;
		break;
	case 40:
		ecc_bit = ECC_CNFG_40BIT;
		break;
	case 44:
		ecc_bit = ECC_CNFG_44BIT;
		break;
	case 48:
		ecc_bit = ECC_CNFG_48BIT;
		break;
	case 52:
		ecc_bit = ECC_CNFG_52BIT;
		break;
	case 56:
		ecc_bit = ECC_CNFG_56BIT;
		break;
	case 60:
		ecc_bit = ECC_CNFG_60BIT;
		break;
	case 68:
		ecc_bit = ECC_CNFG_68BIT;
		break;
	case 72:
		ecc_bit = ECC_CNFG_72BIT;
		break;
	case 80:
		ecc_bit = ECC_CNFG_80BIT;
		break;
	default:
		dprintf(CRITICAL, "invalid strength %d, default to 4 bits\n",
			config->strength);
		break;
	}

	if (config->op == ECC_ENCODE) {
		/* configure ECC encoder (in bits) */
		enc_sz = config->len << 3;

		reg = ecc_bit | (config->mode << ECC_MODE_SHIFT);
		reg |= (enc_sz << ECC_MS_SHIFT);
		writel(reg, ecc->regs + ECC_ENCCNFG);

		if (config->mode == ECC_DMA_MODE) {
			if (config->addr & 0x3)
				dprintf(CRITICAL, "ecc encode address is not 4B aligned !!\n", config->addr);
			writel(config->addr, ecc->regs + ECC_ENCDIADDR);
		}

	} else {
		/* configure ECC decoder (in bits) */
		dec_sz = (config->len << 3) +
					config->strength * ECC_PARITY_BITS;

		reg = ecc_bit | (config->mode << ECC_MODE_SHIFT);
		reg |= (dec_sz << ECC_MS_SHIFT) | (config->deccon << DEC_CON_SHIFT);
		reg |= DEC_EMPTY_EN;
		writel(reg, ecc->regs + ECC_DECCNFG);

		if (config->mode == ECC_DMA_MODE) {
			if (config->addr & 0x3)
				dprintf(CRITICAL, "ecc decode address is not 4B aligned !!\n", config->addr);
			writel(config->addr, ecc->regs + ECC_DECDIADDR);
		}

		if (config->sectors)
			ecc->sectors = 1 << (config->sectors - 1);
	}
}

static inline void mtk_ecc_wait_idle(struct mtk_ecc *ecc,
				     enum mtk_ecc_operation op)
{
	if (!check_with_timeout(readl(ecc->regs + ECC_IDLE_REG(op)) & ECC_IDLE_MASK, ECC_TIMEOUT))
		dprintf(CRITICAL, "%s NOT idle\n", op == ECC_ENCODE ? "encoder" : "decoder");
}

static int mtk_ecc_irq_wait(struct mtk_ecc *ecc, lk_time_t timeout)
{
	int ret;

	ret = event_wait_timeout(&ecc->irq_event, timeout);
	if (ret != 0) {
		dprintf(CRITICAL, "[%s]: failed to get event\n",
			__func__);
		return ret;
	}

	return 0;
}

static enum handler_return mtk_ecc_interrupt_handler(void *arg)
{
	struct mtk_ecc *ecc = arg;
	enum mtk_ecc_operation op;
	u32 dec, enc;

	dec = readw(ecc->regs + ECC_DECIRQ_STA) & ECC_IRQ_EN;
	if (dec) {
		op = ECC_DECODE;
		dec = readw(ecc->regs + ECC_DECDONE);
		if (dec & ecc->sectors) {
			ecc->sectors = 0;
			event_signal(&ecc->irq_event, false);
		} else {
			return INT_NO_RESCHEDULE;
		}
	} else {
		enc = readl(ecc->regs + ECC_ENCIRQ_STA) & ECC_IRQ_EN;
		if (enc) {
			op = ECC_ENCODE;
			event_signal(&ecc->irq_event, false);
		} else {
			return INT_NO_RESCHEDULE;
		}
	}

	writel(0, ecc->regs + ECC_IRQ_REG(op));

	return INT_RESCHEDULE;
}

static int mtk_ecc_request_irq(struct mtk_ecc *ecc)
{
	mt_irq_set_sens(NFIECC_IRQ_BIT_ID, LEVEL_SENSITIVE);
	mt_irq_set_polarity(NFIECC_IRQ_BIT_ID, MT65xx_POLARITY_LOW);
	event_init(&ecc->irq_event, false, EVENT_FLAG_AUTOUNSIGNAL);
	register_int_handler(NFIECC_IRQ_BIT_ID, mtk_ecc_interrupt_handler, ecc);
	unmask_interrupt(NFIECC_IRQ_BIT_ID);

	return 0;
}

int mtk_ecc_hw_init(struct mtk_ecc **ext_ecc)
{
	struct mtk_ecc *ecc;
	u32 reg;

	ecc = (struct mtk_ecc *)malloc(sizeof(*ecc));
	if (!ecc)
		return -ENOMEM;
	memset(ecc, 0, sizeof(*ecc));

	*ext_ecc = ecc;

	ecc->regs = NFIECC_BASE;

	mtk_ecc_wait_idle(ecc, ECC_ENCODE);
	writew(ECC_OP_DISABLE, ecc->regs + ECC_ENCCON);

	mtk_ecc_wait_idle(ecc, ECC_DECODE);
	writel(ECC_OP_DISABLE, ecc->regs + ECC_DECCON);

	mutex_init(&ecc->lock);

	/* register interrupt handler */
	mtk_ecc_request_irq(ecc);

	/* disable ecc bypass */
	reg = readl(ecc->regs + ECC_BYPASS);
	reg &= ~ECC_BYPASS_EN;
	writel(reg, ecc->regs + ECC_BYPASS);
	return 0;
}


int mtk_ecc_enable(struct mtk_ecc *ecc, struct mtk_ecc_config *config, int polling)
{
	enum mtk_ecc_operation op = config->op;
	int ret;

	mutex_acquire(&ecc->lock);

	mtk_ecc_wait_idle(ecc, op);
	mtk_ecc_runtime_config(ecc, config);
	writew(ECC_OP_ENABLE, ecc->regs + ECC_CTL_REG(op));

	if (!polling) {
		writew(ECC_IRQ_EN, ecc->regs + ECC_IRQ_REG(op));
	}

	return 0;
}

void mtk_ecc_disable(struct mtk_ecc *ecc)
{
	enum mtk_ecc_operation op = ECC_ENCODE;

	/* find out the running operation */
	if (readw(ecc->regs + ECC_CTL_REG(op)) != ECC_OP_ENABLE)
		op = ECC_DECODE;

	/* disable it */
	mtk_ecc_wait_idle(ecc, op);
	writew(0, ecc->regs + ECC_IRQ_REG(op));
	writew(ECC_OP_DISABLE, ecc->regs + ECC_CTL_REG(op));

	mutex_release(&ecc->lock);
}

void mtk_ecc_get_stats(struct mtk_ecc *ecc, struct mtk_ecc_stats *stats,
		       int sectors)
{
	u32 offset, i, err;
	u32 bitflips = 0;

	stats->corrected = 0;
	stats->failed = 0;

	for (i = 0; i < sectors; i++) {
		offset = (i >> 2);
		err = readl(ecc->regs + ECC_DECENUM(offset));
		err = err >> ((i % 4) * 8);
		err &= ERR_MASK;
		if (err == ERR_MASK) {
			/* uncorrectable errors */
			stats->failed++;
			dprintf(INFO, "sector %d is uncorrect\n", i);
			continue;
		}

		stats->corrected += err;
		bitflips = max(bitflips, err);
	}

	stats->bitflips = bitflips;
}

int mtk_ecc_wait_done(struct mtk_ecc *ecc, enum mtk_ecc_operation op, int polling)
{
	int ret = 0;

	if (!polling) {
		ret = mtk_ecc_irq_wait(ecc, ECC_TIMEOUT);
		if (!ret)
			dprintf(CRITICAL, "mtk_ecc_wait_done timeout\n");
				return -ETIMEDOUT;
	} else {
		if (op == ECC_ENCODE) {
			if (!check_with_timeout((readl(ecc->regs + ECC_ENCSTA) & ENC_IDLE), ECC_TIMEOUT)) {
				dprintf(CRITICAL, "encoder timeout\n");
				return -ETIMEDOUT;
			}
		} else {
			if (!check_with_timeout((readw(ecc->regs + ECC_DECDONE) & ecc->sectors), ECC_TIMEOUT)) {
				dprintf(CRITICAL, "decoder timeout\n");
				return -ETIMEDOUT;
			}
		}
	}

	return 0;
}

int mtk_ecc_wait_decode_fsm_idle(struct mtk_ecc *ecc)
{
	/* decode done does not stands for ecc all work done.
	 * we need check syn, bma, chien, autoc all idle.
	 * just check it when ECC_DECCNFG[13:12] is 3, which means auto correct.*/
	if (!check_with_timeout(((readl(ecc->regs + ECC_DECFSM) & FSM_MASK) == FSM_IDLE), ECC_TIMEOUT)) {
		dprintf(CRITICAL, "decode fsm(0x%x) is not idle\n", readl(ecc->regs + ECC_DECFSM));
		return -ETIMEDOUT;
	}

	return 0;
}

int mtk_ecc_encode(struct mtk_ecc *ecc, struct mtk_ecc_config *config,
			u8 *data, u32 bytes, int polling)
{
	u32 addr = (u32)data;
	u8 *p;
	u8 *buf = data;
	u32 len, i, val = 0;
	int ret = 0;

	/* encoder memory address should be 4B aligned */
	if ((config->mode == ECC_DMA_MODE) && (addr & 0x3)) {
		buf = (u8 *)NAND_DRAM_BUF_ECCDE_ADDR;//memalign(4, bytes);
		if (!buf)
			return -ENOMEM;
		memcpy(buf, data, bytes);
	}
#ifdef WITH_KERNEL_VM
	addr = (u32)kvaddr_to_paddr(buf);
#else
	addr = (u32)buf;
#endif
	if (config->mode == ECC_DMA_MODE)
		arch_clean_cache_range((addr_t)buf, (size_t)bytes);
	config->op = ECC_ENCODE;
	config->addr = addr;
	config->len = bytes;
	ret = mtk_ecc_enable(ecc, config, polling);
	if (ret)
		goto freebuf;

	if (config->mode == ECC_PIO_MODE) {
		for (i = 0; i < ((config->len + 3) >> 2); i++) {
			mtk_ecc_wait_ioready(ecc);
			writel(*((u32 *)data + i), ecc->regs + ECC_PIO_DI);
		}
	}

	ret = mtk_ecc_wait_done(ecc, ECC_ENCODE, polling);
	if (ret)
		goto timeout;

	mtk_ecc_wait_idle(ecc, ECC_ENCODE);

	/* Program ECC bytes to OOB: per sector oob = FDM + ECC + SPARE */
	len = (config->strength * ECC_PARITY_BITS + 7) >> 3;
	p = data + bytes;

	/* write the parity bytes generated by the ECC back to the OOB region */
	for (i = 0; i < len; i++) {
		if ((i % 4) == 0) {
			if (i < 108)
				val = readl(ecc->regs + ECC_ENCPAR(i / 4));
			else
				val = readl(ecc->regs + ECC_ENCPAR_EXT((i / 4) - 27));
		}
		p[i] = (val >> ((i % 4) * 8)) & 0xff;
	}
timeout:
	mtk_ecc_disable(ecc);
freebuf:
	if (config->mode == ECC_DMA_MODE) {
		arch_invalidate_cache_range((addr_t)buf, (size_t)bytes);
		//free(buf);
	}
	return ret;
}

int mtk_ecc_decode(struct mtk_ecc *ecc, struct mtk_ecc_config *config,
			u8 *data, u32 len, int polling)
{
	struct mtk_ecc_stats stats;
	u8* buf = data;
	u32 addr = (u32)data, decodesize, i;
	int ret;

	decodesize = len + ((config->strength * ECC_PARITY_BITS + 7) >> 3);
	if ((decodesize & 0x3)
		|| ((config->mode == ECC_DMA_MODE) && (addr & 0x3))) {
		decodesize += 4 - (decodesize & 0x3);
		buf = (u8 *)NAND_DRAM_BUF_ECCEN_ADDR;//memalign(4, decodesize);
		if (!buf)
			return -ENOMEM;
	}
	if (config->mode == ECC_DMA_MODE)
		arch_invalidate_cache_range((addr_t)buf, (size_t)decodesize);
#ifdef WITH_KERNEL_VM
	addr = (u32)kvaddr_to_paddr(buf);
#else
	addr = (u32)buf;
#endif
	config->op = ECC_DECODE;
	config->addr = addr;
	config->len = len;
	ret = mtk_ecc_enable(ecc, config, polling);
	if (ret)
		goto freebuf;

	if (config->mode == ECC_PIO_MODE) {
		for (i = 0; i < (decodesize >> 2); i++) {
			mtk_ecc_wait_ioready(ecc);
			*((u32 *)buf + i) = readl(ecc->regs + ECC_PIO_DI);
		}
	}

	stats.bitflips = 0;
	ret = mtk_ecc_cpu_correct(ecc, &stats, buf, 0, polling);
	if (ret)
		goto disecc;

	if (config->mode == ECC_DMA_MODE)
		arch_invalidate_cache_range((addr_t)buf, (size_t)decodesize);
	if (buf != data)
		memcpy(data, buf, len);

disecc:
	mtk_ecc_disable(ecc);

freebuf:
	//if (buf != data)
	//	free(buf);

	return ret;
}

int mtk_ecc_cpu_correct(struct mtk_ecc *ecc, struct mtk_ecc_stats *stats, u8 *data, u32 sector, int polling)
{
	u32 err, offset, i;
	u32 loc, byteloc, bitloc;
	int ret;

	ecc->sectors = 1 << sector;
	ret = mtk_ecc_wait_done(ecc, ECC_DECODE, polling);
	if (ret)
		return ret;

	stats->corrected = 0;
	stats->failed = 0;

	offset = (sector >> 2);
	err = readl(ecc->regs + ECC_DECENUM(offset));
	err = err >> ((sector % 4) * 8);
	err &= ERR_MASK;
	if (err == ERR_MASK) {
		/* uncorrectable errors */
		stats->failed++;
		return 0;
	}

	stats->corrected += err;
	stats->bitflips = max(stats->bitflips, err);

	for (i = 0; i < err; i++) {
		if (i < 60)
			loc = readl(ecc->regs + ECC_DECEL(i >> 1));
		else
			loc = readl(ecc->regs + ECC_DECEL_EXT((i >> 1) - 30));
		loc >>= ((i & 0x1) << 4);
		byteloc = loc >> 3;
		bitloc = loc & 0x7;
		data[byteloc] ^= (1 << bitloc);
	}

	return 0;
}

void mtk_ecc_adjust_strength(u32 *p)
{
	u32 ecc[] = {4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 28, 32, 36,
			40, 44, 48, 52, 56, 60, 68, 72, 80};
	int i;

	for (i = 0; i < sizeof(ecc) / sizeof(u32); i++) {
		if (*p <= ecc[i]) {
			if (!i)
				*p = ecc[i];
			else if (*p != ecc[i])
				*p = ecc[i - 1];
			return;
		}
	}

	*p = ecc[sizeof(ecc) / sizeof(u32) - 1];
}

