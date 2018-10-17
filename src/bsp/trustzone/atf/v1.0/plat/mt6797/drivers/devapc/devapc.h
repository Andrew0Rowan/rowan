#ifndef _ATF_DAPC_H__
#define _ATF_DAPC_H__
/******************************************************************************
*
 * FUNCTION DEFINATION

******************************************************************************/
int start_devapc();


/******************************************************************************
*
 * STRUCTURE DEFINATION

******************************************************************************/
typedef enum
{
    E_L0=0,
    E_L1,
    E_L2,
    E_L3,
    E_MAX_APC_ATTR,
    E_APC_ATTR_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
} APC_ATTR;

// domain index
typedef enum
{
    E_DOMAIN_0 = 0,
    E_DOMAIN_1 ,
    E_DOMAIN_2 ,
    E_DOMAIN_3 ,
    E_DOMAIN_4 ,
    E_DOMAIN_5 ,
    E_DOMAIN_6 ,
    E_DOMAIN_7 ,
    E_MAX,
    E_MASK_DOM_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
} E_MASK_DOM;

typedef struct {
    unsigned char       d0_permission;
    unsigned char       d1_permission;
} DEVICE_INFO;
#define DAPC_ATTR(DEV_NAME, PERM_ATTR1, PERM_ATTR2) {(unsigned char)PERM_ATTR1, (unsigned char)PERM_ATTR2}

typedef enum
{
    E_NonSec=0,
    E_Sec,
    E_MAX_MDAPC_ATTR,
    E_MDAPC_ATTR_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
} MDAPC_ATTR;


typedef struct {
#if 0 //[NO_FEATURE] This platform does not support MDSYS read permission protection
    MDAPC_ATTR          read_permission;
#endif
    MDAPC_ATTR          write_permission;
} MD_MODULE_INFO;
#define MD_ATTR(DEV_NAME, PERM_ATTR1) {(unsigned char)PERM_ATTR1}

#if 0 //[NO_FEATURE] This platform does not support MDSYS read permission protection
#define MD_ATTR(DEV_NAME, PERM_ATTR1, PERM_ATTR2) {(unsigned char)PERM_ATTR1, (unsigned char)PERM_ATTR2}
#endif
/******************************************************************************
*
 * UTILITY DEFINATION

******************************************************************************/

#define READ_REGISTER_UINT32(reg) \
        (*(volatile unsigned int * const)(uintptr_t)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
        (*(volatile unsigned int * const)(uintptr_t)(reg)) = (val)

#define writel(VAL,REG)  WRITE_REGISTER_UINT32(REG,VAL)
#define readl(REG)       READ_REGISTER_UINT32(REG)

/******************************************************************************
*
 * REGISTER ADDRESS DEFINATION

******************************************************************************/
#define DEVAPC_AO_BASE         0x1000E000
#define DEVAPC_PD_BASE         0x10207000
#define INFRACFG_AO_base       0x10001000

/* Device APC AO */
#define DEVAPC_D0_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0000))

#define DEVAPC_D1_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0100))

#define DEVAPC_D2_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0200))

#define DEVAPC_D3_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0300))

#define DEVAPC_D4_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0400))

#define DEVAPC_D5_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0500))

#define DEVAPC_D6_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0600))

#define DEVAPC_D7_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0700))


#define DEVAPC_MAS_DOM_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A00))
#define DEVAPC_MAS_DOM_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A04))
#define DEVAPC_MAS_DOM_2           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A08))
#define DEVAPC_MAS_DOM_3           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A0C))
#define DEVAPC_MAS_DOM_4           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A10))

#define DEVAPC_MAS_SEC_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0B00))
#define DEVAPC_MAS_SEC_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0B04))

#define DEVAPC_MM_SEC_EN_0         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0E00))
#define DEVAPC_MM_SEC_EN_1         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0E04))
#define DEVAPC_MM_SEC_EN_2         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0E08))

#define DEVAPC_MD_SEC_EN_0         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D00))
#define DEVAPC_MD_SEC_EN_1         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D04))

#define DEVAPC_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F00))
#define DEVAPC_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F04))
#define DEVAPC_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F08))
#define DEVAPC_APC_LOCK_2          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F0C))


/* Device APC PD */
#define DEVAPC_PD_APC_CON          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F00))

#define DEVAPC_D0_VIO_MASK_0       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0000))

#define DEVAPC_D0_VIO_STA_0        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0400))

#define DEVAPC_VIO_DBG0            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0900))
#define DEVAPC_VIO_DBG1            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0904))
#define DEVAPC_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F80))
#define DEVAPC_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F84))
#define DEVAPC_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F88))


/* Device APC Infra_AO */
#define INFRA_AO_SEC_CG_CON0        ((volatile unsigned int*)(INFRACFG_AO_base+0x0F84))
#define INFRA_AO_SEC_CG_CON1        ((volatile unsigned int*)(INFRACFG_AO_base+0x0F88))

/******************************************************************************
*
 * Variable DEFINATION

******************************************************************************/

#define SEJ_CG_PROTECT_BIT         ((0x1) << 5)
#define TRNG_CG_PROTECT_BIT        ((0x1) << 9)

#define MOD_NO_IN_1_DEVAPC              16

#define SECURE_TRANSACTION              1
#define NON_SECURE_TRANSACTION          0

#define MASTER_SPM                      10

#define MASTER_MAX_INDEX                32


#endif
