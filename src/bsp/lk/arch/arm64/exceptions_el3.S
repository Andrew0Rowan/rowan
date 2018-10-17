/*
 * Copyright (c) 2014 Travis Geiselbrecht
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
#include <asm.h>
#include <arch/arm64/mmu.h>
#include <arch/asm_macros.h>

FUNCTION(setup_el3_exception_base)
/* install el3 exception table */
ldr     x0, =.Lel3_exception_base
#if WITH_KERNEL_VM
and     x0, x0, #~(~0 << MMU_KERNEL_SIZE_SHIFT)
#endif
msr     vbar_el3, x0
mov     x0, 0x0
ret

.section .text.boot.vectab
.align 12

/*
 * Here only install the exception table
 * ELx to EL3 for secure os jumping to bl31
 */
.Lel3_exception_base:
FUNCTION(arm64_el3_exception_base)
/* exceptions from lower EL, running arm64 */
.org 0x400
LOCAL_FUNCTION(arm64_sync_exc_lower_el_64)
#if WITH_KERNEL_VM
    mov x6, sp
    and sp, x6, #~(~0 << MMU_KERNEL_SIZE_SHIFT)
#endif
    bl el3_mtk_sip
