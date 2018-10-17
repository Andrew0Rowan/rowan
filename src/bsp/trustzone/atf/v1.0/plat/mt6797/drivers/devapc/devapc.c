#include <stdio.h>
#include "platform.h"
#include "plat_private.h"   //for atf_arg_t_ptr
#include "devapc.h"

#if !defined(SPD_no)
//For TEE
const static DEVICE_INFO D_APC0_Devices_TEE[] = {
    /* 0 */
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_IOCFG_CONTROL_REG",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_DRAM_CONTROLLER",                E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_GPIO_CONTROLLER",                E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_RESET_GENERATOR",      E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_GPT",                            E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_RSVD",                           E_L0,  E_L3 ),

    /* 10 */
    DAPC_ATTR("INFRA_AO_SEJ",                            E_L1,  E_L2 ),
    DAPC_ATTR("INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_APMIXEDSYS",                     E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_DEVICE_APC_AO",                  E_L1,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG",             E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_CKSYS_DMY",                      E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG_B",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DRAMC_CONTROL_REG_B",            E_L0,  E_L3 ),

    /* 20 */
    DAPC_ATTR("INFRA_AO_CLDMA_AO_AP_REG_B",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_CLDMA_AO_MD_REG_B",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_AES_TOP0",                       E_L3,  E_L0 ),
    DAPC_ATTR("INFRA_AO_AES_TOP1",                       E_L3,  E_L0 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG_C",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG_D",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_MCUMIXEDSYS",                    E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_C2K_MISC",                       E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_MODEM_TEMP_SHARE",               E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_RESERVED",                       E_L0,  E_L3 ),

    /* 30 */
    DAPC_ATTR("INFRA_AO_SCP",                            E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_EMI_MPU",                        E_L1,  E_L3 ),
    DAPC_ATTR("INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_BOOTROM_SRAM",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_M4U_CONFIGURATION",              E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_EFUSEC",                         E_L2,  E_L0 ),
    DAPC_ATTR("INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DBG_TRACKER",                    E_L0,  E_L3 ),

    /* 40 */
    DAPC_ATTR("INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_CA7_MD1_CCIF",                   E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_CA7_AP_C2K_CCIF",                E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CA7_MD_C2K_CCIF",                E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DRAMC_NAO_REGION_REG",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_TRNG",                           E_L1,  E_L3 ),
    DAPC_ATTR("INFRASYS_GCPU",                           E_L1,  E_L3 ),
    DAPC_ATTR("INFRASYS_MD2MD_MD1_CCIF",                 E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_GCE",                            E_L0,  E_L3 ),

    /* 50 */
    DAPC_ATTR("INFRASYS_MD2MD_MD2_CCIF",                 E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_PERISYS_IOMMU",                  E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DSI0",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DRAMC_NAO_REGION_REG_B",         E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CSI0",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CSI1",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CLDMA_AP",                       E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_CLDMA_MD",                       E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_RESERVED",                       E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MDHW_SMI",                       E_L0,  E_L0 ),

    /* 60 */
    DAPC_ATTR("INFRASYS_GCPU_RSA",                       E_L1,  E_L3 ),
    DAPC_ATTR("INFRASYS_DSI1",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_VAD",                            E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS",                         E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CPUSYS",                         E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS1",                        E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CPUSYS1",                        E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD0",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD1",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD2",                   E_L0,  E_L3 ),

    /* 70 */
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD3",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_0",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_1",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_2",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_3",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_4",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_5",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_6",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_7",                      E_L0,  E_L0 ),
    DAPC_ATTR("DEBUG_CORESIGHT",                         E_L0,  E_L3 ),

    /* 80 */
    DAPC_ATTR("ANC_MD32",                                E_L0,  E_L3 ),
    DAPC_ATTR("DMA",                                     E_L0,  E_L3 ),
    DAPC_ATTR("AUXADC",                                  E_L0,  E_L0 ),
    DAPC_ATTR("UART0",                                   E_L0,  E_L3 ),
    DAPC_ATTR("UART1",                                   E_L0,  E_L3 ),
    DAPC_ATTR("UART2",                                   E_L0,  E_L3 ),
    DAPC_ATTR("UART3",                                   E_L0,  E_L3 ),
    DAPC_ATTR("PWM",                                     E_L0,  E_L3 ),
    DAPC_ATTR("I2C0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C1",                                    E_L0,  E_L3 ),

    /* 90 */
    DAPC_ATTR("I2C2",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("PTP",                                     E_L0,  E_L0 ),
    DAPC_ATTR("BTIF",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C3",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C4",                                    E_L0,  E_L3 ),
    DAPC_ATTR("DISP_PWM",                                E_L0,  E_L3 ),
    DAPC_ATTR("I2C5",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C6",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI1",                                    E_L0,  E_L3 ),

    /* 100 */
    DAPC_ATTR("I2C2_IMM",                                E_L0,  E_L3 ),
    DAPC_ATTR("I2C3_IMM",                                E_L0,  E_L3 ),
    DAPC_ATTR("DVFS_PROC0",                              E_L0,  E_L3 ),
    DAPC_ATTR("DVFS_PROC1",                              E_L0,  E_L3 ),
    DAPC_ATTR("I2C_DUAL",                                E_L0,  E_L3 ),
    DAPC_ATTR("SPI2",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI3",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI4",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI5",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C7",                                    E_L0,  E_L3 ),

    /* 110 */
    DAPC_ATTR("IRTX",                                    E_L0,  E_L3 ),
    DAPC_ATTR("USB20_1",                                 E_L0,  E_L0 ),
    DAPC_ATTR("USBSIF",                                  E_L0,  E_L0 ),
    DAPC_ATTR("AUDIO",                                   E_L0,  E_L0 ),
    DAPC_ATTR("MSDC0",                                   E_L0,  E_L3 ),
    DAPC_ATTR("MSDC1",                                   E_L0,  E_L3 ),
    DAPC_ATTR("MSDC2",                                   E_L0,  E_L3 ),
    DAPC_ATTR("MSDC3",                                   E_L0,  E_L3 ),
    DAPC_ATTR("USB30",                                   E_L0,  E_L0 ),
    DAPC_ATTR("USB30_SIF",                               E_L0,  E_L0 ),

    /* 120 */
    DAPC_ATTR("USB30_SIF2",                              E_L0,  E_L0 ),
    DAPC_ATTR("USB30_REMAP",                             E_L3,  E_L0 ),
    DAPC_ATTR("CONN_PERIPHERALS",                        E_L0,  E_L0 ),
    DAPC_ATTR("MD1_PERIPHERALS",                         E_L0,  E_L0 ),
    DAPC_ATTR("C2K_PERIPHERALS",                         E_L0,  E_L0 ),
    DAPC_ATTR("G3D_CONFIG",                              E_L0,  E_L3 ),
    DAPC_ATTR("MJC_CONFIG",                              E_L0,  E_L3 ),
    DAPC_ATTR("MJC_TOP",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB4",                               E_L0,  E_L3 ),
    DAPC_ATTR("MALI",                                    E_L0,  E_L3 ),

    /* 130 */
    DAPC_ATTR("MMSYS_CONFIG",                            E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RDMA0",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RDMA1",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RSZ0",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RSZ1",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RSZ2",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_WDMA",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_WROT0",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_WROT1",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_TDSHP",                               E_L0,  E_L3 ),

    /* 140 */
    DAPC_ATTR("MDP_COLOR",                               E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL0",                               E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL1",                               E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL0_2L",                            E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL1_2L",                            E_L0,  E_L3 ),
    DAPC_ATTR("DISP_RDMA0",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_RDMA1",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_WDMA0",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_WDMA1",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_COLOR",                              E_L0,  E_L3 ),

    /* 150 */
    DAPC_ATTR("DISP_CCORR",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_AAL",                                E_L0,  E_L3 ),
    DAPC_ATTR("DISP_GAMMA",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OD",                                 E_L0,  E_L3 ),
    DAPC_ATTR("DISP_DITHER",                             E_L0,  E_L3 ),
    DAPC_ATTR("DSI_UFOE",                                E_L0,  E_L3 ),
    DAPC_ATTR("DSI_DSC",                                 E_L0,  E_L3 ),
    DAPC_ATTR("DSI_SPLIT",                               E_L0,  E_L3 ),
    DAPC_ATTR("DSI0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("DSI1",                                    E_L0,  E_L3 ),

    /* 160 */
    DAPC_ATTR("DPI0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("MM_MUTEX",                                E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB0",                               E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB5",                               E_L0,  E_L3 ),
    DAPC_ATTR("SMI_COMMON",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMGSYS_CONFIG",                           E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB2",                               E_L0,  E_L3 ),
    DAPC_ATTR("DIP_A0",                                  E_L0,  E_L3 ),
    DAPC_ATTR("DIP_A1",                                  E_L0,  E_L3 ),
    DAPC_ATTR("DIP_A_NBC",                               E_L0,  E_L3 ),

    /* 170 */
    DAPC_ATTR("DPE",                                     E_L0,  E_L3 ),
    DAPC_ATTR("FD",                                      E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_0",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_1",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_2",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_3",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_4",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_5",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_6",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMSYS_CONFIG",                           E_L0,  E_L3 ),

    /* 180 */
    DAPC_ATTR("SMI_LARB6",                               E_L0,  E_L3 ),
    DAPC_ATTR("CAMTOP",                                  E_L0,  E_L3 ),
    DAPC_ATTR("CAMA",                                    E_L0,  E_L3 ),
    DAPC_ATTR("CAMB",                                    E_L0,  E_L3 ),
    DAPC_ATTR("CAMTOP_SET",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMA_SET",                                E_L0,  E_L3 ),
    DAPC_ATTR("CAMB_SET",                                E_L0,  E_L3 ),
    DAPC_ATTR("CAMTOP_INNER",                            E_L0,  E_L3 ),
    DAPC_ATTR("CAMA_INNER",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMB_INNER",                              E_L0,  E_L3 ),

    /* 190 */
    DAPC_ATTR("CAMTOP_CLR",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMA_CLR",                                E_L0,  E_L3 ),
    DAPC_ATTR("CAMB_CLR",                                E_L0,  E_L3 ),
    DAPC_ATTR("SENINF0",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF1",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF2",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF3",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF4",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF5",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF6",                                 E_L0,  E_L3 ),

    /* 200 */
    DAPC_ATTR("SENINF7",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV00",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV01",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV10",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV11",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV20",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV21",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAM_DEFAULT",                             E_L0,  E_L3 ),
    DAPC_ATTR("CAM_RSVD",                                E_L0,  E_L3 ),
    DAPC_ATTR("VDEC_GLOBAL_CON",                         E_L0,  E_L3 ),

    /* 210 */
    DAPC_ATTR("SMI_LARB1",                               E_L0,  E_L3 ),
    DAPC_ATTR("VDEC",                                    E_L0,  E_L3 ),
    DAPC_ATTR("VENC_GLOBAL_CON",                         E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB3",                               E_L0,  E_L3 ),
    DAPC_ATTR("VENC",                                    E_L0,  E_L3 ),
    DAPC_ATTR("JPGENC",                                  E_L0,  E_L3 ),
    DAPC_ATTR("JPGDEC",                                  E_L0,  E_L3 ),

};

#else

//For ATF
const static DEVICE_INFO D_APC0_Devices[] = {
    /* 0 */
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_IOCFG_CONTROL_REG",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_DRAM_CONTROLLER",                E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_GPIO_CONTROLLER",                E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_RESET_GENERATOR",      E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_GPT",                            E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_RSVD",                           E_L0,  E_L3 ),

    /* 10 */
    DAPC_ATTR("INFRA_AO_SEJ",                            E_L0,  E_L2 ),
    DAPC_ATTR("INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_APMIXEDSYS",                     E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG",             E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_CKSYS_DMY",                      E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG_B",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DRAMC_CONTROL_REG_B",            E_L0,  E_L3 ),

    /* 20 */
    DAPC_ATTR("INFRA_AO_CLDMA_AO_AP_REG_B",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_CLDMA_AO_MD_REG_B",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRA_AO_AES_TOP0",                       E_L3,  E_L0 ),
    DAPC_ATTR("INFRA_AO_AES_TOP1",                       E_L3,  E_L0 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG_C",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_DDRPHY_CONTROL_REG_D",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_MCUMIXEDSYS",                    E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_C2K_MISC",                       E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_MODEM_TEMP_SHARE",               E_L0,  E_L3 ),
    DAPC_ATTR("INFRA_AO_RESERVED",                       E_L0,  E_L3 ),

    /* 30 */
    DAPC_ATTR("INFRA_AO_SCP",                            E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_EMI_MPU",                        E_L1,  E_L3 ),
    DAPC_ATTR("INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_BOOTROM_SRAM",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_M4U_CONFIGURATION",              E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_EFUSEC",                         E_L2,  E_L0 ),
    DAPC_ATTR("INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DBG_TRACKER",                    E_L0,  E_L3 ),

    /* 40 */
    DAPC_ATTR("INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_CA7_MD1_CCIF",                   E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_CA7_AP_C2K_CCIF",                E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CA7_MD_C2K_CCIF",                E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DRAMC_NAO_REGION_REG",           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_TRNG",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_GCPU",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MD2MD_MD1_CCIF",                 E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_GCE",                            E_L0,  E_L3 ),

    /* 50 */
    DAPC_ATTR("INFRASYS_MD2MD_MD2_CCIF",                 E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_PERISYS_IOMMU",                  E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DSI0",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DRAMC_NAO_REGION_REG_B",         E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CSI0",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CSI1",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CLDMA_AP",                       E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_CLDMA_MD",                       E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_RESERVED",                       E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MDHW_SMI",                       E_L0,  E_L0 ),

    /* 60 */
    DAPC_ATTR("INFRASYS_GCPU_RSA",                       E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_DSI1",                           E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_VAD",                            E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS",                         E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CPUSYS",                         E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS1",                        E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_CPUSYS1",                        E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD0",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD1",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD2",                   E_L0,  E_L3 ),

    /* 70 */
    DAPC_ATTR("INFRASYS_MCUSYS_RSVD3",                   E_L0,  E_L3 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_0",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_1",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_2",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_3",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_4",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_5",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_6",                      E_L0,  E_L0 ),
    DAPC_ATTR("INFRASYS_BSI_BPI_7",                      E_L0,  E_L0 ),
    DAPC_ATTR("DEBUG_CORESIGHT",                         E_L0,  E_L3 ),

    /* 80 */
    DAPC_ATTR("ANC_MD32",                                E_L0,  E_L3 ),
    DAPC_ATTR("DMA",                                     E_L0,  E_L3 ),
    DAPC_ATTR("AUXADC",                                  E_L0,  E_L0 ),
    DAPC_ATTR("UART0",                                   E_L0,  E_L3 ),
    DAPC_ATTR("UART1",                                   E_L0,  E_L3 ),
    DAPC_ATTR("UART2",                                   E_L0,  E_L3 ),
    DAPC_ATTR("UART3",                                   E_L0,  E_L3 ),
    DAPC_ATTR("PWM",                                     E_L0,  E_L3 ),
    DAPC_ATTR("I2C0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C1",                                    E_L0,  E_L3 ),

    /* 90 */
    DAPC_ATTR("I2C2",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("PTP",                                     E_L0,  E_L0 ),
    DAPC_ATTR("BTIF",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C3",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C4",                                    E_L0,  E_L3 ),
    DAPC_ATTR("DISP_PWM",                                E_L0,  E_L3 ),
    DAPC_ATTR("I2C5",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C6",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI1",                                    E_L0,  E_L3 ),

    /* 100 */
    DAPC_ATTR("I2C2_IMM",                                E_L0,  E_L3 ),
    DAPC_ATTR("I2C3_IMM",                                E_L0,  E_L3 ),
    DAPC_ATTR("DVFS_PROC0",                              E_L0,  E_L3 ),
    DAPC_ATTR("DVFS_PROC1",                              E_L0,  E_L3 ),
    DAPC_ATTR("I2C_DUAL",                                E_L0,  E_L3 ),
    DAPC_ATTR("SPI2",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI3",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI4",                                    E_L0,  E_L3 ),
    DAPC_ATTR("SPI5",                                    E_L0,  E_L3 ),
    DAPC_ATTR("I2C7",                                    E_L0,  E_L3 ),

    /* 110 */
    DAPC_ATTR("IRTX",                                    E_L0,  E_L3 ),
    DAPC_ATTR("USB20_1",                                 E_L0,  E_L0 ),
    DAPC_ATTR("USBSIF",                                  E_L0,  E_L0 ),
    DAPC_ATTR("AUDIO",                                   E_L0,  E_L0 ),
    DAPC_ATTR("MSDC0",                                   E_L0,  E_L3 ),
    DAPC_ATTR("MSDC1",                                   E_L0,  E_L3 ),
    DAPC_ATTR("MSDC2",                                   E_L0,  E_L3 ),
    DAPC_ATTR("MSDC3",                                   E_L0,  E_L3 ),
    DAPC_ATTR("USB30",                                   E_L0,  E_L0 ),
    DAPC_ATTR("USB30_SIF",                               E_L0,  E_L0 ),

    /* 120 */
    DAPC_ATTR("USB30_SIF2",                              E_L0,  E_L0 ),
    DAPC_ATTR("USB30_REMAP",                             E_L3,  E_L0 ),
    DAPC_ATTR("CONN_PERIPHERALS",                        E_L0,  E_L0 ),
    DAPC_ATTR("MD1_PERIPHERALS",                         E_L0,  E_L0 ),
    DAPC_ATTR("C2K_PERIPHERALS",                         E_L0,  E_L0 ),
    DAPC_ATTR("G3D_CONFIG",                              E_L0,  E_L3 ),
    DAPC_ATTR("MJC_CONFIG",                              E_L0,  E_L3 ),
    DAPC_ATTR("MJC_TOP",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB4",                               E_L0,  E_L3 ),
    DAPC_ATTR("MALI",                                    E_L0,  E_L3 ),

    /* 130 */
    DAPC_ATTR("MMSYS_CONFIG",                            E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RDMA0",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RDMA1",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RSZ0",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RSZ1",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_RSZ2",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_WDMA",                                E_L0,  E_L3 ),
    DAPC_ATTR("MDP_WROT0",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_WROT1",                               E_L0,  E_L3 ),
    DAPC_ATTR("MDP_TDSHP",                               E_L0,  E_L3 ),

    /* 140 */
    DAPC_ATTR("MDP_COLOR",                               E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL0",                               E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL1",                               E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL0_2L",                            E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OVL1_2L",                            E_L0,  E_L3 ),
    DAPC_ATTR("DISP_RDMA0",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_RDMA1",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_WDMA0",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_WDMA1",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_COLOR",                              E_L0,  E_L3 ),

    /* 150 */
    DAPC_ATTR("DISP_CCORR",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_AAL",                                E_L0,  E_L3 ),
    DAPC_ATTR("DISP_GAMMA",                              E_L0,  E_L3 ),
    DAPC_ATTR("DISP_OD",                                 E_L0,  E_L3 ),
    DAPC_ATTR("DISP_DITHER",                             E_L0,  E_L3 ),
    DAPC_ATTR("DSI_UFOE",                                E_L0,  E_L3 ),
    DAPC_ATTR("DSI_DSC",                                 E_L0,  E_L3 ),
    DAPC_ATTR("DSI_SPLIT",                               E_L0,  E_L3 ),
    DAPC_ATTR("DSI0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("DSI1",                                    E_L0,  E_L3 ),

    /* 160 */
    DAPC_ATTR("DPI0",                                    E_L0,  E_L3 ),
    DAPC_ATTR("MM_MUTEX",                                E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB0",                               E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB5",                               E_L0,  E_L3 ),
    DAPC_ATTR("SMI_COMMON",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMGSYS_CONFIG",                           E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB2",                               E_L0,  E_L3 ),
    DAPC_ATTR("DIP_A0",                                  E_L0,  E_L3 ),
    DAPC_ATTR("DIP_A1",                                  E_L0,  E_L3 ),
    DAPC_ATTR("DIP_A_NBC",                               E_L0,  E_L3 ),

    /* 170 */
    DAPC_ATTR("DPE",                                     E_L0,  E_L3 ),
    DAPC_ATTR("FD",                                      E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_0",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_1",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_2",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_3",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_4",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_5",                              E_L0,  E_L3 ),
    DAPC_ATTR("IMG_RSVD_6",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMSYS_CONFIG",                           E_L0,  E_L3 ),

    /* 180 */
    DAPC_ATTR("SMI_LARB6",                               E_L0,  E_L3 ),
    DAPC_ATTR("CAMTOP",                                  E_L0,  E_L3 ),
    DAPC_ATTR("CAMA",                                    E_L0,  E_L3 ),
    DAPC_ATTR("CAMB",                                    E_L0,  E_L3 ),
    DAPC_ATTR("CAMTOP_SET",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMA_SET",                                E_L0,  E_L3 ),
    DAPC_ATTR("CAMB_SET",                                E_L0,  E_L3 ),
    DAPC_ATTR("CAMTOP_INNER",                            E_L0,  E_L3 ),
    DAPC_ATTR("CAMA_INNER",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMB_INNER",                              E_L0,  E_L3 ),

    /* 190 */
    DAPC_ATTR("CAMTOP_CLR",                              E_L0,  E_L3 ),
    DAPC_ATTR("CAMA_CLR",                                E_L0,  E_L3 ),
    DAPC_ATTR("CAMB_CLR",                                E_L0,  E_L3 ),
    DAPC_ATTR("SENINF0",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF1",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF2",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF3",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF4",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF5",                                 E_L0,  E_L3 ),
    DAPC_ATTR("SENINF6",                                 E_L0,  E_L3 ),

    /* 200 */
    DAPC_ATTR("SENINF7",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV00",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV01",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV10",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV11",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV20",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAMSV21",                                 E_L0,  E_L3 ),
    DAPC_ATTR("CAM_DEFAULT",                             E_L0,  E_L3 ),
    DAPC_ATTR("CAM_RSVD",                                E_L0,  E_L3 ),
    DAPC_ATTR("VDEC_GLOBAL_CON",                         E_L0,  E_L3 ),

    /* 210 */
    DAPC_ATTR("SMI_LARB1",                               E_L0,  E_L3 ),
    DAPC_ATTR("VDEC",                                    E_L0,  E_L3 ),
    DAPC_ATTR("VENC_GLOBAL_CON",                         E_L0,  E_L3 ),
    DAPC_ATTR("SMI_LARB3",                               E_L0,  E_L3 ),
    DAPC_ATTR("VENC",                                    E_L0,  E_L3 ),
    DAPC_ATTR("JPGENC",                                  E_L0,  E_L3 ),
    DAPC_ATTR("JPGDEC",                                  E_L0,  E_L3 ),

};

#endif

#if 0 //[NO_FEATURE] This platform does not support MDSYS read permission protection
const static MD_MODULE_INFO D_MD_Devices[] = {
    /*device_name                    read_permission   write_permission*/
    /* 0 */
    MD_ATTR("mdcfgctl_top",                         E_Sec,    E_NonSec),
    MD_ATTR("mduart0_top",                          E_Sec,    E_Sec),
    MD_ATTR("mdgdma_top",                           E_Sec,    E_Sec),
    MD_ATTR("mdgptm_top",                           E_Sec,    E_Sec),
    MD_ATTR("usim1_top",                            E_Sec,    E_Sec),
    MD_ATTR("usim2_top",                            E_Sec,    E_Sec),
    MD_ATTR("mdperisys_misc_reg",                   E_Sec,    E_NonSec),
    MD_ATTR("mdcirq_top",                           E_Sec,    E_Sec),
    MD_ATTR("md_dbgsys1_top",                       E_Sec,    E_NonSec),
    MD_ATTR("md_dbgsys2_top",                       E_Sec,    E_NonSec),

    /* 10 */
    MD_ATTR("md_dbgsys3_top",                       E_Sec,    E_NonSec),
    MD_ATTR("mdpar_dbgmon_top",                     E_Sec,    E_Sec),
    MD_ATTR("mdperisys_clk_ctl_top",                E_Sec,    E_Sec),
    MD_ATTR("md_topsm",                             E_Sec,    E_NonSec),
    MD_ATTR("md_ostimer",                           E_Sec,    E_NonSec),
    MD_ATTR("mdrgu_top",                            E_Sec,    E_Sec),
    MD_ATTR("i2c_top",                              E_Sec,    E_Sec),
    MD_ATTR("md_eint_top",                          E_Sec,    E_Sec),
    MD_ATTR("mdsys_clkctl_top",                     E_Sec,    E_NonSec),
    MD_ATTR("md_global_con_top_dcm",                E_Sec,    E_NonSec),

    /* 20 */
    MD_ATTR("mdtop_pllmixed_top",                   E_Sec,    E_NonSec),
    MD_ATTR("md_clksw_top",                         E_Sec,    E_NonSec),
    MD_ATTR("arm7_ostimer",                         E_Sec,    E_Sec),
    MD_ATTR("md_lite_gptimer",                      E_Sec,    E_Sec),
    MD_ATTR("ps2arm7_pccif",                        E_Sec,    E_Sec),
    MD_ATTR("arm72ps_arm7",                         E_Sec,    E_Sec),
    MD_ATTR("mdperisys_mbist_config",               E_Sec,    E_Sec),
    MD_ATTR("sdf_top",                              E_Sec,    E_Sec),
    MD_ATTR("psmcu_misc_reg",                       E_Sec,    E_Sec),
    MD_ATTR("psmcu_busmon_top",                     E_Sec,    E_Sec),

    /* 30 */
    MD_ATTR("psmcu_etm_pc_monitor_top",             E_Sec,    E_NonSec),
    MD_ATTR("psmcusys_mbist_config",                E_Sec,    E_Sec),
    MD_ATTR("elm_top",                              E_Sec,    E_Sec),
    MD_ATTR("abm/asm",                              E_Sec,    E_Sec),
    MD_ATTR("soe",                                  E_Sec,    E_Sec),
    MD_ATTR("BUSMON",                               E_Sec,    E_Sec),
    MD_ATTR("mduart1_top",                          E_Sec,    E_Sec),
    MD_ATTR("mduart2_top",                          E_Sec,    E_Sec),
    MD_ATTR("mdinfrasys_mbist_config",              E_Sec,    E_Sec),
    MD_ATTR("mdsys_mbist_config",                   E_Sec,    E_Sec),

    /* 40 */
    MD_ATTR("mdsmicfg",                             E_Sec,    E_Sec),
    MD_ATTR("mdinfra_misc_reg",                     E_Sec,    E_Sec),
    MD_ATTR("md_bus_recorder",                      E_Sec,    E_Sec),
    MD_ATTR("ppc_top",                              E_Sec,    E_Sec),
    MD_ATTR("arm7_wdt_top",                         E_Sec,    E_Sec),
    MD_ATTR("arm7_mbist_config",                    E_Sec,    E_Sec),
    MD_ATTR("arm7_cirq",                            E_Sec,    E_Sec),
    MD_ATTR("arm7_misc_reg",                        E_Sec,    E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_Sec),

    /* 50 */
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_NonSec),  //52
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_NonSec),  //53
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec,    E_Sec),
    MD_ATTR("CG",                                   E_Sec,    E_NonSec),
    MD_ATTR("PLLCFG",                               E_Sec,    E_NonSec),
    MD_ATTR("IDMA(DM)",                             E_Sec,    E_NonSec),
    MD_ATTR("C2K_Others",                           E_Sec,    E_Sec),

    /* 60 */
    MD_ATTR("MD1_Others",                           E_Sec,    E_Sec),
};
#endif

const static MD_MODULE_INFO D_MD_Devices[] = {
    /*device_name                    read_permission   write_permission*/
    /* 0 */
    MD_ATTR("mdcfgctl_top",                         E_NonSec),
    MD_ATTR("mduart0_top",                          E_Sec),
    MD_ATTR("mdgdma_top",                           E_Sec),
    MD_ATTR("mdgptm_top",                           E_Sec),
    MD_ATTR("usim1_top",                            E_Sec),
    MD_ATTR("usim2_top",                            E_Sec),
    MD_ATTR("mdperisys_misc_reg",                   E_NonSec),
    MD_ATTR("mdcirq_top",                           E_Sec),
    MD_ATTR("md_dbgsys1_top",                       E_NonSec),
    MD_ATTR("md_dbgsys2_top",                       E_NonSec),

    /* 10 */
    MD_ATTR("md_dbgsys3_top",                       E_NonSec),
    MD_ATTR("mdpar_dbgmon_top",                     E_Sec),
    MD_ATTR("mdperisys_clk_ctl_top",                E_Sec),
    MD_ATTR("md_topsm",                             E_NonSec),
    MD_ATTR("md_ostimer",                           E_NonSec),
    MD_ATTR("mdrgu_top",                            E_Sec),
    MD_ATTR("i2c_top",                              E_Sec),
    MD_ATTR("md_eint_top",                          E_Sec),
    MD_ATTR("mdsys_clkctl_top",                     E_NonSec),
    MD_ATTR("md_global_con_top_dcm",                E_NonSec),

    /* 20 */
    MD_ATTR("mdtop_pllmixed_top",                   E_NonSec),
    MD_ATTR("md_clksw_top",                         E_NonSec),
    MD_ATTR("arm7_ostimer",                         E_Sec),
    MD_ATTR("md_lite_gptimer",                      E_Sec),
    MD_ATTR("ps2arm7_pccif",                        E_Sec),
    MD_ATTR("arm72ps_arm7",                         E_Sec),
    MD_ATTR("mdperisys_mbist_config",               E_Sec),
    MD_ATTR("sdf_top",                              E_Sec),
    MD_ATTR("psmcu_misc_reg",                       E_Sec),
    MD_ATTR("psmcu_busmon_top",                     E_Sec),

    /* 30 */
    MD_ATTR("psmcu_etm_pc_monitor_top",             E_NonSec),
    MD_ATTR("psmcusys_mbist_config",                E_Sec),
    MD_ATTR("elm_top",                              E_Sec),
    MD_ATTR("abm/asm",                              E_Sec),
    MD_ATTR("soe",                                  E_Sec),
    MD_ATTR("BUSMON",                               E_Sec),
    MD_ATTR("mduart1_top",                          E_Sec),
    MD_ATTR("mduart2_top",                          E_Sec),
    MD_ATTR("mdinfrasys_mbist_config",              E_Sec),
    MD_ATTR("mdsys_mbist_config",                   E_Sec),

    /* 40 */
    MD_ATTR("mdsmicfg",                             E_Sec),
    MD_ATTR("mdinfra_misc_reg",                     E_Sec),
    MD_ATTR("md_bus_recorder",                      E_Sec),
    MD_ATTR("ppc_top",                              E_Sec),
    MD_ATTR("arm7_wdt_top",                         E_Sec),
    MD_ATTR("arm7_mbist_config",                    E_Sec),
    MD_ATTR("arm7_cirq",                            E_Sec),
    MD_ATTR("arm7_misc_reg",                        E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec),

    /* 50 */
    MD_ATTR("0x0==addr[15:12]",                     E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_NonSec),  //52
    MD_ATTR("0x0==addr[15:12]",                     E_NonSec),  //53
    MD_ATTR("0x0==addr[15:12]",                     E_Sec),
    MD_ATTR("0x0==addr[15:12]",                     E_Sec),
    MD_ATTR("CG",                                   E_NonSec),
    MD_ATTR("PLLCFG",                               E_NonSec),
    MD_ATTR("IDMA(DM)",                             E_NonSec),
    MD_ATTR("C2K_Others",                           E_Sec),

    /* 60 */
    MD_ATTR("MD1_Others",                           E_NonSec),
};



static void clear_vio_status(unsigned int module)
{
    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;

    apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
    apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

    *((unsigned int*)((size_t)DEVAPC_D0_VIO_STA_0 + (apc_index * 4))) = (0x1 << apc_bit_index);
}

int set_master_transaction(unsigned int master_index, unsigned int transaction_type)
{
    volatile unsigned int* base = 0;
    unsigned int set_bit = 0;
    unsigned int master_register_index = 0;
    unsigned int master_set_index = 0;

    if (master_index > MASTER_MAX_INDEX)
        return -1;

    master_register_index = master_index / (MOD_NO_IN_1_DEVAPC*2);
    master_set_index = master_index % (MOD_NO_IN_1_DEVAPC*2);

    base = (unsigned int*) ((size_t)DEVAPC_MAS_SEC_0 + master_register_index * 4);

    if (transaction_type == 0){
        set_bit = ~(1 << master_set_index);
        writel(readl(base) & set_bit, base);
    } else if (transaction_type == 1) {
        set_bit = 1 << master_set_index;
        writel(readl(base) | set_bit, base);
    } else
        return -2;

    return 0;
}

void set_module_apc(unsigned int module, E_MASK_DOM domain_num , APC_ATTR permission_control)
{
    volatile unsigned int* base = NULL;

    unsigned int clr_bit = 0x3 << ((module % MOD_NO_IN_1_DEVAPC) * 2);
    unsigned int set_bit = permission_control << ((module % MOD_NO_IN_1_DEVAPC) * 2);

    clear_vio_status(module);

    /* Note: "unmask_module_irq" is changed to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
     * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded
     */

    if (domain_num == E_DOMAIN_0)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D0_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_1)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D1_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_2)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D2_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_3)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D3_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_4)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D4_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_5)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D5_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_6)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D6_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_7)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D7_APC_0 + (module/16) * 4);
    }

    if (base != NULL) {
        writel(readl(base) & ~clr_bit, base);
        writel(readl(base) | set_bit, base);
    }
}

#if 0 //[NO_FEATURE] This platform does not support MDSYS read permission protection
void set_mdsys_apc(unsigned int module, MDAPC_ATTR read_permission, MDAPC_ATTR write_permission)
{
    volatile unsigned int* base_read = 0;
    unsigned int set_bit_read = 0;
    volatile unsigned int* base_write = 0;
    unsigned int set_bit_write = 0;

    base_read = (unsigned int*) ((size_t)DEVAPC_MD_SEC_EN_0 + (module/32) * 4);
    set_bit_read = read_permission << (module % 32);
    writel(readl(base_read) | set_bit_read, base_read);

    base_write = (unsigned int*) ((size_t)DEVAPC_MD_SEC_EN_0 + (module/32) * 4);
    set_bit_write = write_permission << (module % 32);
    writel(readl(base_write) | set_bit_write, base_write);
}
#endif

void set_mdsys_apc(unsigned int module, MDAPC_ATTR write_permission)
{
    volatile unsigned int* base_write = 0;
    unsigned int set_bit_write = 0;

    base_write = (unsigned int*) ((size_t)DEVAPC_MD_SEC_EN_0 + (module/32) * 4);
    set_bit_write = write_permission << (module % 32);
    writel(readl(base_write) | set_bit_write, base_write);
}


int start_devapc()
{
    int module_index = 0;
    int i = 0;
    int d = 0;

    /* Enable Devapc */
    writel(0x80000000, DEVAPC_VIO_DBG0); // clear apc0 dbg info if any

    writel(readl(DEVAPC_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC_APC_CON);

    writel(readl(DEVAPC_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC_PD_APC_CON);

    /* Set Necessary Masters to Secure Transaction */
    set_master_transaction(MASTER_SPM, SECURE_TRANSACTION); /* SPM_PDN will access MCUSYS in Normal World, but MCUSYS is secure r/w and non-secure read-only. So this setting is necessary. */

    /* Initial Permission */

#if !defined(SPD_no)
    printf("[DAPC] Walk TEE\n");
    for (module_index = 0; module_index < (sizeof(D_APC0_Devices_TEE)/sizeof(DEVICE_INFO)); module_index++)
    {
        set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices_TEE[module_index].d0_permission);

        set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices_TEE[module_index].d1_permission);
    }
#else
    printf("[DAPC] Walk ATF\n");
    for (module_index = 0; module_index < (sizeof(D_APC0_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices[module_index].d0_permission);

        set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices[module_index].d1_permission);
    }
#endif

    /* [Important] You must open the access permission of AP=>MD1_PERIPHERALS and AP=>C2K_PERIPHERALS because
     *             it is the first level protection. Below is the MD 2nd level protection.*/
    printf("[DAPC] Setting MDSYS\n");
    for (module_index = 0; module_index < (sizeof(D_MD_Devices)/sizeof(MD_MODULE_INFO)); module_index++)
    {
#if 0 //[NO_FEATURE] This platform does not support MDSYS read permission protection
        set_mdsys_apc(module_index, D_MD_Devices[module_index].read_permission, D_MD_Devices[module_index].write_permission);
#else
        set_mdsys_apc(module_index, D_MD_Devices[module_index].write_permission);
#endif
    }

    //d: domain, i: register number
    for (d=0; d<=1; d++) {
        for (i=0; i<=13; i++) {
            printf("[DAPC] reg%d-%d = 0x%x\n", d, i, *((unsigned int*)((size_t)DEVAPC_D0_APC_0 + (d * 0x100) + (i * 4))));
        }
    }

    printf("[DAPC] MAS_SEC_0 = 0x%x\n", *DEVAPC_MAS_SEC_0);

    for (i=0; i<=6; i++) {
        printf("[DAPC] VIO_MASK_%d=0x%x\n", i, *(DEVAPC_D0_VIO_MASK_0 + i));
    }

    for (i=0; i<=7; i++) {
        printf("[DAPC] VIO_STA_%d=0x%x\n", i, *(DEVAPC_D0_VIO_STA_0 + i));
    }

    printf("\n[DAPC] Current MM_EN 0:0x%x, 1:0x%x, 2:0x%x\n",
       *DEVAPC_MM_SEC_EN_0, *DEVAPC_MM_SEC_EN_1,
       *DEVAPC_MM_SEC_EN_2);

    printf("[DAPC] Current MD_EN 0:0x%x, 1:0x%x\n",
       *DEVAPC_MD_SEC_EN_0, *DEVAPC_MD_SEC_EN_1);

    /*Lock DAPC to secure access only*/
    writel(readl(DEVAPC_APC_CON) | (0x1), DEVAPC_APC_CON);

    /*Set Level 2 secure*/
    writel(readl(INFRA_AO_SEC_CG_CON0) | (SEJ_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON0);
    writel(readl(INFRA_AO_SEC_CG_CON1) | (TRNG_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON1);

    printf("[DAPC] Init Done\n");
    printf("[DAPC] APC_CON = 0x%x\n", *DEVAPC_APC_CON);

    return 0;
}
