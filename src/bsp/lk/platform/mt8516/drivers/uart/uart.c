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

#include <debug.h>
#include <reg.h>
#include <dev/uart.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_uart.h>
#include <string.h>

typedef enum {
    UART1 = UART1_BASE,
    UART2 = UART2_BASE,
    UART3 = UART3_BASE,
    UART4 = UART4_BASE
} MTK_UART;

/* FCR */
#define UART_FCR_FIFOE              (1 << 0)
#define UART_FCR_CLRR               (1 << 1)
#define UART_FCR_CLRT               (1 << 2)
#define UART_FCR_DMA1               (1 << 3)
#define UART_FCR_RXFIFO_1B_TRI      (0 << 6)
#define UART_FCR_RXFIFO_6B_TRI      (1 << 6)
#define UART_FCR_RXFIFO_12B_TRI     (2 << 6)
#define UART_FCR_RXFIFO_RX_TRI      (3 << 6)
#define UART_FCR_TXFIFO_1B_TRI      (0 << 4)
#define UART_FCR_TXFIFO_4B_TRI      (1 << 4)
#define UART_FCR_TXFIFO_8B_TRI      (2 << 4)
#define UART_FCR_TXFIFO_14B_TRI     (3 << 4)

#define UART_FCR_FIFO_INIT          (UART_FCR_FIFOE|UART_FCR_CLRR|UART_FCR_CLRT)
#define UART_FCR_NORMAL             (UART_FCR_FIFO_INIT | \
                                     UART_FCR_TXFIFO_4B_TRI| \
                                     UART_FCR_RXFIFO_12B_TRI)

/* LCR */
#define UART_LCR_BREAK              (1 << 6)
#define UART_LCR_DLAB               (1 << 7)

#define UART_WLS_5                  (0 << 0)
#define UART_WLS_6                  (1 << 0)
#define UART_WLS_7                  (2 << 0)
#define UART_WLS_8                  (3 << 0)
#define UART_WLS_MASK               (3 << 0)

#define UART_1_STOP                 (0 << 2)
#define UART_2_STOP                 (1 << 2)
#define UART_1_5_STOP               (1 << 2)    /* Only when WLS=5 */
#define UART_STOP_MASK              (1 << 2)

#define UART_NONE_PARITY            (0 << 3)
#define UART_ODD_PARITY             (0x1 << 3)
#define UART_EVEN_PARITY            (0x3 << 3)
#define UART_MARK_PARITY            (0x5 << 3)
#define UART_SPACE_PARITY           (0x7 << 3)
#define UART_PARITY_MASK            (0x7 << 3)

/* MCR */
#define UART_MCR_DTR                (1 << 0)
#define UART_MCR_RTS                (1 << 1)
#define UART_MCR_OUT1               (1 << 2)
#define UART_MCR_OUT2               (1 << 3)
#define UART_MCR_LOOP               (1 << 4)
#define UART_MCR_XOFF               (1 << 7)    /* read only */
#define UART_MCR_NORMAL             (UART_MCR_DTR|UART_MCR_RTS)

/* LSR */
#define UART_LSR_DR                 (1 << 0)
#define UART_LSR_OE                 (1 << 1)
#define UART_LSR_PE                 (1 << 2)
#define UART_LSR_FE                 (1 << 3)
#define UART_LSR_BI                 (1 << 4)
#define UART_LSR_THRE               (1 << 5)
#define UART_LSR_TEMT               (1 << 6)
#define UART_LSR_FIFOERR            (1 << 7)

/* MSR */
#define UART_MSR_DCTS               (1 << 0)
#define UART_MSR_DDSR               (1 << 1)
#define UART_MSR_TERI               (1 << 2)
#define UART_MSR_DDCD               (1 << 3)
#define UART_MSR_CTS                (1 << 4)
#define UART_MSR_DSR                (1 << 5)
#define UART_MSR_RI                 (1 << 6)
#define UART_MSR_DCD                (1 << 7)

#define CONFIG_BAUDRATE         921600

#define UART_BASE(uart)                   (uart)

#define UART_RBR(uart)                    (UART_BASE(uart)+0x0)  /* Read only */
#define UART_THR(uart)                    (UART_BASE(uart)+0x0)  /* Write only */
#define UART_IER(uart)                    (UART_BASE(uart)+0x4)
#define UART_IIR(uart)                    (UART_BASE(uart)+0x8)  /* Read only */
#define UART_FCR(uart)                    (UART_BASE(uart)+0x8)  /* Write only */
#define UART_LCR(uart)                    (UART_BASE(uart)+0xc)
#define UART_MCR(uart)                    (UART_BASE(uart)+0x10)
#define UART_LSR(uart)                    (UART_BASE(uart)+0x14)
#define UART_MSR(uart)                    (UART_BASE(uart)+0x18)
#define UART_SCR(uart)                    (UART_BASE(uart)+0x1c)
#define UART_DLL(uart)                    (UART_BASE(uart)+0x0)  /* Only when LCR.DLAB = 1 */
#define UART_DLH(uart)                    (UART_BASE(uart)+0x4)  /* Only when LCR.DLAB = 1 */
#define UART_EFR(uart)                    (UART_BASE(uart)+0x8)  /* Only when LCR = 0xbf */
#define UART_XON1(uart)                   (UART_BASE(uart)+0x10) /* Only when LCR = 0xbf */
#define UART_XON2(uart)                   (UART_BASE(uart)+0x14) /* Only when LCR = 0xbf */
#define UART_XOFF1(uart)                  (UART_BASE(uart)+0x18) /* Only when LCR = 0xbf */
#define UART_XOFF2(uart)                  (UART_BASE(uart)+0x1c) /* Only when LCR = 0xbf */
#define UART_AUTOBAUD_EN(uart)            (UART_BASE(uart)+0x20)
#define UART_HIGHSPEED(uart)              (UART_BASE(uart)+0x24)
#define UART_SAMPLE_COUNT(uart)           (UART_BASE(uart)+0x28)
#define UART_SAMPLE_POINT(uart)           (UART_BASE(uart)+0x2c)
#define UART_AUTOBAUD_REG(uart)           (UART_BASE(uart)+0x30)
#define UART_RATE_FIX_AD(uart)            (UART_BASE(uart)+0x34)
#define UART_AUTOBAUD_SAMPLE(uart)        (UART_BASE(uart)+0x38)
#define UART_GUARD(uart)                  (UART_BASE(uart)+0x3c)
#define UART_ESCAPE_DAT(uart)             (UART_BASE(uart)+0x40)
#define UART_ESCAPE_EN(uart)              (UART_BASE(uart)+0x44)
#define UART_SLEEP_EN(uart)               (UART_BASE(uart)+0x48)
#define UART_VFIFO_EN(uart)               (UART_BASE(uart)+0x4c)
#define UART_RXTRI_AD(uart)               (UART_BASE(uart)+0x50)

// output uart port
volatile addr_t g_uart;

//extern unsigned int mtk_get_bus_freq(void);
#define UART_SRC_CLK 26000000

static void uart_setbrg(void)
{
    unsigned int byte,speed;
    unsigned int highspeed;
    unsigned int quot, divisor, remainder;
    unsigned int uartclk;
    unsigned short data, high_speed_div, sample_count, sample_point;
    unsigned int tmp_div;

    speed = CONFIG_BAUDRATE;
    uartclk = UART_SRC_CLK;
    //uartclk = (unsigned int)(mtk_get_bus_freq()*1000/4);
    if (speed <= 115200 ) {
        highspeed = 0;
        quot = 16;
    } else {
        highspeed = 3;
        quot = 1;
    }

    if (highspeed < 3) { /*0~2*/
        /* Set divisor DLL and DLH  */
        divisor   =  uartclk / (quot * speed);
        remainder =  uartclk % (quot * speed);

        if (remainder >= (quot / 2) * speed)
            divisor += 1;

        writel(highspeed, UART_HIGHSPEED(g_uart));
        byte = readl(UART_LCR(g_uart));   /* DLAB start */
        writel((byte | UART_LCR_DLAB), UART_LCR(g_uart));
        writel((divisor & 0x00ff), UART_DLL(g_uart));
        writel(((divisor >> 8)&0x00ff), UART_DLH(g_uart));
        writel(byte, UART_LCR(g_uart));   /* DLAB end */
    } else {
        data=(unsigned short)(uartclk/speed);
        high_speed_div = (data>>8) + 1; // divided by 256

        tmp_div=uartclk/(speed*high_speed_div);
        divisor =  (unsigned short)tmp_div;

        remainder = (uartclk)%(high_speed_div*speed);
        /*get (sample_count+1)*/
        if (remainder >= ((speed)*(high_speed_div))>>1)
            divisor =  (unsigned short)(tmp_div+1);
        else
            divisor =  (unsigned short)tmp_div;

        sample_count=divisor-1;

        /*get the sample point*/
        sample_point=(sample_count-1)>>1;

        /*configure register*/
        writel(highspeed, UART_HIGHSPEED(g_uart));

        byte = readl(UART_LCR(g_uart));    /* DLAB start */
        writel((byte | UART_LCR_DLAB), UART_LCR(g_uart));
        writel((high_speed_div & 0x00ff), UART_DLL(g_uart));
        writel(((high_speed_div >> 8)&0x00ff), UART_DLH(g_uart));
        writel(sample_count, UART_SAMPLE_COUNT(g_uart));
        writel(sample_point, UART_SAMPLE_POINT(g_uart));
        writel(byte, UART_LCR(g_uart));   /* DLAB end */
    }
}

static void mtk_set_current_uart(MTK_UART uart_base)
{
    g_uart = uart_base;
}

void uart_init_early(void)
{
    mtk_set_current_uart(UART1);
    /* clear fifo */
    writel(readl(UART_FCR(g_uart)) + UART_FCR_FIFO_INIT, UART_FCR(g_uart));
    writel(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
    uart_setbrg();
}

void uart_init(void)
{
}

int uart_putc(int port, char c)
{
    while (!(readl(UART_LSR(g_uart)) & UART_LSR_THRE));

    if (c == '\n')
        writel((unsigned int)'\r', UART_THR(g_uart));

    writel((unsigned int)c, UART_THR(g_uart));

    return 0;
}

int uart_getc(int port, bool wait)  /* returns -1 if no data available */
{
    while (!(readl(UART_LSR(g_uart)) & UART_LSR_DR));
    return (int)readl(UART_RBR(g_uart));
}

int uart_pputc(int port, char c)
{
    return uart_putc(port, c);
}

int uart_pgetc(int port)
{
    return uart_getc(port, 0);
}

bool check_uart_enter(void)
{
    if ((int)readl(UART_RBR(g_uart)) == 13)
        return true;
    else
        return false;
}
