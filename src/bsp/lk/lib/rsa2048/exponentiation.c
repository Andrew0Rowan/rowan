
#if defined(__arm__)

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <debug.h>
#if defined(__aarch64__)
typedef __uint128_t uint128_t;
typedef uint128_t uintptr2_t;
#elif defined(__arm__)
typedef uint64_t uintptr2_t;
#else
#error Unsupport architecture
#endif

#define N_BYTES (2048 / 8)
#define N_WORDS (N_BYTES / sizeof(uintptr_t))

int bn_mul_mont(uintptr_t *rp, const uintptr_t *ap, const uintptr_t *bp,
                const uintptr_t *np, const uint64_t *n0, int num);

static uint64_t mod_inverse(uint64_t n)
{
    uint64_t a = (uint64_t)(-n), b = n;
    uint64_t x = 1, y = 1;
    bool neg = false;
    while (b != 0) {
        uint64_t q = a / b, r = a % b, t = x + q * y;
        x = y;
        y = t;
        a = b;
        b = r;
        neg = !neg;
    }
    return neg ? (uint64_t)(-x) : x;
}

static uintptr_t add(uintptr_t *d, const uintptr_t *n)
{
    uintptr_t r, t, i = N_WORDS;

    //dprintf(CRITICAL, "add  1 \n");
#if defined(__aarch64__)
    asm volatile (
        "adds	xzr, xzr, xzr\n"
        "1:	ldr	%[r], [%[d]]\n"
        "	ldr	%[t], [%[n]], #8\n"
        "	sub	%[i], %[i], #1\n"
        "	adcs	%[r], %[r], %[t]\n"
        "	str	%[r], [%[d]], #8\n"
        "	cbnz	%[i], 1b\n"
        "	adcs	%[r], xzr, xzr"
        : [d] "+r" (d), [n] "+r" (n), [i] "+r" (i),
        [r] "=r" (r), [t] "=r" (t)
        :
        : "memory", "cc"
    );
#elif defined(__arm__)
    asm volatile (
        "cmn	%[i], #0\n"
        "1:	ldr	%[r], [%[d]]\n"
        "	ldr	%[t], [%[n]], #4\n"
        "	sub	%[i], #1\n"
        "	adcs	%[r], %[t]\n"
        "	teq	%[i], #0\n"
        "	str	%[r], [%[d]], #4\n"
        "	bne	1b\n"
        "	adcs	%[r], %[i], #0"
        : [d] "+r" (d), [n] "+r" (n), [i] "+r" (i),
        [r] "=r" (r), [t] "=r" (t)
        :
        : "memory", "cc"
    );
#endif
    return r;
}

static uintptr_t msub(uintptr_t *d, const uintptr_t *n, uintptr_t m)
{
    uintptr_t r = 0, s, t, u, i = N_WORDS;

    //dprintf(CRITICAL, "msub  1 \n");
#if defined(__aarch64__)
    asm volatile (
        "subs	xzr, xzr, xzr\n"
        "1:	ldr	%[s], [%[d]]\n"
        "	ldr	%[t], [%[n]], #8\n"
        "	sub	%[i], %[i], #1\n"
        "	mul	%[u], %[t], %[m]\n"
        "	umulh	%[t], %[t], %[m]\n"
        "	sbcs	%[s], %[s], %[r]\n"
        "	cinc	%[r], %[t], cc\n"
        "	subs	%[s], %[s], %[u]\n"
        "	str	%[s], [%[d]], #8\n"
        "	cbnz	%[i], 1b\n"
        "	sbcs	%[r], xzr, %[r]"
        : [d] "+r" (d), [n] "+r" (n), [i] "+r" (i),
        [r] "+r" (r), [s] "=r" (s), [t] "=r" (t), [u] "=r" (u)
        : [m] "r" (m)
        : "memory", "cc"
    );
#elif defined(__arm__)
    asm volatile (
        "cmp	%[i], #0\n"
        "1:	ldr	%[t], [%[n]], #4\n"
        "	mov	%[u], %[r]\n"
        "	mov	%[r], #0\n"
        "	ldr	%[s], [%[d]]\n"
        "	umlal	%[u], %[r], %[t], %[m]\n"
        "	sub	%[i], %[i], #1\n"
        "	sbcs	%[s], %[s], %[u]\n"
        "	teq	%[i], #0\n"
        "	str	%[s], [%[d]], #4\n"
        "	bne	1b\n"
        "	sbcs	%[r], %[i], %[r]"
        : [d] "+r" (d), [n] "+r" (n), [i] "+r" (i),
        [r] "+r" (r), [s] "=r" (s), [t] "=r" (t), [u] "=r" (u),
        [m] "+r" (m)
        :
        : "memory", "cc"
    );
#endif
    return r;
}

static void bn_to_montgomery(uintptr_t *ar, const uintptr_t *a, const uintptr_t *m)
{
    const uintptr_t d1 = m[N_WORDS - 1];
    uintptr_t n[N_WORDS * 2];
    intptr_t i;

    memset(n, 0, N_BYTES);
    memcpy(n + N_WORDS, a, N_BYTES);

    for (i = N_WORDS - 1; i >= 0; i--) {
        const uintptr_t q = (n[i + N_WORDS] != d1) ? *(uintptr2_t *)(n + i + N_WORDS - 1) / d1 : (uintptr_t)(-1);
        n[i + N_WORDS] += msub(n + i, m, q);
        while (n[i + N_WORDS]) n[i + N_WORDS] += add(n + i, m);
    }

    memcpy(ar, n, N_BYTES);
}

void mod_exp_65537_mont(uintptr_t *r, const uintptr_t *a, const uintptr_t *m)
{
    const uint64_t n0 = mod_inverse(*(const uint64_t *)m);
    uintptr_t ar[N_WORDS];
    size_t i ,j;

    bn_to_montgomery(ar, a, m);
    bn_mul_mont(r, ar, ar, m, &n0, N_WORDS);
    for (i = 15; i != 0; i--)
        bn_mul_mont(r, r, r, m, &n0, N_WORDS);
    bn_mul_mont(r, r, ar, m, &n0, N_WORDS);
    memset(ar, 0, sizeof(ar));
    ar[0] = 1;
    bn_mul_mont(r, r, ar, m, &n0, N_WORDS);

}
#endif
