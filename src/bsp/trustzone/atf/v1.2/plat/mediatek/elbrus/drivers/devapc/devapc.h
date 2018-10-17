#ifndef _ATF_DAPC_H__
#define _ATF_DAPC_H__
/******************************************************************************
*
 * FUNCTION DEFINATION

******************************************************************************/
int start_devapc(void);


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
    E_MAX_MASK_DOM,
    E_MASK_DOM_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
} E_MASK_DOM;

typedef struct {
    unsigned char       d0_permission;
    unsigned char       d1_permission;
} INFRA_PERI_DEVICE_INFO;

#define DAPC_INFRA_PERI_ATTR(DEV_NAME, PERM_ATTR1, PERM_ATTR2) {(unsigned char)PERM_ATTR1, (unsigned char)PERM_ATTR2}

typedef struct {
    unsigned char       d0_permission;
} MM_MD_DEVICE_INFO;

#define DAPC_MM_MD_ATTR(DEV_NAME, PERM_ATTR1) {(unsigned char)PERM_ATTR1}

/******************************************************************************
*
 * UTILITY DEFINATION

******************************************************************************/

#define READ_REGISTER_UINT32(reg) \
        (*(volatile unsigned int * const)(uintptr_t)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
        (*(volatile unsigned int * const)(uintptr_t)(reg)) = (val)

#define devapc_writel(VAL,REG)  WRITE_REGISTER_UINT32(REG,VAL)
#define devapc_readl(REG)       READ_REGISTER_UINT32(REG)

/******************************************************************************
*
 * REGISTER ADDRESS DEFINATION

******************************************************************************/
#define DEVAPC_AO_BASE         0x10070000
#define DEVAPC_AO_MM_BASE      0x10071000
#define DEVAPC_AO_MD_BASE      0x10072000
#define DEVAPC_PD_BASE         0x10222000
#define INFRACFG_AO_base       0x10000000

/* Device APC AO for INFRA/PERI */
#define DEVAPC0_D0_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0000))
#define DEVAPC0_D1_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0100))
#define DEVAPC0_D2_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0200))
#define DEVAPC0_D3_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0300))
#define DEVAPC0_D4_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0400))
#define DEVAPC0_D5_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0500))
#define DEVAPC0_D6_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0600))
#define DEVAPC0_D7_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0700))

#define DEVAPC0_MAS_DOM_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A00))
#define DEVAPC0_MAS_DOM_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A04))
#define DEVAPC0_MAS_DOM_2           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A08))
#define DEVAPC0_MAS_DOM_3           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A0C))
#define DEVAPC0_MAS_DOM_4           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A10))
#define DEVAPC0_MAS_DOM_5           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A14))
#define DEVAPC0_MAS_DOM_6           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A18))
#define DEVAPC0_MAS_DOM_7           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A1C))
#define DEVAPC0_MAS_DOM_8           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A20))
#define DEVAPC0_MAS_DOM_9           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A24))
#define DEVAPC0_MAS_DOM_10          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A28))
#define DEVAPC0_MAS_DOM_11          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A2C))

#define DEVAPC0_MAS_SEC_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0B00))
#define DEVAPC0_MAS_SEC_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0B04))

#define DEVAPC0_DOM_RMP_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D00))
#define DEVAPC0_DOM_RMP_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D04))
#define DEVAPC0_DOM_RMP_2           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D08))
#define DEVAPC0_DOM_RMP_3           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D0C))

#define DEVAPC0_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F00))
#define DEVAPC0_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F04))
#define DEVAPC0_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F08))
#define DEVAPC0_APC_LOCK_2          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F0C))
#define DEVAPC0_APC_LOCK_3          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F10))
#define DEVAPC0_APC_LOCK_4          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F14))
#define DEVAPC0_APC_LOCK_5          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F18))


/* Device APC AO for MM */
#define DEVAPC1_D0_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0000))
#define DEVAPC1_D1_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0100))
#define DEVAPC1_D2_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0200))
#define DEVAPC1_D3_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0300))

#define DEVAPC1_DOM_RMP_0           ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0D00))
#define DEVAPC1_DOM_RMP_1           ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0D04))

#define DEVAPC1_MM_SEC_0            ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0E00))

#define DEVAPC1_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0F00))
#define DEVAPC1_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0F04))
#define DEVAPC1_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0F08))
#define DEVAPC1_APC_LOCK_2          ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0F0C))
#define DEVAPC1_APC_LOCK_3          ((volatile unsigned int*)(DEVAPC_AO_MM_BASE+0x0F10))


/* Device APC AO for MD */
#define DEVAPC2_D0_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0000))
#define DEVAPC2_D1_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0100))
#define DEVAPC2_D2_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0200))
#define DEVAPC2_D3_APC_0            ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0300))

#define DEVAPC2_DOM_RMP_0           ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0D00))
#define DEVAPC2_DOM_RMP_1           ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0D04))

#define DEVAPC2_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0F00))
#define DEVAPC2_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0F04))
#define DEVAPC2_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC_AO_MD_BASE+0x0F08))


/* Device APC PD */
#define DEVAPC0_D0_VIO_MASK_0       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0000))
#define DEVAPC0_D0_VIO_MASK_1       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0004))
#define DEVAPC0_D0_VIO_MASK_2       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0008))
#define DEVAPC0_D0_VIO_MASK_3       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x000C))
#define DEVAPC0_D0_VIO_MASK_4       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0010))
#define DEVAPC0_D0_VIO_MASK_5       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0014))
#define DEVAPC0_D0_VIO_MASK_6       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0018))
#define DEVAPC0_D0_VIO_MASK_7       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x001C))
#define DEVAPC0_D0_VIO_MASK_8       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0020))
#define DEVAPC0_D0_VIO_MASK_9       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0024))

#define DEVAPC0_D0_VIO_STA_0        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0400))
#define DEVAPC0_D0_VIO_STA_1        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0404))
#define DEVAPC0_D0_VIO_STA_2        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0408))
#define DEVAPC0_D0_VIO_STA_3        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x040C))
#define DEVAPC0_D0_VIO_STA_4        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0410))
#define DEVAPC0_D0_VIO_STA_5        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0414))
#define DEVAPC0_D0_VIO_STA_6        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0418))
#define DEVAPC0_D0_VIO_STA_7        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x041C))
#define DEVAPC0_D0_VIO_STA_8        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0420))
#define DEVAPC0_D0_VIO_STA_9        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0424))

#define DEVAPC0_VIO_DBG0            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0900))
#define DEVAPC0_VIO_DBG1            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0904))

#define DEVAPC0_PD_APC_CON          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F00))

#define DEVAPC0_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F80))
#define DEVAPC0_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F84))
#define DEVAPC0_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F88))


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

#define MASTER_SPM                      6

#define MASTER_MAX_INDEX                45

#define INFRA_PERI_VIO_INDEX            0
#define MM_VIO_INDEX                    184
#define MD1_VIO_INDEX                   180
#define MD3_VIO_INDEX                   182

#endif
