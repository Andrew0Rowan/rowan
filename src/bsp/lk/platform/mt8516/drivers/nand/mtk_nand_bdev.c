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
#include <platform/nand.h>
#include <lib/bio.h>
#include <lib/partition.h>
#include <malloc.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <pow2.h>
#include <bits.h>
#include <platform.h>
#include <trace.h>
#include "mtk_nand_nal.h"
#include "mtk_nand_common.h"

#define LOCAL_TRACE 0

/* global variable define */
struct mtk_nand_chip *chip;

struct nand_bdev {
	struct bdev dev;
	int firstblk;
	int blkcnt;
	/* this flag enables users to handler bad blocks */
	int check_bad;
};

static struct nand_devices {
	/* Bad block bitmap */
	unsigned long *bitmap;
	/* Bad block mapping table */
	unsigned short *blkmap;
} nand_devs;

static int nand_reg_subdev(struct bdev *bdev);
static int nand_unreg_subdev(struct bdev *bdev);
static bnum_t blk_map(struct bdev *dev, bnum_t page);

static inline int byte2blk(struct mtk_nand_chip *chip, off_t offset)
{
	return offset / chip->blocksize;
}

static inline off_t blk2byte(struct mtk_nand_chip *chip, int blk)
{
	return blk * chip->blocksize;
}

static inline int pg2blk(struct mtk_nand_chip *chip, int page)
{
	return page / chip->page_per_block;
}

static inline int blk2pg(struct mtk_nand_chip *chip, int blk)
{
	return blk * chip->page_per_block;
}

/* partition range check */
static size_t nand_trim_range(struct bdev *dev, off_t offset, size_t len)
{
	struct nand_bdev *ndev = (struct nand_bdev *)dev;
	/* end block, last block of the partition */
	int eb, peb;

	eb = byte2blk(chip, len + offset - 1);
	peb = ndev->firstblk + ndev->blkcnt - 1;

	/* bio_read/write/erase has checked argument "offset" */
	if (eb > peb)
		return 0;

	return len;
}

/* Calculate the absolute adderss */
static off_t abs_addr(struct bdev *dev, off_t offset)
{
	struct nand_bdev *ndev = (struct bdev *)dev;

	return offset + blk2byte(chip, ndev->firstblk);
}

static ssize_t nand_bwrite(bdev_t *bdev, void *buf, u32 blknr, u32 blks)
{
	struct mtk_nand_ops ops;

	dprintf(INFO, "[%s] %s %s lba %d(%d) pagecnt %d\n", __func__,
			bdev->name, bdev->label, pg2blk(chip, blknr),
			((struct nand_bdev *)bdev)->firstblk, blks);
	memset(&ops, 0, sizeof(ops));
	ops.mode = NAND_OPS_ECC_DMA_POLL;
	ops.offset = (u64)blknr * chip->pagesize;
	ops.len = (u64)blks * chip->pagesize;
	ops.datbuf = buf;

	mtk_nand_write(chip, &ops);

	return (ssize_t)blks * chip->pagesize;
}

static ssize_t nand_bread(bdev_t *bdev, void *buf, u32 blknr, u32 blks)
{
	struct mtk_nand_ops ops;

	dprintf(INFO, "[%s] %s %s lba %d(%d) pagecnt %d\n", __func__,
			bdev->name, bdev->label, pg2blk(chip, blknr),
			((struct nand_bdev *)bdev)->firstblk, blks);
	memset(&ops, 0, sizeof(ops));
	ops.mode = NAND_OPS_ECC_DMA_POLL;
	ops.offset = (u64)blknr * chip->pagesize;
	ops.len = (u64)blks * chip->pagesize;
	ops.datbuf = buf;

	mtk_nand_read(chip, &ops);

	return (ssize_t)blks * chip->pagesize;
}

static ssize_t nand_bio_read(struct bdev *dev, void *_buf, off_t offset, size_t len)
{
	uint8_t *buf = (uint8_t *)_buf;
	ssize_t bytes_read = 0;
	bnum_t lba, pba;
	ssize_t err = 0;
	uint32_t page_per_blk = chip->blocksize / chip->pagesize;
	uint8_t *temp = memalign(16, dev->block_size);
	if (temp == NULL)
		return ERR_NO_MEMORY;

	offset = abs_addr(dev, offset);
	len = nand_trim_range(dev, offset, len);
	if (len == 0)
		return 0;

	/* find the starting block */
	lba = offset / dev->block_size;
	dprintf(INFO, "nand_bio_read: page 0x%x, len 0x%x\n", lba, len);

	/* handle partial first block */
	if ((offset % dev->block_size) != 0) {
		/* Convert to physical address */
		pba = blk_map(dev, lba);
		/* read in the block */
		err = nand_bread(dev, temp, pba, 1);
		if (err < 0) {
			goto err;
		} else if ((size_t)err != dev->block_size) {
			err = ERR_IO;
			goto err;
		}

		/* copy what we need */
		size_t block_offset = offset % dev->block_size;
		size_t tocopy = MIN(dev->block_size - block_offset, len);
		memcpy(buf, temp + block_offset, tocopy);

		/* increment our buffers */
		buf += tocopy;
		len -= tocopy;
		bytes_read += tocopy;
		lba++;
	}

	// If the device requires alignment AND our buffer is not alread aligned.
	bool requires_alignment =
		(dev->flags & BIO_FLAG_CACHE_ALIGNED_READS) &&
			(IS_ALIGNED((size_t)buf, CACHE_LINE) == false);
	/* handle middle blocks */
	if (requires_alignment) {
		while (len >= dev->block_size) {
			/* Convert to physical address */
			pba = blk_map(dev, lba);
			/* do the middle reads */
			err = nand_bread(dev, temp, pba, 1);
			if (err < 0) {
				goto err;
			} else if ((size_t)err != dev->block_size) {
				err = ERR_IO;
				goto err;
			}
			memcpy(buf, temp, dev->block_size);

			buf += dev->block_size;
			len -= dev->block_size;
			bytes_read += dev->block_size;
			lba++;
		}
	} else {
		uint32_t num_blocks = divpow2(len, dev->block_shift);
		uint32_t read_blks;

		while (num_blocks) {
			read_blks = page_per_blk - (lba % page_per_blk);
			read_blks = num_blocks > read_blks ? read_blks : num_blocks;

			/* Convert to physical address */
			pba = blk_map(dev, lba);

			err = nand_bread(dev, buf, pba, read_blks);
			if (err < 0) {
				goto err;
			} else if ((size_t)err != dev->block_size * read_blks) {
				err = ERR_IO;
				goto err;
			}
			buf += err;
			len -= err;
			bytes_read += err;
			lba += read_blks;
			num_blocks -= read_blks;
		}
	}

	/* handle partial last block */
	if (len > 0) {
		/* Convert to physical address */
		pba = blk_map(dev, lba);
		/* read the block */
		err = nand_bread(dev, temp, pba, 1);
		if (err < 0) {
			goto err;
		} else if ((size_t)err != dev->block_size) {
			err = ERR_IO;
			goto err;
		}

		/* copy the partial block from our temp buffer */
		memcpy(buf, temp, len);

		bytes_read += len;
	}

	err:
	free(temp);

	/* return error or bytes read */
	return (err >= 0) ? bytes_read : err;
}

static ssize_t nand_bio_write(struct bdev *dev, const void *_buf, off_t offset, size_t len)
{
	const uint8_t *buf = (const uint8_t *)_buf;
	ssize_t bytes_written = 0;
	bnum_t lba, pba;
	uint32_t page_per_blk = chip->blocksize / chip->pagesize;
	ssize_t err = 0;
	uint8_t *temp = memalign(16, dev->block_size);
	if (temp == NULL)
		return ERR_NO_MEMORY;

	offset = abs_addr(dev, offset);
	len = nand_trim_range(dev, offset, len);
	if (len == 0)
		return 0;

	/* find the starting block */
	lba = offset / dev->block_size;
	dprintf(INFO, "nand_bio_write: page 0x%x, len 0x%x\n", lba, len);

	/* handle partial first block */
	if ((offset % dev->block_size) != 0) {
		/* Convert to physical address */
		pba = blk_map(dev, lba);
		err = nand_bread(dev, temp, pba, 1);
		if (err < 0) {
			goto err;
		} else if ((size_t)err != dev->block_size) {
			err = ERR_IO;
			goto err;
		}

		/* copy what we need */
		size_t block_offset = offset % dev->block_size;
		size_t tocopy = MIN(dev->block_size - block_offset, len);
		memcpy(temp + block_offset, buf, tocopy);

		/* write it back out */
		err = nand_bwrite(dev, temp, pba, 1);
		if (err < 0) {
			goto err;
		} else if ((size_t)err != dev->block_size) {
			err = ERR_IO;
			goto err;
		}

		/* increment our buffers */
		buf += tocopy;
		len -= tocopy;
		bytes_written += tocopy;
		lba++;
	}


	// If the device requires alignment AND our buffer is not alread aligned.
	bool requires_alignment =
		(dev->flags & BIO_FLAG_CACHE_ALIGNED_WRITES) &&
		(IS_ALIGNED((size_t)buf, CACHE_LINE) == false);

	/* handle middle blocks */
	if (requires_alignment) {
		while (len >= dev->block_size) {
			/* Convert to physical address */
			pba = blk_map(dev, lba);
			/* do the middle reads */
			memcpy(temp, buf, dev->block_size);
			err = nand_bwrite(dev, temp, pba, 1);
			if (err < 0) {
				goto err;
			} else if ((size_t)err != dev->block_size) {
				err = ERR_IO;
				goto err;
			}

			buf += dev->block_size;
			len -= dev->block_size;
			bytes_written += dev->block_size;
			lba++;
		}
	} else {
		uint32_t num_blocks = divpow2(len, dev->block_shift);
		uint32_t write_blks;

		while (num_blocks) {
			write_blks = page_per_blk - (lba % page_per_blk);
			write_blks = num_blocks > write_blks ? write_blks : num_blocks;

			/* Convert to physical address */
			pba = blk_map(dev, lba);

			err = nand_bwrite(dev, buf, pba, write_blks);
			if (err < 0) {
				goto err;
			} else if ((size_t)err != dev->block_size * write_blks) {
				err = ERR_IO;
				goto err;
			}
			DEBUG_ASSERT((size_t)err == (write_blks * dev->block_size));

			buf += err;
			len -= err;
			bytes_written += err;
			lba += write_blks;
			num_blocks -= write_blks;
		}
	}

	/* handle partial last block */
	if (len > 0) {
		/* Convert to physical address */
		pba = blk_map(dev, lba);
		/* read the block */
		err = nand_bread(dev, temp, pba, 1);
		if (err < 0) {
			goto err;
		} else if ((size_t)err != dev->block_size) {
			err = ERR_IO;
			goto err;
		}

		/* copy the partial block from our temp buffer */
		memcpy(temp, buf, len);

		/* write it back out */
		err = nand_bwrite(dev, temp, pba, 1);
		if (err < 0) {
			goto err;
		} else if ((size_t)err != dev->block_size) {
			err = ERR_IO;
			goto err;
		}

		bytes_written += len;
	}

err:
	free(temp);
	/* return error or bytes written */
	return (err >= 0) ? bytes_written : err;
}

static ssize_t nand_erase(bdev_t *dev, off_t offset, size_t len)
{
	struct mtk_nand_ops ops;
	u32 blocks;
	ssize_t erase_len = 0, ret;

	len = bio_trim_range(dev, offset, len);

	offset = abs_addr(dev, offset);
	len = nand_trim_range(dev, offset, len);
	if (len == 0)
		return 0;

	if (offset % chip->blocksize)
		return ERR_INVALID_ARGS;
	if (len % chip->blocksize)
		return ERR_INVALID_ARGS;

	blocks = len / chip->blocksize;

	memset(&ops, 0, sizeof(ops));
	ops.mode = NAND_OPS_ERASE_POLL;
	ops.offset = offset;
	ops.len = chip->blocksize;

	dprintf(INFO, "[%s] offset %d(%d) len %dbytes\n", __func__,
			byte2blk(chip, offset), byte2blk(chip, ops.offset), len);

	while (blocks) {
		int ret;
		if (!mtk_nand_block_isbad(chip, (u32)(ops.offset / chip->pagesize))) {
			ret = (ssize_t)mtk_nand_erase(chip, &ops);
			if (ret < 0)
				return ret;
			erase_len += chip->blocksize;
		}
		ops.offset += chip->blocksize;
		blocks--;
	}

	return erase_len;
}

static int nand_bio_ioctl(struct bdev *bdev, int request, void *argp)
{
    int ret = NO_ERROR;

    LTRACEF("dev %p, request %d, argp %p\n", bdev, request, argp);

    switch (request) {
		case NAND_IOCTL_GET_ERASE_SIZE:
			if (likely(argp))
				*(unsigned int *)argp = chip->blocksize;
			else
				ret = ERR_NOT_SUPPORTED;
			break;
		case NAND_IOCTL_REGISTER_SUBDEV:
			ret = nand_reg_subdev(bdev);
			break;
		case NAND_IOCTL_UNREGISTER_SUBDEV:
			ret = nand_unreg_subdev(bdev);
			break;
		case NAND_IOCTL_CHECK_BAD_BLOCK:
			((struct nand_bdev *)bdev)->check_bad = 1;
			break;
		case NAND_IOCTL_IS_BAD_BLOCK:
			{
				off_t offset = *(off_t *)argp;
				struct nand_bdev *ndev = (struct nand_bdev *)bdev;
				int blk;

				blk = ndev->firstblk + byte2blk(chip, offset);
				return bitmap_test(nand_devs.bitmap, blk);
			}
		default:
			return ERR_NOT_SUPPORTED;
    }

    return ret;
}

/* Scan all block and create the bad block bitmap */
static int construct_bitmap(void)
{
	uint32_t block;
	uint32_t blkcnt = pg2blk(chip, chip->lbacnt);

	nand_devs.bitmap = malloc(blkcnt / sizeof(char));
	if (!nand_devs.bitmap) {
		dprintf(CRITICAL, "no enough memory for nand bitmap\n");
		return -ENOMEM;
	}

	memset(nand_devs.bitmap, 0, blkcnt / sizeof(char));

	/* Skip the block0 */
	for (block = 1; block < blkcnt; block++) {
		if (unlikely(mtk_nand_block_isbad(chip, blk2pg(chip, block)))) {
			bitmap_set(nand_devs.bitmap, block);
			dprintf(ALWAYS, "block%d is a bad block!\n", block);
		}
	}

	return 0;
}

static int find_good_blk(int pba)
{
	/* Bad block ratio should be less than 2%,  */
	int maxbits = chip->lbacnt * 2 / 100;
	/* byte offset in a WORD */
	int offs_wd = BITMAP_BIT_IN_WORD(pba);
	/* WORD offset in the bitmap */
	int offs = BITMAP_WORD(pba);
	unsigned long bitmap;
	int retblk = 0;

	if (offs_wd) {
		/* in the middle of a WORD */
		bitmap = nand_devs.bitmap[offs];
		bitmap |= ~(~0UL << offs_wd);
		retblk = bitmap_ffz(&bitmap, BITMAP_BITS_PER_WORD);
		if (retblk >= 0) {
			/* we found a good block */
			pba += (retblk - offs_wd);
		} else {
			/* Keep searching good bits in WORDs behind */
			offs++;
			retblk = bitmap_ffz(&nand_devs.bitmap[offs], maxbits);
			if (retblk >= 0) {
				/* found a good block */
				pba += (BITMAP_BITS_PER_WORD - offs_wd + retblk);
			}
		}
	} else {
		retblk = bitmap_ffz(&nand_devs.bitmap[offs], maxbits);
		if (retblk >= 0) {
			/* found a good block */
			pba = offs * BITMAP_BITS_PER_WORD + retblk;
		}
	}

	if (retblk < 0) {
		dprintf(CRITICAL, "Too many bad blocks!\n");
		return -EIO;
	} else
		return pba;
}

static int construct_bmap(struct nand_bdev *dev)
{
	int lba, pba;
	unsigned short *blkmap;

	pba = dev->firstblk;
	blkmap = &nand_devs.blkmap[dev->firstblk];
	for (lba = 0; lba < dev->blkcnt; lba++) {
		pba = find_good_blk(pba);
		if (pba < 0)
			return -EIO;
		blkmap[lba] = pba;
		pba++;
		if (pba >= dev->firstblk + dev->blkcnt)
			/* end of partition */
			break;
	}

	return 0;
}

static int initialize_nand_bdev(struct bdev *dev, bnum_t start_lba, bnum_t lbacnt)
{
	struct nand_bdev *ndev = (struct nand_bdev *)dev;
	int need_bmap;

	if (!ndev) {
		ndev = malloc(sizeof(struct nand_bdev));
		if (!ndev) {
			dprintf(CRITICAL, "no enough memory for nand_bdev\n");
			return -ENOMEM;
		}
		memset(ndev, 0, sizeof(struct nand_bdev));
		need_bmap = 1;
	} else if (dev->label)
		/* only sub-device have 'label' */
		need_bmap = 1;
	else
		need_bmap = 0;

	ndev->firstblk = pg2blk(chip, start_lba);
	ndev->blkcnt = pg2blk(chip, lbacnt);

	if (need_bmap)
		construct_bmap(ndev);

	dprintf(INFO, "[%s] dev %s %s 1st %d len %d\n", __func__,
			ndev->dev.name, ndev->dev.label,
			ndev->firstblk, ndev->blkcnt);

	return 0;
}

static struct bdev *nand_reg_bio_dev(const char *name, size_t lbasize,
		bnum_t lbacnt, const char *label)
{
	struct nand_bdev *ndev;
	struct bdev *dev;

	ndev = malloc(sizeof(struct nand_bdev));
	if (!ndev) {
		dprintf(CRITICAL, "no enough memory for nand_bdev\n");
		return NULL;
	}

	memset(ndev, 0, sizeof(struct nand_bdev));
	dev = &ndev->dev;
	bio_initialize_bdev(dev, name, lbasize, lbacnt, 0, NULL, BIO_FLAGS_NONE);
	dev->read = nand_bio_read;
	dev->write = nand_bio_write;
	dev->read_block = nand_bread;
	dev->write_block = nand_bwrite;
	dev->erase = nand_erase;
	dev->erase_byte = 0xff;
	dev->ioctl = nand_bio_ioctl;
	if (label)
		dev->label = strdup(label);
	bio_register_device(dev);
	dprintf(INFO, "[%s] %s %s %ld %d\n", __func__, name, label,
			lbasize, lbacnt);

	return dev;
}

/* We register BIO device instead of BIO subdev device here,
 * it is a easy way to manage the partitions
 */
static int nand_reg_subdev(struct bdev *bdev)
{
	int i, ret, start_lba;
	struct bdev *dev, *new;
	char devname[32];

	for (i = 0; i < 128; i++) {
		sprintf(devname, "%sp%d", bdev->name, i + 1);
		dev = bio_open(devname);
		if (!dev)
			break;

		/* find the start_lba in structure subdev_t */
		start_lba = *(bnum_t *)((unsigned long)&dev[1] + sizeof(bdev_t *) +
					sizeof(bio_erase_geometry_info_t));
		new = nand_reg_bio_dev(devname, dev->block_size, dev->block_count,
								dev->label);
		if (!new)
			return -EIO;

		dprintf(INFO, "[%s] %s %s\n", __func__, new->name, new->label);
		ret = initialize_nand_bdev(new, start_lba, dev->block_count);
		if (ret)
			return ret;

		/* Remove the subdevs from the list */
		bio_unregister_device(dev);
		bio_close(dev);
	}

	return 0;
}

static int nand_unreg_subdev(struct bdev *bdev)
{
	struct nand_bdev *ndev = (struct nand_bdev *)bdev;

	dprintf(INFO, "[%s] %s %s\n", __func__, bdev->name, bdev->label);
	free(ndev->dev.label);
	free(ndev);

	return 0;
}

/* Convert logical address to physical block address */
static bnum_t blk_map(struct bdev *dev, bnum_t page)
{
	struct nand_bdev *ndev = (struct nand_bdev *)dev;
	int remain, blk;

	/* User check bad block itself, just return the page address */
	if (ndev->check_bad)
		return page;

	blk = nand_devs.blkmap[pg2blk(chip, page)];
	remain = page & (chip->page_per_block - 1);

	if (pg2blk(chip, page) && blk == 0) {
		/* the mapping address is 0, means the good block run out */
		dprintf(CRITICAL, "%s(%s): Too many bad blocks, can not erase/program\n",
				ndev->dev.name, ndev->dev.label);
		return -EIO;
	}

	return blk2pg(chip, blk) + remain;
}

int nand_init_device()
{
	struct bdev *dev;
	u32 lbacnt;
	int ret;

	ret = mtk_nand_init(&chip);
	if (ret) {
		dprintf(CRITICAL, "nand device init error (%d)!\n", ret);
		return ret;
	}

	ret = construct_bitmap();
	if (ret)
		return ret;

	/* bad block mapping table */
	nand_devs.blkmap = (unsigned short *)NAND_DRAM_BUF_BAD_MAP_ADDR;
	memset(nand_devs.blkmap, 0, NAND_DRAM_BUF_BAD_MAP_SIZE);

	/* 4blocks for bbt scan, 4blocks reserved for SGPT */
	lbacnt = chip->lbacnt - (8 * chip->blocksize / chip->pagesize);
	dev = nand_reg_bio_dev("nand0", chip->lbasize, lbacnt, NULL);
	if (!dev)
		return -EIO;

	/* nand0 */
	ret = initialize_nand_bdev(dev, 0, lbacnt);
	if (ret)
		return ret;

	/* MBR */
	ret = initialize_nand_bdev(NULL, 0, 384);
	if (ret)
		return ret;

	partition_publish("nand0", 0);

	return 0;
}

