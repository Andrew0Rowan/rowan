/**
 * @file    mt_ocp_api.c
 * @brief   Driver for Over Current Protect
 *
 */
#include <delay_timer.h> /* for udelay */
#include "mt_ocp_api.h"


struct ocp_reg_setting
{
	unsigned int addr;
	unsigned int value;
};

static const struct ocp_reg_setting big_setting_table[] =
{
	{OCPAPBCFG00, 0x00005000},
	{OCPAPBCFG01, 0x00000000},
	{OCPAPBCFG02, 0x00000000},
	{OCPAPBCFG03, 0x00000000},
	{OCPAPBCFG04, 0x00000777},
	{OCPAPBCFG05, 0x7FFFF001},
	{OCPAPBCFG06, 0x00064000},
	{OCPAPBCFG07, 0x00001000},
	{OCPAPBCFG08, 0x00000000},
	{OCPAPBCFG09, 0x00000000},
	{OCPAPBCFG10, 0x00000000},
	{OCPAPBCFG11, 0x07800000},
	{OCPAPBCFG12, 0x10A00270},
	{OCPAPBCFG13, 0x00000000},
	{OCPAPBCFG14, 0x00180000},
	{OCPAPBCFG15, 0x667E0443},
	{OCPAPBCFG16, 0x2CDB1C2C},
	{OCPAPBCFG17, 0x19DF087A},
	{OCPAPBCFG18, 0x000057BC},
	{OCPAPBCFG19, 0x00000000},
	{OCPAPBCFG20, 0xA4E10418},
	{OCPAPBCFG21, 0x40049CC5},
	{OCPAPBCFG22, 0x18000010},
	{OCPAPBCFG23, 0x00000000},
	/*{PTP3_OCP_CLK_DIVIDER, 0x04000051},*/
};

static const struct ocp_reg_setting little_mp0_setting_table[] =
{
	{MP0_OCPCPUPOST_CTRL0, 0x00041005},
	{MP0_OCPCPUPOST_CTRL1, 0x08060021},
	{MP0_OCPCPUPOST_CTRL2, 0x3004D0ED},
	{MP0_OCPCPUPOST_CTRL3, 0x30020280},
	{MP0_OCPCPUPOST_CTRL4, 0x09000210},
	{MP0_OCPCPUPOST_CTRL5, 0x00848229},
	{MP0_OCPCPUPOST_CTRL6, 0x00108008},
	{MP0_OCPCPUPOST_CTRL7, 0x00000008},
	{MP0_OCPNCPUPOST_CTRL, 0x01249249},
	{MP0_OCPAPBCFG00, 0x00005000},
	{MP0_OCPAPBCFG11, 0x07800000},
	{MP0_OCPAPBCFG12, 0x137000F5},
	{MP0_OCPAPBCFG13, 0x0B83106F},
	{MP0_OCPAPBCFG14, 0x00BF0081},
	{MP0_OCPAPBCFG15, 0x5D98060A},
	{MP0_OCPAPBCFG16, 0x11830191},
	{MP0_OCPAPBCFG17, 0x1F710219},
	{MP0_OCPAPBCFG18, 0x065D0130},
	{MP0_OCPAPBCFG19, 0x3DD80841},
	{MP0_OCPAPBCFG20, 0x166D4729},
	{MP0_OCPAPBCFG21, 0x170549EC},
	{MP0_OCPAPBCFG22, 0x09720E32},
	{MP0_OCPAPBCFG23, 0x300C2A3A},
	{MP0_OCPAPBCFG24, 0x29B30742},
	{MP0_OCPAPBCFG25, 0x0E010CCD},
	{MP0_OCPAPBCFG26, 0x0A9D5401},
	{MP0_OCPAPBCFG27, 0x05312B74},
	{MP0_OCPAPBCFG28, 0xFFFFFFFF},
	{MP0_OCPAPBCFG29, 0xFFFFFFFF},
	{MP0_OCPAPBCFG30, 0xFFFFFFFF},
	{MP0_OCPAPBCFG31, 0x000010FF},
	{MP0_OCPAPBCFG32, 0x00011000},
	{MP0_OCPAPBCFG33, 0x00000000},
	{MP0_OCPAPBCFG34, 0x00330000},
	/*{MP0_OCP_GENERAL_CTRL, 0x0},*/
};

static const struct ocp_reg_setting little_mp1_setting_table[] =
{
	{MP1_OCPCPUPOST_CTRL0, 0x00040004},
	{MP1_OCPCPUPOST_CTRL1, 0x0000F0A1},
	{MP1_OCPCPUPOST_CTRL2, 0x30048009},
	{MP1_OCPCPUPOST_CTRL3, 0x00200080},
	{MP1_OCPCPUPOST_CTRL4, 0x09000204},
	{MP1_OCPCPUPOST_CTRL5, 0x09B08241},
	{MP1_OCPCPUPOST_CTRL6, 0x32424008},
	{MP1_OCPCPUPOST_CTRL7, 0x00000008},
	{MP1_OCPNCPUPOST_CTRL, 0x01249249},
	{MP1_OCPAPBCFG00, 0x00005000},
	{MP1_OCPAPBCFG11, 0x07800000},
	{MP1_OCPAPBCFG12, 0x137000F5},
	{MP1_OCPAPBCFG13, 0x14891731},
	{MP1_OCPAPBCFG14, 0x01880108},
	{MP1_OCPAPBCFG15, 0x49F8124C},
	{MP1_OCPAPBCFG16, 0x41C7033C},
	{MP1_OCPAPBCFG17, 0x43C30CBC},
	{MP1_OCPAPBCFG18, 0x35C70462},
	{MP1_OCPAPBCFG19, 0x3C1E2F08},
	{MP1_OCPAPBCFG20, 0x49104172},
	{MP1_OCPAPBCFG21, 0x0FAA4185},
	{MP1_OCPAPBCFG22, 0x07990A3A},
	{MP1_OCPAPBCFG23, 0x543E5273},
	{MP1_OCPAPBCFG24, 0x33851553},
	{MP1_OCPAPBCFG25, 0x13CB29D6},
	{MP1_OCPAPBCFG26, 0x0FAA4ACF},
	{MP1_OCPAPBCFG27, 0x0CB064F0},
	{MP1_OCPAPBCFG28, 0xFFFFFFFF},
	{MP1_OCPAPBCFG29, 0xFFFFFFFF},
	{MP1_OCPAPBCFG30, 0xFFFFFFFF},
	{MP1_OCPAPBCFG31, 0x000020FF},
	{MP1_OCPAPBCFG32, 0x01022010},
	{MP1_OCPAPBCFG33, 0x00001000},
	{MP1_OCPAPBCFG34, 0x00330001},
	/*{MP1_OCP_GENERAL_CTRL, 0x0},*/
};


/* Big CPU */
static int BigOCPConfig(unsigned int MinClkPctSel, enum ocp_unit OCPUnits)
{
	int TopLkgTrim, Cpu0LkgTrim, Cpu1LkgTrim;
	int MinClkPct;
	int i;

	if (MinClkPctSel < OCP_CLK_PCT_MIN_BIG) {
		ocp_err("MinClkPctSel(%d) < %d!\n", MinClkPctSel, OCP_CLK_PCT_MIN_BIG);
		return -1;
	}

	/* 1. Write all initial values of OCPCFG. */
	for (i = 0; i < sizeof(big_setting_table)/sizeof(struct ocp_reg_setting); i++)
		ocp_write(big_setting_table[i].addr, big_setting_table[i].value);

	/*
	2. Read LkgMonTRIM[3:0] values from eFuse for each leakage monitor
	3. If any 4-bit  value is all zeros, use 4’h7
	4. Write ALL Lkg TRIM values
	*/
#if 0
	TopLkgTrim = ocp_read_field(PTP3_OD0, 19:16);
	TopLkgTrim = (!TopLkgTrim) ? 0x7 : TopLkgTrim;
	Cpu0LkgTrim = ocp_read_field(PTP3_OD0, 23:20);
	Cpu0LkgTrim = (!Cpu0LkgTrim) ? 0x7 : Cpu0LkgTrim;
	Cpu1LkgTrim = ocp_read_field(PTP3_OD0, 27:24);
	Cpu1LkgTrim = (!Cpu1LkgTrim) ? 0x7 : Cpu1LkgTrim;
	ocp_write((OCPAPBCFG04), ((Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#else
	/* no efuse */
	TopLkgTrim = 0x7;
	Cpu0LkgTrim = 0x7;
	Cpu1LkgTrim = 0x7;
	ocp_write((OCPAPBCFG04), ((Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#endif

	/* 5. Write MinClkPctSel & OCPUnits to OCPCFG */
	MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
	ocp_write_field(OCPAPBCFG05, 10:7, MinClkPct);
	ocp_write_field(OCPAPBCFG05, 0:0, OCPUnits & 0x1);

	ocp_dbg("B CFG done, MinClkPct = %d, OCPUnits = %d\n", MinClkPct, OCPUnits);

	return 0;
}

static int BigOCPTarget(unsigned int OCTgt)
{
	int OCWATgt;

	/* Convert Target to to UQ8.12 */
	OCWATgt = ((OCTgt << 12) / 1000) & BITMASK(19:0);
	ocp_write_field(OCPAPBCFG05, 31:12, OCWATgt);

	ocp_dbg("Big OCTgt = 0x%x\n", OCWATgt);

	return 0;
}

static int BigOCPFreqPct(unsigned int FreqPct)
{
	int FreqPctVal;

	/* Convert FreqPct to to UQ7.12 */
	FreqPctVal = ((FreqPct << 12) / 100) & BITMASK(18:0);
	ocp_write_field(OCPAPBCFG06, 18:0, FreqPctVal);

	ocp_dbg("Big FreqPct = 0x%x\n", FreqPctVal);

	return 0;
}

static int BigOCPVoltage(unsigned int Voltage)
{
	int VoltVal;

	/* Convert Voltage to to UQ4.12 */
	VoltVal = ((Voltage << 12) / 1000) & BITMASK(15:0);
	ocp_write_field(OCPAPBCFG07, 15:0, VoltVal);

	ocp_dbg("Big Voltage = 0x%x\n", VoltVal);

	return 0;
}

static int BigOCPClkDiv(enum ocp_clk_div NDiv)
{
#define CLKDIV_MASK	0x1B6	/* 0b110110110 */
#define CLKDIV_EN_MASK	0x49	/* 0b1001001 *//* bit 0, 3, 6 */

	int ClkDiv = 0;

	switch (NDiv) {
	case CLK_DISABLE:
		/*
		a. ocp_rx_ckdiv_en = 0
		b. ocp_clk26mdiv_en = 0
		c. ocp_ocpclkdiv_en = 0
		*/
		ocp_write(PTP3_OCP_CLK_DIVIDER, ocp_read(PTP3_OCP_CLK_DIVIDER) & ~(CLKDIV_EN_MASK));
		break;
	case CLK_DIV_4:
	case CLK_DIV_8:
	case CLK_DIV_16:
		/*
		a. ocp_rx_ckdiv_ratio = NDiv-1
		b. ocp_clk26mdiv_ratio = NDiv-1
		c. ocp_ocpclkdiv_ratio = NDiv-1
		*/
		ClkDiv = ((NDiv - 1) << 7) | ((NDiv - 1) << 4) | ((NDiv - 1) << 1);
		ocp_write(PTP3_OCP_CLK_DIVIDER, (ocp_read(PTP3_OCP_CLK_DIVIDER) & ~(CLKDIV_MASK)) | ClkDiv);
		/*
		d. ocp_rx_ckdiv_en = 1
		e. ocp_clk26mdiv_en = 1
		f. ocp_ocpclkdiv_en = 1
		*/
		ocp_write(PTP3_OCP_CLK_DIVIDER, ocp_read(PTP3_OCP_CLK_DIVIDER) | (CLKDIV_EN_MASK));
		break;
	default:
		ocp_err("Invalid Big ClkDiv value: %d\n", NDiv);
		return -1;
	}

	return 0;
}

static int BigOCPAvg(unsigned int FRatio, unsigned int AvgWindowSel)
{
	ocp_write_field(OCPAPBCFG00, 18:16, AvgWindowSel & BITMASK(2:0));
	ocp_write_field(OCPAPBCFG00, 31:20, FRatio & BITMASK(11:0));

	ocp_dbg("Big FRatio = %d, AvgWindowSel = %d\n", FRatio, AvgWindowSel);

	return 0;
}

static int BigOCPSleepEnDis(unsigned int EnDis)
{
	ocp_write_field(OCPAPBCFG00, 1:1, EnDis);

	ocp_dbg("Big SleepEn = %d\n", EnDis);

	return 0;
}

static int BigOCPEnDis(unsigned int EnDis)
{
	if (EnDis) {
		/* a. Write TOP/CPU0 LkgEn bits = 1 */
		ocp_write_field(OCPAPBCFG00, 3:2, 0x3);
		/* b. Write TOP/CPU0 LkgInit bits = 1 */
		ocp_write_field(OCPAPBCFG00, 8:7, 0x3);
		/* c. Write OCPIEn = 1 */
		ocp_write_field(OCPAPBCFG00, 0:0, 1);
	} else {
		/* a. Write CGMin = 0xF */
		ocp_write_field(OCPAPBCFG05, 10:7, 0xF);
		/* b. Write OCPIEn = 0 */
		ocp_write_field(OCPAPBCFG00, 0:0, 0);
		/* c. Write TOP/CPU0 LkgEn = 0, LkgInit = 0 */
		ocp_write_field(OCPAPBCFG00, 3:2, 0);
		ocp_write_field(OCPAPBCFG00, 8:7, 0);
	}

	ocp_dbg("Big En = %d\n", EnDis);

	return 0;
}

static int BigOCPIntLimit(enum ocp_int_select Select, int Limit)
{
	unsigned int Val = 0;

	switch (Select) {
	case IRQ_CLK_PCT_MIN:
		Val = ((Limit / 625) - 1) & BITMASK(3:0);
		ocp_write_field(OCPAPBCFG10, 23:20, Val);
		break;
	case IRQ_WA_MAX:
		/* convert to UQ8.12 */
		Val = ((Limit << 12) / 1000) & BITMASK(19:0);
		ocp_write_field(OCPAPBCFG09, 19:0, Val);
 		break;
	case IRQ_WA_MIN:
		/* convert to UQ8.12 */
		Val = ((Limit << 12) / 1000) & BITMASK(19:0);
		ocp_write_field(OCPAPBCFG10, 19:0, Val);
		break;
	default:
		ocp_err("Invalid Big Int Limit Select value: %d\n", Select);
		return -1;
	}

 	ocp_dbg("Big Int select = %d, limit = 0x%x\n", Select, Val);

	return 0;
}

static int BigOCPIntEnDis(int Value2, int Value1, int Value0)
{
	unsigned int Val = (Value2 << 16) | (Value1 << 8) | Value0;

	ocp_write(OCPAPBCFG02, Val);

	ocp_dbg("Big Int EnDis = 0x%x\n", Val);

	return 0;
}

static int BigOCPIntClr(int Value2, int Value1, int Value0)
{
	unsigned int Val = (Value2 << 16) | (Value1 << 8) | Value0;

	ocp_write(OCPAPBCFG01, Val);

	ocp_dbg("Big Int Clr = 0x%x\n", Val);

	return 0;
}

static int BigOCPLkgMonEnDis(unsigned int Cpu, unsigned int EnDis)
{
	unsigned int bits = (1 << (Cpu+8)) | (1 << (Cpu+3));

	/* first cpu will be on/off in OCPEnDis */
	if (!Cpu)
		return 0;

	if (EnDis)
		/* Write CPUx LkgEn and LkgInit bits = 1 */
		ocp_write(OCPAPBCFG00, ocp_read(OCPAPBCFG00) | bits);
	else
		/* Write CPUx LkgEn and LkgInit bits = 0 */
		ocp_write(OCPAPBCFG00, ocp_read(OCPAPBCFG00) & ~(bits));

	ocp_dbg("Big CPU %d LkgMon = %d\n", Cpu, EnDis);

	return 0;
}

/* Little CPU */
static int LittleOCPP2SInit(enum ocp_cluster Cluster)
{
	unsigned int Addr;

	if (Cluster == OCP_LL)
		Addr = MP0_OCP_GENERAL_CTRL;
	else if (Cluster == OCP_L)
		Addr = MP1_OCP_GENERAL_CTRL;
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	/* 1: enable P2S procedure: mpx_ocp_general_ctrl[4]=1 */
	ocp_write_field(Addr, 4:4, 1);
	/* 2:  Wait for P2S finish: Polling mpx_ocp_general_ctrl[31] wait until this bit become 1 */
	while (!ocp_read_field(Addr, 31:31));

	ocp_dbg("%s P2SInit done\n", (Cluster == OCP_LL) ? "LL" : "L");

	return 0;
}

static int LittleOCPConfig(enum ocp_cluster Cluster, unsigned int MinClkPctSel, enum ocp_unit OCPUnits)
{
	int TopLkgTrim, Cpu0LkgTrim, Cpu1LkgTrim, Cpu2LkgTrim, Cpu3LkgTrim;
	int MinClkPct;
	int i;

	if (MinClkPctSel < OCP_CLK_PCT_MIN_LITTLE) {
		ocp_err("MinClkPctSel(%d) < %d!\n", MinClkPctSel, OCP_CLK_PCT_MIN_LITTLE);
		return -1;
	}

	if (Cluster == OCP_LL) {
		/* 1. Write all initial values of OCPCFG. */
		for (i = 0; i < sizeof(little_mp0_setting_table)/sizeof(struct ocp_reg_setting); i++)
			ocp_write(little_mp0_setting_table[i].addr, little_mp0_setting_table[i].value);

		/* fire to HW logic */
		LittleOCPP2SInit(Cluster);

		/*
		2. Read LkgMonTRIM[3:0] values from eFuse for each leakage monitor
		3. If any 4-bit  value is all zeros, use 4’h7
		4. Write ALL Lkg TRIM values
		*/
#if 0
		TopLkgTrim = ocp_read_field(PTP3_OD2, 19:16);
		TopLkgTrim = (!TopLkgTrim) ? 0x7 : TopLkgTrim;
		Cpu0LkgTrim = ocp_read_field(PTP3_OD2, 23:20);
		Cpu0LkgTrim = (!Cpu0LkgTrim) ? 0x7 : Cpu0LkgTrim;
		Cpu1LkgTrim = ocp_read_field(PTP3_OD2, 27:24);
		Cpu1LkgTrim = (!Cpu1LkgTrim) ? 0x7 : Cpu1LkgTrim;
		Cpu2LkgTrim = ocp_read_field(PTP3_OD2, 31:28);
		Cpu2LkgTrim = (!Cpu2LkgTrim) ? 0x7 : Cpu2LkgTrim;
		Cpu3LkgTrim = ocp_read_field(PTP3_OD3, 31:28);
		Cpu3LkgTrim = (!Cpu3LkgTrim) ? 0x7 : Cpu3LkgTrim;
		ocp_write((MP0_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
				(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#else
		/* no efuse */
		TopLkgTrim = 0x7;
		Cpu0LkgTrim = 0x7;
		Cpu1LkgTrim = 0x7;
		Cpu2LkgTrim = 0x7;
		Cpu3LkgTrim = 0x7;
		ocp_write((MP0_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
				(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#endif
		/* 5. Write MinClkPctSel & OCPUnits to OCPCFG */
		MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
		ocp_write_field(MP0_OCPAPBCFG05, 10:7, MinClkPct);
		ocp_write_field(MP0_OCPAPBCFG05, 0:0, OCPUnits & 0x1);
	} else if (Cluster == OCP_L) {
		/* 1. Write all initial values of OCPCFG. */
		for (i = 0; i < sizeof(little_mp1_setting_table)/sizeof(struct ocp_reg_setting); i++)
			ocp_write(little_mp1_setting_table[i].addr, little_mp1_setting_table[i].value);

		/* fire to HW logic */
		LittleOCPP2SInit(Cluster);

		/*
		2. Read LkgMonTRIM[3:0] values from eFuse for each leakage monitor
		3. If any 4-bit  value is all zeros, use 4’h7
		4. Write ALL Lkg TRIM values
		*/
#if 0
		TopLkgTrim = ocp_read_field(PTP3_OD1, 19:16);
		TopLkgTrim = (!TopLkgTrim) ? 0x7 : TopLkgTrim;
		Cpu0LkgTrim = ocp_read_field(PTP3_OD1, 23:20);
		Cpu0LkgTrim = (!Cpu0LkgTrim) ? 0x7 : Cpu0LkgTrim;
		Cpu1LkgTrim = ocp_read_field(PTP3_OD1, 27:24);
		Cpu1LkgTrim = (!Cpu1LkgTrim) ? 0x7 : Cpu1LkgTrim;
		Cpu2LkgTrim = ocp_read_field(PTP3_OD1, 31:28);
		Cpu2LkgTrim = (!Cpu2LkgTrim) ? 0x7 : Cpu2LkgTrim;
		Cpu3LkgTrim = ocp_read_field(PTP3_OD3, 23:20);
		Cpu3LkgTrim = (!Cpu3LkgTrim) ? 0x7 : Cpu3LkgTrim;
		ocp_write((MP1_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
				(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#else
		/* no efuse */
		TopLkgTrim = 0x7;
		Cpu0LkgTrim = 0x7;
		Cpu1LkgTrim = 0x7;
		Cpu2LkgTrim = 0x7;
		Cpu3LkgTrim = 0x7;
		ocp_write((MP1_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
				(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));

#endif
		/* 5. Write MinClkPctSel & OCPUnits to OCPCFG */
		MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
		ocp_write_field(MP1_OCPAPBCFG05, 10:7, MinClkPct);
		ocp_write_field(MP1_OCPAPBCFG05, 0:0, OCPUnits & 0x1);
	} else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s CFG done, MinClkPct = %d, OCPUnits = %d\n", (Cluster == OCP_LL) ? "LL" : "L", MinClkPct, OCPUnits);

	return 0;
}

static int LittleOCPTarget(enum ocp_cluster Cluster, unsigned int OCTgt)
{
	int OCWATgt;

	/* Convert Target to to UQ8.12 */
	OCWATgt = ((OCTgt << 12) / 1000) & BITMASK(19:0);

	if (Cluster == OCP_LL)
		ocp_write_field(MP0_OCPAPBCFG05, 31:12, OCWATgt);
	else if (Cluster == OCP_L)
		ocp_write_field(MP1_OCPAPBCFG05, 31:12, OCWATgt);
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s OCTgt = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", OCWATgt);

	return 0;
}

static int LittleOCPFreqPct(enum ocp_cluster Cluster, unsigned int FreqPct)
{
	int FreqPctVal;

	/* Convert FreqPct to to UQ7.12 */
	FreqPctVal = ((FreqPct << 12) / 100) & BITMASK(18:0);

	if (Cluster == OCP_LL)
		ocp_write_field(MP0_OCPAPBCFG06, 18:0, FreqPctVal);
	else if (Cluster == OCP_L)
		ocp_write_field(MP1_OCPAPBCFG06, 18:0, FreqPctVal);
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s FreqPct = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", FreqPctVal);

	return 0;
}

static int LittleOCPVoltage(enum ocp_cluster Cluster, unsigned int Voltage)
{
	int VoltVal;

	/* Convert Voltage to to UQ4.12 */
	VoltVal = ((Voltage << 12) / 1000) & BITMASK(15:0);

	if (Cluster == OCP_LL)
		ocp_write_field(MP0_OCPAPBCFG07, 15:0, VoltVal);
	else if (Cluster == OCP_L)
		ocp_write_field(MP1_OCPAPBCFG07, 15:0, VoltVal);
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s Volt = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", VoltVal);

	return 0;
}

static int LittleOCPClkDiv(enum ocp_cluster Cluster, enum ocp_clk_div NDiv)
{
	unsigned int Addr;

	if (Cluster == OCP_LL)
		Addr = MP0_OCP_GENERAL_CTRL;
	else if (Cluster == OCP_L)
		Addr = MP1_OCP_GENERAL_CTRL;
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	if (NDiv > CLK_DIV_16)
		NDiv = CLK_DIV_16;

	/* 1: disable OCP clock: mpx_ocp_general_ctrl[2]=0 */
	ocp_write_field(Addr, 2:2, 0);
	/* 2: change clock divide ratio: mpx_ocp_general_ctrl[17:12]=0xF */
	ocp_write_field(Addr, 17:12, 0xF);
	/* 3: change ACC shift ratio: mpx_ocp_general_ctrl[11:9]=NDiv */
	ocp_write_field(Addr, 11:9, NDiv & BITMASK(2:0));
	/* 4: enable low power mode: mpx_ocp_general_ctrl[8]=0x0 */
	ocp_write_field(Addr, 8:8, 0);
	/* 5: enable OCP clock: mpx_ocp_general_ctrl[2]=1 */
	ocp_write_field(Addr, 2:2, 1);
	/* 6: unmask OCP DCM EN */
	ocp_write_field(Addr, 0:0, 0);

	ocp_dbg("%s NDiv = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", NDiv);

	return 0;
}

static int LittleOCPAvg(enum ocp_cluster Cluster, unsigned int FRatio, unsigned int AvgWindowSel)
{
	if (Cluster == OCP_LL) {
		ocp_write_field(MP0_OCPAPBCFG00, 18:16, AvgWindowSel & BITMASK(2:0));
		ocp_write_field(MP0_OCPAPBCFG00, 31:20, FRatio & BITMASK(11:0));
	} else if (Cluster == OCP_L) {
		ocp_write_field(MP1_OCPAPBCFG00, 18:16, AvgWindowSel & BITMASK(2:0));
		ocp_write_field(MP1_OCPAPBCFG00, 31:20, FRatio & BITMASK(11:0));
	} else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s FRatio = %d, AvgWindowSel = %d\n", (Cluster == OCP_LL) ? "LL" : "L", FRatio, AvgWindowSel);

	return 0;
}

static int LittleOCPSleepEnDis(enum ocp_cluster Cluster, unsigned int EnDis)
{
	if (Cluster == OCP_LL)
		ocp_write_field(MP0_OCPAPBCFG00, 1:1, EnDis);
	else if (Cluster == OCP_L)
		ocp_write_field(MP1_OCPAPBCFG00, 1:1, EnDis);
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s SleepEn = %d\n", (Cluster == OCP_LL) ? "LL" : "L", EnDis);

	return 0;
}

static int LittleOCPEnDis(enum ocp_cluster Cluster, unsigned int EnDis)
{
	unsigned int AddrCfg00, AddrCfg05;

	if (Cluster == OCP_LL) {
		AddrCfg00 = MP0_OCPAPBCFG00;
		AddrCfg05 = MP0_OCPAPBCFG05;
	} else if (Cluster == OCP_L) {
		AddrCfg00 = MP1_OCPAPBCFG00;
		AddrCfg05 = MP1_OCPAPBCFG05;
	} else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	if (EnDis) {
		/* a. Write TOP/CPU0 LkgEn bits = 1 */
		ocp_write_field(AddrCfg00, 3:2, 0x3);
		/* b. Write TOP/CPU0 LkgInit bits = 1 */
		ocp_write_field(AddrCfg00, 8:7, 0x3);
		/* c. Write OCPIEn = 1 */
		ocp_write_field(AddrCfg00, 0:0, 1);
	} else {
		/* a. Write CGMin = 0xF */
		ocp_write_field(AddrCfg05, 10:7, 0xF);
		/* b. Write OCPIEn = 0 */
		ocp_write_field(AddrCfg00, 0:0, 0);
		/* c. Write TOP/CPU0 LkgEn = 0, LkgInit = 0 */
		ocp_write_field(AddrCfg00, 8:7, 0);
		ocp_write_field(AddrCfg00, 3:2, 0);
	}

	ocp_dbg("%s OCEn = %d\n", (Cluster == OCP_LL) ? "LL" : "L", EnDis);

	return 0;
}

static int LittleOCPIntLimit(enum ocp_cluster Cluster, enum ocp_int_select Select, int Limit)
{
	unsigned int Val = 0;
	unsigned int AddrCfg09, AddrCfg10;

	if (Cluster == OCP_LL) {
		AddrCfg09 = MP0_OCPAPBCFG09;
		AddrCfg10 = MP0_OCPAPBCFG10;
	} else if (Cluster == OCP_L) {
		AddrCfg09 = MP1_OCPAPBCFG09;
		AddrCfg10 = MP1_OCPAPBCFG10;
	} else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	switch (Select) {
	case IRQ_CLK_PCT_MIN:
		Val = ((Limit / 625) - 1) & BITMASK(3:0);
		ocp_write_field(AddrCfg10, 23:20, Val);
		break;
	case IRQ_WA_MAX:
		/* convert to UQ8.12 */
		Val = ((Limit << 12) / 1000) & BITMASK(19:0);
		ocp_write_field(AddrCfg09, 19:0, Val);
 		break;
	case IRQ_WA_MIN:
		/* convert to UQ8.12 */
		Val = ((Limit << 12) / 1000) & BITMASK(19:0);
		ocp_write_field(AddrCfg10, 19:0, Val);
		break;
	default:
		ocp_err("Invalid Big Int Limit Select value: %d\n", Select);
		return -1;
	}

	ocp_dbg("%s Int select = %d, limit = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", Select, Val);

	return 0;
}

static int LittleOCPIntEnDis(enum ocp_cluster Cluster, int Value2, int Value1, int Value0)
{
	unsigned int Val = (Value2 << 16) | (Value1 << 8) | Value0;

	if (Cluster == OCP_LL)
		ocp_write(MP0_OCPAPBCFG02, Val);
	else if (Cluster == OCP_L)
		ocp_write(MP1_OCPAPBCFG02, Val);
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s Int EnDis = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", Val);

	return 0;
}

static int LittleOCPIntClr(enum ocp_cluster Cluster, int Value2, int Value1, int Value0)
{
	unsigned int Val = (Value2 << 16) | (Value1 << 8) | Value0;

	if (Cluster == OCP_LL)
		ocp_write(MP0_OCPAPBCFG01, Val);
	else if (Cluster == OCP_L)
		ocp_write(MP1_OCPAPBCFG01, Val);
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s Int Clr = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", Val);

	return 0;
}

static int LittleOCPLkgMonEnDis(enum ocp_cluster Cluster, unsigned int Cpu, unsigned int EnDis)
{
	unsigned int bits = (1 << (Cpu+8)) | (1 << (Cpu+3));

	/* first cpu will be on/off in OCPEnDis */
	if (!Cpu)
		return 0;

	if (Cluster == OCP_LL) {
		ocp_write(MP0_OCPAPBCFG00, (EnDis)
			? ocp_read(MP0_OCPAPBCFG00) | bits
			: ocp_read(MP0_OCPAPBCFG00) & ~(bits)
		);
	} else if (Cluster == OCP_L) {
		ocp_write(MP1_OCPAPBCFG00, (EnDis)
			? ocp_read(MP1_OCPAPBCFG00) | bits
			: ocp_read(MP1_OCPAPBCFG00) & ~(bits)
		);
	} else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	ocp_dbg("%s CPU %d LkgMon = %d\n", (Cluster == OCP_LL) ? "LL" : "L", Cpu, EnDis);

	return 0;
}

/* Public APIs */
int OCPRegWrite(unsigned int addr, unsigned int val)
{
	if (addr < OCP_REG_BASE_ADDR || addr > (OCP_REG_BASE_ADDR + OCP_REG_BANK_SIZE)) {
		ocp_err("Invalid addr(0x%x) to write!\n", addr);
		return -1;
	}

	ocp_write(addr, val);

	return 0;
}

int OCPRegRead(unsigned int addr)
{
	if (addr < OCP_REG_BASE_ADDR || addr > (OCP_REG_BASE_ADDR + OCP_REG_BANK_SIZE)) {
		ocp_err("Invalid addr(0x%x) to read!\n", addr);
		return -1;
	}

	return ocp_read(addr);
}

int OCPEnDis(enum ocp_cluster cluster, unsigned int enable)
{
	if (cluster == OCP_B) {
		if (enable) {
			/* Set clkdiv to max value */
			BigOCPClkDiv(CLK_DIV_16);
			/* Config with min Clkminpct and Watts for Units */
			BigOCPConfig(OCP_CLK_PCT_MIN_BIG, OCP_MW);
			/* Set target to Max value */
			BigOCPTarget(OCP_TARGET_MAX);
			/* Set default F and V */
			BigOCPFreqPct(OCP_B_DEFAULT_FREQ_PCT);
			BigOCPVoltage(OCP_B_DEFAULT_VOLT);
			/* Set FRatio and Avg window */
			BigOCPAvg(OCP_B_DEFAULT_FRATIO, OCP_DEFAULT_AVG_WINDOW);
			/* Disable sleep mode*/
			BigOCPSleepEnDis(0);
			/* Configure IRQ0 */
			/* generate an interrupt if ClkAvg drops below 75% */
			//BigOCPIntLimit(IRQ_CLK_PCT_MIN, OCP_DEFAULT_INT_THRES);
			BigOCPIntClr(0x7, 0x7, 0x7);
			/* Enable OCP */
			BigOCPEnDis(1);
			/* Wait 2 windows */
			//udelay(1500);
			/* Enable interrupt */
			//BigOCPIntEnDis(0, 0, 1 << IRQ_CLK_PCT_MIN);
		} else {
			/* Disable all INT */
			BigOCPIntEnDis(0, 0, 0);
			/* Clear all INT */
			BigOCPIntClr(0x7, 0x7, 0x7);
			/* Disable OCP */
			BigOCPEnDis(0);
		}
	} else {
		if (enable) {
			/* Set clkdiv to max value */
			LittleOCPClkDiv(cluster, CLK_DIV_16);
			/* Config with min Clkminpct and Watts for Units */
			LittleOCPConfig(cluster, OCP_CLK_PCT_MIN_LITTLE, OCP_MW);
			/* Set target to Max value */
			LittleOCPTarget(cluster, OCP_TARGET_MAX);
			/* Set default F, V, FRatio and Avg window */
			if (cluster == OCP_LL) {
				LittleOCPFreqPct(cluster, OCP_LL_DEFAULT_FREQ_PCT);
				LittleOCPVoltage(cluster, OCP_LL_DEFAULT_VOLT);
				LittleOCPAvg(cluster, OCP_LL_DEFAULT_FRATIO, OCP_DEFAULT_AVG_WINDOW);
			} else {
				LittleOCPFreqPct(cluster, OCP_L_DEFAULT_FREQ_PCT);
				LittleOCPVoltage(cluster, OCP_L_DEFAULT_VOLT);
				LittleOCPAvg(cluster, OCP_L_DEFAULT_FRATIO, OCP_DEFAULT_AVG_WINDOW);
			}

			/* Disable sleep mode*/
			LittleOCPSleepEnDis(cluster, 0);
			/* Configure IRQ0 */
			/* generate an interrupt if ClkAvg drops below 75% */
			//LittleOCPIntLimit(cluster, IRQ_CLK_PCT_MIN, OCP_DEFAULT_INT_THRES);
			LittleOCPIntClr(cluster, 0x7, 0x7, 0x7);
			/* Enable OCP */
			LittleOCPEnDis(cluster, 1);
			/* Wait 2 windows */
			//udelay(1500);
			/* Enable interrupt */
			//LittleOCPIntEnDis(cluster, 0, 0, 1 << IRQ_CLK_PCT_MIN);
		} else {
			/* Disable all INT */
			LittleOCPIntEnDis(cluster, 0, 0, 0);
			/* Clear all INT */
			LittleOCPIntClr(cluster, 0x7, 0x7, 0x7);
			/* Disable OCP */
			LittleOCPEnDis(cluster, 0);
			/* Disable clock */
			if (cluster == OCP_LL)
				ocp_write(MP0_OCP_GENERAL_CTRL, 0x0);
			else
				ocp_write(MP1_OCP_GENERAL_CTRL, 0x0);
		}
	}

	return 0;
}

int OCPTarget(enum ocp_cluster cluster, unsigned int target)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPTarget(target);
	else
		ret = LittleOCPTarget(cluster, target);

	return ret;
}

int OCPFreqPct(enum ocp_cluster cluster, unsigned int freqpct)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPFreqPct(freqpct);
	else
		ret = LittleOCPFreqPct(cluster, freqpct);

	return ret;
}

int OCPVoltage(enum ocp_cluster cluster, unsigned int volt)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPVoltage(volt);
	else
		ret = LittleOCPVoltage(cluster, volt);

	return ret;
}

int OCPIntClr(enum ocp_cluster cluster, int value2, int value1, int value0)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPIntClr(value2, value1, value0);
	else
		ret = LittleOCPIntClr(cluster, value2, value1, value0);

	return ret;
}

int OCPIntEnDis(enum ocp_cluster cluster, int value2, int value1, int value0)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPIntEnDis(value2, value1, value0);
	else
		ret = LittleOCPIntEnDis(cluster, value2, value1, value0);

	return ret;
}

int OCPIntLimit(enum ocp_cluster cluster, enum ocp_int_select select, int limit)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPIntLimit(select, limit);
	else
		ret = LittleOCPIntLimit(cluster, select, limit);

	return ret;
}

int OCPLkgMonEnDis(enum ocp_cluster cluster, unsigned int cpu, unsigned int enable)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPLkgMonEnDis(cpu, enable);
	else
		ret = LittleOCPLkgMonEnDis(cluster, cpu, enable);

	return ret;
}

