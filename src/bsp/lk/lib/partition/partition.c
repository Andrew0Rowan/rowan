/*
 * Copyright (c) 2009 Travis Geiselbrecht
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
#include <arch.h>
#include <assert.h>
#include <compiler.h>
#include <debug.h>
#include <lib/bio.h>
#include <lib/cksum.h>
#include <lib/partition.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpt.h"
#include <platform/nand.h>

struct chs {
	uint8_t c;
	uint8_t h;
	uint8_t s;
} __PACKED;

struct mbr_part {
	uint8_t status;
	struct chs start;
	uint8_t type;
	struct chs end;
	uint32_t lba_start;
	uint32_t lba_length;
} __PACKED;

struct gpt_header {
	uint64_t first_usable_lba;
	uint64_t backup_header_lba;
	uint32_t partition_entry_size;
	uint32_t header_size;
	uint32_t max_partition_count;
};

#define WRITE_BUF_SIZE  (512)
#define GPT_BUFFER_SIZE (17920)
static char gpt_buffer[GPT_BUFFER_SIZE] = {0};

static status_t validate_mbr_partition(bdev_t *dev, const struct mbr_part *part)
{
	/* check for invalid types */
	if (part->type == 0)
		return -1;
	/* check for invalid status */
	if (part->status != 0x80 && part->status != 0x00)
		return -1;

	/* make sure the range fits within the device */
	if (part->lba_start >= dev->block_count)
		return -1;
	if ((part->lba_start + part->lba_length) > dev->block_count)
		return -1;

	/* that's about all we can do, MBR has no other good way to see if it's valid */

	return 0;
}

/*
 * Parse the gpt header and get the required header fields
 * Return 0 on valid signature
 */
static unsigned int
partition_parse_gpt_header(unsigned char *buffer, struct gpt_header* header)
{
	/* Check GPT Signature */
	if (((uint32_t *) buffer)[0] != GPT_SIGNATURE_2 ||
	    ((uint32_t *) buffer)[1] != GPT_SIGNATURE_1)
		return 1;

	header->header_size = GET_LWORD_FROM_BYTE(&buffer[HEADER_SIZE_OFFSET]);
	header->backup_header_lba =
	    GET_LLWORD_FROM_BYTE(&buffer[BACKUP_HEADER_OFFSET]);
	header->first_usable_lba =
	    GET_LLWORD_FROM_BYTE(&buffer[FIRST_USABLE_LBA_OFFSET]);
	header->max_partition_count =
	    GET_LWORD_FROM_BYTE(&buffer[PARTITION_COUNT_OFFSET]);
	header->partition_entry_size =
	    GET_LWORD_FROM_BYTE(&buffer[PENTRY_SIZE_OFFSET]);

	return 0;
}

int partition_publish(const char *device, off_t offset)
{
	int err = 0;
	int count = 0;

	// clear any partitions that may have already existed
	partition_unpublish(device);

	bdev_t *dev = bio_open(device);
	if (!dev) {
		printf("partition_publish: unable to open device\n");
		return -1;
	}

	// get a dma aligned and padded block to read info
	STACKBUF_DMA_ALIGN(buf, dev->block_size);

	/* sniff for MBR partition types */
	do {
		unsigned int i, j, n;
		int gpt_partitions_exist = 0;

		err = bio_read(dev, buf, offset, 512);
		if (err < 0)
			goto err;

		/* sniff for DEV header */
		if (strncmp("BOOTLOADER!", buf, 11) == 0) {
			/* skip 128 pages to find MBR & GPT */
			offset += 128 * dev->block_size;

			err = bio_read(dev, buf, offset, 512);
			if (err < 0)
				goto err;
		}

		/* look for the aa55 tag */
		if (buf[510] != 0x55 || buf[511] != 0xaa)
			break;

		/* see if a partition table makes sense here */
		struct mbr_part part[4];
		memcpy(part, buf + 446, sizeof(part));

#if LK_DEBUGLEVEL >= INFO
		dprintf(INFO, "mbr partition table dump:\n");
		for (i=0; i < 4; i++) {
			dprintf(INFO, "\t%i: status 0x%hhx, type 0x%hhx, start 0x%x, len 0x%x\n", i, part[i].status, part[i].type, part[i].lba_start, part[i].lba_length);
		}
#endif

		/* validate each of the partition entries */
		for (i=0; i < 4; i++) {
			if (validate_mbr_partition(dev, &part[i]) >= 0) {
				// publish it
				char subdevice[128];

				/* Type 0xEE indicates end of MBR and GPT partitions exist */
				if(part[i].type==0xee) {
					gpt_partitions_exist = 1;
					break;
				}

				sprintf(subdevice, "%sp%d", device, i);

				err = bio_publish_subdevice(device, subdevice, part[i].lba_start, part[i].lba_length);
				if (err < 0) {
					dprintf(INFO, "error publishing subdevice '%s'\n", subdevice);
					continue;
				}
				count++;
			}
		}

		if(!gpt_partitions_exist) break;
		dprintf(INFO, "found GPT\n");

		err = bio_read(dev, buf, offset + dev->block_size, dev->block_size);
		if (err < 0)
			goto err;

		struct gpt_header gpthdr;
		err = partition_parse_gpt_header(buf, &gpthdr);
		if (err) {
			/* Check the backup gpt */

			uint64_t backup_header_lba = dev->block_count - 1;
			err = bio_read(dev, buf, (backup_header_lba * dev->block_size), dev->block_size);
			if (err < 0) {
				dprintf(CRITICAL, "GPT: Could not read backup gpt from mmc\n");
				break;
			}

			err = partition_parse_gpt_header(buf, &gpthdr);
			if (err) {
				dprintf(CRITICAL, "GPT: Primary and backup signatures invalid\n");
				break;
			}
		}

		uint32_t part_entry_cnt = dev->block_size / ENTRY_SIZE;
		uint64_t partition_0 = GET_LLWORD_FROM_BYTE(&buf[PARTITION_ENTRIES_OFFSET]);
		/* Read GPT Entries */
		for (i = 0; i < (ROUNDUP(gpthdr.max_partition_count, part_entry_cnt)) / part_entry_cnt; i++) {
			err = bio_read(dev, buf, offset + (partition_0 * dev->block_size) + (i * dev->block_size),
							dev->block_size);

			if (err < 0) {
				dprintf(CRITICAL,
					"GPT: mmc read card failed reading partition entries.\n");
				break;
			}

			for (j = 0; j < part_entry_cnt; j++) {
				unsigned char type_guid[PARTITION_TYPE_GUID_SIZE];
				unsigned char name[MAX_GPT_NAME_SIZE];
				unsigned char UTF16_name[MAX_GPT_NAME_SIZE];
				uint64_t first_lba, last_lba, size;

				// guid
				memcpy(&type_guid,
			       &buf[(j * gpthdr.partition_entry_size)],
			       PARTITION_TYPE_GUID_SIZE);
				if (type_guid[0]==0 && type_guid[1]==0) {
					i = ROUNDUP(gpthdr.max_partition_count, part_entry_cnt);
					break;
				}

				// size
				first_lba = GET_LLWORD_FROM_BYTE(&buf[(j * gpthdr.partition_entry_size) + FIRST_LBA_OFFSET]);
				last_lba = GET_LLWORD_FROM_BYTE(&buf[(j * gpthdr.partition_entry_size) + LAST_LBA_OFFSET]);
				size = last_lba - first_lba + 1;

				// name
				memset(&UTF16_name, 0x00, MAX_GPT_NAME_SIZE);
				memcpy(UTF16_name, &buf[(j * gpthdr.partition_entry_size) +
					PARTITION_NAME_OFFSET], MAX_GPT_NAME_SIZE);

				/*
				 * Currently partition names in *.xml are UTF-8 and lowercase
				 * Only supporting english for now so removing 2nd byte of UTF-16
				 */
				for (n = 0; n < MAX_GPT_NAME_SIZE / 2; n++) {
					name[n] = UTF16_name[n * 2];
				}

				//dprintf(CRITICAL, "got part '%s' size=%llu!\n", name, size);
				char subdevice[128];
				sprintf(subdevice, "%sp%d", device, count+1);

				err = bio_publish_subdevice(device, subdevice, first_lba, size);
				if (err < 0) {
					dprintf(INFO, "error publishing subdevice '%s'\n", name);
					continue;
				}

				bdev_t *partdev = bio_open(subdevice);
				partdev->label = strdup((char*)name);
				partdev->is_gpt = true;

				count++;
			}
		}

		/* Notify driver bio device is registered */
		bio_ioctl(dev, NAND_IOCTL_REGISTER_SUBDEV, NULL);
	} while (0);

	bio_close(dev);

err:
	return (err < 0) ? err : count;
}

int partition_unpublish(const char *device)
{
	int i;
	int count;
	bdev_t *dev;
	char devname[512];

	count = 0;
	for (i=0; i < NUM_PARTITIONS; i++) {
		sprintf(devname, "%sp%d", device, i);

		dev = bio_open(devname);
		if (!dev)
			continue;

		bio_unregister_device(dev);
		bio_close(dev);

		bio_ioctl(dev, NAND_IOCTL_UNREGISTER_SUBDEV, NULL);

		count++;
	}

	return count;
}

static void
patch_gpt(uint8_t *gptImage, uint64_t density, uint32_t array_size,
          uint32_t max_part_count, uint32_t part_entry_size, uint32_t block_size)
{
    uint8_t *partition_entry_array_start;
    unsigned char *primary_gpt_header;
    unsigned char *secondary_gpt_header;
    unsigned long long card_size_sec;
    int total_part = 0, phy_last_part = 0;
    unsigned long last_part_offset;
    unsigned int crc_value;
    unsigned long long last_part_first_lba, last_part_last_lba;

    /* Get size of storage */
    card_size_sec = (density) / block_size;

    /* Generate second gpt header */
    memcpy(gptImage + (block_size * 2) + array_size,
           gptImage + block_size,
           block_size);

    /* Patching primary header */
    primary_gpt_header = (gptImage + block_size);
    PUT_LONG_LONG(primary_gpt_header + BACKUP_HEADER_OFFSET,
              ((long long)(card_size_sec - 1)));
    PUT_LONG_LONG(primary_gpt_header + LAST_USABLE_LBA_OFFSET,
              ((long long)(card_size_sec - 34)));

    /* Patching backup GPT */
    secondary_gpt_header = primary_gpt_header + block_size + array_size;
    PUT_LONG_LONG(secondary_gpt_header + PRIMARY_HEADER_OFFSET,
                    ((long long)(card_size_sec - 1)));
    PUT_LONG_LONG(secondary_gpt_header + LAST_USABLE_LBA_OFFSET,
                    ((long long)(card_size_sec - 34)));
    PUT_LONG_LONG(secondary_gpt_header + PARTITION_ENTRIES_OFFSET,
                    ((long long)(card_size_sec - 33)));
    PUT_LONG_LONG(secondary_gpt_header + BACKUP_HEADER_OFFSET,
                    ((long long)(1)));

    /* Find last partition */
    while (*(primary_gpt_header + block_size + total_part * ENTRY_SIZE) != 0) {
        if (GET_LLWORD_FROM_BYTE(primary_gpt_header + block_size + total_part * ENTRY_SIZE + FIRST_LBA_OFFSET) >=
            GET_LLWORD_FROM_BYTE(primary_gpt_header + block_size + phy_last_part * ENTRY_SIZE + FIRST_LBA_OFFSET)) {
            phy_last_part = total_part;
        }
        total_part++;
    }

    /* Patching last partition */
    last_part_offset = (unsigned long)(primary_gpt_header + block_size + phy_last_part * ENTRY_SIZE);
    /* last partition size should align 64KB */
    last_part_first_lba =  GET_LLWORD_FROM_BYTE(last_part_offset + PARTITION_ENTRY_FIRST_LBA);
    last_part_last_lba = (card_size_sec - 34) - (((card_size_sec - 34) - last_part_first_lba + 1) % 128);
    PUT_LONG_LONG(last_part_offset + PARTITION_ENTRY_LAST_LBA, (long long)last_part_last_lba);

    /* Updating CRC of the Partition entry array in both headers */
    partition_entry_array_start = primary_gpt_header + block_size;
    crc_value = (unsigned int)crc32(0x0, partition_entry_array_start,
                    max_part_count * part_entry_size);
    PUT_LONG(primary_gpt_header + PARTITION_CRC_OFFSET, crc_value);
    PUT_LONG(secondary_gpt_header + PARTITION_CRC_OFFSET, crc_value);

    /* Clearing CRC fields to calculate */
    PUT_LONG(primary_gpt_header + HEADER_CRC_OFFSET, 0);
    crc_value = (unsigned int)crc32(0x0, primary_gpt_header, 92);
    PUT_LONG(primary_gpt_header + HEADER_CRC_OFFSET, crc_value);

    PUT_LONG(secondary_gpt_header + HEADER_CRC_OFFSET, 0);
    crc_value = (unsigned int)crc32(0x0, secondary_gpt_header, 92);
    PUT_LONG(secondary_gpt_header + HEADER_CRC_OFFSET, crc_value);
}

static int write_gpt(const char *device, uint32_t size, uint8_t *gptImage, uint32_t block_size)
{
    int ret;
    uint64_t device_density;
    char *w_buf;
    unsigned int i,j;
    unsigned int iStep;
    char *sgpt_buf;
    uint64_t sgpt_addr;

    bdev_t *dev = bio_open(device);
    if (!dev) {
        dprintf(INFO, "write_gpt: unable to open device\n");
        return -1;
    }

    w_buf = malloc(WRITE_BUF_SIZE);
    if(!w_buf)
    {
        dprintf(CRITICAL, "Failed to Allocate memory(512B) to write partition table\n");
        goto end;
    }

    /* check size */
    if (size < (MIN_PARTITION_ARRAY_SIZE + (block_size * 3))) {
        dprintf(INFO,
            "write_gpt check size fail:size(%d) < MIN_PARTITION_ARRAY_SIZE(%d) + block_size(%d) * 3\n",
            size, MIN_PARTITION_ARRAY_SIZE, block_size);
        ret = -1;
        goto end;
    }

    /* Get the density of the storage device */
    device_density = (uint64_t)dev->block_count * dev->block_size;

    /* Patching the primary and the backup header of the GPT table */
    patch_gpt(gptImage, device_density, MIN_PARTITION_ARRAY_SIZE, 128, 128, block_size);

#if 1
    /* write primary */
    iStep = 0;
    for(i=0; i < (((block_size*2) + MIN_PARTITION_ARRAY_SIZE) / WRITE_BUF_SIZE); i++)
    {
        memcpy(w_buf,(gptImage + iStep),WRITE_BUF_SIZE);


        ret = bio_write(dev, (unsigned int *)w_buf, iStep, WRITE_BUF_SIZE);
        iStep += WRITE_BUF_SIZE;

        if (ret < 0) {
            dprintf(CRITICAL, "Failed to write primary\n");
            goto end;
        }
    }

    /* write secondary */
    iStep = 0;
    sgpt_buf = (unsigned int *)(gptImage + (block_size*2));
    sgpt_addr = device_density - (MIN_PARTITION_ARRAY_SIZE + block_size);
    for(i=0; i < ((MIN_PARTITION_ARRAY_SIZE + block_size) / WRITE_BUF_SIZE); i++)
    {
        memcpy(w_buf, (sgpt_buf + iStep) ,WRITE_BUF_SIZE);
        //hexdump(w_buf, WRITE_BUF_SIZE);

        ret = bio_write(dev,(unsigned int *)w_buf, (sgpt_addr + iStep),WRITE_BUF_SIZE);

        iStep += WRITE_BUF_SIZE;

        if (ret < 0) {
            dprintf(CRITICAL, "Failed to write secondary\n");
            goto end;
        }
    }
#else
    /* write primary */
    ret = bio_write(dev, (unsigned int *)gptImage, 0, ((block_size*2) + MIN_PARTITION_ARRAY_SIZE));

    if (ret < 0) {
        dprintf(INFO, "Failed to write primary\n");
        goto end;
    }

    /* write secondary */
    ret = bio_write(dev, (unsigned int *)(gptImage + (block_size*2)),
                       (device_density - (MIN_PARTITION_ARRAY_SIZE + block_size)),
                       (MIN_PARTITION_ARRAY_SIZE + block_size));
    if (ret < 0) {
        dprintf(INFO, "Failed to write secondary\n");
        goto end;
    }
#endif

end:
    free(w_buf);
    bio_close(dev);
    return ret;
}

int partition_update(const char *device, off_t offset, const char *data, size_t sz)
{
    uint8_t *buffer;
    int err = 0;
    size_t rsize;
    unsigned int gpt_size;

    bdev_t *dev = bio_open(device);
    if (!dev) {
        dprintf(INFO, "partition_update: unable to open device\n");
        return -1;
    }
    gpt_size = MIN_PARTITION_ARRAY_SIZE + dev->block_size * 2;

    // get a dma aligned and padded block to read info
    STACKBUF_DMA_ALIGN(buf, dev->block_size);

    /* sniff for MBR partition types */
    unsigned int i;
    int gpt_partitions_exist = 0;

    memcpy(buf, data + offset, dev->block_size);


    /* look for the aa55 tag */
    if (buf[510] != 0x55 || buf[511] != 0xaa) {
        err = -1;
        dprintf(INFO, "look for the aa55 tag fail\n");
        goto err;
    }

    /* see if a partition table makes sense here */
    struct mbr_part part[4];
    memcpy(part, buf + 446, sizeof(part));

    /* check each entry to find GPT exist or not */
    for (i=0; i < 4; i++) {
        if (validate_mbr_partition(dev, &part[i]) >= 0) {
            /* Type 0xEE indicates end of MBR and GPT partitions exist */
            if(part[i].type==0xee) {
                gpt_partitions_exist = 1;
                break;
            }
        }
    }

    if (!gpt_partitions_exist) {
        err = -1;
        dprintf(INFO, "gpt partition is not exist\n");
        goto err;
    }
    dprintf(INFO, "found GPT\n");

    memcpy(buf, data + offset + dev->block_size, dev->block_size);

    struct gpt_header gpthdr;
    err = partition_parse_gpt_header(buf, &gpthdr);
    if (err) {
        err = -1;
        dprintf(INFO, "GPT: Primary signatures invalid\n");
        goto err;
    }

    /* check whether to resize userdata partition */
    if (gpthdr.backup_header_lba == (dev->block_count - 1))
    {
        err = -1;
        dprintf(INFO, "GPT: Already up to date\n");
        goto err;
    }

#if 0
    buffer = malloc(gpt_size + dev->block_size);
    if (!buffer) {
        err = -1;
        dprintf(CRITICAL, "Failed to Allocate memory to read partition table\n");
        goto err;
    }

    memcpy(buffer, data + offset, gpt_size);
    err = write_gpt(dev->name, gpt_size + dev->block_size, buffer, dev->block_size);

free_buf:
    free(buffer);
#endif

    memcpy(gpt_buffer, data + offset, gpt_size);
    err = write_gpt(dev->name, gpt_size + dev->block_size, gpt_buffer, dev->block_size);

    if (err < 0) {
        dprintf(INFO, "Failed to write partition table\n");
    }

err:
    if (err < 0) {
        rsize = (size_t)bio_write(dev, data, 0, sz);
        if (rsize != sz) {
            err = -1;
            dprintf(INFO, "bio_write size is not match!\n");
        } else
            err = 0;
    } else {
        rsize = (size_t)bio_write(dev, data + gpt_size, gpt_size, sz-gpt_size);
        if (rsize != sz-gpt_size) {
            err = -1;
            dprintf(INFO, "gpt update finish, bio_write size is not match!\n");
        }
    }

    bio_close(dev);

    return err;
}
