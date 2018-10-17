#ifndef _SHA2_EXPORT_H_
#define _SHA2_EXPORT_H_


/* ==================== */
/* exported functions   */
/* ==================== */
#define CRYPTO_HW_TZCC
#ifdef CRYPTO_HW_TZCC /* Use TZCC */
    #define sha256 TZCC_SHA256
    extern unsigned int TZCC_SHA256(const unsigned char *in, unsigned int inlen, unsigned char *hash);
#else /* Use GCPU */
#define sha256 GCPU_SHA256
extern unsigned int GCPU_SHA256(const unsigned char *in, unsigned int inlen, unsigned char *hash);
extern unsigned int GCPU_SHA256_test(void);
extern unsigned int GCPU_SHA256_init(void);
#endif

#endif /* _SHA2_EXPORT_H_ */

