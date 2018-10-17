#include <string.h>
#include <sys/types.h>

#define htobe64 __builtin_bswap64
#define SHA256_BLOCK_SIZE       64

typedef unsigned int __be32;
typedef unsigned long long __be64;

struct sha256_context {
    u32 state[8];
    u64 count;
    u8 buf[SHA256_BLOCK_SIZE];
};

extern void sha256_block_data_order_neon(u32 *digest, const void *data,
                                  unsigned int num_blks);

static inline int sha256_start(struct sha256_context *s_ctx)
{
    struct sha256_context *sctx = s_ctx;
    if(NULL ==s_ctx) return -1; 
    sctx->state[0] = 0x6a09e667UL;
    sctx->state[1] = 0xbb67ae85UL;
    sctx->state[2] = 0x3c6ef372UL;
    sctx->state[3] = 0xa54ff53aUL;
    sctx->state[4] = 0x510e527fUL;
    sctx->state[5] = 0x9b05688cUL;
    sctx->state[6] = 0x1f83d9abUL;
    sctx->state[7] = 0x5be0cd19UL;
    sctx->count = 0;

    return 0;
}

static inline int sha256_padding(struct sha256_context *s_ctx)
{
    struct sha256_context *sctx = (s_ctx);
    const int bit_offset = SHA256_BLOCK_SIZE - sizeof(__be64);
    __be64 *bits = (__be64 *)(sctx->buf + bit_offset);
    unsigned int non_block_align = sctx->count % SHA256_BLOCK_SIZE;

    sctx->buf[non_block_align++] = 0x80;
    if (non_block_align > bit_offset) {
        memset(sctx->buf + non_block_align, 0x0, SHA256_BLOCK_SIZE - non_block_align);
        sha256_block_data_order_neon(sctx, sctx->buf, 1);
        non_block_align = 0;
    }

    memset(sctx->buf + non_block_align, 0x0, bit_offset - non_block_align);
    *bits = __builtin_bswap64(sctx->count << 3);
    sha256_block_data_order_neon(sctx, sctx->buf, 1);

    return 0;
}

static inline void u32_split_u8(u32 val, u8 *p)
{
    *p++ = val >> 24;
    *p++ = val >> 16;
    *p++ = val >> 8;
    *p++ = val;
}

static int sha256_process(struct sha256_context *s_ctx, const u8 *input,
                         unsigned int len)
{
    struct sha256_context *sctx = s_ctx;
    int block_num= len / SHA256_BLOCK_SIZE;;
    unsigned int non_block_align = sctx->count % SHA256_BLOCK_SIZE;
    int fill=SHA256_BLOCK_SIZE - non_block_align;
	
    if(s_ctx ==NULL) return -1;
    sctx->count += len;

    if ((non_block_align + len) >= SHA256_BLOCK_SIZE) {    
        if (non_block_align) {
            memcpy(sctx->buf + non_block_align, input, fill);
            sha256_block_data_order_neon(sctx, sctx->buf, 1);
            input += fill;
			len -= fill;
        }
        len %= SHA256_BLOCK_SIZE;

        if (block_num) {
            sha256_block_data_order_neon(sctx, input, block_num);
            input += block_num * SHA256_BLOCK_SIZE;
        }
        non_block_align = 0;
    }
    if (len)
        memcpy(sctx->buf + non_block_align, input, len);

    return 0;
}

static int sha256_end(struct sha256_context *s_ctx, u8 *out)
{
    unsigned int digest_size = 32;
    struct sha256_context *sctx = s_ctx;
    __be32 *digest = (__be32 *)out;
    int i;

    sha256_padding(sctx);

    for (i = 0; digest_size > 0; i++, digest_size -= sizeof(__be32))
        u32_split_u8(sctx->state[i], digest++);

    *sctx = (struct sha256_context) {};
    return 0;

}

int sha256_hash(const void *input, int len, u8 *output)
{
    struct sha256_context s_ctx;
    memset((void*)&s_ctx, 0, sizeof(s_ctx));
    sha256_start(&s_ctx);
    sha256_process(&s_ctx,input,len);
    sha256_end(&s_ctx,output);
    return 0;
}
