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
#ifndef MSDC_H
#define MSDC_H

#include <reg.h>
#include <platform/msdc_cfg.h>
#include <platform/mt_reg_base.h>
#include <platform/mmc_core.h>

/*--------------------------------------------------------------------------*/
/* Common Macro                                                             */
/*--------------------------------------------------------------------------*/
#define REG_ADDR(x)             ((volatile uint32_t *)(uintptr_t)(base + OFFSET_##x))

/*--------------------------------------------------------------------------*/
/* Common Definition                                                        */
/*--------------------------------------------------------------------------*/
#define MSDC_FIFO_SZ            (128)
#define MSDC_FIFO_THD           (128)
//#define MSDC_MAX_NUM            (2)

#define MSDC_MS                 (0)
#define MSDC_SDMMC              (1)

#define MSDC_MODE_UNKNOWN       (0)
#define MSDC_MODE_PIO           (1)
#define MSDC_MODE_DMA_BASIC     (2)
#define MSDC_MODE_DMA_DESC      (3)
#define MSDC_MODE_DMA_ENHANCED  (4)
#define MSDC_MODE_MMC_STREAM    (5)

#define MSDC_BUS_1BITS          (0)
#define MSDC_BUS_4BITS          (1)
#define MSDC_BUS_8BITS          (2)

#define MSDC_BURST_8B           (3)
#define MSDC_BURST_16B          (4)
#define MSDC_BURST_32B          (5)
#define MSDC_BURST_64B          (6)

#define MSDC_PIN_PULL_NONE      (0)
#define MSDC_PIN_PULL_DOWN      (1)
#define MSDC_PIN_PULL_UP        (2)
#define MSDC_PIN_KEEP           (3)

#ifdef FPGA_PLATFORM
#define MSDC_OP_SCLK            (12000000)
#define MSDC_MAX_SCLK           MSDC_OP_SCLK / 2
#else
#define MSDC_OP_SCLK            (200000000)
#define MSDC_MAX_SCLK           (200000000)
#endif

#define MSDC_MIN_SCLK           (260000)

#define MSDC_350K_SCLK          (350000)
#define MSDC_400K_SCLK          (400000)
#define MSDC_25M_SCLK           (25000000)
#define MSDC_26M_SCLK           (26000000)
#define MSDC_50M_SCLK           (50000000)
#define MSDC_52M_SCLK           (52000000)
#define MSDC_100M_SCLK          (100000000)
#define MSDC_179M_SCLK          (179000000)
#define MSDC_200M_SCLK          (200000000)
#define MSDC_208M_SCLK          (208000000)
#define MSDC_400M_SCLK          (400000000)
#define MSDC_800M_SCLK          (800000000)

#define MSDC_AUTOCMD12          (0x0001)
#define MSDC_AUTOCMD23          (0x0002)
#define MSDC_AUTOCMD19          (0x0003)

#define TYPE_CMD_RESP_EDGE      (0)
#define TYPE_WRITE_CRC_EDGE     (1)
#define TYPE_READ_DATA_EDGE     (2)
#define TYPE_WRITE_DATA_EDGE    (3)

#define START_AT_RISING                 (0x0)
#define START_AT_FALLING                (0x1)
#define START_AT_RISING_AND_FALLING     (0x2)
#define START_AT_RISING_OR_FALLING      (0x3)

#define MSDC_DMA_BURST_8B       (3)
#define MSDC_DMA_BURST_16B      (4)
#define MSDC_DMA_BURST_32B      (5)
#define MSDC_DMA_BURST_64B      (6)

/*--------------------------------------------------------------------------*/
/* Register Offset                                                          */
/*--------------------------------------------------------------------------*/
#define OFFSET_MSDC_CFG                  (0x00)
#define OFFSET_MSDC_IOCON                (0x04)
#define OFFSET_MSDC_PS                   (0x08)
#define OFFSET_MSDC_INT                  (0x0c)
#define OFFSET_MSDC_INTEN                (0x10)
#define OFFSET_MSDC_FIFOCS               (0x14)
#define OFFSET_MSDC_TXDATA               (0x18)
#define OFFSET_MSDC_RXDATA               (0x1c)
#define OFFSET_SDC_CFG                   (0x30)
#define OFFSET_SDC_CMD                   (0x34)
#define OFFSET_SDC_ARG                   (0x38)
#define OFFSET_SDC_STS                   (0x3c)
#define OFFSET_SDC_RESP0                 (0x40)
#define OFFSET_SDC_RESP1                 (0x44)
#define OFFSET_SDC_RESP2                 (0x48)
#define OFFSET_SDC_RESP3                 (0x4c)
#define OFFSET_SDC_BLK_NUM               (0x50)
#define OFFSET_SDC_VOL_CHG               (0x54)
#define OFFSET_SDC_CSTS                  (0x58)
#define OFFSET_SDC_CSTS_EN               (0x5c)
#define OFFSET_SDC_DCRC_STS              (0x60)

/* Only for EMMC Controller 4 registers below */
#define OFFSET_EMMC_CFG0                 (0x70)
#define OFFSET_EMMC_CFG1                 (0x74)
#define OFFSET_EMMC_STS                  (0x78)
#define OFFSET_EMMC_IOCON                (0x7c)

#define OFFSET_SDC_ACMD_RESP             (0x80)
#define OFFSET_SDC_ACMD19_TRG            (0x84)
#define OFFSET_SDC_ACMD19_STS            (0x88)
#define OFFSET_MSDC_DMA_HIGH4BIT         (0x8C)
#define OFFSET_MSDC_DMA_SA               (0x90)
#define OFFSET_MSDC_DMA_CA               (0x94)
#define OFFSET_MSDC_DMA_CTRL             (0x98)
#define OFFSET_MSDC_DMA_CFG              (0x9c)
#define OFFSET_MSDC_DBG_SEL              (0xa0)
#define OFFSET_MSDC_DBG_OUT              (0xa4)
#define OFFSET_MSDC_DMA_LEN              (0xa8)
#define OFFSET_MSDC_PATCH_BIT0           (0xb0)
#define OFFSET_MSDC_PATCH_BIT1           (0xb4)
#define OFFSET_MSDC_PATCH_BIT2           (0xb8)

/* Only for SD/SDIO Controller 6 registers below */
#define OFFSET_DAT0_TUNE_CRC             (0xc0)
#define OFFSET_DAT1_TUNE_CRC             (0xc4)
#define OFFSET_DAT2_TUNE_CRC             (0xc8)
#define OFFSET_DAT3_TUNE_CRC             (0xcc)
#define OFFSET_CMD_TUNE_CRC              (0xd0)
#define OFFSET_SDIO_TUNE_WIND            (0xd4)

#define OFFSET_MSDC_PAD_TUNE             (0xF0)
#define OFFSET_MSDC_PAD_TUNE0            (0xF0)
#define OFFSET_MSDC_PAD_TUNE1            (0xF4)
#define OFFSET_MSDC_DAT_RDDLY0           (0xF8)
#define OFFSET_MSDC_DAT_RDDLY1           (0xFC)

#define OFFSET_MSDC_DAT_RDDLY2           (0x100)
#define OFFSET_MSDC_DAT_RDDLY3           (0x104)

#define OFFSET_MSDC_HW_DBG               (0x110)
#define OFFSET_MSDC_VERSION              (0x114)
#define OFFSET_MSDC_ECO_VER              (0x118)

#define OFFSET_SDC_FIFO_CFG	         (0x228)
/*--------------------------------------------------------------------------*/
/* Register Address                                                         */
/*--------------------------------------------------------------------------*/
/* common register */
#define MSDC_CFG                         REG_ADDR(MSDC_CFG)
#define MSDC_IOCON                       REG_ADDR(MSDC_IOCON)
#define MSDC_PS                          REG_ADDR(MSDC_PS)
#define MSDC_INT                         REG_ADDR(MSDC_INT)
#define MSDC_INTEN                       REG_ADDR(MSDC_INTEN)
#define MSDC_FIFOCS                      REG_ADDR(MSDC_FIFOCS)
#define MSDC_TXDATA                      REG_ADDR(MSDC_TXDATA)
#define MSDC_RXDATA                      REG_ADDR(MSDC_RXDATA)

/* sdmmc register */
#define SDC_CFG                          REG_ADDR(SDC_CFG)
#define SDC_CMD                          REG_ADDR(SDC_CMD)
#define SDC_ARG                          REG_ADDR(SDC_ARG)
#define SDC_STS                          REG_ADDR(SDC_STS)
#define SDC_RESP0                        REG_ADDR(SDC_RESP0)
#define SDC_RESP1                        REG_ADDR(SDC_RESP1)
#define SDC_RESP2                        REG_ADDR(SDC_RESP2)
#define SDC_RESP3                        REG_ADDR(SDC_RESP3)
#define SDC_BLK_NUM                      REG_ADDR(SDC_BLK_NUM)
#define SDC_VOL_CHG                      REG_ADDR(SDC_VOL_CHG)
#define SDC_CSTS                         REG_ADDR(SDC_CSTS)
#define SDC_CSTS_EN                      REG_ADDR(SDC_CSTS_EN)
#define SDC_DCRC_STS                     REG_ADDR(SDC_DCRC_STS)

/* emmc register*/
#define EMMC_CFG0                        REG_ADDR(EMMC_CFG0)
#define EMMC_CFG1                        REG_ADDR(EMMC_CFG1)
#define EMMC_STS                         REG_ADDR(EMMC_STS)
#define EMMC_IOCON                       REG_ADDR(EMMC_IOCON)

/* auto command register */
#define SDC_ACMD_RESP                    REG_ADDR(SDC_ACMD_RESP)
#define SDC_ACMD19_TRG                   REG_ADDR(SDC_ACMD19_TRG)
#define SDC_ACMD19_STS                   REG_ADDR(SDC_ACMD19_STS)

/* dma register */
#define MSDC_DMA_HIGH4BIT                REG_ADDR(MSDC_DMA_HIGH4BIT)
#define MSDC_DMA_SA                      REG_ADDR(MSDC_DMA_SA)
#define MSDC_DMA_CA                      REG_ADDR(MSDC_DMA_CA)
#define MSDC_DMA_CTRL                    REG_ADDR(MSDC_DMA_CTRL)
#define MSDC_DMA_CFG                     REG_ADDR(MSDC_DMA_CFG)

/* debug register */
#define MSDC_DBG_SEL                     REG_ADDR(MSDC_DBG_SEL)
#define MSDC_DBG_OUT                     REG_ADDR(MSDC_DBG_OUT)
#define MSDC_DMA_LEN                     REG_ADDR(MSDC_DMA_LEN)

/* misc register */
#define MSDC_PATCH_BIT0                  REG_ADDR(MSDC_PATCH_BIT0)
#define MSDC_PATCH_BIT1                  REG_ADDR(MSDC_PATCH_BIT1)
#define MSDC_PATCH_BIT2                  REG_ADDR(MSDC_PATCH_BIT2)
#define DAT0_TUNE_CRC                    REG_ADDR(DAT0_TUNE_CRC)
#define DAT1_TUNE_CRC                    REG_ADDR(DAT1_TUNE_CRC)
#define DAT2_TUNE_CRC                    REG_ADDR(DAT2_TUNE_CRC)
#define DAT3_TUNE_CRC                    REG_ADDR(DAT3_TUNE_CRC)
#define CMD_TUNE_CRC                     REG_ADDR(CMD_TUNE_CRC)
#define SDIO_TUNE_WIND                   REG_ADDR(SDIO_TUNE_WIND)
#define MSDC_PAD_TUNE                    REG_ADDR(MSDC_PAD_TUNE)
#define MSDC_PAD_TUNE0                   REG_ADDR(MSDC_PAD_TUNE0)
#define MSDC_PAD_TUNE1                   REG_ADDR(MSDC_PAD_TUNE1)

/* data read delay */
#define MSDC_DAT_RDDLY0                  REG_ADDR(MSDC_DAT_RDDLY0)
#define MSDC_DAT_RDDLY1                  REG_ADDR(MSDC_DAT_RDDLY1)
#define MSDC_DAT_RDDLY2                  REG_ADDR(MSDC_DAT_RDDLY2)
#define MSDC_DAT_RDDLY3                  REG_ADDR(MSDC_DAT_RDDLY3)

#define MSDC_HW_DBG                      REG_ADDR(MSDC_HW_DBG)
#define MSDC_VERSION                     REG_ADDR(MSDC_VERSION)
#define MSDC_ECO_VER                     REG_ADDR(MSDC_ECO_VER)
#define SDC_FIFO_CFG			 REG_ADDR(SDC_FIFO_CFG)

/*--------------------------------------------------------------------------*/
/* Register Mask                                                            */
/*--------------------------------------------------------------------------*/

/* MSDC_CFG mask */
#define MSDC_CFG_MODE           (0x1   <<  0)     /* RW */
#define MSDC_CFG_CKPDN          (0x1   <<  1)     /* RW */
#define MSDC_CFG_RST            (0x1   <<  2)     /* A0 */
#define MSDC_CFG_PIO            (0x1   <<  3)     /* RW */
#define MSDC_CFG_CKDRVEN        (0x1   <<  4)     /* RW */
#define MSDC_CFG_BV18SDT        (0x1   <<  5)     /* RW */
#define MSDC_CFG_BV18PSS        (0x1   <<  6)     /* R  */
#define MSDC_CFG_CKSTB          (0x1   <<  7)     /* R  */
#define MSDC_CFG_CKDIV          (0xFFF <<  8)     /* RW */
#define MSDC_CFG_CKMOD          (0x3   << 20)     /* W1C */
#define MSDC_CFG_SCLK_STOP_DDR  (0x1   << 25)     /* RW  */

/* MSDC_IOCON mask */
#define MSDC_IOCON_SDR104CKS    (0x1   <<  0)     /* RW */
#define MSDC_IOCON_RSPL         (0x1   <<  1)     /* RW */
#define MSDC_IOCON_R_D_SMPL     (0x1   <<  2)     /* RW */
#define MSDC_IOCON_DSPL          MSDC_IOCON_R_D_SMPL /* alias */

#define MSDC_IOCON_DDLSEL       (0x1   <<  3)     /* RW */
#define MSDC_IOCON_DDR50CKD     (0x1   <<  4)     /* RW */
#define MSDC_IOCON_R_D_SMPL_SEL (0x1   <<  5)     /* RW */
#define MSDC_IOCON_W_D_SMPL     (0x1   <<  8)     /* RW */
#define MSDC_IOCON_W_D_SMPL_SEL (0x1   <<  9)     /* RW */
#define MSDC_IOCON_W_D0SPL      (0x1   << 10)     /* RW */
#define MSDC_IOCON_W_D1SPL      (0x1   << 11)     /* RW */
#define MSDC_IOCON_W_D2SPL      (0x1   << 12)     /* RW */
#define MSDC_IOCON_W_D3SPL      (0x1   << 13)     /* RW */

#define MSDC_IOCON_R_D0SPL      (0x1   << 16)     /* RW */
#define MSDC_IOCON_R_D1SPL      (0x1   << 17)     /* RW */
#define MSDC_IOCON_R_D2SPL      (0x1   << 18)     /* RW */
#define MSDC_IOCON_R_D3SPL      (0x1   << 19)     /* RW */
#define MSDC_IOCON_R_D4SPL      (0x1   << 20)     /* RW */
#define MSDC_IOCON_R_D5SPL      (0x1   << 21)     /* RW */
#define MSDC_IOCON_R_D6SPL      (0x1   << 22)     /* RW */
#define MSDC_IOCON_R_D7SPL      (0x1   << 23)     /* RW */
//#define MSDC_IOCON_RISCSZ       (0x3   << 24)     /* RW  !!! MT2701  remove*/

/* MSDC_PS mask */
#define MSDC_PS_CDEN            (0x1   <<  0)     /* RW */
#define MSDC_PS_CDSTS           (0x1   <<  1)     /* RU  */

#define MSDC_PS_CDDEBOUNCE      (0xF   << 12)     /* RW */
#define MSDC_PS_DAT             (0xFF  << 16)     /* RU */
#define MSDC_PS_DAT8PIN         (0xFF  << 16)     /* RU */
#define MSDC_PS_DAT4PIN         (0xF   << 16)     /* RU */
#define MSDC_PS_DAT0            (0x1   << 16)     /* RU */

#define MSDC_PS_CMD             (0x1   << 24)     /* RU */

#define MSDC_PS_WP              (0x1   << 31)     /* RU  */

/* MSDC_INT mask */
#define MSDC_INT_MMCIRQ         (0x1   <<  0)     /* W1C */
#define MSDC_INT_CDSC           (0x1   <<  1)     /* W1C */

#define MSDC_INT_ACMDRDY        (0x1   <<  3)     /* W1C */
#define MSDC_INT_ACMDTMO        (0x1   <<  4)     /* W1C */
#define MSDC_INT_ACMDCRCERR     (0x1   <<  5)     /* W1C */
#define MSDC_INT_DMAQ_EMPTY     (0x1   <<  6)     /* W1C */
#define MSDC_INT_SDIOIRQ        (0x1   <<  7)     /* W1C Only for SD/SDIO */
#define MSDC_INT_CMDRDY         (0x1   <<  8)     /* W1C */
#define MSDC_INT_CMDTMO         (0x1   <<  9)     /* W1C */
#define MSDC_INT_RSPCRCERR      (0x1   << 10)     /* W1C */
#define MSDC_INT_CSTA           (0x1   << 11)     /* R */
#define MSDC_INT_XFER_COMPL     (0x1   << 12)     /* W1C */
#define MSDC_INT_DXFER_DONE     (0x1   << 13)     /* W1C */
#define MSDC_INT_DATTMO         (0x1   << 14)     /* W1C */
#define MSDC_INT_DATCRCERR      (0x1   << 15)     /* W1C */
#define MSDC_INT_ACMD19_DONE    (0x1   << 16)     /* W1C */
#define MSDC_INT_BDCSERR        (0x1   << 17)     /* W1C */
#define MSDC_INT_GPDCSERR       (0x1   << 18)     /* W1C */
#define MSDC_INT_DMAPRO         (0x1   << 19)     /* W1C */
#define MSDC_INT_GOBOUND        (0x1   << 20)     /* W1C Only for SD/SDIO ACMD 53*/
#define MSDC_INT_ACMD53_DONE    (0x1   << 21)     /* W1C Only for SD/SDIO ACMD 53*/
#define MSDC_INT_ACMD53_FAIL    (0x1   << 22)     /* W1C Only for SD/SDIO ACMD 53*/
#define MSDC_INT_AXI_RESP_ERR   (0x1   << 23)     /* W1C Only for eMMC 5.0*/

/* MSDC_INTEN mask */
#define MSDC_INTEN_MMCIRQ       (0x1   <<  0)     /* RW */
#define MSDC_INTEN_CDSC         (0x1   <<  1)     /* RW */

#define MSDC_INTEN_ACMDRDY      (0x1   <<  3)     /* RW */
#define MSDC_INTEN_ACMDTMO      (0x1   <<  4)     /* RW */
#define MSDC_INTEN_ACMDCRCERR   (0x1   <<  5)     /* RW */
#define MSDC_INTEN_DMAQ_EMPTY   (0x1   <<  6)     /* RW */
#define MSDC_INTEN_SDIOIRQ      (0x1   <<  7)     /* RW Only for SDIO*/
#define MSDC_INTEN_CMDRDY       (0x1   <<  8)     /* RW */
#define MSDC_INTEN_CMDTMO       (0x1   <<  9)     /* RW */
#define MSDC_INTEN_RSPCRCERR    (0x1   << 10)     /* RW */
#define MSDC_INTEN_CSTA         (0x1   << 11)     /* RW */
#define MSDC_INTEN_XFER_COMPL   (0x1   << 12)     /* RW */
#define MSDC_INTEN_DXFER_DONE   (0x1   << 13)     /* RW */
#define MSDC_INTEN_DATTMO       (0x1   << 14)     /* RW */
#define MSDC_INTEN_DATCRCERR    (0x1   << 15)     /* RW */
#define MSDC_INTEN_ACMD19_DONE  (0x1   << 16)     /* RW */
#define MSDC_INTEN_BDCSERR      (0x1   << 17)     /* RW */
#define MSDC_INTEN_GPDCSERR     (0x1   << 18)     /* RW */
#define MSDC_INTEN_DMAPRO       (0x1   << 19)     /* RW */
#define MSDC_INTEN_GOBOUND      (0x1   << 20)     /* RW  Only for SD/SDIO ACMD 53*/
#define MSDC_INTEN_ACMD53_DONE  (0x1   << 21)     /* RW  Only for SD/SDIO ACMD 53*/
#define MSDC_INTEN_ACMD53_FAIL  (0x1   << 22)     /* RW  Only for SD/SDIO ACMD 53*/
#define MSDC_INTEN_AXI_RESP_ERR (0x1   << 23)     /* RW  Only for eMMC 5.0*/

#define MSDC_INTEN_DFT       (  MSDC_INTEN_MMCIRQ        |MSDC_INTEN_CDSC        | MSDC_INTEN_ACMDRDY\
                                |MSDC_INTEN_ACMDTMO      |MSDC_INTEN_ACMDCRCERR  | MSDC_INTEN_DMAQ_EMPTY /*|MSDC_INTEN_SDIOIRQ*/\
                                |MSDC_INTEN_CMDRDY       |MSDC_INTEN_CMDTMO      | MSDC_INTEN_RSPCRCERR   |MSDC_INTEN_CSTA\
                                |MSDC_INTEN_XFER_COMPL   |MSDC_INTEN_DXFER_DONE  | MSDC_INTEN_DATTMO      |MSDC_INTEN_DATCRCERR\
                                |MSDC_INTEN_BDCSERR      |MSDC_INTEN_ACMD19_DONE | MSDC_INTEN_GPDCSERR     /*|MSDC_INTEN_DMAPRO*/\
                                /*|MSDC_INTEN_GOBOUND   |MSDC_INTEN_ACMD53_DONE |MSDC_INTEN_ACMD53_FAIL |MSDC_INTEN_AXI_RESP_ERR*/)


/* MSDC_FIFOCS mask */
#define MSDC_FIFOCS_RXCNT       (0xFF  <<  0)     /* R */
#define MSDC_FIFOCS_TXCNT       (0xFF  << 16)     /* R */
#define MSDC_FIFOCS_CLR         (0x1   << 31)     /* RW */

/* SDC_CFG mask */
#define SDC_CFG_SDIOINTWKUP     (0x1   <<  0)     /* RW */
#define SDC_CFG_INSWKUP         (0x1   <<  1)     /* RW */
#define SDC_CFG_BUSWIDTH        (0x3   << 16)     /* RW */
#define SDC_CFG_SDIO            (0x1   << 19)     /* RW */
#define SDC_CFG_SDIOIDE         (0x1   << 20)     /* RW */
#define SDC_CFG_INTATGAP        (0x1   << 21)     /* RW */
#define SDC_CFG_DTOC            (0xFF  << 24)     /* RW */

/* SDC_CMD mask */
#define SDC_CMD_OPC             (0x3F  <<  0)     /* RW */
#define SDC_CMD_BRK             (0x1   <<  6)     /* RW */
#define SDC_CMD_RSPTYP          (0x7   <<  7)     /* RW */
#define SDC_CMD_DTYP            (0x3   << 11)     /* RW */
#define SDC_CMD_RW              (0x1   << 13)     /* RW */
#define SDC_CMD_STOP            (0x1   << 14)     /* RW */
#define SDC_CMD_GOIRQ           (0x1   << 15)     /* RW */
#define SDC_CMD_BLKLEN          (0xFFF << 16)     /* RW */
#define SDC_CMD_AUTOCMD         (0x3   << 28)     /* RW */
#define SDC_CMD_VOLSWTH         (0x1   << 30)     /* RW */
#define SDC_CMD_ACMD53          (0x1   << 31)     /* RW Only for SD/SDIO ACMD 53*/

/* SDC_STS mask */
#define SDC_STS_SDCBUSY         (0x1   <<  0)     /* RW */
#define SDC_STS_CMDBUSY         (0x1   <<  1)     /* RW */
#define SDC_STS_CMD_WR_BUSY     (0x1   << 16)     /* RW */
#define SDC_STS_SWR_COMPL       (0x1   << 31)     /* RW */

/* SDC_VOL_CHG mask */
#define SDC_VOL_CHG_VCHGCNT     (0xFFFF<<  0)     /* RW */
/* SDC_DCRC_STS mask */
#define SDC_DCRC_STS_POS        (0xFF  <<  0)     /* RO */
#define SDC_DCRC_STS_NEG        (0xFF  <<  8)     /* RO */
/* EMMC_CFG0 mask */
#define EMMC_CFG0_BOOTSTART     (0x1   <<  0)     /* WO Only for eMMC */
#define EMMC_CFG0_BOOTSTOP      (0x1   <<  1)     /* WO Only for eMMC */
#define EMMC_CFG0_BOOTMODE      (0x1   <<  2)     /* RW Only for eMMC */
#define EMMC_CFG0_BOOTACKDIS    (0x1   <<  3)     /* RW Only for eMMC */

#define EMMC_CFG0_BOOTWDLY      (0x7   << 12)     /* RW Only for eMMC */
#define EMMC_CFG0_BOOTSUPP      (0x1   << 15)     /* RW Only for eMMC */

/* EMMC_CFG1 mask */
#define EMMC_CFG1_BOOTDATTMC   (0xFFFFF<<  0)     /* RW Only for eMMC */
#define EMMC_CFG1_BOOTACKTMC    (0xFFF << 20)     /* RW Only for eMMC */

/* EMMC_STS mask */
#define EMMC_STS_BOOTCRCERR     (0x1   <<  0)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTACKERR     (0x1   <<  1)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTDATTMO     (0x1   <<  2)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTACKTMO     (0x1   <<  3)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTUPSTATE    (0x1   <<  4)     /* RU Only for eMMC */
#define EMMC_STS_BOOTACKRCV     (0x1   <<  5)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTDATRCV     (0x1   <<  6)     /* RU Only for eMMC */

/* EMMC_IOCON mask */
#define EMMC_IOCON_BOOTRST      (0x1   <<  0)     /* RW Only for eMMC */

/* SDC_ACMD19_TRG mask */
#define SDC_ACMD19_TRG_TUNESEL  (0xF   <<  0)     /* RW */

/* DMA_SA_HIGH4BIT mask */
#define DMA_SA_HIGH4BIT_L4BITS  (0xF   <<  0)     /* RW */
/* MSDC_DMA_CTRL mask */
#define MSDC_DMA_CTRL_START     (0x1   <<  0)     /* WO */
#define MSDC_DMA_CTRL_STOP      (0x1   <<  1)     /* AO */
#define MSDC_DMA_CTRL_RESUME    (0x1   <<  2)     /* WO */
#define MSDC_DMA_CTRL_READYM    (0x1   <<  3)     /* RO */

#define MSDC_DMA_CTRL_MODE      (0x1   <<  8)     /* RW */
#define MSDC_DMA_CTRL_ALIGN     (0x1   <<  9)     /* RW */
#define MSDC_DMA_CTRL_LASTBUF   (0x1   << 10)     /* RW */
#define MSDC_DMA_CTRL_SPLIT1K   (0x1   << 11)     /* RW */
#define MSDC_DMA_CTRL_BURSTSZ   (0x7   << 12)     /* RW */
// #define MSDC_DMA_CTRL_XFERSZ    (0xffffUL << 16)/* RW */

/* MSDC_DMA_CFG mask */
#define MSDC_DMA_CFG_STS              (0x1  <<  0)     /* R */
#define MSDC_DMA_CFG_DECSEN           (0x1  <<  1)     /* RW */
#define MSDC_DMA_CFG_LOCKDISABLE      (0x1  <<  2)     /* RW */
//#define MSDC_DMA_CFG_BDCSERR        (0x1  <<  4)     /* R */
//#define MSDC_DMA_CFG_GPDCSERR       (0x1  <<  5)     /* R */
#define MSDC_DMA_CFG_AHBEN            (0x3  <<  8)     /* RW */
#define MSDC_DMA_CFG_ACTEN            (0x3  << 12)     /* RW */

#define MSDC_DMA_CFG_CS12B            (0x1  << 16)     /* RW */
#define MSDC_DMA_CFG_OUTB_STOP        (0x1  << 17)     /* RW */

/* MSDC_PATCH_BIT0 mask */
//#define MSDC_PB0_RESV1              (0x1  <<  0)
#define MSDC_PB0_EN_8BITSUP           (0x1  <<  1)    /* RW */
#define MSDC_PB0_DIS_RECMDWR          (0x1  <<  2)    /* RW */
//#define MSDC_PB0_RESV2                        (0x1  <<  3)
#define MSDC_PB0_RDDATSEL             (0x1  <<  3)    /* RW */
#define MSDC_PB0_ACMD53_CRCINTR       (0x1  <<  4)    /* RW */
#define MSDC_PB0_ACMD53_ONESHOT       (0x1  <<  5)    /* RW */
//#define MSDC_PB0_RESV3                        (0x1  <<  6)
#define MSDC_PB0_DESC_UP_SEL          (0x1  <<  6)    /* RW */
#define MSDC_PB0_INT_DAT_LATCH_CK_SEL (0x7  <<  7)    /* RW */
#define MSDC_INT_DAT_LATCH_CK_SEL      MSDC_PB0_INT_DAT_LATCH_CK_SEL  /* alias */

#define MSDC_PB0_CKGEN_MSDC_DLY_SEL   (0x1F << 10)    /* RW */
#define MSDC_CKGEN_MSDC_DLY_SEL        MSDC_PB0_CKGEN_MSDC_DLY_SEL  /* alias */


#define MSDC_PB0_FIFORD_DIS           (0x1  << 15)    /* RW */
#define MSDC_PB0_MSDC_BLKNUMSEL       (0x1  << 16)    /* RW */
#define MSDC_PB0_BLKNUM_SEL           MSDC_PB0_MSDC_BLKNUMSEL /* alias */

#define MSDC_PB0_SDIO_INTCSEL         (0x1  << 17)    /* RW */
#define MSDC_PB0_SDIO_BSYDLY          (0xF  << 18)    /* RW */
#define MSDC_PB0_SDC_WDOD             (0xF  << 22)    /* RW */
#define MSDC_PB0_CMDIDRTSEL           (0x1  << 26)    /* RW */
#define MSDC_PB0_CMDFAILSEL           (0x1  << 27)    /* RW */
#define MSDC_PB0_SDIO_INTDLYSEL       (0x1  << 28)    /* RW */
#define MSDC_PB0_SPCPUSH              (0x1  << 29)    /* RW */
#define MSDC_PB0_DETWR_CRCTMO         (0x1  << 30)    /* RW */
#define MSDC_PB0_EN_DRVRSP            (0x1  << 31)    /* RW */

/* MSDC_PATCH_BIT1 mask */
#define MSDC_PB1_WRDAT_CRCS_TA_CNTR   (0x7  <<  0)    /* RW */
#define MSDC_PATCH_BIT1_WRDAT_CRCS    MSDC_PB1_WRDAT_CRCS_TA_CNTR /* alias */


#define MSDC_PB1_CMD_RSP_TA_CNTR      (0x7  <<  3)    /* RW */
#define MSDC_PATCH_BIT1_CMD_RSP       MSDC_PB1_CMD_RSP_TA_CNTR  /* alias */

//#define MSDC_PB1_RESV3                        (0x3  <<  6)
#define MSDC_PB1_GET_BUSY_MARGIN      (0x1  <<  6)    /* RW */
#define MSDC_BUSY_CHECK_SEL           (0x1  <<  7)    /* RW */
#define MSDC_PB1_STOP_DLY_SEL	      (0xF  <<  8)    /* RW */
#define MSDC_PB1_BIAS_EN18IO_28NM     (0x1  << 12)    /* RW */
#define MSDC_PB1_BIAS_EXT_28NM        (0x1  << 13)    /* RW */

//#define MSDC_PB1_RESV2                        (0x3  << 14)
#define MSDC_PB1_RESET_GDMA           (0x1  << 15)    /* RW */
//#define MSDC_PB1_RESV1                        (0x7F << 16)
#define MSDC_PB1_EN_SINGLE_BURST      (0x1  << 16)    /* RW */
#define MSDC_PB1_DCM_EN               (0x1  << 21)    /* RW */
#define MSDC_PB1_AHBCKEN              (0x1  << 23)    /* RW */
#define MSDC_PB1_CKSPCEN              (0x1  << 24)    /* RW */
#define MSDC_PB1_CKPSCEN              (0x1  << 25)    /* RW */
#define MSDC_PB1_CKVOLDETEN           (0x1  << 26)    /* RW */
#define MSDC_PB1_CKACMDEN             (0x1  << 27)    /* RW */
#define MSDC_PB1_CKSDEN               (0x1  << 28)    /* RW */
#define MSDC_PB1_CKWCTLEN             (0x1  << 29)    /* RW */
#define MSDC_PB1_CKRCTLEN             (0x1  << 30)    /* RW */
#define MSDC_PB1_CKSHBFFEN            (0x1  << 31)    /* RW */

/* MSDC_PATCH_BIT2 mask */
#define MSDC_PB2_ENHANCEGPD           (0x1  <<  0)    /* RW */
#define MSDC_PB2_SUPPORT64G           (0x1  <<  1)    /* RW */
#define MSDC_PB2_RESPWAIT             (0x3  <<  2)    /* RW */
#define MSDC_PB2_CFGRDATCNT           (0x1F <<  4)    /* RW */
#define MSDC_PB2_CFGRDAT              (0x1  <<  9)    /* RW */

#define MSDC_PB2_INTCRESPSEL          (0x1  << 11)    /* RW */
#define MSDC_PB2_CFGRESPCNT           (0x7  << 12)    /* RW */
#define MSDC_PB2_CFGRESP              (0x1  << 15)    /* RW */
#define MSDC_PB2_RESPSTSENSEL         (0x7  << 16)    /* RW */

#define MSDC_PB2_POPENCNT             (0xF  << 20)    /* RW */
#define MSDC_PB2_CFGCRCSTSSEL         (0x1  << 24)    /* RW */
#define MSDC_PB2_CFGCRCSTSEDGE        (0x1  << 25)    /* RW */
#define MSDC_PB2_CFGCRCSTSCNT         (0x3  << 26)    /* RW */
#define MSDC_PB2_CFGCRCSTS            (0x1  << 28)    /* RW */
#define MSDC_PB2_CRCSTSENSEL          (0x7  << 29)    /* RW */


/* SDIO_TUNE_WIND mask */
#define SDIO_TUNE_WIND_TUNEWINDOW     (0x1F  <<  0)     /* RW */

/* MSDC_PAD_TUNE/MSDC_PAD_TUNE0 mask */
#define MSDC_PAD_TUNE_DATWRDLY        (0x1F  <<  0)     /* RW */

#define MSDC_PAD_TUNE_DELAYEN         (0x1   <<  7)     /* RW */
#define MSDC_PAD_TUNE_DATRRDLY        (0x1F  <<  8)     /* RW */
#define MSDC_PAD_TUNE_DATRRDLYSEL     (0x1   << 13)     /* RW */

#define MSDC_PAD_TUNE_RXDLYSEL        (0x1   << 15)     /* RW */
#define MSDC_PAD_TUNE_CMDRDLY         (0x1F  << 16)     /* RW */
#define MSDC_PAD_TUNE_CMDRDLYSEL      (0x1   << 21)     /* RW */
#define MSDC_PAD_TUNE_CMDRRDLY        (0x1F  << 22)     /* RW */
#define MSDC_PAD_TUNE_CLKTXDLY        (0x1F  << 27)     /* RW */

/* MSDC_PAD_TUNE1 mask */

#define MSDC_PAD_TUNE1_DATRRDLY2      (0x1F  <<  8)     /* RW */
#define MSDC_PAD_TUNE1_DATRDLY2SEL    (0x1   << 13)     /* RW */

#define MSDC_PAD_TUNE1_CMDRDLY2       (0x1F  << 16)     /* RW */
#define MSDC_PAD_TUNE1_CMDRDLY2SEL    (0x1   << 21)     /* RW */


/* MSDC_DAT_RDDLY0 mask */
#define MSDC_DAT_RDDLY0_D3            (0x1F  <<  0)     /* RW */
#define MSDC_DAT_RDDLY0_D2            (0x1F  <<  8)     /* RW */
#define MSDC_DAT_RDDLY0_D1            (0x1F  << 16)     /* RW */
#define MSDC_DAT_RDDLY0_D0            (0x1F  << 24)     /* RW */

/* MSDC_DAT_RDDLY1 mask */
#define MSDC_DAT_RDDLY1_D7            (0x1F  <<  0)     /* RW */

#define MSDC_DAT_RDDLY1_D6            (0x1F  <<  8)     /* RW */

#define MSDC_DAT_RDDLY1_D5            (0x1F  << 16)     /* RW */

#define MSDC_DAT_RDDLY1_D4            (0x1F  << 24)     /* RW */

/* MSDC_DAT_RDDLY2 mask */
#define MSDC_DAT_RDDLY2_D3            (0x1F  <<  0)     /* RW */

#define MSDC_DAT_RDDLY2_D2            (0x1F  <<  8)     /* RW */

#define MSDC_DAT_RDDLY2_D1            (0x1F  << 16)     /* RW */

#define MSDC_DAT_RDDLY2_D0            (0x1F  << 24)     /* RW */

/* MSDC_DAT_RDDLY3 mask */
#define MSDC_DAT_RDDLY3_D7            (0x1F  <<  0)     /* RW */

#define MSDC_DAT_RDDLY3_D6            (0x1F  <<  8)     /* RW */

#define MSDC_DAT_RDDLY3_D5            (0x1F  << 16)     /* RW */

#define MSDC_DAT_RDDLY3_D4            (0x1F  << 24)     /* RW */

/* MSDC_HW_DBG_SEL mask */
#define MSDC_HW_DBG0_SEL              (0xFF  <<  0)     /* RW */
#define MSDC_HW_DBG1_SEL              (0x3F  <<  8)     /* RW */

#define MSDC_HW_DBG2_SEL              (0xFF  << 16)     /* RW */
#define MSDC_HW_DBG3_SEL              (0x3F  << 24)     /* RW */
#define MSDC_HW_DBG_WRAP_SEL          (0x1   << 30)     /* RW */

/* SDC_FIFO_CFG mask */
#define SDC_FIFO_CFG_WRVALIDSEL   (0x1 << 24)  /* RW */
#define SDC_FIFO_CFG_RDVALIDSEL   (0x1 << 25)  /* RW */

#if 1
/* Chaotian Add GPIO top layer */
#define MSDC_DRVN_GEAR0                       0
#define MSDC_DRVN_GEAR1                       1
#define MSDC_DRVN_GEAR2                       2
#define MSDC_DRVN_GEAR3                       3
#define MSDC_DRVN_GEAR4                       4
#define MSDC_DRVN_GEAR5                       5
#define MSDC_DRVN_GEAR6                       6
#define MSDC_DRVN_GEAR7                       7
#define MSDC_DRVN_DONT_CARE                   MSDC_DRVN_GEAR0

/* for MT8516 */
#define MSDC0_IO_PAD_BASE       (GPIO_BASE)
/*--------------------------------------------------------------------------*/
/* MSDC GPIO Related Register                                               */
/*--------------------------------------------------------------------------*/
#define MSDC0_GPIO_MODE17   (MSDC_GPIO_BASE + 0x460)
#define MSDC0_GPIO_MODE18   (MSDC_GPIO_BASE + 0x470)
#define MSDC0_GPIO_MODE19   (MSDC_GPIO_BASE + 0x480)
#define MSDC0_GPIO_IES_ADDR (MSDC0_IO_PAD_BASE + 0x920)
#define MSDC0_GPIO_SMT_ADDR (MSDC0_IO_PAD_BASE + 0xA20)
#define MSDC0_GPIO_TDSEL0_ADDR   (MSDC0_IO_PAD_BASE + 0xB60)
#define MSDC0_GPIO_TDSEL1_ADDR   (MSDC0_IO_PAD_BASE + 0xB70)
#define MSDC0_GPIO_RDSEL0_ADDR   (MSDC0_IO_PAD_BASE + 0xC60)
#define MSDC0_GPIO_RDSEL1_ADDR   (MSDC0_IO_PAD_BASE + 0xC70)
#define MSDC0_GPIO_DRV0_ADDR (MSDC0_IO_PAD_BASE + 0xD60)
#define MSDC0_GPIO_DRV1_ADDR (MSDC0_IO_PAD_BASE + 0xD70)
#define MSDC0_GPIO_PUPD0_ADDR   (MSDC0_IO_PAD_BASE + 0xE00)
#define MSDC0_GPIO_PUPD1_ADDR   (MSDC0_IO_PAD_BASE + 0xE10)
#define MSDC0_GPIO_PUPD2_ADDR   (MSDC0_IO_PAD_BASE + 0xE20)

#define MSDC1_GPIO_MODE15    (MSDC_GPIO_BASE + 0x440)
#define MSDC1_GPIO_MODE16    (MSDC_GPIO_BASE + 0x450)
#define MSDC1_GPIO_IES0_ADDR (MSDC1_IO_PAD_BASE + 0x920)
#define MSDC1_GPIO_IES1_ADDR (MSDC1_IO_PAD_BASE + 0x930)
#define MSDC1_GPIO_SMT0_ADDR (MSDC1_IO_PAD_BASE + 0xA20)
#define MSDC1_GPIO_SMT1_ADDR (MSDC1_IO_PAD_BASE + 0xA30)
#define MSDC1_GPIO_TDSEL0_ADDR   (MSDC1_IO_PAD_BASE + 0xB50)
#define MSDC1_GPIO_TDSEL1_ADDR   (MSDC1_IO_PAD_BASE + 0xB60)
#define MSDC1_GPIO_RDSEL0_ADDR  (MSDC1_IO_PAD_BASE + 0xC40)
#define MSDC1_GPIO_RDSEL1_ADDR  (MSDC1_IO_PAD_BASE + 0xC50)
#define MSDC1_GPIO_DRV0_ADDR (MSDC1_IO_PAD_BASE + 0xD50)
#define MSDC1_GPIO_DRV1_ADDR (MSDC1_IO_PAD_BASE + 0xD60)
#define MSDC1_GPIO_PUPD0_ADDR    (MSDC1_IO_PAD_BASE + 0xE20)
#define MSDC1_GPIO_PUPD1_ADDR    (MSDC1_IO_PAD_BASE + 0xE30)
#define MSDC1_GPIO_PUPD2_ADDR    (MSDC1_IO_PAD_BASE + 0xE40)

#define MSDC2_GPIO_MODEE   (MSDC_GPIO_BASE + 0x3D0)
#define MSDC2_GPIO_MODEF   (MSDC_GPIO_BASE + 0x3E0)
#define MSDC2_GPIO_IES_ADDR (MSDC2_IO_PAD_BASE + 0x930)
#define MSDC2_GPIO_SMT_ADDR (MSDC2_IO_PAD_BASE + 0xA30)
#define MSDC2_GPIO_TDSEL0_ADDR   (MSDC2_IO_PAD_BASE + 0xB40)
#define MSDC2_GPIO_TDSEL1_ADDR   (MSDC2_IO_PAD_BASE + 0xB50)
#define MSDC2_GPIO_RDSEL0_ADDR   (MSDC2_IO_PAD_BASE + 0xC30)
#define MSDC2_GPIO_RDSEL1_ADDR   (MSDC2_IO_PAD_BASE + 0xC40)
#define MSDC2_GPIO_DRV0_ADDR (MSDC2_IO_PAD_BASE + 0xD40)
#define MSDC2_GPIO_DRV1_ADDR (MSDC2_IO_PAD_BASE + 0xD50)
#define MSDC2_GPIO_PUPD0_ADDR    (MSDC2_IO_PAD_BASE + 0xE40)
#define MSDC2_GPIO_PUPD1_ADDR    (MSDC2_IO_PAD_BASE + 0xE50)

/*
 * MSDC0 GPIO and PAD register and bitfields definition
*/
/*MSDC0_GPIO_MODE17, Set as pinmux value as 1*/
#define MSDC0_MODE_DAT7_MASK            (7 <<  0)
#define MSDC0_MODE_DAT6_MASK            (7 <<  3)
#define MSDC0_MODE_DAT5_MASK            (7 <<  6)
#define MSDC0_MODE_DAT4_MASK            (7 <<  9)
#define MSDC0_MODE_RST_MASK             (7 << 12)

/*MSDC0_GPIO_MODE18, Set as pinmux value as 1*/
#define MSDC0_MODE_CMD_MASK             (7 <<  0)
#define MSDC0_MODE_CLK_MASK             (7 <<  3)
#define MSDC0_MODE_DAT3_MASK            (7 << 6)
#define MSDC0_MODE_DAT2_MASK            (7 << 9)
#define MSDC0_MODE_DAT1_MASK            (7 << 12)

/*MSDC0_GPIO_MODE19, Set as pinmux value as 1*/
#define MSDC0_MODE_DAT1_MASK            (7 << 0)

/* MSDC0 IES mask*/
#define MSDC0_IES_CLK_MASK              (0x1  <<  0)
#define MSDC0_IES_CMD_MASK             (0x1  << 1)
#define MSDC0_IES_DAT0_MASK            (0x1  <<  2)
#define MSDC0_IES_DAT1_MASK            (0x1  <<  3)
#define MSDC0_IES_DAT2_MASK            (0x1  <<  4)
#define MSDC0_IES_DAT3_MASK            (0x1  <<  5)
#define MSDC0_IES_DAT4_MASK            (0x1  <<  6)
#define MSDC0_IES_DAT5_MASK            (0x1  <<  7)
#define MSDC0_IES_DAT6_MASK            (0x1  <<  8)
#define MSDC0_IES_DAT7_MASK            (0x1  <<  9)
#define MSDC0_IES_RSTB_MASK            (0x1  <<  10)
#define MSDC0_IES_ALL_MASK              (0x7FF << 0)

/* MSDC0 SMT mask*/
#define MSDC0_SMT_CLK_MASK              (0x1  <<  0)
#define MSDC0_SMT_CMD_MASK             (0x1  << 1)
#define MSDC0_SMT_DAT0_MASK            (0x1  <<  2)
#define MSDC0_SMT_DAT1_MASK            (0x1  <<  3)
#define MSDC0_SMT_DAT2_MASK            (0x1  <<  4)
#define MSDC0_SMT_DAT3_MASK            (0x1  <<  5)
#define MSDC0_SMT_DAT4_MASK            (0x1  <<  6)
#define MSDC0_SMT_DAT5_MASK            (0x1  <<  7)
#define MSDC0_SMT_DAT6_MASK            (0x1  <<  8)
#define MSDC0_SMT_DAT7_MASK            (0x1  <<  9)
#define MSDC0_SMT_RSTB_MASK            (0x1  <<  10)
#define MSDC0_SMT_ALL_MASK              (0x7FF << 0)


/* MSDC0 TDSEL mask*/
#define MSDC0_TDSEL_DAT_MASK            (0xF  <<  0)
#define MSDC0_TDSEL_RSTB_MASK           (0xF  << 4)
#define MSDC0_TDSEL0_MASK                    (0xFF  << 0)

#define MSDC0_TDSEL_CMD_MASK            (0xF  <<  8)
#define MSDC0_TDSEL_CLK_MASK            (0xF  <<  12)
#define MSDC0_TDSEL1_MASK                    (0xFF  << 8)

/* MSDC0 RDSEL mask*/
#define MSDC0_RDSEL_CLK_MASK            (0x3F <<  0)
#define MSDC0_RDSEL_CMD_MASK            (0x3F << 6)
#define MSDC0_RDSEL0_MASK                    (0xFFF<< 0)

#define MSDC0_RDSEL_DAT_MASK            (0x3F <<  0)
#define MSDC0_RDSEL_RSTB_MASK           (0x3F << 6)
#define MSDC0_RDSEL1_MASK                    (0xFFF<< 0)

/* MSDC0 DRV mask*/
#define MSDC0_DRV_CLK_MASK              (0xF  <<  12)
#define MSDC0_DRV_CMD_MASK              (0xF  <<  8)
#define MSDC0_DRV0_MASK                    (0xFF<< 8)

#define MSDC0_DRV_DAT_MASK              (0xF <<  0)
#define MSDC0_DRV_RSTB_MASK             (0xF  << 4)
#define MSDC0_DRV1_MASK                    (0xFF<< 8)

/* MSDC0 PUPD mask*/
#define MSDC0_PUPD_DAT0_MASK            (0xF <<  0)
#define MSDC0_PUPD_DAT1_MASK            (0xF <<  4)
#define MSDC0_PUPD_DAT2_MASK            (0xF <<  8)
#define MSDC0_PUPD_DAT3_MASK            (0xF <<  12)
#define MSDC0_PUPD0_MASK                (0xFFFF << 0)

#define MSDC0_PUPD_DAT4_MASK            (0xF <<  0)
#define MSDC0_PUPD_DAT5_MASK            (0xF <<  4)
#define MSDC0_PUPD_DAT6_MASK            (0xF <<  8)
#define MSDC0_PUPD_DAT7_MASK            (0xF <<  12)
#define MSDC0_PUPD1_MASK                (0xFFFF << 0)

#define MSDC0_PUPD_CMD_MASK             (0xF  <<  0)
#define MSDC0_PUPD_CLK_MASK             (0xF  <<  4)
#define MSDC0_PUPD_RSTB_MASK            (0x7  << 8)
#define MSDC0_PUPD2_MASK                (0xFFF << 0)

#define MSDC_CTRL0	(GPIO_BASE + 0xc40)
#define MSDC0_CLK_SMT	    (0x1 << 13)
#define MSDC0_DRV_CLK_MASK  (0x7 << 8)
#define MSDC0_CLK_PUPD	    (0x1 << 2) /* set to 1 --> pull down */
#define MSDC0_CLK_R1R0	    (0x3) /* set to 0x2 --> 50K */

/* MSDC_CTRL1 is for MSDC0_CMD */
#define MSDC_CTRL1	(GPIO_BASE + 0xc50)
#define MSDC0_CMD_SMT	    (0x1 << 13)
#define MSDC0_DRV_CMD_MASK  (0x7 << 8)
#define MSDC0_CMD_PUPD	    (0x1 << 2) /* set to 0 --> pull up */
#define MSDC0_CMD_R1R0	    (0x3) /* set to 0x1 --> 10K */

/* MSDC_CTRL2 is for MSDC0_DAT */
#define MSDC_CTRL2	(GPIO_BASE + 0xc60)
#define MSDC0_DAT_SMT	    (0x1 << 13)
#define MSDC0_DRV_DAT_MASK  (0x7 << 8)
#define MSDC0_DAT_PUPD	    (0x1 << 2) /* set to 0 --> pull up */
#define MSDC0_DAT_R1R0	    (0x3) /* set to 0x1 --> 10K */

/* MSDC_CTRL4 is for MSDC0_DS */
#define MSDC_CTRL4	(GPIO_BASE + 0xda0)
#define MSDC0_DS_SMT	    (0x1 << 13)
#define MSDC0_DRV_DS_MASK  (0x7 << 8)
#define MSDC0_DS_PUPD	    (0x1 << 2) /* set to 1 --> pull down */
#define MSDC0_DS_R1R0	    (0x3) /* set to 0x2 --> 50K */

#endif

typedef enum __MSDC_PIN_STATE {
     MSDC_PST_PU_HIGHZ = 0,
     MSDC_PST_PU_10KOHM,
     MSDC_PST_PU_50KOHM,
     MSDC_PST_PU_08KOHM,
     MSDC_PST_PD_HIGHZ,
     MSDC_PST_PD_10KOHM,
     MSDC_PST_PD_50KOHM,
     MSDC_PST_PD_08KOHM,
     MSDC_PST_MAX
}MSDC_PIN_STATE;


/* MSDC0/1/2
     PLL MUX SEL List */
enum {
    MSDC45_CLKSRC_26MHZ   = 0,
    MSDC45_CLKSRC_208MHZ,
    MSDC45_CLKSRC_188MHZ,
    MSDC45_CLKSRC_156MHZ,
    MSDC45_CLKSRC_94MHZ,
    MSDC45_DONOTCARE_HCLK,
    MSDC45_CLKSRC_125MHZ,
    MSDC45_CLKSRC_MAX
};

typedef enum MT65XX_POWER_VOL_TAG {
    VOL_DEFAULT,
    VOL_0900 = 900,
    VOL_1000 = 1000,
    VOL_1100 = 1100,
    VOL_1200 = 1200,
    VOL_1300 = 1300,
    VOL_1350 = 1350,
    VOL_1500 = 1500,
    VOL_1800 = 1800,
    VOL_2000 = 2000,
    VOL_2100 = 2100,
    VOL_2500 = 2500,
    VOL_2800 = 2800,
    VOL_3000 = 3000,
    VOL_3300 = 3300,
    VOL_3400 = 3400,
    VOL_3500 = 3500,
    VOL_3600 = 3600
} MT65XX_POWER_VOLTAGE;

/*--------------------------------------------------------------------------*/
/* Descriptor Structure                                                     */
/*--------------------------------------------------------------------------*/
#define DMA_FLAG_NONE       (0x00000000)
#define DMA_FLAG_EN_CHKSUM  (0x00000001)
#define DMA_FLAG_PAD_BLOCK  (0x00000002)
#define DMA_FLAG_PAD_DWORD  (0x00000004)

#define MSDC_WRITE32(addr, data)    writel(data, addr)
#define MSDC_READ32(addr)           readl(addr)
#define MSDC_WRITE8(addr, data)     writeb(data, addr)
#define MSDC_READ8(addr)            readb(addr)

#define MSDC_SET_BIT32(addr,mask) \
    do {    \
        unsigned int tv = MSDC_READ32(addr); \
        tv |=((u32)(mask)); \
        MSDC_WRITE32(addr,tv); \
    } while (0)
#define MSDC_CLR_BIT32(addr,mask) \
    do {    \
        unsigned int tv = MSDC_READ32(addr); \
        tv &= ~((u32)(mask)); \
        MSDC_WRITE32(addr,tv); \
    } while (0)

#define MSDC_SET_FIELD(reg,field,val) \
    do { \
        u32 tv = MSDC_READ32(reg); \
        tv &= ~((u32)(field)); \
        tv |= ((val) << (__builtin_ffs((u32)(field)) - 1)); \
        MSDC_WRITE32(reg, tv); \
    } while (0)

#define MSDC_GET_FIELD(reg,field,val) \
    do { \
        u32 tv = MSDC_READ32(reg); \
        val = ((tv & (field)) >> (__builtin_ffs((u32)(field)) - 1)); \
    } while (0)

#define MSDC_RETRY(expr,retry,cnt) \
    do { \
        uint32_t t = cnt; \
        uint32_t r = retry; \
        uint32_t c = cnt; \
        while (r) { \
            if (!(expr)) break; \
            if (c-- == 0) { \
                r--; spin(200); c = t; \
            } \
        } \
        if (r == 0) \
            dprintf(CRITICAL, "%s->%d: retry %d times failed!\n", __func__, \
                    __LINE__, retry); \
    } while (0)

#define MSDC_RESET() \
    do { \
        MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_RST); \
        MSDC_RETRY(MSDC_READ32(MSDC_CFG) & MSDC_CFG_RST, 5, 1000); \
    } while (0)

#define MSDC_CLR_INT() \
    do { \
        volatile uint32_t val = MSDC_READ32(MSDC_INT); \
        MSDC_WRITE32(MSDC_INT, val); \
    } while (0)

#define MSDC_CLR_FIFO() \
    do { \
        MSDC_SET_BIT32(MSDC_FIFOCS, MSDC_FIFOCS_CLR); \
        MSDC_RETRY(MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_CLR, 5, 1000); \
    } while (0)

#define MSDC_FIFO_WRITE32(val)  MSDC_WRITE32(MSDC_TXDATA, val)
#define MSDC_FIFO_READ32()      MSDC_READ32(MSDC_RXDATA)
#define MSDC_FIFO_WRITE8(val)   MSDC_WRITE8(MSDC_TXDATA, val)
#define MSDC_FIFO_READ8()       MSDC_READ8(MSDC_RXDATA)

#define MSDC_TXFIFOCNT() \
    ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_TXCNT) >> 16)
#define MSDC_RXFIFOCNT() \
    ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_RXCNT) >> 0)

#define SDC_IS_BUSY()       (MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)
#define SDC_IS_CMD_BUSY()   (MSDC_READ32(SDC_STS) & SDC_STS_CMDBUSY)

#define SDC_SEND_CMD(cmd,arg) \
    do { \
        MSDC_WRITE32(SDC_ARG, (arg)); \
        MSDC_WRITE32(SDC_CMD, (cmd)); \
    } while (0)

#define MSDC_DMA_ON     MSDC_CLR_BIT32(MSDC_CFG, MSDC_CFG_PIO);
#define MSDC_DMA_OFF    MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_PIO);

#define MSDC_RELIABLE_WRITE     (0x1 << 0)
#define MSDC_PACKED             (0x1 << 1)
#define MSDC_TAG_REQUEST        (0x1 << 2)
#define MSDC_CONTEXT_ID         (0x1 << 3)
#define MSDC_FORCED_PROG        (0x1 << 4)

#ifdef FPGA_PLATFORM
#define msdc_pin_set(...)
#define msdc_set_smt(...)
#define msdc_set_driving(...)
#endif

int msdc_init(struct mmc_host *host);
void msdc_config_bus(struct mmc_host *host, u32 width);
int msdc_dma_transfer(struct mmc_host *host, struct mmc_data *data);
int msdc_tune_bwrite(struct mmc_host *host, u32 dst, u8 *src, u32 nblks);
int msdc_tune_bread(struct mmc_host *host, u8 *dst, u32 src, u32 nblks);
void msdc_reset_tune_counter(struct mmc_host *host);
int msdc_abort_handler(struct mmc_host *host, int abort_card);
int msdc_tune_read(struct mmc_host *host);
void msdc_config_clock(struct mmc_host *host, int state, u32 hz);
int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd);
void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks);
void msdc_set_autocmd(struct mmc_host *host, int cmd);
int msdc_get_autocmd(struct mmc_host *host);

#endif
