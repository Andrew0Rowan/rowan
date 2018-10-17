#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <libfdt.h>
#include <sys/types.h>

#define SPEW_D 0
#define FIT_MAX_HASH_LEN    32

#ifndef CHUNKSZ_SHA1
#define CHUNKSZ_SHA1 (64 * 1024)
#endif

#define IMAGE_ENABLE_TIMESTAMP 0
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define FIT_IMAGES_PATH     "/images"

/* hash/signature node */
#define FDT_HASH_NODE   "hash"
#define FDT_ALGO_NODE   "algo"
#define FDT_VAL_NODE    "value"
#define FDT_SIG_NODE    "signature"
#define FDT_HASHED_NODE "hashed-nodes"
#define FDT_HASHED_STR  "hashed-strings"

/*blob node */
#define BLOB_REQ_NODE   "required"
#define BLOB_NBITS_NODE "rsa,num-bits"
#define BLOB_N0INV_NODE "rsa,n0-inverse"
#define BLOB_RSQU_NODE  "rsa,r-squared"
#define BLOB_MOD_NODE   "rsa,modulus"
#define BLOB_EXP_NODE   "rsa,exponent"

/* image node */
#define FDT_DATA_NODE       "data"

struct fdt_region {
    int offset;
    int size;
};

struct image_region {
    const void *data;
    int size;
};

struct sig_info {
    void *fit_image;
    const void *pubkey;
    struct sig_algo *algo;
    int req_offset;
};

struct hash_algo {
    const char *hash;
    const int hash_len;
    const int pad_len;
    int (*hash_cal)(const struct image_region region[],int region_count, uint8_t *checksum);
    const uint8_t *hash_padding;
};

struct sig_algo {
    const char *rsa;
    struct hash_algo *hash_info;
    int (*sig_verify)(struct sig_info *info,
			const struct fdt_region region[],
			int region_count, uint8_t *sig, uint sig_len);

};

static inline const char *fit_get_name(const void *fit_hdr,
                                       int noffset, int *len)
{
    return fdt_get_name(fit_hdr, noffset, len);
}

struct sig_algo *image_get_sig_algo(const char *name);

int subimage_check_integrity(const void *fit, int rd_noffset);

int rsa_check_enabled(void);
int hash_check_enabled(void);
int fit_verify_sign(const void *fit, int conf_noffset);

#endif
