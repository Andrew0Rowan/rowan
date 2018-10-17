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

#include <arch/ops.h>
#include <debug.h>
#include <platform/mt_reg_base.h>
#include <reg.h>
#include <stdlib.h>
#include <string.h>

#if CFG_FPGA_PLATFORM
#define MTK_I2C_SOURCE_CLK 12000
#define MTK_I2C_CLK_DIV 1
#else
#define MTK_I2C_SOURCE_CLK 68250
#define MTK_I2C_CLK_DIV 1
#endif

#define MTK_I2C0_BASE (IO_PHYS + 0x01009000)
#define MTK_I2C1_BASE (IO_PHYS + 0x0100A000)
#define MTK_I2C2_BASE (IO_PHYS + 0x0100B000)

#define MTK_I2C0_DMA (IO_PHYS + 0x01000180)
#define MTK_I2C1_DMA (IO_PHYS + 0x01000200)
#define MTK_I2C2_DMA (IO_PHYS + 0x01000280)

#define MTK_I2C0_GIC_IRQ 112
#define MTK_I2C1_GIC_IRQ 113
#define MTK_I2C2_GIC_IRQ 114

#define MTK_GPIO_I2C_BASE0 (IO_PHYS + 0x000053B0)
#define MTK_GPIO_I2C_BASE1 (IO_PHYS + 0x000053A0)
#define MTK_GPIO_I2C_BASE2 (IO_PHYS + 0x000053C0)

#define MTK_GPIO_SDA0 9
#define MTK_GPIO_SCL0 12
#define MTK_GPIO_SDA1 6
#define MTK_GPIO_SCL1 9
#define MTK_GPIO_SDA2 0
#define MTK_GPIO_SCL2 3

#define MTK_I2C_INFRACFG (IO_PHYS + 0x00001080)
#define MTK_I2C_CLK_SET0 (IO_PHYS + 0x00000054)
#define MTK_I2C_CLK_CLR0 (IO_PHYS + 0x00000084)
#define MTK_I2C_CLK_STA0 (IO_PHYS + 0x00000024)
#define MTK_I2C0_CLK_OFFSET (0x1 << 3)
#define MTK_I2C1_CLK_OFFSET (0x1 << 4)
#define MTK_I2C2_CLK_OFFSET (0x1 << 16)
#define MTK_APDMA_CLK_OFFSET (0x1 << 2)

#define I2C_CONTROL_RS                    (0x1 << 1)
#define I2C_CONTROL_DMA_EN                (0x1 << 2)
#define I2C_CONTROL_CLK_EXT_EN            (0x1 << 3)
#define I2C_CONTROL_DIR_CHANGE            (0x1 << 4)
#define I2C_CONTROL_ACKERR_DET_EN         (0x1 << 5)
#define I2C_CONTROL_TRANSFER_LEN_CHANGE   (0x1 << 6)
#define I2C_CONTROL_WRAPPER               (0x1 << 0)

#define I2C_RS_TRANSFER             (1 << 4)
#define I2C_HS_NACKERR              (1 << 2)
#define I2C_ACKERR                  (1 << 1)
#define I2C_TRANSAC_COMP            (1 << 0)
#define I2C_TRANSAC_START           (1 << 0)
#define I2C_RS_MUL_CNFG             (1 << 15)
#define I2C_RS_MUL_TRIG             (1 << 14)

#define I2C_DCM_DISABLE             0x0000
#define I2C_IO_CONFIG_OPEN_DRAIN    0x0003
#define I2C_IO_CONFIG_PUSH_PULL     0x0000
#define I2C_SOFT_RST                0x0001
#define I2C_FIFO_ADDR_CLR           0x0001
#define I2C_DELAY_LEN               0x0002
#define I2C_ST_START_CON            0x8001
#define I2C_FS_START_CON            0x1800
#define I2C_TIME_CLR_VALUE          0x0000
#define I2C_TIME_DEFAULT_VALUE      0x0003
#define I2C_WRRD_TRANAC_VALUE       0x0002
#define I2C_RD_TRANAC_VALUE         0x0001
#define I2C_DMA_CON_TX              0x0000
#define I2C_DMA_CON_RX              0x0001
#define I2C_DMA_START_EN            0x0001
#define I2C_DMA_INT_FLAG_NONE       0x0000
#define I2C_DMA_CLR_FLAG            0x0000
#define I2C_DMA_HARD_RST            0x0002

#define I2C_FIFO_SIZE             8
#define I2C_DEFAULT_CLK_DIV       5
#define MAX_ST_MODE_SPEED         100   /* khz */
#define MAX_FS_MODE_SPEED         400   /* khz */
#define MAX_HS_MODE_SPEED         3400  /* khz */
#define MAX_DMA_TRANS_SIZE        65532 /* max(65535) aligned to 4 bytes = 65532 */
#define MAX_DMA_TRANS_NUM         256
#define MAX_SAMPLE_CNT_DIV        8
#define MAX_STEP_CNT_DIV          64
#define MAX_HS_STEP_CNT_DIV       8
#define DMA_ADDRESS_HIGH          (0xC0000000)

#define I2C_HS_NACKERR            (1 << 2)
#define I2C_ACKERR                (1 << 1)
#define I2C_TRANSAC_COMP          (1 << 0)

#define I2C_OK         0
#define EAGAIN_I2C     11  /* try again */
#define EINVAL_I2C     22  /* invalid argument */
#define EOPNOTSUPP_I2C 95  /* operation not supported on transport endpoint */
#define ETIMEDOUT_I2C  110 /* connection timed out */
#define EREMOTEIO_I2C  121 /* remote I/O error */

#define I2CTAG         "[I2C] "

#define DIV_ROUND_UP(x,y) (((x) + ((y) - 1)) / (y))

enum I2C_REGS_OFFSET {
    OFFSET_DATA_PORT            = 0x0,
    OFFSET_SLAVE_ADDR           = 0x04,
    OFFSET_INTR_MASK            = 0x08,
    OFFSET_INTR_STAT            = 0x0C,
    OFFSET_CONTROL              = 0x10,
    OFFSET_TRANSFER_LEN         = 0x14,
    OFFSET_TRANSAC_LEN          = 0x18,
    OFFSET_DELAY_LEN            = 0x1C,
    OFFSET_TIMING               = 0x20,
    OFFSET_START                = 0x24,
    OFFSET_EXT_CONF             = 0x28,
    OFFSET_FIFO_STAT            = 0x30,
    OFFSET_FIFO_THRESH          = 0x34,
    OFFSET_FIFO_ADDR_CLR        = 0x38,
    OFFSET_IO_CONFIG            = 0x40,
    OFFSET_RSV_DEBUG            = 0x44,
    OFFSET_HS                   = 0x48,
    OFFSET_SOFTRESET            = 0x50,
    OFFSET_DCM_EN               = 0x54,
    OFFSET_DEBUGSTAT            = 0x64,
    OFFSET_DEBUGCTRL            = 0x68,
    OFFSET_TRANSFER_LEN_AUX     = 0x6C,
    OFFSET_CLOCK_DIV            = 0x70,
    OFFSET_SCL_HL_RATIO         = 0x74,
    OFFSET_SCL_HS_HL_RATIO      = 0x78,
    OFFSET_SCL_MIS_COMP_POINT   = 0x7C,
    OFFSET_STA_STOP_AC_TIME     = 0x80,
    OFFSET_HS_STA_STOP_AC_TIME  = 0x84,
    OFFSET_DATA_TIME            = 0x88,
};

enum DMA_REGS_OFFSET {
    OFFSET_INT_FLAG       = 0x0,
    OFFSET_INT_EN         = 0x04,
    OFFSET_EN             = 0x08,
    OFFSET_RST            = 0x0C,
    OFFSET_CON            = 0x18,
    OFFSET_TX_MEM_ADDR    = 0x1C,
    OFFSET_RX_MEM_ADDR    = 0x20,
    OFFSET_TX_LEN         = 0x24,
    OFFSET_RX_LEN         = 0x28,
};

typedef enum {
    ST_MODE,
    FS_MODE,
    HS_MODE,
} I2C_SPEED_MODE;

enum mt_trans_op {
    I2C_MASTER_NONE = 0,
    I2C_MASTER_WR = 1,
    I2C_MASTER_RD,
    I2C_MASTER_WRRD,
};

struct i2c_msg {
    u16 addr;                       /* slave address */
    u16 flags;
#define I2C_M_TEN           0x0010  /* this is a ten bit chip address */
#define I2C_M_RD            0x0001  /* read data, from slave to master */
#define I2C_M_STOP          0x8000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART       0x4000  /* if I2C_FUNC_NOSTART */
#define I2C_M_REV_DIR_ADDR  0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK    0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK     0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN      0x0400  /* length will be first received byte */
    u16 len;                        /* msg length */
    u8 *buf;                        /* pointer to msg data */
};

struct mt_i2c {
    bool   dma_en;
    bool   poll_en;
    bool   pushpull;                /* open drain */
    bool   filter_msg;              /* filter msg error log */
    bool   dcm;
    bool   auto_restart;
    bool   aux_len_reg;
    bool   hscode;
    bool   msg_complete;
    u8     addr;                    /* slave device 7bit addr */
    u8     mode;                    /* ST/FS/HS mode */
    u16    id;
    u16    irqnr;                   /* i2c interrupt number */
    u16    irq_stat;                /* i2c interrupt status */
    u16    delay_len;               /* num of half pulse between transfers */
    u16    timing_reg;
    u16    high_speed_reg;
    u16    control_reg;
    u16    ext_time;
    u16    scl_ratio;
    u16    scl_hs_ratio;
    u16    scl_mis_comp;
    u16    sta_stop_time;
    u16    hs_sta_stop_time;
    u16    data_time;
    u32    clk;                     /* source clock khz */
    u32    clk_src_div;
    u32    speed;                   /* khz */
    addr_t base;                    /* i2c base addr */
    addr_t pdmabase;
    u8     *tx_buff;
    u8     *rx_buff;
    enum   mt_trans_op op;
};

int mtk_i2c_transfer(struct mt_i2c *i2c, struct i2c_msg msgs[], int num);

int mtk_i2c_read(u16 bus_num, u8 device_addr,
                 u32 speed_khz, u8 *buffer, u32 len);
int mtk_i2c_write(u16 bus_num, u8 device_addr,
                  u32 speed_khz, u8 *buffer, u32 len);
int mtk_i2c_write_read(u16 bus_num, u8 device_addr, u32 speed_khz,
                       u8 *write_buffer, u8 *read_buffer,
                       u32 write_len, u32 read_len);
