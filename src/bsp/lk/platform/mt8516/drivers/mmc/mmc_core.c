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

/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/
#include <config.h>
#include <platform/msdc.h>
#include <platform/mmc_core.h>
#include <platform/mmc_ioctl.h>
#include <lib/bio.h>
#include <lib/heap.h>
#include <lib/partition.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <kernel/mutex.h>

#define CMD_RETRIES        (5)
#define CMD_TIMEOUT        (100)    /* 100ms */
#define PAD_DELAY_MAX 32

static const u8 tuning_blk_pattern_4bit[] = {
	0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
	0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
	0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
	0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
	0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
	0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
	0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
	0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const u8 tuning_blk_pattern_8bit[] = {
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

static int mmc_set_ext_csd(struct mmc_card *card, u8 addr, u8 value);

typedef struct {
	bdev_t bdev;
	u32 part_id;
	struct mmc_host *host;
	struct mmc_card *card;
} mmc_dev_t;

struct msdc_delay_phase {
	u8 maxlen;
	u8 start;
	u8 final_phase;
};

static const unsigned int tran_exp[] = {
	10000,      100000,     1000000,    10000000,
	0,      0,      0,      0
};

static const unsigned char mmc_tran_mant[] = {
	0,  10, 12, 13, 15, 20, 26, 30,
	35, 40, 45, 52, 55, 60, 70, 80,
};

static u32 unstuff_bits(u32 *resp, u32 start, u32 size)
{
	const u32 __mask = (1 << (size)) - 1;
	const int __off = 3 - ((start) / 32);
	const int __shft = (start) & 31;
	u32 __res;

	__res = resp[__off] >> __shft;
	if ((size) + __shft >= 32)
		__res |= resp[__off-1] << (32 - __shft);
	return __res & __mask;
}

#define UNSTUFF_BITS(r,s,sz)    unstuff_bits(r,s,sz)

static int mmc_switch_part(mmc_dev_t *dev)
{
	int err = MMC_ERR_NONE;
	struct mmc_card *card;
	struct mmc_host *host;
	u8 cfg;

	host = dev->host;
	if (host->curr_part == dev->part_id)
		/* already set to specific partition */
		return MMC_ERR_NONE;

	if (dev->part_id > EXT_CSD_PART_CFG_GP_PART_4) {
		dprintf(CRITICAL, "[MSDC] Unsupported partid: %u\n", dev->part_id);
		return MMC_ERR_INVALID;
	}

	card = dev->card;
	ASSERT(card);

	cfg = card->ext_csd.part_cfg;
	cfg = (cfg & ~0x7) | dev->part_id;
	err = mmc_set_ext_csd(card, EXT_CSD_PART_CFG, cfg);
	if (err)
		dprintf(CRITICAL, "[MSDC] switch to part %u failed!\n", dev->part_id);
	else
		card->ext_csd.part_cfg = cfg;

	return err;
}

static int mmc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
	int err;
	int retry = cmd->retries;

	do {
		err = msdc_cmd(host, cmd);
		if (err == MMC_ERR_NONE || cmd->opcode == MMC_CMD21) /* do not tuning CMD21 */
			break;
	} while (retry--);

	return err;
}

static u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
{
	int bit;

	ocr &= host->ocr_avail;

	bit = __builtin_ffs(ocr);
	if (bit) {
		bit -= 1;
		ocr &= 3 << bit;
	} else {
		ocr = 0;
	}
	return ocr;
}

static inline int mmc_go_idle(struct mmc_host *host)
{
	struct mmc_command cmd = {
		MMC_CMD_GO_IDLE_STATE, 0, RESP_NONE, {0}, CMD_TIMEOUT, CMD_RETRIES, 0
	};
	return mmc_cmd(host, &cmd);
}

static int mmc_send_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
	int i, err = 0;
	struct mmc_command cmd = {
		MMC_CMD_SEND_OP_COND, 0, RESP_R3, {0}, CMD_TIMEOUT, 0, 0
	};

	cmd.arg = ocr;

	for (i = 100; i; i--) {
		err = mmc_cmd(host, &cmd);
		if (err)
			break;

		/* if we're just probing, do a single pass */
		if (ocr == 0)
			break;

		if (cmd.resp[0] & MMC_CARD_BUSY)
			break;

		err = MMC_ERR_TIMEOUT;

		spin(10000);

	}

	if (!err && rocr)
		*rocr = cmd.resp[0];

	return err;
}

static int mmc_all_send_cid(struct mmc_host *host)
{
	struct mmc_command cmd = {
		MMC_CMD_ALL_SEND_CID, 0, RESP_R2, {0}, CMD_TIMEOUT, CMD_RETRIES, 0
	};
	return mmc_cmd(host, &cmd);
}

static int mmc_send_relative_addr(struct mmc_host *host,
		struct mmc_card *card, unsigned int *rca)
{
	struct mmc_command cmd = {
		MMC_CMD_SET_RELATIVE_ADDR, 0, RESP_R1, {0}, CMD_TIMEOUT, CMD_RETRIES, 0
	};
	cmd.arg = *rca << 16;
	return mmc_cmd(host, &cmd);
}

static int mmc_select_card(struct mmc_host *host, struct mmc_card *card)
{
	struct mmc_command cmd = {
		MMC_CMD_SELECT_CARD, 0, RESP_R1B, {0}, CMD_TIMEOUT, CMD_RETRIES, 0
	};
	cmd.arg = card->rca << 16;
	return mmc_cmd(host, &cmd);
}

static int mmc_send_status(struct mmc_host *host, struct mmc_card *card,
		u32 *status)
{
	int err;
	struct mmc_command cmd = {
		MMC_CMD_SEND_STATUS, 0, RESP_R1, {0}, CMD_TIMEOUT, CMD_RETRIES, 0
	};
	cmd.arg = card->rca << 16;

	err = mmc_cmd(host, &cmd);
	if (err == MMC_ERR_NONE)
		*status = cmd.resp[0];
	return err;
}

static int mmc_switch(struct mmc_host *host, struct mmc_card *card,
		u8 set, u8 index, u8 value)
{
	int err;
	u32 status = 0, count = 0;
	struct mmc_command cmd = {
		MMC_CMD_SWITCH, 0, RESP_R1B, {0}, CMD_TIMEOUT, CMD_RETRIES, 0
	};

	cmd.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) | (index << 16) |
		(value << 8) | set;

	err = mmc_cmd(host, &cmd);
	if (err != MMC_ERR_NONE)
		return err;

	do {
		err = mmc_send_status(host, card, &status);
		if (err) {
			dprintf(CRITICAL, "[eMMC] Fail to send status %d\n", err);
			break;
		}
		if (status & R1_SWITCH_ERROR) {
			dprintf(CRITICAL, "[eMMC] switch error. arg(0x%x)\n", cmd.arg);
			return MMC_ERR_FAILED;
		}
		if (count++ >= 600000) {
			dprintf(CRITICAL, "[%s]: timeout happend, count=%d, status=0x%x\n",
					__func__, count, status);
			break;
		}
	} while (!(status & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(status) == 7));

	if (!err && (index == EXT_CSD_PART_CFG))
		host->curr_part = value & 0x7;

	return err;
}

static int mmc_read_csds(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	struct mmc_command cmd = {
		MMC_CMD_SEND_CSD, 0, RESP_R2, {0}, CMD_TIMEOUT * 100, CMD_RETRIES, 0
	};

	cmd.arg = card->rca << 16;

	err = mmc_cmd(host, &cmd);
	if (err == MMC_ERR_NONE) {
		unsigned int e, m;
		card->csd.mmca_vsn = UNSTUFF_BITS(&cmd.resp[0], 122, 4);
		m = UNSTUFF_BITS(&cmd.resp[0], 99, 4);
		e = UNSTUFF_BITS(&cmd.resp[0], 96, 3);
		card->csd.max_dtr = tran_exp[e] * mmc_tran_mant[m];
		e = UNSTUFF_BITS(&cmd.resp[0], 47, 3);
		m = UNSTUFF_BITS(&cmd.resp[0], 62, 12);
		card->csd.capacity = (1 + m) << (e + 2);
		card->csd.read_blkbits = UNSTUFF_BITS(&cmd.resp[0], 80, 4);
	}

	return err;
}

static void mmc_decode_ext_csd(struct mmc_card *card, u8 *ext_csd)
{
	u32 caps = card->host->caps;
	u8 card_type = ext_csd[EXT_CSD_CARD_TYPE];

	card->ext_csd.sectors =
		ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
		ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
		ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
		ext_csd[EXT_CSD_SEC_CNT + 3] << 24;

	card->ext_csd.rev = ext_csd[EXT_CSD_REV];
	card->ext_csd.boot_info   = ext_csd[EXT_CSD_BOOT_INFO];
	card->ext_csd.boot_part_sz = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
	card->ext_csd.rpmb_sz = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;

	if (card->ext_csd.sectors)
		mmc_card_set_blockaddr(card);

	if (caps & MMC_CAP_EMMC_HS400 &&
			card_type & EXT_CSD_CARD_TYPE_HS400_1_8V) {
		card->ext_csd.hs400_support = 1;
		card->ext_csd.hs_max_dtr = 200000000;
	} else if (caps & MMC_CAP_EMMC_HS200 &&
			card_type & EXT_CSD_CARD_TYPE_HS200_1_8V) {
		card->ext_csd.hs200_support = 1;
		card->ext_csd.hs_max_dtr = 200000000;
	} else if (caps & MMC_CAP_DDR &&
			card_type & EXT_CSD_CARD_TYPE_DDR_52) {
		card->ext_csd.ddr_support = 1;
		card->ext_csd.hs_max_dtr = 52000000;
	} else if (caps & MMC_CAP_MMC_HIGHSPEED &&
			card_type & EXT_CSD_CARD_TYPE_52) {
		card->ext_csd.hs_max_dtr = 52000000;
	} else if (card_type & EXT_CSD_CARD_TYPE_26) {
		card->ext_csd.hs_max_dtr = 26000000;
	} else {
		/* MMC v4 spec says this cannot happen */
		dprintf(CRITICAL, "[eMMC] MMCv4 but HS unsupported\n");
	}

	card->ext_csd.part_cfg = ext_csd[EXT_CSD_PART_CFG];
	card->ext_csd.sec_support = ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT];
	card->ext_csd.reset_en = ext_csd[EXT_CSD_RST_N_FUNC];

	return;
}

/* Read and decode extended CSD. */
static int mmc_read_ext_csd(struct mmc_host *host, struct mmc_card *card)
{
	int err = MMC_ERR_NONE;
	u8 *ext_csd;
	int result = MMC_ERR_NONE;
	struct mmc_data data;
	addr_t base = host->base;
	struct mmc_command cmd = {
		MMC_CMD_SEND_EXT_CSD, 0, RESP_R1, {0}, CMD_TIMEOUT, CMD_RETRIES, 0
	};

	if (card->csd.mmca_vsn < CSD_SPEC_VER_4) {
		dprintf(INFO, "[eMMC] MMCA_VSN: %d. Skip EXT_CSD\n",
				card->csd.mmca_vsn);
		return MMC_ERR_NONE;
	}

	/*
	 * As the ext_csd is so large and mostly unused, we don't store the
	 * raw block in mmc_card.
	 */
	ext_csd = malloc(512);
	ASSERT(ext_csd);
	memset(ext_csd, 0, 512);

	msdc_reset_tune_counter(host);

	do {
		MSDC_DMA_ON;
		MSDC_CLR_FIFO();
		MSDC_WRITE32(SDC_BLK_NUM, 1);
		host->blklen = 512;
		msdc_set_timeout(host, 100000000, 0);
		err = mmc_cmd(host, &cmd);
		if (err != MMC_ERR_NONE)
			goto out;

		data.cmd = &cmd;
		data.blks = 1;
		data.buf = ext_csd;
		data.timeout = 100;
		err = msdc_dma_transfer(host, &data);
		MSDC_DMA_OFF;
		if (err != MMC_ERR_NONE) {
			if (msdc_abort_handler(host, 1))
				dprintf(CRITICAL, "[eMMC] data abort failed\n");
			result = msdc_tune_read(host);
		}
	} while (err && result != MMC_ERR_READTUNEFAIL);
	msdc_reset_tune_counter(host);
	mmc_decode_ext_csd(card, ext_csd);

out:
	free(ext_csd);
	return err;
}

static void mmc_set_clock(struct mmc_host *host, int state, unsigned int hz)
{
	if (hz >= host->f_max) {
		hz = host->f_max;
	} else if (hz < host->f_min) {
		hz = host->f_min;
	}
	msdc_config_clock(host, state, hz);
}

static int mmc_set_bus_width(struct mmc_host *host, struct mmc_card *card, int width)
{
	int err = MMC_ERR_NONE;
	u32 arg = 0;

	if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
		goto out;

	if (width == HOST_BUS_WIDTH_8) {
		if (host->caps & MMC_CAP_8_BIT_DATA) {
			arg = EXT_CSD_BUS_WIDTH_8;
		} else {
			width = HOST_BUS_WIDTH_4;
		}
	}
	if (width == HOST_BUS_WIDTH_4) {
		if (host->caps & MMC_CAP_4_BIT_DATA) {
			arg = EXT_CSD_BUS_WIDTH_4;
		} else {
			width = HOST_BUS_WIDTH_1;
		}
	}
	if (width == HOST_BUS_WIDTH_1)
		arg = EXT_CSD_BUS_WIDTH_1;

	err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH, arg);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Switch to bus width(%d) failed\n", arg);
		goto out;
	}
	mmc_card_clr_ddr(card);

	msdc_config_bus(host, width);

out:
	return err;
}

static u32 test_delay_bit(u32 delay, u32 bit)
{
	bit %= PAD_DELAY_MAX;
	return delay & (1 << bit);
}

static int get_delay_len(u32 delay, u32 start_bit)
{
	u32 i;

	for (i = 0; i < (PAD_DELAY_MAX - start_bit); i++) {
		if (test_delay_bit(delay, start_bit + i) == 0)
			return i;
	}
	return PAD_DELAY_MAX - start_bit;
}

static struct msdc_delay_phase get_best_delay(u32 delay)
{
	int start = 0, len = 0;
	int start_final = 0, len_final = 0;
	u8 final_phase = 0xff;
	struct msdc_delay_phase delay_phase = { 0, };

	if (delay == 0) {
		dprintf(CRITICAL, "phase error: [map:%x]\n", delay);
		delay_phase.final_phase = final_phase;
		return delay_phase;
	}

	while (start < PAD_DELAY_MAX) {
		len = get_delay_len(delay, start);
		if (len_final < len) {
			start_final = start;
			len_final = len;
		}
		start += len ? len : 1;
		if (len >= 12 && start_final < 4)
			break;
	}

	/* The rule is that to find the smallest delay cell */
	if (start_final == 0)
		final_phase = (start_final + len_final / 3) % PAD_DELAY_MAX;
	else
		final_phase = (start_final + len_final / 2) % PAD_DELAY_MAX;
	dprintf(ALWAYS, "phase: [map:%x] [maxlen:%d] [final:%d]\n",
		 delay, len_final, final_phase);

	delay_phase.maxlen = len_final;
	delay_phase.start = start_final;
	delay_phase.final_phase = final_phase;
	return delay_phase;
}

static int mmc_send_tuning(struct mmc_host *host, int *cmd_error)
{
	int err = MMC_ERR_NONE;
	const u8 *tuning_block_pattern;
	u8 *tune_data;
	u16 data_len;
	struct mmc_data data;
	addr_t base = host->base;
	struct mmc_command cmd = {
		MMC_CMD21, 0, RESP_R1, {0}, CMD_TIMEOUT, 0, 0
	};

	if (host->caps & MMC_CAP_8_BIT_DATA) {
		tuning_block_pattern = tuning_blk_pattern_8bit;
		data_len = sizeof(tuning_blk_pattern_8bit);
	} else {
		tuning_block_pattern = tuning_blk_pattern_4bit;
		data_len = sizeof(tuning_blk_pattern_4bit);
	}

	tune_data = malloc(data_len);
	ASSERT(tune_data);
	memset(tune_data, 0, data_len);
	*cmd_error = MMC_ERR_NONE;

	msdc_reset_tune_counter(host);

	MSDC_DMA_ON;
	MSDC_CLR_FIFO();
	MSDC_WRITE32(SDC_BLK_NUM, 1);
	host->blklen = data_len;
	msdc_set_timeout(host, 100000000, 0);
	err = mmc_cmd(host, &cmd);
	if (err != MMC_ERR_NONE)
		*cmd_error = err; /* still need receive data, or will impact the next cmd21 */

	data.cmd = &cmd;
	data.blks = 1;
	data.buf = tune_data;
	data.timeout = 100;
	err = msdc_dma_transfer(host, &data);
	MSDC_DMA_OFF;
	msdc_reset_tune_counter(host);
	if (err)
		goto out;
	else {
		if(memcmp(tune_data, tuning_block_pattern, data_len))
			err = -EIO;
	}
out:
	free(tune_data);
	return err;
}

static int msdc_tune_response(struct mmc_host *mmc)
{
	addr_t base = mmc->base;
	u32 rise_delay = 0, fall_delay = 0;
	struct msdc_delay_phase final_rise_delay, final_fall_delay = { 0,};
	u8 final_delay, final_maxlen;
	int cmd_err;
	int i, j;

	MSDC_CLR_BIT32(MSDC_IOCON, MSDC_IOCON_RSPL);
	for (i = 0 ; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, i);
		for (j = 0; j < 3; j++) {
			mmc_send_tuning(mmc, &cmd_err);
			if (!cmd_err) {
				rise_delay |= (1 << i);
			} else {
				rise_delay &= ~(1 << i);
				break;
			}
		}
	}
	final_rise_delay = get_best_delay(rise_delay);
	/* if rising edge has enough margin, then do not scan falling edge */
	if (final_rise_delay.maxlen >= 12 ||
	    (final_rise_delay.start == 0 && final_rise_delay.maxlen >= 4))
		goto skip_fall;

	MSDC_SET_BIT32(MSDC_IOCON, MSDC_IOCON_RSPL);
	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, i);
		for (j = 0; j < 3; j++) {
			mmc_send_tuning(mmc, &cmd_err);
			if (!cmd_err) {
				fall_delay |= (1 << i);
			} else {
				fall_delay &= ~(1 << i);
				break;
			}
		}
	}
	final_fall_delay = get_best_delay(fall_delay);

skip_fall:
	final_maxlen = MAX(final_rise_delay.maxlen, final_fall_delay.maxlen);
	if (final_maxlen == final_rise_delay.maxlen) {
		MSDC_CLR_BIT32(MSDC_IOCON, MSDC_IOCON_RSPL);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY,
			      final_rise_delay.final_phase);
		final_delay = final_rise_delay.final_phase;
	} else {
		MSDC_SET_BIT32(MSDC_IOCON, MSDC_IOCON_RSPL);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY,
			      final_fall_delay.final_phase);
		final_delay = final_fall_delay.final_phase;
	}

	dprintf(ALWAYS, "Final cmd pad delay: %x\n", final_delay);
	return final_delay == 0xff ? -EIO : 0;
}

static int msdc_tune_data(struct mmc_host *mmc)
{
	addr_t base = mmc->base;
	u32 rise_delay = 0, fall_delay = 0;
	struct msdc_delay_phase final_rise_delay, final_fall_delay = { 0, };
	u8 final_delay, final_maxlen;
	int cmd_err;
	int i, ret;

	MSDC_CLR_BIT32(MSDC_IOCON, MSDC_IOCON_DSPL);
	MSDC_CLR_BIT32(MSDC_IOCON, MSDC_IOCON_W_D_SMPL);
	for (i = 0 ; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, i);
		ret = mmc_send_tuning(mmc, &cmd_err);
		if (!ret)
			rise_delay |= (1 << i);
		else if (cmd_err) {
			/* in this case, need retune response */
			ret = msdc_tune_response(mmc);
			if (ret)
				break;
		}
	}
	final_rise_delay = get_best_delay(rise_delay);
	if (final_rise_delay.maxlen >= 12 ||
	    (final_rise_delay.start == 0 && final_rise_delay.maxlen >= 4))
		goto skip_fall;

	MSDC_SET_BIT32(MSDC_IOCON, MSDC_IOCON_DSPL);
	MSDC_SET_BIT32(MSDC_IOCON, MSDC_IOCON_W_D_SMPL);
	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, i);
		ret = mmc_send_tuning(mmc, &cmd_err);
		if (!ret)
			fall_delay |= (1 << i);
		else if (cmd_err) {
			/* in this case, need retune response */
			ret = msdc_tune_response(mmc);
			if (ret)
				break;
		}
	}
	final_fall_delay = get_best_delay(fall_delay);

skip_fall:
	final_maxlen = MAX(final_rise_delay.maxlen, final_fall_delay.maxlen);
	if (final_maxlen == final_rise_delay.maxlen) {
		MSDC_CLR_BIT32(MSDC_IOCON, MSDC_IOCON_DSPL);
		MSDC_CLR_BIT32(MSDC_IOCON, MSDC_IOCON_W_D_SMPL);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
			      final_rise_delay.final_phase);
		final_delay = final_rise_delay.final_phase;
	} else {
		MSDC_SET_BIT32(MSDC_IOCON, MSDC_IOCON_DSPL);
		MSDC_SET_BIT32(MSDC_IOCON, MSDC_IOCON_W_D_SMPL);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
			      final_fall_delay.final_phase);
		final_delay = final_fall_delay.final_phase;
	}

	dprintf(ALWAYS, "Final data pad delay: %x\n", final_delay);
	return final_delay == 0xff ? -EIO : 0;
}

static int mmc_select_hs200(struct mmc_card *card)
{
	struct mmc_host *host = card->host;
	int ret;

	ret = mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8);
	if (ret != MMC_ERR_NONE) {
		dprintf(CRITICAL, "failed to set bus width!\n");
		return ret;
	}

	ret = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING,
			 EXT_CSD_HS_TIMEING_HS200);
	if (ret != MMC_ERR_NONE) {
		dprintf(CRITICAL, "failed to switch to hs200 mode!\n");
		return ret;
	}

	mmc_card_set_hs200(card);
	mmc_set_clock(host, card->state, card->ext_csd.hs_max_dtr);

	return 0;
}

static int mmc_select_hs400(struct mmc_card *card)
{
	struct mmc_host *host = card->host;
	int ret;

	mmc_set_clock(host, card->state, 50000000);
	ret = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING,
			 EXT_CSD_HS_TIMEING_HS);
	if (ret != MMC_ERR_NONE) {
		dprintf(CRITICAL, "switch to high-speed from hs200 failed, err:%d\n", ret);
		return ret;
	}

	ret = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_8_DDR);
	if (ret != MMC_ERR_NONE) {
		dprintf(CRITICAL, "switch to bus width for hs400 failed, err:%d\n", ret);
		return ret;
	}

	ret = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING,
			 EXT_CSD_HS_TIMEING_HS400);
	if (ret != MMC_ERR_NONE) {
		dprintf(CRITICAL, "switch to hs400 failed, err:%d\n", ret);
		return ret;
	}
	mmc_card_set_hs400(card);
	mmc_set_clock(host, card->state, card->ext_csd.hs_max_dtr);

	return ret;
}

static int mmc_hs200_tuning(struct mmc_card *card)
{
	struct mmc_host *host = card->host;
	addr_t base = host->base;
	int ret;

	ret = msdc_tune_response(host);
	if (ret == -EIO) {
		dprintf(CRITICAL, "hs200 tuning cmd error!\n");
		return ret;
	}

	ret = msdc_tune_data(host);
	if (ret == -EIO) {
		dprintf(CRITICAL, "hs200 tuning data error!\n");
		return ret;
	}

	return MMC_ERR_NONE;
}

static int mmc_erase_start(struct mmc_card *card, u32 blknr)
{
	struct mmc_command cmd = {
		MMC_CMD_ERASE_GROUP_START, 0, RESP_R1, {0}, CMD_TIMEOUT, 3, 0
	};
	cmd.arg = blknr;
	return mmc_cmd(card->host, &cmd);
}

static int mmc_erase_end(struct mmc_card *card, u32 blknr)
{
	struct mmc_command cmd = {
		MMC_CMD_ERASE_GROUP_END, 0, RESP_R1, {0}, CMD_TIMEOUT, 3, 0
	};
	cmd.arg = blknr;
	return mmc_cmd(card->host, &cmd);
}

static int mmc_erase(struct mmc_card *card, u32 arg)
{
	int err;
	u32 status;
	struct mmc_command cmd = {
		MMC_CMD_ERASE, 0, RESP_R1B, {0}, CMD_TIMEOUT, 3, 0
	};
	cmd.arg = arg;

	if (arg & MMC_ERASE_SECURE_REQ) {
		if (!(card->ext_csd.sec_support & EXT_CSD_SEC_FEATURE_ER_EN))
			return MMC_ERR_INVALID;
	}
	if ((arg & MMC_ERASE_GC_REQ) || (arg & MMC_ERASE_TRIM)) {
		if (!(card->ext_csd.sec_support & EXT_CSD_SEC_FEATURE_GB_CL_EN))
			return MMC_ERR_INVALID;
	}

	err = mmc_cmd(card->host, &cmd);
	if (err)
		return err;

	do {
		err = mmc_send_status(card->host, card, &status);
		if (err)
			break;
		if (R1_STATUS(status) != 0)
			break;
	} while (R1_CURRENT_STATE(status) == 7);

	return err;
}

static int mmc_do_trim(struct mmc_card *card, off_t start_addr, size_t len)
{
	int err = MMC_ERR_NONE;
	off_t end_addr;

	if (len < card->blklen) {
		dprintf(CRITICAL, "%s: invalid len: %ld\n", __func__, len);
		return MMC_ERR_INVALID;
	}

	end_addr =((start_addr + len) / card->blklen - 1) * card->blklen;

	if (mmc_card_highcaps(card)) {
		start_addr >>= MMC_BLOCK_BITS_SHFT;
		end_addr >>= MMC_BLOCK_BITS_SHFT;
	}

	err = mmc_erase_start(card, start_addr);
	if (err)
		goto error;

	err = mmc_erase_end(card, end_addr);
	if (err)
		goto error;

	err = mmc_erase(card, MMC_ERASE_TRIM);

error:
	if (err)
		dprintf(CRITICAL, "%s: erase range (0x%llx~0x%llx) failed,Err<%d>\n",
				__func__, start_addr, end_addr, err);

	return err;
}

static int mmc_set_ext_csd(struct mmc_card *card, u8 addr, u8 value)
{
	int err;

	/* can't write */
	if (192 <= addr || !card)
		return MMC_ERR_INVALID;

	err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, addr, value);

	if (err == MMC_ERR_NONE)
		err = mmc_read_ext_csd(card->host, card);

	return err;
}

static int mmc_set_reset_func(struct mmc_card *card, u8 enable)
{
	int err = MMC_ERR_FAILED;

	if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
		goto out;

	if (card->ext_csd.reset_en == 0) {
		err = mmc_set_ext_csd(card, EXT_CSD_RST_N_FUNC, enable);
		if (err == MMC_ERR_NONE)
			card->ext_csd.reset_en = enable;
	} else {
		/* no need set */
		return MMC_ERR_NONE;
	}
out:
	return err;
}

static int mmc_set_boot_bus(struct mmc_card *card, u8 rst_bwidth, u8 mode, u8 bwidth)
{
	int err = MMC_ERR_FAILED;
	u8 arg;

	if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
		goto out;

	arg = mode | rst_bwidth | bwidth;

	err = mmc_set_ext_csd(card, EXT_CSD_BOOT_BUS_WIDTH, arg);
out:
	return err;
}

static int mmc_set_part_config(struct mmc_card *card, u8 cfg)
{
	int err = MMC_ERR_FAILED;

	if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
		goto out;

	err = mmc_set_ext_csd(card, EXT_CSD_PART_CFG, cfg);
out:
	return err;
}

static int mmc_boot_config(struct mmc_card *card, u8 acken, u8 enpart, u8 buswidth, u8 busmode)
{
	int err = MMC_ERR_FAILED;
	u8 val;
	u8 rst_bwidth = 0;
	u8 cfg;

	if (card->csd.mmca_vsn < CSD_SPEC_VER_4 ||
	    !card->ext_csd.boot_info || card->ext_csd.rev < 3)
		goto out;

	cfg = card->ext_csd.part_cfg;
	/* configure boot partition */
	val = acken | enpart | (cfg & 0x7);
	err = mmc_set_part_config(card, val);
	if (err != MMC_ERR_NONE)
		goto out;
	else
		card->ext_csd.part_cfg = val;

	/* configure boot bus mode and width */
	rst_bwidth = (buswidth != EXT_CSD_BOOT_BUS_WIDTH_1 ? 1 : 0) << 2;
	dprintf(INFO, " =====Set boot Bus Width<%d>=======\n", buswidth);
	dprintf(INFO, " =====Set boot Bus mode<%d>=======\n", busmode);
	err = mmc_set_boot_bus(card, rst_bwidth, busmode, buswidth);
out:

	return err;
}

static int emmc_boot_prepare(struct mmc_card *card)
{
	int err = MMC_ERR_NONE;

	err = mmc_boot_config(card, EXT_CSD_PART_CFG_EN_ACK,
				EXT_CSD_PART_CFG_EN_BOOT_PART_1,
				EXT_CSD_BOOT_BUS_WIDTH_1,
				EXT_CSD_BOOT_BUS_MODE_DEFT);
	if (err)
		goto exit;

	err = mmc_set_reset_func(card, 1);
exit:
	return err;
}

static int mmc_dev_bread(struct mmc_card *card, unsigned long blknr, u32 blkcnt, u8 *dst)
{
    struct mmc_host *host = card->host;
    u32 blksz = host->blklen;
    int tune = 0;
    int retry = 3;
    int err;
    unsigned long src;

    src = mmc_card_highcaps(card) ? blknr : blknr * blksz;

    do {
        if (!tune) {
            err = host->blk_read(host, (uchar *)dst, src, blkcnt);
        } else {
#ifdef FEATURE_MMC_RD_TUNING
            err = msdc_tune_bread(host, (uchar *)dst, src, blkcnt);
#endif
            if (err && (host->sclk > (host->f_max >> 4)))
                mmc_set_clock(host, card->state, host->sclk >> 1);
        }
        if (err == MMC_ERR_NONE) {
            break;
        }

        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC) {
            tune = 1;
            retry++;
        } else if (err == MMC_ERR_READTUNEFAIL || err == MMC_ERR_CMDTUNEFAIL) {
            dprintf(CRITICAL, "[eMMC] Fail to tuning,%s",
                    (err == MMC_ERR_CMDTUNEFAIL) ?
                    "cmd tune failed!\n" : "read tune failed!\n");
            break;
        }
    } while (retry--);

    return err;
}

static int mmc_dev_bwrite(struct mmc_card *card, unsigned long blknr,
                          u32 blkcnt, const u8 *src)
{
	struct mmc_host *host = card->host;
	u32 blksz = host->blklen;
	u32 status;
	int tune = 0;
	int retry = 3;
	int err;
	unsigned long dst;

	dst = mmc_card_highcaps(card) ? blknr : blknr * blksz;

	do {
		if (!tune) {
			err = host->blk_write(host, dst, (uchar *)src, blkcnt);
		} else {
#ifdef FEATURE_MMC_WR_TUNING
			err = msdc_tune_bwrite(host, dst, (uchar *)src, blkcnt);
#endif
			if (err && (host->sclk > (host->f_max >> 4)))
				mmc_set_clock(host, card->state, host->sclk >> 1);
		}

		if (err == MMC_ERR_NONE) {
			do {
				err = mmc_send_status(host, card, &status);
				if (err) {
					dprintf(CRITICAL, "[eMMC] Fail to send status %d\n", err);
					break;
				}
			} while (!(status & R1_READY_FOR_DATA) ||(R1_CURRENT_STATE(status) == 7));
			dprintf(INFO, "[eMMC] Write %d bytes (DONE)\n", blkcnt * blksz);
			break;
		}

		if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC) {
			tune = 1;
			retry++;
		} else if (err == MMC_ERR_WRITETUNEFAIL || err == MMC_ERR_CMDTUNEFAIL) {
			dprintf(CRITICAL, "[eMMC] Fail to tuning,%s",
				(err == MMC_ERR_CMDTUNEFAIL) ?
				"cmd tune failed!\n" : "write tune failed!\n");
			break;
		}
	} while (retry--);

	return err;
}

static ssize_t mmc_block_read(struct bdev *dev, void *buf, bnum_t block,
                              uint count)
{
	mmc_dev_t *__dev = (mmc_dev_t *)dev;
	struct mmc_host *host = __dev->host;
	struct mmc_card *card = __dev->card;
	u32 maxblks = host->max_phys_segs;
	u32 leftblks, totalblks = count;
	ssize_t ret = 0;

	mutex_acquire(&host->lock);
	if (mmc_switch_part(__dev)) {
		ret = ERR_IO;
		goto done;
	}

	do {
		leftblks = ((count > maxblks) ? maxblks : count);
		if (mmc_dev_bread(card, (unsigned long)block, leftblks, buf)) {
			ret = ERR_IO;
			goto done;
		}
		block += leftblks;
		buf += maxblks * dev->block_size;
		count -= leftblks;
	} while (count);

	if (dev->block_size * totalblks > 0x7fffffffU)
		/* ssize_t is defined as signed, should take a look here */
		dprintf(CRITICAL, "[MSDC] %s: WARN! The return size is overflow! 0x%lx\n",
				__func__, dev->block_size * totalblks);

done:
	mutex_release(&host->lock);
	return ret ? ret : (ssize_t)dev->block_size * totalblks;
}

static ssize_t mmc_block_write(struct bdev *dev, const void *buf, bnum_t block,
		uint count)
{
	mmc_dev_t *__dev = (mmc_dev_t *)dev;
	struct mmc_host *host = __dev->host;
	struct mmc_card *card = __dev->card;
	u32 maxblks = host->max_phys_segs;
	u32 leftblks, totalblks = count;
	ssize_t ret = 0;

	mutex_acquire(&host->lock);
	if (mmc_switch_part(__dev)) {
		ret = ERR_IO;
		goto done;
	}

	do {
		leftblks = ((count > maxblks) ? maxblks : count);
		if (mmc_dev_bwrite(card, (unsigned long)block, leftblks, buf)) {
			ret = ERR_IO;
			goto done;
		}
		block += leftblks;
		buf = (u8 *)buf + maxblks * dev->block_size;
		count -= leftblks;
	} while (count);

	if (dev->block_size * totalblks > 0x7fffffffU)
		/* ssize_t is defined as signed, should take a look here */
		dprintf(CRITICAL, "[MSDC] %s: WARN! The return size is overflow! 0x%lx\n",
				__func__, dev->block_size * totalblks);

done:
	mutex_release(&host->lock);
	return ret ? ret: (ssize_t)dev->block_size * totalblks;
}

static ssize_t mmc_wrap_erase(struct bdev *bdev, off_t offset, size_t len)
{
	mmc_dev_t *dev = (mmc_dev_t *)bdev;
	struct mmc_host *host = dev->host;
	ssize_t ret = 0;

	mutex_acquire(&host->lock);
	if (mmc_switch_part(dev)) {
		ret = ERR_IO;
		goto done;
	}

	/* ATTENTION:
	 * We use TRIM here, which is block-based(512B) wipping,
	 * If using ERASE here, please ensure the offset & size are
	 * erase-group aligned,
	 * OTHERWISE, some valid data may be wiped. refer to JEDEC spec:
	 * The Device will ignore all LSB's below the Erase Group size,
	 * effectively ROUNDING the address DOWN to the Erase Group boundary. */
	ASSERT(dev && len);
	if ((offset % MMC_BLOCK_SIZE) || (len % MMC_BLOCK_SIZE)) {
		dprintf(CRITICAL, "%s: offset(0x%llx)/len(%lu) is not block-aligned!\n",
				__func__, offset, len);
		ret = ERR_IO;
		goto done;
	}

	ASSERT(dev->card);
	if (mmc_do_trim(dev->card, offset, len)) {
		ret = ERR_IO;
		goto done;
	}

done:
	mutex_release(&host->lock);
	return ret ? ret: (ssize_t)len;
}

static ssize_t mmc_rpmb_dummy_read(struct bdev *dev, void *buf, bnum_t block,
		uint count)
{
	return 0;
}

static ssize_t mmc_rpmb_dummy_write(struct bdev *dev, const void *buf, bnum_t block,
		uint count)
{
	return 0;
}

static ssize_t mmc_rpmb_dummy_erase(struct bdev *bdev, off_t offset, size_t len)
{
	return 0;
}

static int mmc_set_block_count(struct mmc_host *host, unsigned int blockcount,
		bool is_rel_write)
{
	struct mmc_command cmd = {0};

	cmd.opcode = MMC_CMD_SET_BLOCK_COUNT;
	cmd.arg = blockcount & 0x0000FFFF;
	if (is_rel_write)
		cmd.arg |= 1 << 31;
	cmd.rsptyp = RESP_R1;

	return mmc_cmd(host, &cmd);
}

static int mmc_rpmb_ioctl_cmd(struct bdev *dev, struct mmc_ioc_cmd *arg)
{
	mmc_dev_t *__dev = (mmc_dev_t *)dev;
	struct mmc_host *host = __dev->host;
	//struct mmc_card *card = __dev->card;
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	addr_t base = host->base;
	int ret = 0;
	int old_autocmd = msdc_get_autocmd(host);

	msdc_set_autocmd(host, 0);
	cmd.opcode = arg->opcode;
	cmd.arg = arg->arg;
	cmd.rsptyp = arg->flags; /* arg->flags must be type of enum of RESP_NONE ~ RESP_R1B */

	if (arg->blocks) {
		ret = mmc_set_block_count(host, arg->blocks,
				arg->write_flag & (1 << 31));
		if (ret != MMC_ERR_NONE) {
			dprintf(CRITICAL, "mmc cmd23 failed!\n");
			goto out;
		}
	}

	if (arg->blocks) {
		MSDC_DMA_ON;
		MSDC_CLR_FIFO();
		MSDC_WRITE32(SDC_BLK_NUM, arg->blocks);
		host->blklen = 512;
		msdc_set_timeout(host, 100000000, 0);
		ret = mmc_cmd(host, &cmd);
		if (ret != MMC_ERR_NONE) {
			dprintf(CRITICAL, "mmc cmd failed\n");
			goto out;
		}

		data.cmd = &cmd;
		data.blks = arg->blocks;
		data.buf = (uchar *)arg->data_ptr;
		data.timeout = 100;
		ret = msdc_dma_transfer(host, &data);
		MSDC_DMA_OFF;

	} else {
		ret = mmc_cmd(host, &cmd);
	}

out:
	msdc_set_autocmd(host, old_autocmd);
	return ret;
}

static int mmc_rpmb_ioctl(struct bdev *dev, int request, void *argp)
{
	mmc_dev_t *__dev = (mmc_dev_t *)dev;
	struct mmc_host *host = __dev->host;
	int ret = 0;

	mutex_acquire(&host->lock);
	if (mmc_switch_part(__dev)) {
		ret = ERR_IO;
		goto done;
	}

	switch (request) {
		case MMC_IOC_CMD:
			ret = mmc_rpmb_ioctl_cmd(dev, (struct mmc_ioc_cmd *)argp);
			break;
		default:
			ret = ERR_INVALID_ARGS;
			break;
	}

done:
	mutex_release(&host->lock);
	return ret;
}


static int mmc_init_mem_card(struct mmc_host *host, struct mmc_card *card, u32 ocr)
{
	int err;

	/*
	 * Sanity check the voltages that the card claims to
	 * support.
	 */
	if (ocr & 0x7F)
		ocr &= ~0x7F;

	ocr = host->ocr = mmc_select_voltage(host, ocr);

	/*
	 * Can we support the voltage(s) of the card(s)?
	 */
	if (!host->ocr) {
		err = MMC_ERR_FAILED;
		goto out;
	}

	err = mmc_go_idle(host);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in GO_IDLE_STATE cmd\n");
		goto out;
	}

	/* host support HCS[30] */
	ocr |= (1 << 30);

	/* send operation condition */
	/* The extra bit indicates that we support high capacity */
	err = mmc_send_op_cond(host, ocr, &card->ocr);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in SEND_OP_COND cmd\n");
		goto out;
	}

	/* set hcs bit if a high-capacity card */
	card->state |= ((card->ocr >> 30) & 0x1) ? MMC_STATE_HIGHCAPS : 0;
	/* send cid */
	err = mmc_all_send_cid(host);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in SEND_CID cmd\n");
		goto out;
	}

	/* assign a rca */
	card->rca = 0x1;

	/* set/send rca */
	err = mmc_send_relative_addr(host, card, &card->rca);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in SEND_RCA cmd\n");
		goto out;
	}

	/* send csd */
	err = mmc_read_csds(host, card);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in SEND_CSD cmd\n");
		goto out;
	}

	/* select this card */
	err = mmc_select_card(host, card);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in select card cmd\n");
		goto out;
	}

	/* send ext csd */
	err = mmc_read_ext_csd(host, card);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in SEND_EXT_CSD cmd\n");
		goto out;
	}

	if ((host->caps & MMC_CAP_EMMC_HS200) && card->ext_csd.hs200_support) {
		err = mmc_select_hs200(card);
		if (err != MMC_ERR_NONE)
			goto select_hs;
		err = mmc_hs200_tuning(card);
		if (err != MMC_ERR_NONE)
			goto select_hs;

		if ((host->caps & MMC_CAP_EMMC_HS400) && card->ext_csd.hs400_support) {
			err = mmc_select_hs400(card);
			if (err != MMC_ERR_NONE)
				goto select_hs;
			else
				goto card_init_done;
		}
		else
			goto card_init_done;
	}

select_hs:
	/* activate high speed (if supported) */
	if ((card->ext_csd.hs_max_dtr != 0) && (host->caps & MMC_CAP_MMC_HIGHSPEED)) {
		mmc_set_clock(host, 0, host->f_min);
		err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);
		if (err == MMC_ERR_NONE) {
			dprintf(INFO, "[eMMC] Switched to High-Speed mode!\n");
			mmc_card_clear_hs200(card);
			mmc_card_clear_hs400(card);
			mmc_card_clear_ddr(card);
			mmc_card_set_highspeed(card);
			mmc_set_clock(host, card->state, 50000000);
			/* set bus width */
			mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8);
		}
	}

card_init_done:
	/* compute bus speed. */
	card->maxhz = (unsigned int)-1;

	if (mmc_card_highspeed(card) || mmc_card_hs200(card) || mmc_card_hs400(card)) {
		if (card->maxhz > card->ext_csd.hs_max_dtr)
			card->maxhz = card->ext_csd.hs_max_dtr;
	} else if (card->maxhz > card->csd.max_dtr) {
		card->maxhz = card->csd.max_dtr;
	}

	if (mmc_card_blockaddr(card)) {
		/* The EXT_CSD sector count is in number or 512 byte sectors. */
		card->blklen = MMC_BLOCK_SIZE;
		card->nblks  = card->ext_csd.sectors;
	} else {
		/* The CSD capacity field is in units of read_blkbits.
		 * set_capacity takes units of 512 bytes.
		 */
		card->blklen = MMC_BLOCK_SIZE;
		card->nblks  = card->csd.capacity << (card->csd.read_blkbits - 9);
	}

	dprintf(CRITICAL,"[eMMC] Size: %d MB, Max.Speed: %d kHz, blklen(%d), nblks(%d), ro(%d)\n",
			((card->nblks / 1024) * card->blklen) / 1024 , card->maxhz / 1000,
			card->blklen, card->nblks, mmc_card_readonly(card));

	card->ready = 1;

	dprintf(INFO, "[eMMC] Initialized\n");
out:
	return err;
}

static int mmc_init_card(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	u32 ocr;

	dprintf(INFO, "[%s]: start\n", __func__);
	memset(card, 0, sizeof(struct mmc_card));

	mmc_card_set_present(card);
	mmc_card_set_host(card, host);
	mmc_card_set_unknown(card);

	err = mmc_go_idle(host);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in GO_IDLE_STATE cmd\n");
		goto out;
	}

	/* query operation condition */
	err = mmc_send_op_cond(host, 0, &ocr);
	if (err != MMC_ERR_NONE) {
		dprintf(CRITICAL, "[eMMC] Fail in MMC_CMD_SEND_OP_COND/SD_ACMD_SEND_OP_COND cmd\n");
		goto out;
	}

	err = mmc_init_mem_card(host, card, ocr);
	if (err)
		goto out;

out:
	if (err) {
		dprintf(CRITICAL, "[%s]: failed, err=%d\n", __func__, err);
		return err;
	}
	host->card = card;
	dprintf(INFO, "[%s]: finish successfully\n",__func__);
	return 0;
}

static inline int mmc_init_host(struct mmc_host *host)
{
	mutex_init(&host->lock);
	return msdc_init(host);
}

static void mmc_bio_ops(const void *name, const int part_id, const int nblks,
		struct mmc_host *host, struct mmc_card *card)
{
	mmc_dev_t *dev;

	dev = malloc(sizeof(mmc_dev_t));
	/* malloc fail */
	ASSERT(dev);
	/* construct the block device */
	memset(dev, 0, sizeof(mmc_dev_t));

	/* setup partition id*/
	dev->part_id = part_id;
	/* setup host */
	dev->host = host;
	/* setup card */
	dev->card = card;
	/* bio block device register */
	bio_initialize_bdev(&dev->bdev, name,
			card->blklen, nblks,
			0, NULL, BIO_FLAGS_NONE);
	/* override our block device hooks */
	if (part_id == EXT_CSD_PART_CFG_RPMB_PART) {
		dev->bdev.read_block = mmc_rpmb_dummy_read;
		dev->bdev.write_block = mmc_rpmb_dummy_write;
		dev->bdev.erase = mmc_rpmb_dummy_erase;
		dev->bdev.ioctl = mmc_rpmb_ioctl;
	} else {
		dev->bdev.read_block = mmc_block_read;
		dev->bdev.write_block = mmc_block_write;
		dev->bdev.erase = mmc_wrap_erase;
	}
	bio_register_device(&dev->bdev);
	partition_publish(dev->bdev.name, 0x0);
}

int emmc_init(void)
{
	int err = MMC_ERR_NONE;
	struct mmc_host *host;
	struct mmc_card *card;
	int boot_part_nblks = 0;
	int rpmb_part_nblks = 0;

	host = malloc(sizeof(struct mmc_host));
	/* malloc fail */
	ASSERT(host);
	/* construct the block device */
	memset(host, 0, sizeof(struct mmc_host));

	card = malloc(sizeof(struct mmc_card));
	/* malloc fail */
	ASSERT(card);
	/* construct the block device */
	memset(card, 0, sizeof(struct mmc_card));

	err = mmc_init_host(host);

	if (err == MMC_ERR_NONE)
		err = mmc_init_card(host, card);
	/* mmc init fail */
	ASSERT(err == MMC_ERR_NONE);

	err = emmc_boot_prepare(card);
	ASSERT(err == MMC_ERR_NONE);

	mmc_bio_ops("mmc0", EXT_CSD_PART_CFG_DEFT_PART, card->nblks, host, card);
	boot_part_nblks = card->ext_csd.boot_part_sz/card->blklen;
	mmc_bio_ops("mmc0boot0", EXT_CSD_PART_CFG_BOOT_PART_1, boot_part_nblks,
			host, card);
	mmc_bio_ops("mmc0boot1", EXT_CSD_PART_CFG_BOOT_PART_2, boot_part_nblks,
			host, card);
	rpmb_part_nblks = card->ext_csd.rpmb_sz/card->blklen;
	mmc_bio_ops("mmc0rpmb", EXT_CSD_PART_CFG_RPMB_PART, rpmb_part_nblks,
			host, card);

    return err;
}
