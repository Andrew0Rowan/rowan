#ifndef _RSA_H
#define _RSA_H

#include <errno.h>
#include "image.h"

#define RSA2048_BYTES   (2048 / 8)
#define RSA4096_BYTES   (4096 / 8)

/* This is the minimum/maximum key size we support, in bits */
#define RSA_MIN_KEY_BITS    2048
#define RSA_MAX_KEY_BITS    4096

/* This is the maximum signature length that we support, in bits */
#define RSA_MAX_SIG_BITS    4096

struct key_prop {

    const void *rr;
    const void *modulus;
    const void *public_exponent;
    uint32_t n0inv;
    int num_bits;
    uint32_t exp_len;
};

/**
 * struct rsa_public_key - holder for a public key
 *
 * An RSA public key consists of a modulus (typically called N), the inverse
 * and R^2, where R is 2^(# key bits).
 */
struct rsa_public_key {
    uint len;           /* len of modulus[] in number of uint32_t */
    uint32_t n0inv;     /* -1 / modulus[0] mod 2^32 */
    uint32_t *modulus;  /* modulus as little endian array */
    uint32_t *rr;       /* R^2 as little endian array */
    uint64_t exponent;  /* public exponent */
};

int rsa_verify(struct sig_info *info,
               const struct fdt_region region[], int region_count,
               uint8_t *sig, uint sig_len);

#endif
