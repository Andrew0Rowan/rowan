/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "print.h"
#include "uart.h"
#include <stdarg.h>

static void outchar(const char c)
{
	uart_putc(c);
}

static void outstr(const unsigned char *s)
{
	while (*s) {
		if (*s == '\n')
			outchar('\r');
		outchar(*s++);
	}
}

static void outdec(unsigned long n)
{
	if (n >= 10) {
		outdec(n / 10);
		n %= 10;
	}
	outchar((unsigned char)(n + '0'));
}

static void outhex(unsigned long n, long depth)
{
	if (depth)
		depth--;

	if ((n & ~0xf) || depth) {
		outhex(n >> 4, depth);
		n &= 0xf;
	}

	if (n < 10) {
		outchar((unsigned char)(n + '0'));
	} else {
		outchar((unsigned char)(n - 10 + 'A'));
	}
}

void tl_vprint(char *fmt, va_list vl)
{
	unsigned char c;
	unsigned int reg = 1;	/* argument register number (32-bit) */

	while (*fmt) {
		c = *fmt++;
		switch (c) {
		case '%':
			c = *fmt++;
			switch (c) {
			case 'x':
				outhex(va_arg(vl, unsigned long), 0);
				break;
			case 'B':
				outhex(va_arg(vl, unsigned long), 2);
				break;
			case 'H':
				outhex(va_arg(vl, unsigned long), 4);
				break;
			case 'X':
				outhex(va_arg(vl, unsigned long), 8);
				break;
			case 'l':
				if (*fmt == 'l' && *(fmt + 1) == 'x') {
					u32 ltmp;
					u32 htmp;

					ltmp = va_arg(vl, unsigned int);
					htmp = va_arg(vl, unsigned int);

					outhex(htmp, 8);
					outhex(ltmp, 8);
					fmt += 2;
				}
				break;
			case 'd':
				{
					long l;

					l = va_arg(vl, long);
					if (l < 0) {
						outchar('-');
						l = -l;
					}
					outdec((unsigned long)l);
				}
				break;
			case 'u':
				outdec(va_arg(vl, unsigned long));
				break;
			case 's':
				outstr((const unsigned char *)
				       va_arg(vl, char *));
				break;
			case '%':
				outchar('%');
				break;
			case 'c':
				c = va_arg(vl, int);
				outchar(c);
				break;
			default:
				outchar(' ');
				break;
			}
			reg++;	/* one argument uses 32-bit register */
			break;
		case '\r':
			if (*fmt == '\n')
				fmt++;
			c = '\n';
			// fall through
		case '\n':
			outchar('\r');
			// fall through
		default:
			outchar(c);
		}
	}
}

void tl_printf(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	tl_vprint(fmt, args);
	va_end(args);
}

