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

#include <app.h>
#include <assert.h>
#include <libfdt.h>
#include <kernel/event.h>
#include <kernel/thread.h>
#include <kernel/vm.h>
#include <platform.h>
#include <platform/mt_uart.h>
#include <platform/mtk_key.h>
#include <platform/mtk_wdt.h>

#include "buffer.h"
#include "fit.h"
#include <lib/bio.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>


/* BL33 load and entry point address */
#define CFG_BL33_LOAD_EP_ADDR   (BL33_ADDR)
#define ERR_ADDR    (0xffffffff)

#define RECOVERY_BOOT 1
#define FASTBOOT_BOOT 2
#define NORMAL_BOOT   0
#define UPG_SUCCEED   2

typedef void (*jump_func_type)(u32 addr ,u32 arg1, u32 arg2) __attribute__ ((__noreturn__));

struct fit_load_data {
    char *part_name;
    char *recovery_part_name;
    void *buf;
    u32 boot_mode;
    ulong kernel_entry;
    ulong dtb_load;
    ulong trustedos_entry;
};

typedef struct boot_flag boot_flag;
struct boot_flag
{
	unsigned int lastboot;
	unsigned int usea;
	unsigned int useb;
	unsigned int current;
};

extern int errno;

u32 set_currently_boot_flag(int last_flag, u32 current_flag, const char *part_name)
{
	int ret = 0;
	long len = 0;
	u32 writesize = 2048;
	int index = -1;
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	char *buf;
	boot_flag set_flag;

	/* read partition */
	struct bdev *nand_MISC = bio_open_by_label("MISC");
	buf = malloc(writesize);
	memset(buf, 0, writesize);
	
	len = bio_read(nand_MISC, buf, 0, sizeof(boot_flag));
	if (len < 0) {
		dprintf(CRITICAL, "%s read error. LINE: %d\n", part_name, __LINE__);
		return -1;
	}

	/* dump flag for debug */
	dprintf(CRITICAL, "current boot flag is %d\n", current_flag);

	/* set currently flag to buf */
	set_flag.lastboot = last_flag;
	set_flag.current = current_flag;
	set_flag.usea = -1;
	set_flag.useb = -1;

	memset(buf, 0, writesize);
	memcpy(buf, (void*)&set_flag, sizeof(boot_flag));
   
	/* write buf to offset 0, which size is 2048 */
	len = bio_write(nand_MISC, (char*)buf, 0, (u32)writesize);
	if (len <=  0) {
		dprintf(CRITICAL, "nand write fail, return : %d,  error: \n",len, strerror(errno));
		dprintf(CRITICAL, "buf: %s\n", buf);
		ret = -1;
		goto err;
	}
	dprintf(CRITICAL, "set flag: lastboot = %d, use A = %d, use B = %d, current = %d\n", set_flag.lastboot, set_flag.usea, set_flag.useb, set_flag.current);
	if (buf) {
        free(buf);
        buf = NULL;
    }
	return 0;
err:
	if (buf) {
        free(buf);
        buf = NULL;
    }
	return ret;
}

u32 check_boot_partition(const char *part_name)
{
	int ret = 0;
	boot_flag flag;
	u32 boot = 0;
	
	struct bdev *nand_MISC = bio_open_by_label("MISC");
	int len = -1;
	char *buf;

	if (!nand_MISC) {
		printf("failed to open MISC\n");
		return 0;
	}
	printf("open MISC successfully\n");

	/* read partition */
	buf = malloc(sizeof(boot_flag));
	len = bio_read(nand_MISC, buf, 0, sizeof(boot_flag));
	memcpy(&flag, (void*)buf, sizeof(boot_flag));
	if (len < 0) {
		dprintf(CRITICAL, "read %s: boot flag read error. LINE: %d\n", part_name, __LINE__);
		return -1;
	}

	/* dump flag for debug */
	dprintf(CRITICAL, "lastboot = %d, use A = %d, use B = %d, current = %d\n", flag.lastboot, flag.usea, flag.useb, flag.current);

	/* make dicision */
	if (flag.lastboot == 0) {
		if (flag.useb == UPG_SUCCEED){
			boot = 1;
			dprintf(CRITICAL,"***last succeed boot from A system,upgrade B succeed***\n");
			dprintf(CRITICAL,"***now boot from system B***\n");
		}
		else{
			boot = 0;
			dprintf(CRITICAL,"***last succeed boot from A system,upgrade B failed or no upgrade B***\n");
			dprintf(CRITICAL,"***now boot from system A***\n");
		}
	} else if (flag.lastboot == 1) {
		if (flag.usea == UPG_SUCCEED){
			boot = 0;
			dprintf(CRITICAL,"***last succeed boot from B system,upgrade A succeed***\n");
			dprintf(CRITICAL,"***now boot from system A***\n");
		}
		else{
			boot = 1;
			dprintf(CRITICAL,"***last succeed boot from B system,upgrade A failed or no upgrade A***\n");
			dprintf(CRITICAL,"***now boot from system B***\n");
		}
	} else {
		dprintf(CRITICAL, "boot flag is not match, use default boot partition\n");
		boot = 0;
	}

	if ((flag.current != boot) || (flag.usea == UPG_SUCCEED) || (flag.useb == UPG_SUCCEED)) {
		ret = bio_erase(nand_MISC, 0, 0x20000);  //erase one block
		printf("bio erase ret %d\n", ret);
		ret = set_currently_boot_flag(flag.lastboot, boot, part_name);
		if(ret!=0)
			dprintf(CRITICAL, "set flags fail. LINE: %d\n", __LINE__);
	}
	if (buf) {
			free(buf);
			buf = NULL;
		}
	return boot;
}


static void setup_bl33(uint *bl33, ulong fdt, ulong kernel_ep)
{
    bl33[12] = (ulong)fdt;
    bl33[14] = (unsigned)0;
    bl33[16] = (unsigned)0;
    bl33[18] = (unsigned)0;
    bl33[20] = (ulong)kernel_ep;
    bl33[21] = (ulong)0;
    bl33[22] = (unsigned)MACH_TYPE;
}

static int extract_fdt(void *fdt, int size)
{
    int ret = -1;

    /* DTB maximum size is 2MB */
    ret = fdt_open_into(fdt, fdt, size);
    if (ret) {
        dprintf(CRITICAL, "open fdt failed\n");
        return ret;
    }
    ret = fdt_check_header(fdt);
    if (ret) {
        dprintf(CRITICAL, "check fdt failed\n");
        return ret;
    }

    return ret;
}

static int emmc_cmdlineoverlay(void* boot_dtb, char* cmdline, int len)
{
	int chosen_node_offset = 0;
	int ret = -1;
	ret = extract_fdt(boot_dtb, MAX_DTB_SIZE);
	if (ret != 0) {
		dprintf(CRITICAL, "extract_fdt error.\n");
		return -1;
	}

	chosen_node_offset = fdt_path_offset(boot_dtb, "/chosen");
	char *cmdline_read;
	int lenth;
	cmdline_read = fdt_getprop(boot_dtb, chosen_node_offset, "bootargs", &lenth);
	dprintf(CRITICAL, "dtsi cmdline: %s ,lenth:%d\n", cmdline_read, strlen(cmdline_read));
	char *pos1;
	char *pos2;
	pos1 = strstr(cmdline_read,"root=/dev/mmcblk0p");
	if( pos1 == NULL) {
		dprintf(CRITICAL, "no root= in cmdline, error!\n");
		return -1;
	}
	pos2 = strstr(pos1, " ");
	if( pos2 == NULL) {
		dprintf(CRITICAL, "no root= in cmdline, error!\n");
		return -1;
	}
	if( (pos2 - pos1 -18) <=0 ) {
		dprintf(CRITICAL, "no root= in cmdline, error!\n");
		return -1;
	}

	char mtdnum_str[3];
	char mtdnum_str_new[3];
	strncpy(mtdnum_str, pos1+18, (pos2-pos1-18));
	mtdnum_str[pos2-pos1-18] = '\0';
	int mtdnum = atoi(mtdnum_str);
	mtdnum ++;
	sprintf(mtdnum_str_new, "%d", mtdnum);
	if (mtdnum >= 10) {
		strncpy(pos1+18, mtdnum_str_new, 2);
	}
	else {
		strncpy(pos1+18, mtdnum_str_new, 1);
	}
	printf("cmdline new: %s , length: %d", cmdline_read, strlen(cmdline_read));
	ret = fdt_setprop(boot_dtb, chosen_node_offset, "bootargs", cmdline_read, strlen(cmdline_read) + 1);
	if (ret != 0) {
		dprintf(CRITICAL, "fdt_setprop error.\n");
		return -1;
	}
	ret = fdt_pack(boot_dtb);
	if (ret != 0) {
		dprintf(CRITICAL, "fdt_pack error.\n");
		return -1;
	}

	return 0;
}

static int cmdlineoverlay(void* boot_dtb, char* cmdline, int len)
{
	int chosen_node_offset = 0;
	int ret = -1;
	ret = extract_fdt(boot_dtb, MAX_DTB_SIZE);
	if (ret != 0) {
		dprintf(CRITICAL, "extract_fdt error.\n");
		return -1;
	}
	
	chosen_node_offset = fdt_path_offset(boot_dtb, "/chosen");
	char *cmdline_read;
	int lenth;
	cmdline_read = fdt_getprop(boot_dtb, chosen_node_offset, "bootargs", &lenth);
	dprintf(CRITICAL, "dtsi cmdline: %s ,lenth:%d\n", cmdline_read, strlen(cmdline_read));
	char *pos1;
	char *pos2;
	pos1 = strstr(cmdline_read,"ubi.mtd=");
	if( pos1 == NULL) {
		dprintf(CRITICAL, "no ubi.mtd= in cmdline, error!\n");
		return -1;
	}
	pos2 = strstr(pos1, " ");
	if( pos2 == NULL) {
		dprintf(CRITICAL, "no ubi.mtd= in cmdline, error!\n");
		return -1;
	}
	if( (pos2 - pos1 -8) <=0 ) {
		dprintf(CRITICAL, "no ubi.mtd= in cmdline, error!\n");
		return -1;
	}
	
	char mtdnum_str[3];
	char mtdnum_str_new[3];
	strncpy(mtdnum_str, pos1+8, (pos2-pos1-8));
	mtdnum_str[pos2-pos1-8] = '\0';
	int mtdnum = atoi(mtdnum_str);
	mtdnum ++;
	sprintf(mtdnum_str_new, "%d", mtdnum);
	if (mtdnum >= 10) {
		strncpy(pos1+8, mtdnum_str_new, 2);
	}
	else {
		strncpy(pos1+8, mtdnum_str_new, 1);
	}
	printf("cmdline new: %s , length: %d", cmdline_read, strlen(cmdline_read));
	ret = fdt_setprop(boot_dtb, chosen_node_offset, "bootargs", cmdline_read, strlen(cmdline_read) + 1);
	if (ret != 0) {
		dprintf(CRITICAL, "fdt_setprop error.\n");
		return -1;
	}
	ret = fdt_pack(boot_dtb);
	if (ret != 0) {
		dprintf(CRITICAL, "fdt_pack error.\n");
		return -1;
	}

	return 0;
}


static bool download_check(void)
{
    if (check_fastboot_mode()) {
        set_clr_fastboot_mode(false);
        return true;
    } else {
        return (check_uart_enter() || check_download_key());
    }
}

static bool recovery_check(void)
{
    if (check_recovery_mode()) {
        set_clr_recovery_mode(false);
        return true;
    } else
        return false;
}

static int fit_load_images(void *fit, struct fit_load_data *fit_data)
{
    int ret;
    int verify_kernel;
    int verify_ramdisk;
    int verify_fdt;
    int verify_trustedos;

    /* TODO: decide verify policy with config. currently verify kernel only. */
    verify_kernel = 1;
    verify_ramdisk = 1;
    verify_fdt = 1;
    verify_trustedos = 1;

    ret = fit_load_image(NULL, "kernel", fit, NULL, &fit_data->kernel_entry);
    if (verify_kernel && ret &&(ret != -ENOENT)) {
        dprintf(CRITICAL, "%s load kernel failed\n", fit_data->part_name);
        return ret;
    }

    ret = fit_load_image(NULL, "tee", fit, NULL, &fit_data->trustedos_entry);
    if (verify_trustedos && ret&& (ret != -ENOENT)) {
        dprintf(CRITICAL, "%s load trustedos failed\n", fit_data->part_name);
        return ret;
    }

    ret = fit_load_image(NULL, "ramdisk", fit, NULL, NULL);
    if (verify_ramdisk && ret &&(ret != -ENOENT)) {
        dprintf(CRITICAL, "%s load ramdisk failed\n", fit_data->part_name);
        return ret;
    }

    ret = fit_load_image(NULL, "fdt", fit, &fit_data->dtb_load, NULL);
    if (ret &&(ret != -ENOENT))
        fit_data->dtb_load = ERR_ADDR;

    if (verify_fdt && ret &&(ret != -ENOENT)) {
        dprintf(CRITICAL, "%s load fdt failed\n", fit_data->part_name);
        return ret;
    }

    return 0;
}


static int fit_load_thread(void *arg)
{
    int err;
    void *fit;
    struct fit_load_data *fit_data = (struct fit_load_data *)arg;

    if (fit_data->boot_mode == FASTBOOT_BOOT) {
        fit = fit_data->buf;
        err = fit_load_images(fit, fit_data);
        return err;
    }

    while (fit_data->boot_mode == NORMAL_BOOT) {
        err = fit_get_image(fit_data->part_name, &fit, fit_data->buf);
        if (err)
            break;

        err = fit_load_images(fit, fit_data);
        if (err)
            break;

        return 0;
    }

	//remove recovery mode here----
	//dprintf(CRITICAL, "%s try recovery mode !!\n", fit_data->part_name);
    // RECOVERY_BOOT
    //err = fit_get_image(fit_data->recovery_part_name, &fit, fit_data->buf);
    //if (err)
        //return err;

    //err = fit_load_images(fit, fit_data);

    return err;
}

extern void ext_boot(void);
static void fitboot_task(const struct app_descriptor *app, void *args)
{
    void *fit;
    struct fit_load_data tz, bootimg;
    thread_t *tz_t, *bootimg_t;
    int ret_tz, ret_bootimg;
    int ret;
    u32 boot_mode = NORMAL_BOOT;

    uint bl33[] = { 0xea000005,  /* b BL33_32_ENTRY  | ands x5, x0, x0  */
                    0x58000160,  /* .word 0x58000160 | ldr x0, _X0      */
                    0x58000181,  /* .word 0x58000181 | ldr x1, _X1      */
                    0x580001a2,  /* .word 0x580001a2 | ldr x2, _X2      */
                    0x580001c3,  /* .word 0x580001c3 | ldr x3, _X3      */
                    0x580001e4,  /* .word 0x580001e4 | ldr x4, _X4      */
                    0xd61f0080,  /* .word 0xd61f0080 | br  x4           */
                    0xe59f0030,  /* BL33_32_ENTRY:   |                  */
                                 /*    ldr r0, _R0   | .word 0xe59f0030 */
                    0xe59f1030,  /*    ldr r1, _R1   | .word 0xe59f1030 */
                    0xe59f2004,  /*    ldr r2, _X0   | .word 0xe59f2004 */
                    0xe59ff020,  /*    ldr pc, _X4   | .word 0xe59ff020 */
                    0x00000000,  /*      .word   0x00000000 */
                    0x00000000,  /* _X0: .word   0x00000000 */
                    0x00000000,  /*      .word   0x00000000 */
                    0x00000000,  /* _X1: .word   0x00000000 */
                    0x00000000,  /*      .word   0x00000000 */
                    0x00000000,  /* _X2: .word   0x00000000 */
                    0x00000000,  /*      .word   0x00000000 */
                    0x00000000,  /* _X3: .word   0x00000000 */
                    0x00000000,  /*      .word   0x00000000 */
                    0x00000000,  /* _X4: .word   0x00000000 */
                    0x00000000,  /* _R0: .word   0x00000000 */
                    0x00000000,  /* _R1: .word   0x00000000 */
                    0x00000000   /*      .word   0x00000000 */
                  };

    /* recovery */
    //if (recovery_check()) {
    //    boot_mode = RECOVERY_BOOT;
    //}

    /* fastboot */
    if (download_check()) {
        ext_boot();
        boot_mode = FASTBOOT_BOOT;
    }

    /* create a bootimg thread to load kernel + dtb */

	/*1.choose A/B boot & tz img.*/  
	u32 boot_part = 0;
	boot_part = check_boot_partition("MISC");
	if (boot_part == 0) {
		dprintf(CRITICAL, "choose first boot partition:%s  , tee choose: %s\n",(char *)BOOT_PART_NAME, (char *)TZ_PART_NAME);
		bootimg.part_name = (char *)BOOT_PART_NAME;
		tz.part_name = (char *)TZ_PART_NAME;
		//cmdlineoverlay(bootimg.dtb_load, NULL, 0); from b partition,need to set
		
	} else if (boot_part == 1) {
		dprintf(CRITICAL, "choose second boot partition: %s  , tee choose: %s\n", (char *)RECOVERY_BOOT_PART_NAME, (char *)RECOVERY_TZ_PART_NAME);
		bootimg.part_name = (char *)RECOVERY_BOOT_PART_NAME;
		tz.part_name = (char *)RECOVERY_TZ_PART_NAME;	
		
	} else {
		dprintf(CRITICAL, "unknow boot_part (%d), using first boot partition\n", boot_part);
		bootimg.part_name = (char *)BOOT_PART_NAME;
		tz.part_name = (char *)TZ_PART_NAME;

	}
	
	
    //bootimg.part_name = (char *)BOOT_PART_NAME;
    bootimg.recovery_part_name = (char *)RECOVERY_BOOT_PART_NAME;
    bootimg.boot_mode = boot_mode;
    bootimg.buf = kernel_buf;
    bootimg_t = thread_create("bootimg_ctl", fit_load_thread, &bootimg,
                              DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);

    /* create a tz thread to load tz */;
    //tz.part_name = (char *)TZ_PART_NAME;
    tz.recovery_part_name = (char *)RECOVERY_TZ_PART_NAME;
    tz.boot_mode = boot_mode;
    tz.buf = tz_buf;
    tz_t = thread_create("tz_ctl", fit_load_thread, &tz,
                         DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);

    if (!bootimg_t || !tz_t) {
        dprintf(CRITICAL, "create load threads failed\n");
        return;
    }

    thread_resume(bootimg_t);
    thread_resume(tz_t);

    thread_join(bootimg_t, &ret_bootimg, INFINITE_TIME);
    thread_join(tz_t, &ret_tz, INFINITE_TIME);

    if (ret_bootimg) {
        dprintf(CRITICAL, "load boot image failed\n");
        return;
    }

    if (ret_tz) {
        dprintf(CRITICAL, "load tz image failed\n");
        return;
    }

	/*2.overlay cmdline to choose A/B rootfs*/
	int cmd_ret = -1;
	if (boot_part == 1) {
		dprintf(CRITICAL, "load second partitions, need to overlay cmdline\n");
		cmd_ret = cmdlineoverlay((void *)bootimg.dtb_load, NULL, 0);
		if (cmd_ret)
		emmc_cmdlineoverlay((void *)bootimg.dtb_load, NULL, 0); //from b partition,need to set
	}
	/* check if dtbo is existed */
    ret = fit_get_image(DTBO_PART_NAME, &fit, dtbo_buf);
    if (ret == 0) {
        void *fdt_dtbo;
        void *fdt_dtb;

        if (bootimg.dtb_load == ERR_ADDR) {
            dprintf(CRITICAL, "dtbo failed, no dtb\n");
            return;
        }
        fdt_dtb = (void *)bootimg.dtb_load;

        /* extract fdt */
        ret = extract_fdt(fdt_dtb, MAX_DTB_SIZE);
        if (ret) {
            dprintf(CRITICAL, "extract fdt failed\n");
            return;
        }

        dprintf(ALWAYS, "[fitboot] do overlay\n");
        fdt_dtbo = (void *)dtbo_buf;
        ret = fdt_overlay_apply(fdt_dtb, fdt_dtbo);
        if (ret) {
            dprintf(CRITICAL, "fdt merge failed, ret %d\n", ret);
            return;
        }

        /* pack fdt */
        ret = fdt_pack(fdt_dtb);
        if (ret) {
            dprintf(CRITICAL, "ft pack failed\n");
            return;
        }
    }

    /* load bl33 for tz to jump*/
    extern __WEAK paddr_t kvaddr_to_paddr(void *ptr);
    addr_t fdt_pa = kvaddr_to_paddr?kvaddr_to_paddr((void *)bootimg.dtb_load):bootimg.dtb_load;
    setup_bl33(bl33, fdt_pa, (uint)(bootimg.kernel_entry));
    memmove((void *)CFG_BL33_LOAD_EP_ADDR, bl33, sizeof(bl33));

#if IS_64BIT
    extern void jump_to_bl31(uint32_t smc_fid, uint64_t x1, uint64_t x2);
    extern __WEAK void dcache_disable(void);
    paddr_t bl33_pa = kvaddr_to_paddr?kvaddr_to_paddr((void *)CFG_BL33_LOAD_EP_ADDR):CFG_BL33_LOAD_EP_ADDR;

    dcache_disable();
    dprintf(ALWAYS, "LK run time: %lld (us)\n", current_time_hires());
    dprintf(ALWAYS, "jump to tz 0x%lx\n\n", tz.kernel_entry);
    /* not return */
    jump_to_bl31(0xc3200000UL, tz.kernel_entry, bl33_pa);
#else
    jump_func_type jump_func;

    jump_func = (jump_func_type)tz.kernel_entry;
    arch_disable_cache(DCACHE);
    arch_disable_mmu();
    dprintf(ALWAYS, "LK run time: %lld (us)\n", current_time_hires());
    dprintf(ALWAYS, "jump to tz 0x%lx\n\n", tz.kernel_entry);
    (*jump_func)(CFG_BL33_LOAD_EP_ADDR, 0, 0);
#endif
}

APP_START(fitboot)
.entry = fitboot_task,
 .flags = 0,
  APP_END
