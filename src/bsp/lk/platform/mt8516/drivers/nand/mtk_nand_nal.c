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
#include <platform/mtk_timer.h>
#include <platform/mt8516.h>
#include <kernel/mutex.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <malloc.h>
#include "mtk_nand_nal.h"
#include "mtk_nand_common.h"
#include "mtk_nfi_hal.h"


#define NOTALIGNED(x)	((x & (chip->subpagesize - 1)) != 0)

static int mtk_nand_do_read_ops(struct mtk_nand_chip *chip, struct mtk_nand_ops *ops);

static int mtk_nand_is_dram_buf(u8* buf)
{
	return (buf < DRAM_BASE_VIRT) ? 0 : 1;
}

static int mtk_nand_get_controller(struct mtk_nand_chip *chip)
{
	struct mtk_nfc *nfc = nand_get_controller_data(chip);

	mutex_acquire(&nfc->lock);

	return 0;
}

static int mtk_nand_release_controller(struct mtk_nand_chip *chip)
{
	struct mtk_nfc *nfc = nand_get_controller_data(chip);

	mutex_release(&nfc->lock);

	return 0;
}

static int mtk_nand_wait_func(struct mtk_nand_chip *chip, int polling)
{
	struct mtk_nfc *nfc = nand_get_controller_data(chip);
	int status;
	unsigned long timeo = 1000000;

	chip->cmdfunc(chip, NAND_CMD_STATUS, -1, -1);

	if (!polling) {
		if (chip->wait_busy_irq(chip))
			dprintf(CRITICAL, "nand dev ready timeout\n");
	} else {
		if (!check_with_timeout(chip->dev_ready(chip), timeo))
			dprintf(CRITICAL, "nand dev ready timeout\n");
	}

	status = (int)chip->read_byte(chip);

	return status;
}

void mtk_nand_wait_ready(struct mtk_nand_chip *chip)
{
	unsigned long timeo = 1000000;

	if (!check_with_timeout(chip->dev_ready(chip), timeo))
		dprintf(CRITICAL, "nand dev ready timeout\n");

}


static int mtk_nand_check_wp(struct mtk_nand_chip *chip)
{
	/* Check the WP bit */
	chip->cmdfunc(chip, NAND_CMD_STATUS, -1, -1);
	return (chip->read_byte(chip) & NAND_STATUS_WP) ? 0 : 1;
}

static int mtk_nand_block_bad(struct mtk_nand_chip *chip, u64 ofs)
{
	int page, res = 0, i = 0;
	u16 bad;

	if (chip->bbt_options & NAND_BBT_SCANLASTPAGE)
		ofs += chip->blocksize - chip->pagesize;

	page = (int)(ofs / chip->pagesize) % chip->page_per_chip;

	do {
		chip->cmdfunc(chip, NAND_CMD_READOOB, chip->badblockpos, page);
		bad = chip->read_byte(chip);

		if (chip->badblockbits == 8)
			res = bad != 0xFF;

		ofs += chip->pagesize;
		page = (int)(ofs / chip->pagesize) % chip->page_per_chip;
		i++;
	} while (!res && i < 2 && (chip->bbt_options & NAND_BBT_SCAN2NDPAGE));

	return res;
}

static int mtk_nand_block_checkbad(struct mtk_nand_chip *chip, u32 page)
{
	struct mtk_nand_ops ops;
	int ret;

	/* be careful of randomizer on case, may get a wrong feedback by chip->read_byte */
	if (!(chip->options & NAND_NEED_SCRAMBLING)) {
		ret = chip->block_bad(chip, (u64)page * chip->pagesize);
	} else {
		memset(&ops, 0, sizeof(ops));
		ops.mode = NAND_OPS_ECC_DMA_POLL;
		/* check the 1st page of the block */
		ops.offset = (u64)(page & ~(chip->page_per_block - 1)) * chip->pagesize;
		ops.len = chip->pagesize;
		ops.datbuf = chip->databuf;
		mtk_nand_do_read_ops(chip, &ops);
		ret = (chip->oob_poi[chip->badblockpos] != 0xFF);
	}

	return ret;
}

int mtk_nand_block_isbad(struct mtk_nand_chip *chip, u32 page)
{
	int ret;

	mtk_nand_get_controller(chip);
	ret = mtk_nand_block_checkbad(chip, page);
	mtk_nand_release_controller(chip);

	return ret;
}

int nand_reset(struct mtk_nand_chip *chip, int chipnr)
{
	/* power on sequence delay */
	udelay(300);

	/*
	 * The CS line has to be released before we can apply the new NAND
	 * interface settings, hence this weird ->select_chip() dance.
	 */
	chip->select_chip(chip, chipnr);
	chip->cmdfunc(chip, NAND_CMD_RESET, -1, -1);
	chip->select_chip(chip, -1);

	return 0;
}

static inline int mtk_nand_opcode_8bits(unsigned int command)
{
	switch (command) {
	case NAND_CMD_READID:
	case NAND_CMD_PARAM:
	case NAND_CMD_GET_FEATURES:
	case NAND_CMD_SET_FEATURES:
		return 1;
	default:
		break;
	}
	return 0;
}

static void mtk_nand_command_lp(struct mtk_nand_chip *chip, unsigned int command,
			    int column, int page_addr)
{
	/* Emulate NAND_CMD_READOOB */
	if (command == NAND_CMD_READOOB) {
		column += chip->pagesize;
		command = NAND_CMD_READ0;
	}

	/* Command latch cycle */
	chip->cmd_ctrl(chip, command, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

	if (column != -1 || page_addr != -1) {
		int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;

		/* Serially input address */
		if (column != -1) {
			chip->cmd_ctrl(chip, column, ctrl);
			ctrl &= ~NAND_CTRL_CHANGE;

			/* Only output a single addr cycle for 8bits opcodes. */
			if (!mtk_nand_opcode_8bits(command))
				chip->cmd_ctrl(chip, column >> 8, ctrl);
		}
		if (page_addr != -1) {
			chip->cmd_ctrl(chip, page_addr, ctrl);
			chip->cmd_ctrl(chip, page_addr >> 8,
				       NAND_NCE | NAND_ALE);
			/* One more address cycle for devices > 128MiB */
			if (chip->chipsize > (128 << 20))
				chip->cmd_ctrl(chip, page_addr >> 16,
					       NAND_NCE | NAND_ALE);
		}
	}
	chip->cmd_ctrl(chip, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	/*
	 * Program and erase have their own busy handlers status, sequential
	 * in and status need no delay.
	 */
	switch (command) {
	case NAND_CMD_CACHEDPROG:
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_STATUS:
		return;

	case NAND_CMD_RNDOUT:
		/* No ready / busy check necessary */
		chip->cmd_ctrl(chip, NAND_CMD_RNDOUTSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(chip, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
		return;

	case NAND_CMD_READ0:
		chip->cmd_ctrl(chip, NAND_CMD_READSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(chip, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);

		/* This applies to read commands */
	default:
		break;
	}

	mtk_nand_wait_ready(chip);
}

static int mtk_nand_block_markbad(struct mtk_nand_chip *chip, u64 ofs)
{
	return 0;
}


int mtk_nand_scan_bbt(struct mtk_nand_chip *chip)
{
	/* implement if need */
	return 0;
}

static void mtk_nand_set_defaults(struct mtk_nand_chip *chip)
{
	/* chip_delay setup set 20us if not */
	chip->chip_delay = 20;

	/* command function*/
	chip->cmdfunc = mtk_nand_command_lp;

	/* wait function */
	chip->waitfunc = mtk_nand_wait_func;

	/* bad block check */
	chip->block_bad = mtk_nand_block_bad;
	/* bad block mark */
	chip->block_markbad = mtk_nand_block_markbad;

	/* scan bbt */
	chip->scan_bbt = mtk_nand_scan_bbt;

	/* variable defalut value */
	chip->badblockbits = 8;
	chip->badblockpos = 0;

	chip->activechip = -1;

}

static u16 mtk_onfi_crc16(u16 crc, u8 const *p, size_t len)
{
	int i;
	while (len--) {
		crc ^= *p++ << 8;
		for (i = 0; i < 8; i++)
			crc = (crc << 1) ^ ((crc & 0x8000) ? 0x8005 : 0);
	}

	return crc;
}

/**
 * skip_spaces - Removes leading whitespace from @str.
 * @str: The string to be stripped.
 *
 * Returns a pointer to the first non-whitespace character in @str.
 */
static char *skip_spaces(const char *str)
{
	while (isspace(*str))
		++str;
	return (char *)str;
}

/**
 * strim - Removes leading and trailing whitespace from @s.
 * @s: The string to be stripped.
 *
 * Note that the first trailing whitespace is replaced with a %NUL-terminator
 * in the given string @s. Returns a pointer to the first non-whitespace
 * character in @s.
 */
static char *strim(char *s)
{
	size_t size;
	char *end;

	size = strlen(s);
	if (!size)
		return s;

	end = s + size - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	return skip_spaces(s);
}

/* Sanitize ONFI strings so we can safely print them */
static void nand_sanitize_string(uint8_t *s, size_t len)
{
	ssize_t i;

	/* Null terminate */
	s[len - 1] = 0;

	/* Remove non printable chars */
	for (i = 0; i < len - 1; i++) {
		if (s[i] < ' ' || s[i] > 127)
			s[i] = '?';
	}

	/* Remove trailing spaces */
	strim(s);
}

static u16 nand_le16_to_cpu(u8 *buf, int offset)
{
	u16 val;

	val = (u16)buf[offset] |((u16)buf[offset + 1] << 8);

	return val;
}

static u32 nand_le32_to_cpu(u8 *buf, int offset)
{
	u32 val;

	val = (u32)buf[offset] |((u32)buf[offset + 1] << 8)
		|((u32)buf[offset + 2] << 16) | ((u32)buf[offset + 3] << 24) ;

	return val;
}

/*
 * Check if the NAND chip is ONFI compliant, returns 1 if it is, 0 otherwise.
 */
static int mtk_nand_flash_detect_onfi(struct mtk_nand_chip *chip, struct mtk_nand_flash_dev *type)
{
	u8 *temp_buf;
	int i, j;
	u32 val, onfi_version = 0;
	static u8 name[NAND_CHIP_NAME_LEN];
	int async_timing;
	int ret = 0;

	temp_buf= malloc(MTK_NAND_ONFI_PARAMS_LEN);
	if (temp_buf == NULL) {
		dprintf(CRITICAL, "malloc temp buffer %x failed\n", MTK_NAND_ONFI_PARAMS_LEN);
		return -1;
	}

	memset(temp_buf, 0, MTK_NAND_ONFI_PARAMS_LEN);

	/* Try ONFI for unknown chip or LP */
	chip->cmdfunc(chip, NAND_CMD_READID, 0x20, -1);
	if (chip->read_byte(chip) != 'O' || chip->read_byte(chip) != 'N' ||
		chip->read_byte(chip) != 'F' || chip->read_byte(chip) != 'I') {
		dprintf(INFO, "Not ONFI CHIP\n");
		ret = 1;
		goto err_end;
	}

	chip->cmdfunc(chip, NAND_CMD_PARAM, 0, -1);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < MTK_NAND_ONFI_PARAMS_LEN; j++) {
			temp_buf[j] = chip->read_byte(chip);

		}
		val = nand_le16_to_cpu(temp_buf, ONFI_PAR_CRC_OFS);
		if (mtk_onfi_crc16(ONFI_CRC_BASE, temp_buf, 254) ==
				val) {
			dprintf(INFO, "ONFI CRC OK :%d\n", i);
			break;
		}
	}

	if (i == 3) {
		dprintf(CRITICAL, "Could not find valid ONFI parameter page; aborting\n");
		ret = 2;
		goto err_end;
	}

	/* Check version */
	onfi_version = nand_le16_to_cpu(temp_buf, ONFI_PAR_REVISION_OFS);
	if (onfi_version & (1 << 5))
		onfi_version = 23;
	else if (onfi_version & (1 << 4))
		onfi_version = 22;
	else if (onfi_version & (1 << 3))
		onfi_version = 21;
	else if (onfi_version & (1 << 2))
		onfi_version = 20;
	else if (onfi_version & (1 << 1))
		onfi_version = 10;

	if (!onfi_version) {
		dprintf(CRITICAL, "unsupported ONFI version: %d\n", val);
		ret = 3;
		goto err_end;
	}

	type->name = &name[0];
	/* type->name = malloc(NAND_CHIP_NAME_LEN);
	if (type->name)  {
		dprintf(CRITICAL, "Failed to malloc type->name: %d\n", NAND_CHIP_NAME_LEN);
		ret = 4;
		goto err_end;
	} */

	memset(type->name, '\0', NAND_CHIP_NAME_LEN);
	nand_sanitize_string(&temp_buf[ONFI_PAR_MODEL_OFS], 20);
	memcpy(type->name, &temp_buf[ONFI_PAR_MODEL_OFS], 20);

	type->pagesize = nand_le32_to_cpu(temp_buf, ONFI_PAR_DATA_PER_PAGE_OFS);

	/*
	 * pages_per_block and blocks_per_lun may not be a power-of-2 size
	 * (don't ask me who thought of this...). MTD assumes that these
	 * dimensions will be power-of-2, so just truncate the remaining area.
	 */
	type->erasesize = nand_le32_to_cpu(temp_buf, ONFI_PAR_PAGE_PER_BLK_OFS);
	type->erasesize *= type->pagesize;

	type->oobsize = nand_le16_to_cpu(temp_buf, ONFI_PAR_SPARE_PER_PGAE_OFS);

	/* See erasesize comment */
	type->chipsize = nand_le32_to_cpu(temp_buf, ONFI_PAR_BLK_PER_LUN_OFS);
	type->chipsize *= (u32)((u32)(type->erasesize>>10) * (u32)temp_buf[ONFI_PAR_NUM_LUN_OFS]);


	async_timing = nand_le16_to_cpu(temp_buf, ONFI_PAR_TIMING_MODE_OFS);
	if (async_timing == 0x3F) {
		if (type->oobsize > 64) {
			type->acctiming = 0x10804111;
			dprintf(INFO, "support timing mode 5 but  force mode 4\n");
		} else {
			type->acctiming = 0x10401011;
			dprintf(INFO, "support async timing mode 5\n");
		}
	} else if (async_timing == 0x1F) {
		type->acctiming = 0x10804111;
		dprintf(INFO, "support async timing mode 4\n");
	} else {
		dprintf(CRITICAL, "unsupported async timing mode: %d\n", async_timing);
		ret = 5;
		goto err_end;
	}

	val = temp_buf[ONFI_PAR_BIT_PER_CELL_OFS];
	if (val != 1) {
		dprintf(CRITICAL, "Not SLC bits_per_cell:%d\n", val);
		ret = 6;
		goto err_end;
	}

	/* set default parameter here */
	type->bits_per_cell = 1;
	type->ecc_size = 1024;
	type->fdmeccsize = 8;
	type->bbt_options = 0;
	type->options = 0;

	/* type->ecc_strength =
		((type->oobsize/(type->pagesize /type->ecc_size) - type->fdmeccsize)<<3)/14; */
	switch (type->oobsize/(type->pagesize /type->ecc_size))
	{
		case 32:
			type->ecc_strength = 12;
			break;
		case 56:
			type->ecc_strength = 24;
			break;
		case 64:
			type->ecc_strength = 32;
			break;
		default:
			dprintf(CRITICAL, "Unsupport spare size:%d Bytes\n", type->oobsize);
			break;
	}
	dprintf(ALWAYS,
	    "Name:[%s]  pagesize:[%d], oobsize:[%d] erasesize:[%d] chipsize:[%d]KB ecc_strength:%d acctiming:0x%x\n",
	     type->name, type->pagesize, type->oobsize,
	     type->erasesize, type->chipsize, type->ecc_strength, type->acctiming);
err_end:
	free(temp_buf);
	return ret;
}

int mtk_nand_flash_get(struct mtk_nand_chip *chip, int maxchips)
{
	u32 i;
	u8 id_data[8];
	struct mtk_nand_flash_dev *type = nand_flash_devs;
	struct mtk_nand_flash_dev *type_tmp = NULL;
	int ret = 0;

	nand_reset(chip, 0);

	/* Select the device */
	chip->select_chip(chip, 0);

	/* Send the command for reading device ID */
	chip->cmdfunc(chip, NAND_CMD_READID, 0x00, -1);

	/* Read entire ID string */
	for (i = 0; i < 8; i++) {
		id_data[i] = chip->read_byte(chip);
		dprintf(INFO, "nand id[%d] [%x] \n", i, id_data[i]);
	}

	for (; type->name != NULL; type++) {
		if (!strncmp(type->id, id_data, type->id_len)) {
			dprintf(ALWAYS, "nand found [%s] \n", type->name);
			break;
		}
	}

	chip->select_chip(chip, -1);
	if (!type->name) {
		if ((id_data[0] == NAND_MFR_MICRON)
		    ||(id_data[0] == NAND_MFR_HYNIX)
		    || (id_data[0] == NAND_MFR_MACRONIX)) {
		    	type_tmp = malloc(sizeof(struct mtk_nand_flash_dev));
			if (type_tmp == NULL) {
				dprintf(CRITICAL, "Failed to malloc mtk_nand_flash_dev %d \n",
					sizeof(struct mtk_nand_flash_dev));
				return -ENOMEM;
			}

			if (mtk_nand_flash_detect_onfi(chip, type_tmp) != 0) {
				ret = -ENODEV;
				goto unsupport;
			}
			type = type_tmp;

		} else
			return -ENODEV;
	}

	chip->numchips = 1;

	/* Check for a chip array */
	for (i = 1; i < maxchips; i++) {
		/* See comment in nand_get_flash_type for reset */
		nand_reset(chip, i);

		chip->select_chip(chip, i);
		/* Send the command for reading device ID */
		chip->cmdfunc(chip, NAND_CMD_READID, 0x00, -1);
		/* Read manufacturer and device IDs */
		if (id_data[0] != chip->read_byte(chip) ||
		    id_data[1] != chip->read_byte(chip)) {
			chip->select_chip(chip, -1);
			break;
		}
		dprintf(ALWAYS, "chip %d is found\n", i);
		chip->select_chip(chip, -1);
		chip->numchips++;
	}

	/* set nand chip parameters */
	chip->pagesize = type->pagesize;
	chip->oobsize = type->oobsize;
	if ((chip->pagesize == 4096) && (chip->oobsize == 224)) {
		chip->oobsize = 208;
		dprintf(ALWAYS, "Force 224bytes oobsize to 208bytes for MT8516\n");
	}

	chip->bits_per_cell = type->bits_per_cell;
	/* KB to B */
	chip->chipsize = ((u64)type->chipsize) << 10;
	chip->blocksize = type->erasesize;
	chip->bbt_options |= type->bbt_options;
	chip->options |= type->options;
	chip->ecc_size = type->ecc_size;
	chip->ecc_strength = type->ecc_strength;
	chip->fdm_ecc_size = type->fdmeccsize;

	chip->totalsize = i * chip->chipsize;

	chip->acctiming = type->acctiming;

	chip->ecc_steps = chip->pagesize / chip->ecc_size;
	if (nand_is_slc(chip)) {
		if (chip->ecc_steps == 2)
			chip->subpagesize = chip->pagesize / 2;
		else if (chip->ecc_steps > 2)
			chip->subpagesize = chip->pagesize / 4;
		else
			chip->subpagesize = chip->pagesize;
	}
	chip->page_per_block = chip->blocksize / chip->pagesize;
	chip->page_per_chip = chip->chipsize / chip->pagesize;

	chip->lbasize = chip->pagesize;
	/* change lbacnt if want to reserve blocks */
	chip->lbacnt = chip->totalsize / chip->lbasize;

	chip->databuf = (u8 *)NAND_DRAM_BUF_DATABUF_ADDR;//memalign(16, chip->pagesize + chip->oobsize);
	if (!chip->databuf) {
		ret = -ENOMEM;
		goto unsupport;
	}
	chip->oob_poi = chip->databuf + chip->pagesize;

unsupport:
	if (type_tmp != NULL)
		free(type_tmp);

	return ret;
}

int mtk_nand_scan(struct mtk_nand_chip *chip, int maxchips)
{
	int ret;

	/* Set the defaults */
	mtk_nand_set_defaults(chip);

	ret = mtk_nand_flash_get(chip, maxchips);
	if (ret) {
		dprintf(CRITICAL, "no nand device found\n");
		return ret;
	}

	return 0;
}

static int mtk_nand_fill_ecc_oob()
{
	return 0;
}

static int mtk_nand_fill_free_oob()
{
	return 0;
}

static int mtk_nand_transfer_ecc_oob()
{
	return 0;
}

static int mtk_nand_transfer_free_oob()
{
	return 0;
}

static int mtk_nand_do_read_ops(struct mtk_nand_chip *chip, struct mtk_nand_ops *ops)
{
	int chipnr, page, realpage, col, bytes, aligned;
	u8 *buf, *oob_ecc, *oob_free, *bufpoi;
	u64 readlen = ops->len, from = ops->offset;
	u32 max_bitflips = 0;
	u32 ecc_failures = chip->stats.failed;
	int ret, ecc_fail = 0;

	chipnr = (int)(from / chip->chipsize);
	chip->select_chip(chip, chipnr);

	realpage = (int)(from / chip->pagesize);
	page = realpage % chip->page_per_chip;

	col = (int)(from & (chip->pagesize - 1));

	buf = ops->datbuf;
	oob_ecc = ops->oobeccbuf;
	oob_free = ops->oobfreebuf;

	while (1) {
		bytes = min(chip->pagesize - col, readlen);
		aligned = (bytes == chip->pagesize);
		/* workaround for dma to sram */
		if(!mtk_nand_is_dram_buf(buf))
			aligned = 0;
		bufpoi = aligned ? buf : chip->databuf;

		/* send read page command */
		dprintf(INFO, "[nand] read page %d chip %d\n", page, chipnr);
		mtk_nfc_randomizer_enable(chip, page, RAND_DECODE, 0);
		chip->cmdfunc(chip, NAND_CMD_READ0, 0x00, page);

		if (!aligned) {
			if (ops->mode == NAND_OPS_ECC_DMA_IRQ)
				ret = chip->read_subpage_ecc_dma_irq(chip, col, bytes, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_DMA_POLL)
				ret = chip->read_subpage_ecc_dma_polling(chip, col, bytes, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_PIO_IRQ)
				ret = chip->read_subpage_ecc_pio_irq(chip, col, bytes, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_PIO_POLL)
				ret = chip->read_subpage_ecc_pio_polling(chip, col, bytes, bufpoi, page);
		} else {
			if (ops->mode == NAND_OPS_RAW_DMA_IRQ)
				ret = chip->read_page_raw_dma_irq(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_RAW_DMA_POLL)
				ret = chip->read_page_raw_dma_polling(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_RAW_PIO_IRQ)
				ret = chip->read_page_raw_pio_irq(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_RAW_PIO_POLL)
				ret = chip->read_page_raw_pio_polling(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_DMA_IRQ)
				ret = chip->read_page_ecc_dma_irq(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_DMA_POLL)
				ret = chip->read_page_ecc_dma_polling(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_PIO_IRQ)
				ret = chip->read_page_ecc_pio_irq(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_PIO_POLL)
				ret = chip->read_page_ecc_pio_polling(chip, bufpoi, page);
		}
		mtk_nfc_randomizer_disable(chip);
		if (ret < 0)
			break;

		max_bitflips = max(max_bitflips, ret);

		if (chip->stats.failed - ecc_failures) {
			ecc_fail = 1;
			dprintf(ALWAYS, "[nand] read page %d chip %d UNCORRECT\n", page, chipnr);
			break;
		}

		if (!aligned)
			memcpy(buf, chip->databuf + col, bytes);
		if (!oob_ecc)
			mtk_nand_transfer_ecc_oob();
		else if (!oob_free)
			mtk_nand_transfer_free_oob();

		readlen -= bytes;
		buf += bytes;

		if (!readlen)
			break;

		/* For subsequent reads align to page boundary */
		col = 0;
		/* Increment page address */
		realpage++;

		page = realpage % chip->page_per_chip;
		/* Check, if we cross a chip boundary */
		if (!page) {
			chipnr++;
			chip->select_chip(chip, -1);
			chip->select_chip(chip, chipnr);
		}
	}
	chip->select_chip(chip, -1);

	if (ecc_fail)
		return -EBADMSG;

	return max_bitflips;
}

int mtk_nand_read(struct mtk_nand_chip *chip, struct mtk_nand_ops *ops)
{
	int ret;

	mtk_nand_get_controller(chip);
	ret = mtk_nand_do_read_ops(chip, ops);
	mtk_nand_release_controller(chip);

	return ret;
}

static int mtk_nand_do_write_ops(struct mtk_nand_chip *chip, struct mtk_nand_ops *ops)
{
	int chipnr, realpage, page, col, bytes, aligned;
	u32 writelen = ops->len;
	u64 to = ops->offset;
	u8 *buf = ops->datbuf;
	u8 *oob_ecc = ops->oobeccbuf;
	u8 *oob_free = ops->oobfreebuf;
	u8 *bufpoi;
	int ret = 0, status, polling_wait = 1;

	/* Reject writes, which are not subpage aligned */
	if (NOTALIGNED(to) || NOTALIGNED(ops->len)) {
		dprintf(CRITICAL, "attempt to write non page aligned data (offset 0x%llx, len 0x%llx)\n", to, ops->len);
		return -EINVAL;
	}

	col = to & (chip->pagesize - 1);
	chipnr = (int)(to / chip->chipsize);
	chip->select_chip(chip, chipnr);

	/* Check, if it is write protected */
	if (mtk_nand_check_wp(chip)) {
		ret = -EIO;
		dprintf(CRITICAL, "write protected!\n");
		goto err_out;
	}

	realpage = (int)(to / chip->pagesize);
	page = realpage % chip->page_per_chip;

	while (1) {
		bytes = min(chip->pagesize - col, writelen);
		aligned = (bytes == chip->pagesize);
		/* workaround for dma to sram */
		if(!mtk_nand_is_dram_buf(buf))
			aligned = 0;
		bufpoi = aligned ? buf : chip->databuf;

		if (!aligned) {
			memset(chip->databuf, 0xff, chip->pagesize);
			memcpy(chip->databuf + col, buf, bytes);
		}

		memset(chip->oob_poi, 0xff, chip->oobsize);
		if (!oob_ecc)
			mtk_nand_fill_ecc_oob();
		else if(!oob_free)
			mtk_nand_fill_free_oob();

		dprintf(INFO, "[nand] write page %d chip %d\n", page, chipnr);
		mtk_nfc_randomizer_enable(chip, page, RAND_ENCODE, 0);
		chip->cmdfunc(chip, NAND_CMD_SEQIN, 0x00, page);

		if (!aligned) {
			if (ops->mode == NAND_OPS_ECC_DMA_IRQ) {
				polling_wait = 0;
				ret = chip->write_subpage_ecc_dma_irq(chip, col, bytes, bufpoi, page);
			} else if (ops->mode == NAND_OPS_ECC_DMA_POLL)
				ret = chip->write_subpage_ecc_dma_polling(chip, col, bytes, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_PIO_IRQ) {
				polling_wait = 0;
				ret = chip->write_subpage_ecc_pio_irq(chip, col, bytes, bufpoi, page);
			} else if (ops->mode == NAND_OPS_ECC_PIO_POLL)
				ret = chip->write_subpage_ecc_pio_polling(chip, col, bytes, bufpoi, page);
		} else {
			if (ops->mode == NAND_OPS_RAW_DMA_IRQ) {
				polling_wait = 0;
				ret = chip->write_page_raw_dma_irq(chip, bufpoi, page);
			} else if (ops->mode == NAND_OPS_RAW_DMA_POLL)
				ret = chip->write_page_raw_dma_polling(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_RAW_PIO_IRQ) {
				polling_wait = 0;
				ret = chip->write_page_raw_pio_irq(chip, bufpoi, page);
			} else if (ops->mode == NAND_OPS_RAW_PIO_POLL)
				ret = chip->write_page_raw_pio_polling(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_DMA_IRQ) {
				polling_wait = 0;
				ret = chip->write_page_ecc_dma_irq(chip, bufpoi, page);
			} else if (ops->mode == NAND_OPS_ECC_DMA_POLL)
				ret = chip->write_page_ecc_dma_polling(chip, bufpoi, page);
			else if (ops->mode == NAND_OPS_ECC_PIO_IRQ) {
				polling_wait = 0;
				ret = chip->write_page_ecc_pio_irq(chip, bufpoi, page);
			} else if (ops->mode == NAND_OPS_ECC_PIO_POLL)
				ret = chip->write_page_ecc_pio_polling(chip, bufpoi, page);
		}
		mtk_nfc_randomizer_disable(chip);
		if (ret < 0)
			break;

		chip->cmdfunc(chip, NAND_CMD_PAGEPROG, -1, -1);
		status = chip->waitfunc(chip, polling_wait);
		if (status & NAND_STATUS_FAIL) {
			ret = -EIO;
			dprintf(CRITICAL, "write failed at page 0x%x\n", realpage);
			goto err_out;
		}

		writelen -= bytes;
		if (!writelen)
			break;

		col = 0;
		buf += bytes;
		realpage++;

		page = realpage % chip->page_per_chip;
		/* Check, if we cross a chip boundary */
		if (!page) {
			chipnr++;
			chip->select_chip(chip, -1);
			chip->select_chip(chip, chipnr);
		}
	}

err_out:
	chip->select_chip(chip, -1);

	return ret;
}

int mtk_nand_write(struct mtk_nand_chip *chip, struct mtk_nand_ops *ops)
{
	int ret;

	mtk_nand_get_controller(chip);
	ret = mtk_nand_do_write_ops(chip, ops);
	mtk_nand_release_controller(chip);

	return ret;
}

static int mtk_nand_do_erase_ops(struct mtk_nand_chip *chip, struct mtk_nand_ops *ops)
{
	u64 offset = ops->offset;
	u64 eraselen = ops->len;
	int page, status, ret = 0, chipnr, polling_wait = 0;

	if ((offset % chip->blocksize) || (eraselen % chip->blocksize)) {
		dprintf(CRITICAL, "erase is not aligned (off 0x%llx, len 0x%llx)\n", offset, eraselen);
		return -EINVAL;
	}

	page = (int)(offset / chip->pagesize);
	chipnr = (int)(offset / chip->chipsize);

	chip->select_chip(chip, chipnr);

	/* Check, if it is write protected */
	if (mtk_nand_check_wp(chip)) {
		ret = -EIO;
		dprintf(CRITICAL, "write protected!\n");
		goto err_out;
	}

	while (1) {
		if (mtk_nand_block_checkbad(chip, page)) {
			ret = -EIO;
			dprintf(CRITICAL, "attempt to erase bad block at page 0x%x\n", page);
			goto err_out;
		}

		dprintf(INFO, "[nand] erase page %d chip %d\n", page, chipnr);
		chip->cmdfunc(chip, NAND_CMD_ERASE1, -1, (page % chip->page_per_chip));
		chip->cmdfunc(chip, NAND_CMD_ERASE2, -1, -1);
		if (ops->mode == NAND_OPS_ERASE_IRQ)
			polling_wait = 0;
		else if (ops->mode == NAND_OPS_ERASE_POLL)
			polling_wait = 1;
		status = chip->waitfunc(chip, polling_wait);

		if (status & NAND_STATUS_FAIL) {
			ret = -EIO;
			dprintf(CRITICAL, "erase failed at page 0x%x\n", page);
			goto err_out;
		}

		eraselen -= chip->blocksize;
		if (!eraselen)
			break;
		page += chip->page_per_block;

		if (eraselen && !(page % chip->page_per_chip)) {
			chipnr++;
			chip->select_chip(chip, -1);
			chip->select_chip(chip, chipnr);
		}
	}
err_out:
	chip->select_chip(chip, -1);

	return ret;
}

int mtk_nand_erase(struct mtk_nand_chip *chip, struct mtk_nand_ops *ops)
{
	int ret;

	mtk_nand_get_controller(chip);
	ret = mtk_nand_do_erase_ops(chip, ops);
	mtk_nand_release_controller(chip);

	return ret;
}

void mtk_nand_unit_test(struct mtk_nand_chip *chip)
{
	struct mtk_nand_ops ops;
	u8 *buf;
	int total_block, i, j;
	int ret = 0;

	total_block = chip->chipsize/chip->blocksize;
	dprintf(CRITICAL, "mtk_nand_unit_test start total_block: %d\n", total_block);

	buf = malloc(chip->pagesize*2);
	if (buf == NULL) {
		dprintf(CRITICAL, "malloc buf failed: %d\n", chip->pagesize);
		return;
	}

	for (i = 0; i < chip->pagesize*2; i++)
		buf[i] = i;

	for (i = 0; i < total_block; i++) {

		if (mtk_nand_block_isbad(chip, i*chip->page_per_block)) {
			dprintf(CRITICAL, "check bad blk: %d\n", i);
			continue;
		}

		memset(&ops, 0, sizeof(ops));
		ops.mode = NAND_OPS_ERASE_POLL;
		ops.offset = i * chip->blocksize;
		ops.len = chip->blocksize;

		ret = mtk_nand_erase(chip, &ops);
		if (ret) {
			dprintf(CRITICAL, "Erase failed at blk: %d\n", i);
			continue;
		}

		for (j = i*chip->page_per_block;
			j < i*chip->page_per_block+ chip->page_per_block; j++) {

			memset(&ops, 0, sizeof(ops));
			ops.mode = NAND_OPS_ECC_DMA_POLL;
			ops.offset = (u64)j * chip->pagesize;
			ops.len = (u64)chip->pagesize;
			ops.datbuf = buf;

			ret = mtk_nand_write(chip, &ops);
			if (ret) {
				dprintf(CRITICAL, "Write failed at blk:%d, page:%d\n", i, j);
				break;
			}

			memset(&ops, 0, sizeof(ops));
			ops.mode = NAND_OPS_ECC_DMA_POLL;
			ops.offset = (u64)j * chip->pagesize;
			ops.len = (u64)chip->pagesize;
			ops.datbuf = buf;

			ret = mtk_nand_read(chip, &ops);
			if (ret) {
				dprintf(CRITICAL, "Read failed at blk:%d page:%d ret:%d\n", i, j, ret);
				break;
			}
		}
		dprintf(CRITICAL, "mtk_nand_unit_test blk: %d OK\n", i);
	}

	dprintf(CRITICAL, "mtk_nand_unit_test start end\n");
}

int mtk_nand_init(struct mtk_nand_chip **ext_nand)
{
	int ret;

	ret = mtk_nfc_nand_chip_init(ext_nand);
	if(ret)
		dprintf(CRITICAL, "chip init failed ret 0x%x\n", ret);

	/* mtk_nand_unit_test(*ext_nand); */

	return ret;
}
