#include <debug.h>
#include <devapc.h>
#include <platform.h>
#include <plat_private.h>   //for atf_arg_t_ptr


#if !defined(SPD_no)
//For TEE
const static INFRA_PERI_DEVICE_INFO D_APC_INFRA_PERI_Devices_TEE[] = {
	/* 0 */
	DAPC_INFRA_PERI_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_PMIC_WRAP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_PERICFG",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_KEYPAD", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_APXGPT", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_AP_CIRQ_EINT",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_DEVICE_APC_MPU", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_DEVICE_APC_AO",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SEJ",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_RESERVE_0",						E_L0,  E_L0),

	/* 10 */
	DAPC_INFRA_PERI_ATTR("INFRA_AO_TOP_MISC",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_AES_TOP_0",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_AES_TOP_1",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_MDEM_TEMP_SHARE",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_RESERVE_1",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_CLDMA_AO_TOP_AP",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_CLDMA_AO_TOP_MD",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_DVFS_CTRL_PROC", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_0",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_1",						E_L0,  E_L0),

	/* 20 */
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_2",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_3",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_4",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_5",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_6",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_7",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SLEEP_CONTROL_PROCESSOR_0",				E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SLEEP_CONTROL_PROCESSOR_1",				E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SLEEP_CONTROL_PROCESSOR_2",				E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SSHUB",							E_L0,  E_L0),

	/* 30 */
	DAPC_INFRA_PERI_ATTR("INFRASYS_MCUSYS_CONFIG_REG",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_TOP_MODULE", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_SECURITY_GROUP", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_EMI_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF0_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF1_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CLDMA_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_MD2MD_CCIF_MD1", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF0_MD",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF1_MD",						E_L0,  E_L0),

	/* 40 */
	DAPC_INFRA_PERI_ATTR("INFRASYS_CLDMA_MD",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_MD2MD_CCIF_MD2", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_SYS_CIRQ",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_GCE_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_SMI_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_UFOZIP_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_LOW_POWER_GROUP",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_M4U",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CQ_DMA_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DRAMC_GROUP",						E_L0,  E_L0),

	/* 50 */
	DAPC_INFRA_PERI_ATTR("INFRASYS_CONFIGURATION_GROUP",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_BSI_BPI_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_AES_FDE_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_RESERVE_0",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DXCC_PUBLIC",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DXCC_PRIVATE",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF2_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF3_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF2_MD",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF3_MD",						E_L0,  E_L0),

	/* 60 */
	DAPC_INFRA_PERI_ATTR("EMI_MPU", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_PDN_MCSIB", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DEBUG_TOP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("DMA", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("AUXADC",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART1",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART2",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART3",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),

	/* 70 */
	DAPC_INFRA_PERI_ATTR("PWM", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C1",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C2",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("PTP_THERMAL_CTL", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C4",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C5",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C6",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C3",								E_L0,  E_L0),

	/* 80 */
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("IRTX",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("DISP_PWM0",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI1",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI2",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C7",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI_NOR", 							E_L0,  E_L0),

	/* 90 */
	DAPC_INFRA_PERI_ATTR("SPI3",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI4",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI5",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C8",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C9",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("USB3.0_PORT0_CSR",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("USB2.0_PORR1",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("AUDIO",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MSDC0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MSDC1",								E_L0,  E_L0),

	/* 100 */
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MSDC3",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UFS", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SSUSB_SIF",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MD32_ANC",							E_L0,  E_L0),

	/* 110 */
	DAPC_INFRA_PERI_ATTR("PERI_POWER_METER",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI7",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("BTIF",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI6",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_0",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_IO_RB",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_2",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_SSUSB_DIG_PHY_TOP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_USBSIF_TOP", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_5",							E_L0,  E_L0),

	/* 120 */
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_6",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_7",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_8",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_9",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_A",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_B",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_MIPI_TX0_BASE",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_MIPI_TX1_BASE",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_E",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_F",							E_L0,  E_L0),

	/* 130 */
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_0", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_EFUSE_TOP", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_UFS_MPHY",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_IO_BR", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_4", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_5", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RG_PERI_MSDC_PAD_MACRO",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_7", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_8", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_9", 						E_L0,  E_L0),

	/* 140 */
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_A", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_B", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_C", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_D", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_E", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_F", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_IO_LB",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_IO_LT",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_2",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE3",							E_L0,  E_L0),

	/* 150 */
	DAPC_INFRA_PERI_ATTR("WEST_MIPI_RX_TOP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_5",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_6",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_7",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_8",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_9",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_A",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_B",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_C",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_D",							E_L0,  E_L0),

	/* 160 */
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_E",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_F",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_IO_TL", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_EFUSE_TOP", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_IO_TR", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_3", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_4", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_5", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_6", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_7", 						E_L0,  E_L0),

	/* 170 */
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_8", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_9", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_A", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_B", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_C", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_D", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_E", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_F", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI_NOR_EXT", 							E_L0,  E_L0),

	/* 180 */
	DAPC_INFRA_PERI_ATTR("MD1", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("CONN",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MD3", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
};

#else

//For ATF
const static INFRA_PERI_DEVICE_INFO D_APC_INFRA_PERI_Devices[] = {
	/* 0 */
	DAPC_INFRA_PERI_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_PMIC_WRAP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_PERICFG",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_KEYPAD", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_APXGPT", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_AP_CIRQ_EINT",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_DEVICE_APC_MPU", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_DEVICE_APC_AO",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SEJ",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_RESERVE_0",						E_L0,  E_L0),

	/* 10 */
	DAPC_INFRA_PERI_ATTR("INFRA_AO_TOP_MISC",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_AES_TOP_0",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_AES_TOP_1",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_MDEM_TEMP_SHARE",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_RESERVE_1",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_CLDMA_AO_TOP_AP",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_CLDMA_AO_TOP_MD",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_DVFS_CTRL_PROC", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_0",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_1",						E_L0,  E_L0),

	/* 20 */
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_2",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_3",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_4",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_5",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_6",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_POWER_MCU_7",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SLEEP_CONTROL_PROCESSOR_0",				E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SLEEP_CONTROL_PROCESSOR_1",				E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SLEEP_CONTROL_PROCESSOR_2",				E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_AO_SSHUB",							E_L0,  E_L0),

	/* 30 */
	DAPC_INFRA_PERI_ATTR("INFRASYS_MCUSYS_CONFIG_REG",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_TOP_MODULE", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_SECURITY_GROUP", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_EMI_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF0_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF1_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CLDMA_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_MD2MD_CCIF_MD1", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF0_MD",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF1_MD",						E_L0,  E_L0),

	/* 40 */
	DAPC_INFRA_PERI_ATTR("INFRASYS_CLDMA_MD",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_MD2MD_CCIF_MD2", 					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_SYS_CIRQ",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_GCE_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_SMI_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_UFOZIP_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_LOW_POWER_GROUP",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_M4U",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CQ_DMA_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DRAMC_GROUP",						E_L0,  E_L0),

	/* 50 */
	DAPC_INFRA_PERI_ATTR("INFRASYS_CONFIGURATION_GROUP",					E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_BSI_BPI_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_AES_FDE_GROUP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_RESERVE_0",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DXCC_PUBLIC",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DXCC_PRIVATE",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF2_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF3_AP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF2_MD",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_CCIF3_MD",						E_L0,  E_L0),

	/* 60 */
	DAPC_INFRA_PERI_ATTR("EMI_MPU", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRA_PDN_MCSIB", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("INFRASYS_DEBUG_TOP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("DMA", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("AUXADC",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART1",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART2",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UART3",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),

	/* 70 */
	DAPC_INFRA_PERI_ATTR("PWM", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C1",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C2",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("PTP_THERMAL_CTL", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C4",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C5",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C6",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C3",								E_L0,  E_L0),

	/* 80 */
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("IRTX",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("DISP_PWM0",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI1",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI2",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C7",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI_NOR", 							E_L0,  E_L0),

	/* 90 */
	DAPC_INFRA_PERI_ATTR("SPI3",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI4",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI5",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C8",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("I2C9",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("USB3.0_PORT0_CSR",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("USB2.0_PORR1",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("AUDIO",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MSDC0",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MSDC1",								E_L0,  E_L0),

	/* 100 */
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MSDC3",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("UFS", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SSUSB_SIF",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MD32_ANC",							E_L0,  E_L0),

	/* 110 */
	DAPC_INFRA_PERI_ATTR("PERI_POWER_METER",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI7",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("BTIF",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI6",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_0",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_IO_RB",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_2",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_SSUSB_DIG_PHY_TOP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_USBSIF_TOP", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_5",							E_L0,  E_L0),

	/* 120 */
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_6",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_7",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_8",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_9",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_A",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_B",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_MIPI_TX0_BASE",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_MIPI_TX1_BASE",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_E",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("EAST_RESERVE_F",							E_L0,  E_L0),

	/* 130 */
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_0", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_EFUSE_TOP", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_UFS_MPHY",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_IO_BR", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_4", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_5", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RG_PERI_MSDC_PAD_MACRO",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_7", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_8", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_9", 						E_L0,  E_L0),

	/* 140 */
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_A", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_B", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_C", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_D", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_E", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SOUTH_RESERVE_F", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_IO_LB",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_IO_LT",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_2",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE3",							E_L0,  E_L0),

	/* 150 */
	DAPC_INFRA_PERI_ATTR("WEST_MIPI_RX_TOP",						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_5",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_6",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_7",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_8",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_9",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_A",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_B",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_C",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_D",							E_L0,  E_L0),

	/* 160 */
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_E",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("WEST_RESERVE_F",							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_IO_TL", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_EFUSE_TOP", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_IO_TR", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_3", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_4", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_5", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_6", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_7", 						E_L0,  E_L0),

	/* 170 */
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_8", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_9", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_A", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_B", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_C", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_D", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_E", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("NORTH_RESERVE_F", 						E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("SPI_NOR_EXT", 							E_L0,  E_L0),

	/* 180 */
	DAPC_INFRA_PERI_ATTR("MD1", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("CONN",								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("MD3", 								E_L0,  E_L0),
	DAPC_INFRA_PERI_ATTR("RESERVE", 							E_L0,  E_L0),
};

#endif

const static MM_MD_DEVICE_INFO D_APC_MM_Devices[] = {
    /* 0 */
    DAPC_MM_MD_ATTR("G3D_CONFIG",                              E_L0),
    DAPC_MM_MD_ATTR("MFG_VAD",                                 E_L0),
    DAPC_MM_MD_ATTR("SC0_VAD",                                 E_L0),
    DAPC_MM_MD_ATTR("SC1_VAD",                                 E_L0),
    DAPC_MM_MD_ATTR("SC2_VAD",                                 E_L0),
    DAPC_MM_MD_ATTR("SC3_VAD",                                 E_L0),
    DAPC_MM_MD_ATTR("DFP",                                     E_L0),
    DAPC_MM_MD_ATTR("MALI",                                    E_L0),
    DAPC_MM_MD_ATTR("MFG_RESERVE",                             E_L0),
    DAPC_MM_MD_ATTR("MJC_CONFIG",                              E_L0),

    /* 10 */
    DAPC_MM_MD_ATTR("MJC_TOP",                                 E_L0),
    DAPC_MM_MD_ATTR("SMI_LARB6",                               E_L0),
    DAPC_MM_MD_ATTR("DFP_PWR",                                 E_L0),
    DAPC_MM_MD_ATTR("DFP_VAD",                                 E_L0),
    DAPC_MM_MD_ATTR("MMSYS_CONFIG",                            E_L0),
    DAPC_MM_MD_ATTR("MDP_RDMA0",                               E_L0),
    DAPC_MM_MD_ATTR("MDP_RDMA1",                               E_L0),
    DAPC_MM_MD_ATTR("MDP_RSZ0",                                E_L0),
    DAPC_MM_MD_ATTR("MDP_RSZ1",                                E_L0),
    DAPC_MM_MD_ATTR("MDP_RSZ2",                                E_L0),

    /* 20 */
    DAPC_MM_MD_ATTR("MDP_WDMA",                                E_L0),
    DAPC_MM_MD_ATTR("MDP_WROT0",                               E_L0),
    DAPC_MM_MD_ATTR("MDP_WROT1",                               E_L0),
    DAPC_MM_MD_ATTR("MDP_TDSHP",                               E_L0),
    DAPC_MM_MD_ATTR("MDP_COLOR",                               E_L0),
    DAPC_MM_MD_ATTR("DISP_OVL0",                               E_L0),
    DAPC_MM_MD_ATTR("DISP_OVL1",                               E_L0),
    DAPC_MM_MD_ATTR("DISP_OVL0_2L",                            E_L0),
    DAPC_MM_MD_ATTR("DISP_OVL1_2L",                            E_L0),
    DAPC_MM_MD_ATTR("DISP_RDMA0",                              E_L0),

    /* 30 */
    DAPC_MM_MD_ATTR("DISP_RDMA1",                              E_L0),
    DAPC_MM_MD_ATTR("DISP_RDMA2",                              E_L0),
    DAPC_MM_MD_ATTR("DISP_WDMA0",                              E_L0),
    DAPC_MM_MD_ATTR("DISP_WDMA1",                              E_L0),
    DAPC_MM_MD_ATTR("DISP_COLOR0",                             E_L0),
    DAPC_MM_MD_ATTR("DISP_COLOR1",                             E_L0),
    DAPC_MM_MD_ATTR("DISP_CCORR0",                             E_L0),
    DAPC_MM_MD_ATTR("DISP_CCORR1",                             E_L0),
    DAPC_MM_MD_ATTR("DISP_AAL0",                               E_L0),
    DAPC_MM_MD_ATTR("DISP_AAL1",                               E_L0),

    /* 40 */
    DAPC_MM_MD_ATTR("DISP_GAMMA0",                             E_L0),
    DAPC_MM_MD_ATTR("DISP_GAMMA1",                             E_L0),
    DAPC_MM_MD_ATTR("DISP_OD",                                 E_L0),
    DAPC_MM_MD_ATTR("DISP_DITHER0",                            E_L0),
    DAPC_MM_MD_ATTR("DISP_DITHER1",                            E_L0),
    DAPC_MM_MD_ATTR("DSI_UFOE",                                E_L0),
    DAPC_MM_MD_ATTR("DSI_DSC",                                 E_L0),
    DAPC_MM_MD_ATTR("DSI_SPLIT",                               E_L0),
    DAPC_MM_MD_ATTR("DSI0",                                    E_L0),
    DAPC_MM_MD_ATTR("DSI1",                                    E_L0),

    /* 50 */
    DAPC_MM_MD_ATTR("DPI0",                                    E_L0),
    DAPC_MM_MD_ATTR("MM_MUTEX",                                E_L0),
    DAPC_MM_MD_ATTR("SMI_LARB0",                               E_L0),
    DAPC_MM_MD_ATTR("SMI_LARB4",                               E_L0),
    DAPC_MM_MD_ATTR("SMI_COMMON",                              E_L0),
    DAPC_MM_MD_ATTR("MMSYS_VAD",                               E_L0),
    DAPC_MM_MD_ATTR("MMSYS_DFP",                               E_L0),
    DAPC_MM_MD_ATTR("IMGSYS_CONFIG",                           E_L0),
    DAPC_MM_MD_ATTR("SMI_LARB5",                               E_L0),
    DAPC_MM_MD_ATTR("DIP_A0",                                  E_L0),

    /* 60 */
    DAPC_MM_MD_ATTR("DIP_A1",                                  E_L0),
    DAPC_MM_MD_ATTR("DIP_A2",                                  E_L0),
    DAPC_MM_MD_ATTR("VAD",                                     E_L0),
    DAPC_MM_MD_ATTR("DPE",                                     E_L0),
    DAPC_MM_MD_ATTR("RSC",                                     E_L0),
    DAPC_MM_MD_ATTR("FDVT",                                    E_L0),
    DAPC_MM_MD_ATTR("GEPF",                                    E_L0),
    DAPC_MM_MD_ATTR("DFP",                                     E_L0),
    DAPC_MM_MD_ATTR("VDEC_TOP_GLOBAL_CON",                     E_L0),
    DAPC_MM_MD_ATTR("VDEC_TOP_SMI_LARB",                       E_L0),

    /* 70 */
    DAPC_MM_MD_ATTR("VDEC_TOP_FULL_TOP",                       E_L0),
    DAPC_MM_MD_ATTR("VDEC_TOP_VAD",                            E_L0),
    DAPC_MM_MD_ATTR("VDEC_TOP_DPM",                            E_L0),
    DAPC_MM_MD_ATTR("VENC_GLOBAL_CON",                         E_L0),
    DAPC_MM_MD_ATTR("SMI_LARB3",                               E_L0),
    DAPC_MM_MD_ATTR("VENC",                                    E_L0),
    DAPC_MM_MD_ATTR("JPGENC",                                  E_L0),
    DAPC_MM_MD_ATTR("JPGDEC",                                  E_L0),
    DAPC_MM_MD_ATTR("VENC_DFP",                                E_L0),
    DAPC_MM_MD_ATTR("VENC_VAD",                                E_L0),

    /* 80 */
    DAPC_MM_MD_ATTR("CAMSYS_TOP",                              E_L0),
    DAPC_MM_MD_ATTR("LARB2",                                   E_L0),
    DAPC_MM_MD_ATTR("CAM_TOP",                                 E_L0),
    DAPC_MM_MD_ATTR("CAM_A",                                   E_L0),
    DAPC_MM_MD_ATTR("CAM_B",                                   E_L0),
    DAPC_MM_MD_ATTR("CAM_DFP",                                 E_L0),
    DAPC_MM_MD_ATTR("CAM_VAD",                                 E_L0),
    DAPC_MM_MD_ATTR("CAM_TOP_SET",                             E_L0),
    DAPC_MM_MD_ATTR("CAM_A_SET",                               E_L0),
    DAPC_MM_MD_ATTR("CAM_B_SET",                               E_L0),

    /* 90 */
    DAPC_MM_MD_ATTR("CAM_TOP_INNER",                           E_L0),
    DAPC_MM_MD_ATTR("CAM_A_INNER",                             E_L0),
    DAPC_MM_MD_ATTR("CAM_B_INNER",                             E_L0),
    DAPC_MM_MD_ATTR("CAM_TOP_CLR",                             E_L0),
    DAPC_MM_MD_ATTR("CAM_A_CLR",                               E_L0),
    DAPC_MM_MD_ATTR("CAM_B_CLR",                               E_L0),
    DAPC_MM_MD_ATTR("SENINF_A",                                E_L0),
    DAPC_MM_MD_ATTR("SENINF_B",                                E_L0),
    DAPC_MM_MD_ATTR("SENINF_C",                                E_L0),
    DAPC_MM_MD_ATTR("SENINF_D",                                E_L0),

    /* 100 */
    DAPC_MM_MD_ATTR("SENINF_E",                                E_L0),
    DAPC_MM_MD_ATTR("SENINF_F",                                E_L0),
    DAPC_MM_MD_ATTR("SENINF_G",                                E_L0),
    DAPC_MM_MD_ATTR("SENINF_H",                                E_L0),
    DAPC_MM_MD_ATTR("CAMSV_A",                                 E_L0),
    DAPC_MM_MD_ATTR("CAMSV_B",                                 E_L0),
    DAPC_MM_MD_ATTR("CAMSV_C",                                 E_L0),
    DAPC_MM_MD_ATTR("CAMSV_D",                                 E_L0),
    DAPC_MM_MD_ATTR("CAMSV_E",                                 E_L0),
    DAPC_MM_MD_ATTR("CAMSV_F",                                 E_L0),

    /* 110 */
    DAPC_MM_MD_ATTR("TSF",                                     E_L0),
};

const static MM_MD_DEVICE_INFO D_APC_MD_Devices[] = {
    /* 0 */
    DAPC_MM_MD_ATTR("MDPERISYS_1",                             E_L0),
    DAPC_MM_MD_ATTR("MDPERISYS_2/MDTOP",                       E_L0),
    DAPC_MM_MD_ATTR("MDMCUAPB_MACRO",                          E_L0),
    DAPC_MM_MD_ATTR("MDMCUAPB_NONMACRO",                       E_L0),
    DAPC_MM_MD_ATTR("MDINFRA_APB_1",                           E_L0),
    DAPC_MM_MD_ATTR("MDINFRA_APB_2",                           E_L0),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("LTEL2SYS",                                E_L3),
    DAPC_MM_MD_ATTR("HSPAL2",                                  E_L3),

    /* 10 */
    DAPC_MM_MD_ATTR("PSPERI_CONFG",                            E_L3),
    DAPC_MM_MD_ATTR("FCS",                                     E_L3),
    DAPC_MM_MD_ATTR("GCU",                                     E_L3),
    DAPC_MM_MD_ATTR("BYC_ACC",                                 E_L3),
    DAPC_MM_MD_ATTR("UEA_UIA_1",                               E_L3),
    DAPC_MM_MD_ATTR("UEA_UIA_0",                               E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L0),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),

    /* 20 */
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L0),
    DAPC_MM_MD_ATTR("DFESYS_1",                                E_L3),
    DAPC_MM_MD_ATTR("LTXBSYS_1",                               E_L3),
    DAPC_MM_MD_ATTR("MODEM_AO_APB",                            E_L3),
    DAPC_MM_MD_ATTR("L1MCUSYS_APB",                            E_L3),
    DAPC_MM_MD_ATTR("DFESYS_0",                                E_L3),
    DAPC_MM_MD_ATTR("CSTXBSYS",                                E_L3),

    /* 30 */
    DAPC_MM_MD_ATTR("LTXBSYS",                                 E_L3),
    DAPC_MM_MD_ATTR("RXBRP",                                   E_L3),
    DAPC_MM_MD_ATTR("RAKE",                                    E_L3),
    DAPC_MM_MD_ATTR("EQ1",                                     E_L3),
    DAPC_MM_MD_ATTR("EQ2",                                     E_L3),
    DAPC_MM_MD_ATTR("EQ3",                                     E_L3),
    DAPC_MM_MD_ATTR("TD1",                                     E_L3),
    DAPC_MM_MD_ATTR("TD2",                                     E_L3),
    DAPC_MM_MD_ATTR("CSSYS",                                   E_L3),
    DAPC_MM_MD_ATTR("MPCSYS",                                  E_L3),

    /* 40 */
    DAPC_MM_MD_ATTR("IMCSYS",                                  E_L3),
    DAPC_MM_MD_ATTR("MMUSYS",                                  E_L3),
    DAPC_MM_MD_ATTR("ICCSYS",                                  E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L0),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),

    /* 50 */
    DAPC_MM_MD_ATTR("RESERVE",                                 E_L3),
};

static void clear_vio_status(unsigned int module)
{
    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;

    apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
    apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

    *((volatile unsigned int*)((size_t)DEVAPC0_D0_VIO_STA_0 + apc_index * 4)) = (0x1 << apc_bit_index);
}

static int set_master_transaction(unsigned int master_index, unsigned int transaction_type)
{
    volatile unsigned int* base = NULL;
    unsigned int set_bit = 0;
    unsigned int master_register_index = 0;
    unsigned int master_set_index = 0;

    if (master_index > MASTER_MAX_INDEX)
        return -1;

    master_register_index = master_index / (MOD_NO_IN_1_DEVAPC*2);
    master_set_index = master_index % (MOD_NO_IN_1_DEVAPC*2);

    base = (volatile unsigned int*) ((size_t)DEVAPC0_MAS_SEC_0 + master_register_index * 4);

    if (transaction_type == 0){
        set_bit = 0xFFFFFFFF ^ (1 << master_set_index);
        devapc_writel(devapc_readl(base) & set_bit, base);
    } else if (transaction_type == 1) {
        set_bit = 1 << master_set_index;
        devapc_writel(devapc_readl(base) | set_bit, base);
    } else
        return -2;

    return 0;
}

static void set_module_apc(unsigned int module, E_MASK_DOM domain_num , APC_ATTR permission_control)
{
    volatile unsigned int* base = NULL;

    unsigned int clr_bit = 0xFFFFFFFF ^ (0x3 << ((module % MOD_NO_IN_1_DEVAPC) * 2));
    unsigned int set_bit = permission_control << ((module % MOD_NO_IN_1_DEVAPC) * 2);

    clear_vio_status(module + INFRA_PERI_VIO_INDEX);

    /* Note: "unmask_module_irq" is changed to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
     * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded
     */

    if (domain_num == E_DOMAIN_0)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D0_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_1)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D1_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_2)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D2_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_3)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D3_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_4)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D4_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_5)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D5_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_6)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D6_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_7)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC0_D7_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }

    if (base != NULL) {
        devapc_writel(devapc_readl(base) & clr_bit, base);
        devapc_writel(devapc_readl(base) | set_bit, base);
    }
}

static void set_mm_apc(unsigned int module, E_MASK_DOM domain_num , APC_ATTR permission_control)
{
    volatile unsigned int* base = NULL;

    unsigned int clr_bit = 0xFFFFFFFF ^ (0x3 << ((module % MOD_NO_IN_1_DEVAPC) * 2));
    unsigned int set_bit = permission_control << ((module % MOD_NO_IN_1_DEVAPC) * 2);

    clear_vio_status(module + MM_VIO_INDEX);

    /* Note: "unmask_module_irq" is changed to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
     * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded
     */

    if (domain_num == E_DOMAIN_0)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC1_D0_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_1)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC1_D1_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_2)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC1_D2_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_3)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC1_D3_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }

    if (base != NULL) {
        devapc_writel(devapc_readl(base) & clr_bit, base);
        devapc_writel(devapc_readl(base) | set_bit, base);
    }
}

static void set_md_apc(unsigned int module, E_MASK_DOM domain_num , APC_ATTR permission_control)
{
    volatile unsigned int* base = NULL;

    unsigned int clr_bit = 0xFFFFFFFF ^ (0x3 << ((module % MOD_NO_IN_1_DEVAPC) * 2));
    unsigned int set_bit = permission_control << ((module % MOD_NO_IN_1_DEVAPC) * 2);

    clear_vio_status(MD1_VIO_INDEX);
    clear_vio_status(MD3_VIO_INDEX);

    /* Note: "unmask_module_irq" is changed to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
     * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded
     */

    if (domain_num == E_DOMAIN_0)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC2_D0_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_1)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC2_D1_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_2)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC2_D2_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_3)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC2_D3_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }

    if (base != NULL) {
        devapc_writel(devapc_readl(base) & clr_bit, base);
        devapc_writel(devapc_readl(base) | set_bit, base);
    }
}

int start_devapc(void)
{
    unsigned int module_index = 0;

    //d: domain, i: register number
    int i = 0;
    int d = 0;

    /* Enable Devapc */
    devapc_writel(0x80000000, DEVAPC0_VIO_DBG0); // clear apc0 dbg info if any

    devapc_writel(devapc_readl(DEVAPC0_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC0_APC_CON);
    devapc_writel(devapc_readl(DEVAPC1_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC1_APC_CON);
	devapc_writel(devapc_readl(DEVAPC2_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC2_APC_CON);

    devapc_writel(devapc_readl(DEVAPC0_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC0_PD_APC_CON);

    /* Set Necessary Masters to Secure Transaction */
    set_master_transaction(MASTER_SPM, SECURE_TRANSACTION); /* SPM_PDN will access MCUSYS in Normal World, but MCUSYS is secure r/w and non-secure read-only. So this setting is necessary. */

    /* Initial Permission */

#if !defined(SPD_no)
    INFO("[DAPC] Walk TEE\n");
    for (module_index = 0; module_index < (sizeof(D_APC_INFRA_PERI_Devices_TEE)/sizeof(INFRA_PERI_DEVICE_INFO)); module_index++)
    {
        set_module_apc(module_index, E_DOMAIN_0 , D_APC_INFRA_PERI_Devices_TEE[module_index].d0_permission);

        set_module_apc(module_index, E_DOMAIN_1 , D_APC_INFRA_PERI_Devices_TEE[module_index].d1_permission);
    }
#else
    INFO("[DAPC] Walk ATF\n");
    for (module_index = 0; module_index < (sizeof(D_APC_INFRA_PERI_Devices)/sizeof(INFRA_PERI_DEVICE_INFO)); module_index++)
    {
        set_module_apc(module_index, E_DOMAIN_0 , D_APC_INFRA_PERI_Devices[module_index].d0_permission);

        set_module_apc(module_index, E_DOMAIN_1 , D_APC_INFRA_PERI_Devices[module_index].d1_permission);
    }
#endif

    /* [Important] You must open the access permission of AP=>MD1_PERIPHERALS and AP=>C2K_PERIPHERALS because
     *             it is the first level protection. Below is the MD 2nd level protection.*/

    INFO("[DAPC] Setting MMSYS\n");
    for (module_index = 0; module_index < (sizeof(D_APC_MM_Devices)/sizeof(MM_MD_DEVICE_INFO)); module_index++)
    {
        set_mm_apc(module_index, E_DOMAIN_0, D_APC_MM_Devices[module_index].d0_permission);
    }

    INFO("[DAPC] Setting MDSYS\n");
    for (module_index = 0; module_index < (sizeof(D_APC_MD_Devices)/sizeof(MM_MD_DEVICE_INFO)); module_index++)
    {
        set_md_apc(module_index, E_DOMAIN_0, D_APC_MD_Devices[module_index].d0_permission);
    }


    //d: domain, i: register number
    for (d=0; d<=1; d++) {
        for (i=0; i<=11; i++) {
            INFO("[DAPC] D%d_APC_%d = 0x%x\n", d, i, *((volatile unsigned int*)((size_t)DEVAPC0_D0_APC_0 + (d * 0x100) + (i * 4))));
        }
    }

    for (i=0; i<=6; i++) {
        INFO("[DAPC] D0_APC_MM_%d = 0x%x\n", i, *((volatile unsigned int*)((size_t)DEVAPC1_D0_APC_0 + (i * 4))));
    }

    for (i=0; i<=3; i++) {
        INFO("[DAPC] D0_APC_MD_%d = 0x%x\n", i, *((volatile unsigned int*)((size_t)DEVAPC2_D0_APC_0 + (i * 4))));
    }

    INFO("[DAPC] MAS_SEC_0 = 0x%x, MAS_SEC_1 = 0x%x\n", *DEVAPC0_MAS_SEC_0, *DEVAPC0_MAS_SEC_1);

    for (i=0; i<=9; i++) {
        INFO("[DAPC] VIO_MASK_%d=0x%x\n", i, *((volatile unsigned int*)((size_t)DEVAPC0_D0_VIO_MASK_0 + (i * 4))));
    }

    for (i=0; i<=9; i++) {
        INFO("[DAPC] VIO_STA_%d=0x%x\n", i, *((volatile unsigned int*)((size_t)DEVAPC0_D0_VIO_STA_0 + (i * 4))));
    }

    INFO("\n[DAPC] Current MM_SEC_0: 0x%x\n", *DEVAPC1_MM_SEC_0);

    /*Lock DAPC to secure access only*/
    devapc_writel(devapc_readl(DEVAPC0_APC_CON) | (0x1), DEVAPC0_APC_CON);
    devapc_writel(devapc_readl(DEVAPC1_APC_CON) | (0x1), DEVAPC1_APC_CON);
	devapc_writel(devapc_readl(DEVAPC2_APC_CON) | (0x1), DEVAPC2_APC_CON);

    /*Set Level 2 secure*/
    devapc_writel(devapc_readl(INFRA_AO_SEC_CG_CON0) | (SEJ_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON0);
    devapc_writel(devapc_readl(INFRA_AO_SEC_CG_CON1) | (TRNG_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON1);

    INFO("[DAPC] Init Done\n");
    INFO("[DAPC] APC_CON = 0x%x\n", *DEVAPC0_APC_CON);
    INFO("[DAPC] MM_APC_CON = 0x%x\n", *DEVAPC1_APC_CON);
    INFO("[DAPC] MD_APC_CON = 0x%x\n", *DEVAPC2_APC_CON);

    return 0;
}
