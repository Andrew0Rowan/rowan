/*
 * Copyright (c) 2014, Google Inc. All rights reserved
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
#include <arch/ops.h>
#include <arch/defines.h>

#define LOC_SHIFT		24
#define CLIDR_FIELD_WIDTH	3
#define LEVEL_SHIFT		1
#define DCCISW			0x1

.text

.macro cache_range_op, cache op
    add     x2, x0, x1                  // calculate the end address
    bic     x3, x0, #(CACHE_LINE-1)     // align the start with a cache line
.Lcache_range_op_loop\@:
    \cache  \op, x3
    add     x3, x3, #CACHE_LINE
    cmp     x3, x2
    blo     .Lcache_range_op_loop\@
    dsb     sy
.endm

    /* void arch_flush_cache_range(addr_t start, size_t len); */
FUNCTION(arch_clean_cache_range)
    cache_range_op dc cvac         // clean cache to PoC by MVA
    ret

    /* void arch_flush_invalidate_cache_range(addr_t start, size_t len); */
FUNCTION(arch_clean_invalidate_cache_range)
    cache_range_op dc civac        // clean & invalidate dcache to PoC by MVA
    ret

    /* void arch_invalidate_cache_range(addr_t start, size_t len); */
FUNCTION(arch_invalidate_cache_range)
    cache_range_op dc ivac         // invalidate dcache to PoC by MVA
    ret

    /* void arch_sync_cache_range(addr_t start, size_t len); */
FUNCTION(arch_sync_cache_range)
    cache_range_op dc cvau         // clean dcache to PoU by MVA
    cache_range_op ic ivau         // invalidate icache to PoU by MVA
    ret

FUNCTION(do_dcsw_op)
        cbz     x3, exit
        adr     x14, dcsw_loop_table    // compute inner loop address
        add     x14, x14, x0, lsl #5    // inner loop is 8x32-bit instructions
        mov     x0, x9
        mov     w8, #1
loop1:
        add     x2, x10, x10, lsr #1    // work out 3x current cache level
        lsr     x1, x0, x2              // extract cache type bits from clidr
        and     x1, x1, #7              // mask the bits for current cache only
        cmp     x1, #2                  // see what cache we have at this level
        b.lt    level_done              // nothing to do if no cache or icache

        msr     csselr_el1, x10         // select current cache level in csselr
        isb                             // isb to sych the new cssr&csidr
        mrs     x1, ccsidr_el1          // read the new ccsidr
        and     x2, x1, #7              // extract the length of the cache lines
        add     x2, x2, #4              // add 4 (line length offset)
        ubfx    x4, x1, #3, #10         // maximum way number
        clz     w5, w4                  // bit position of way size increment
        lsl     w9, w4, w5              // w9 = aligned max way number
        lsl     w16, w8, w5             // w16 = way number loop decrement
        orr     w9, w10, w9             // w9 = combine way and cache number
        ubfx    w6, w1, #13, #15        // w6 = max set number
        lsl     w17, w8, w2             // w17 = set number loop decrement
        dsb     sy                      // barrier before we start this level
        br      x14                     // jump to DC operation specific loop

        .macro  dcsw_loop _op
loop2_\_op:
        lsl     w7, w6, w2              // w7 = aligned max set number

loop3_\_op:
        orr     w11, w9, w7             // combine cache, way and set number
        dc      \_op, x11
        subs    w7, w7, w17             // decrement set number
        b.ge    loop3_\_op

        subs    x9, x9, x16             // decrement way number
        b.ge    loop2_\_op

        b       level_done
.endm

level_done:
        add     x10, x10, #2            // increment cache number
        cmp     x3, x10
        b.gt    loop1
        msr     csselr_el1, xzr         // select cache level 0 in csselr
        dsb     sy                      // barrier to complete final cache operation
        isb
exit:
        ret

dcsw_loop_table:
        dcsw_loop isw
        dcsw_loop cisw
        dcsw_loop csw

.macro  dcsw_op shift, fw, ls
        mrs     x9, clidr_el1
        ubfx    x3, x9, \shift, \fw
        lsl     x3, x3, \ls
        mov     x10, xzr
        b       do_dcsw_op
.endm

FUNCTION(_dcsw_op_all)
    mov x0, #DCCISW
    dcsw_op #LOC_SHIFT, #CLIDR_FIELD_WIDTH, #LEVEL_SHIFT
    ret

FUNCTION(_tlbivmalle1)
	tlbi	vmalle1
	ret
