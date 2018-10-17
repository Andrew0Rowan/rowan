#include <debug.h>
#include <platform/mtk_wdt.h>
#include <platform/mtk_timer.h>

#if ENABLE_WDT_MODULE

static bool mtk_wd_CheckNonResetReg2(unsigned int offset)
{
    u32 tmp;
    tmp = readl(MTK_WDT_NONRST_REG2);
    if (tmp & (1U << offset))
        return true;
    else
        return false;
}

static void mtk_wd_SetNonResetReg2(unsigned int offset, bool value)
{
    u32 reg;

    reg = readl(MTK_WDT_NONRST_REG2);
    if (value)
        reg |= (1U << offset);
    else
        reg &= ~(1U << offset);

    writel(reg, MTK_WDT_NONRST_REG2);
}

void set_clr_fastboot_mode(bool flag)
{
    if (flag==true)
        mtk_wd_SetNonResetReg2(0x2, 1);
    else if (flag==false)
        mtk_wd_SetNonResetReg2(0x2, 0);

    dprintf(INFO, "set_clr_fastboot_mode\n");
}

void set_clr_recovery_mode(bool flag)
{
    if (flag==true)
        mtk_wd_SetNonResetReg2(0x1, 1);
    else if (flag==false)
        mtk_wd_SetNonResetReg2(0x1, 0);

    dprintf(INFO, "set_clr_recovery_mode\n");
}

bool check_fastboot_mode(void)
{
    return mtk_wd_CheckNonResetReg2(0x2);
}

bool check_recovery_mode(void)
{
    return mtk_wd_CheckNonResetReg2(0x1);
}

void mtk_wdt_disable(void)
{
    u32 tmp;

    tmp = readl(MTK_WDT_MODE);
    tmp &= ~MTK_WDT_MODE_ENABLE;       /* disable watchdog */
    tmp |= (MTK_WDT_MODE_KEY);         /* need key then write is allowed */
    writel(tmp, MTK_WDT_MODE);
}

static void mtk_wdt_reset(char mode)
{
    /* Watchdog Rest */
    unsigned int wdt_mode_val;
    writel(MTK_WDT_RESTART_KEY, MTK_WDT_RESTART);

    wdt_mode_val = readl(MTK_WDT_MODE);
    /* clear autorestart bit: autoretart: 1, bypass power key, 0: not bypass power key */
    wdt_mode_val &=(~MTK_WDT_MODE_AUTO_RESTART);
    /* make sure WDT mode is hw reboot mode, can not config isr mode  */
    wdt_mode_val &=(~(MTK_WDT_MODE_IRQ|MTK_WDT_MODE_ENABLE | MTK_WDT_MODE_DUAL_MODE));

    if (mode) { /* mode != 0 means by pass power key reboot, We using auto_restart bit as by pass power key flag */
        wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN|MTK_WDT_MODE_AUTO_RESTART);
        writel(wdt_mode_val, MTK_WDT_MODE);

    } else {
        wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN);
        writel(wdt_mode_val,MTK_WDT_MODE);

    }

    spin(100);
    writel(MTK_WDT_SWRST_KEY, MTK_WDT_SWRST);
}

static unsigned int mtk_wdt_check_status(void)
{
    static unsigned int status = 0;

    /*
     * Because WDT_STA register will be cleared after writing WDT_MODE,
     * we use a static variable to store WDT_STA.
     * After reset, static varialbe will always be clear to 0,
     * so only read WDT_STA when static variable is 0 is OK
     */
    if (0 == status)
        status = readl(MTK_WDT_STATUS);

    return status;
}

static void mtk_wdt_mode_config(bool dual_mode_en,
                                bool irq,
                                bool ext_en,
                                bool ext_pol,
                                bool wdt_en)
{
    unsigned int tmp;

    tmp = readl(MTK_WDT_MODE);
    tmp |= MTK_WDT_MODE_KEY;

    // Bit 0 : Whether enable watchdog or not
    if (wdt_en == true)
        tmp |= MTK_WDT_MODE_ENABLE;
    else
        tmp &= ~MTK_WDT_MODE_ENABLE;

    // Bit 1 : Configure extern reset signal polarity.
    if (ext_pol == true)
        tmp |= MTK_WDT_MODE_EXT_POL;
    else
        tmp &= ~MTK_WDT_MODE_EXT_POL;

    // Bit 2 : Whether enable external reset signal
    if (ext_en == true)
        tmp |= MTK_WDT_MODE_EXTEN;
    else
        tmp &= ~MTK_WDT_MODE_EXTEN;

    // Bit 3 : Whether generating interrupt instead of reset signal
    if (irq == true)
        tmp |= MTK_WDT_MODE_IRQ;
    else
        tmp &= ~MTK_WDT_MODE_IRQ;

    // Bit 6 : Whether enable debug module reset
    if (dual_mode_en == true)
        tmp |= MTK_WDT_MODE_DUAL_MODE;
    else
        tmp &= ~MTK_WDT_MODE_DUAL_MODE;

    // Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
    //      Because HW reboot always need reboot to kernel, we set it always.
    tmp |= MTK_WDT_MODE_AUTO_RESTART;

    writel(tmp, MTK_WDT_MODE);
    //dual_mode(1); //always dual mode
    //mdelay(100);
    dprintf(INFO,"mtk_wdt_mode_config LK mode value=%x", readl(MTK_WDT_MODE));
}

static void mtk_wdt_set_time_out_value(uint32_t value)
{
    static unsigned int timeout;

    /*
    * TimeOut = BitField 15:5
    * Key      = BitField  4:0 = 0x08
    */

    // sec * 32768 / 512 = sec * 64 = sec * 1 << 6
    timeout = (unsigned int)(value * ( 1 << 6) );
    timeout = timeout << 5;
    writel((timeout | MTK_WDT_LENGTH_KEY), MTK_WDT_LENGTH);
}

static void mtk_wdt_restart(void)
{
    // Reset WatchDogTimer's counting value to time out value
    // ie., keepalive()
    writel(MTK_WDT_RESTART_KEY, MTK_WDT_RESTART);
}

static void mtk_wdt_sw_reset(void)
{
    printf ("UB WDT SW RESET\n");
    //DRV_WriteReg32 (0x70025000, 0x2201);
    //DRV_WriteReg32 (0x70025008, 0x1971);
    //DRV_WriteReg32 (0x7002501C, 0x1209);
    mtk_wdt_reset(1);/* NOTE here, this reset will cause by pass power key */

    // system will reset

    while (1) {
        printf ("UB SW reset fail ... \n");
    };
}

static void mtk_wdt_hw_reset(void)
{
    dprintf(INFO,"UB WDT_HW_Reset\n");

    // 1. set WDT timeout 1 secs, 1*64*512/32768 = 1sec
    mtk_wdt_set_time_out_value(1);

    // 2. enable WDT debug reset enable, generating irq disable, ext reset disable
    //    ext reset signal low, wdt enalbe
    mtk_wdt_mode_config(true, false, false, false, true);

    // 3. reset the watch dog timer to the value set in WDT_LENGTH register
    mtk_wdt_restart();

    // 4. system will reset
    while (1);
}


static char *parsing_reset_reason(unsigned int wdt_status)
{
    char *rst_reason="normal";
    switch (wdt_status) {
        case MTK_WDT_STATUS_HWWDT_RST:
            rst_reason="hw_rst";
            break;

        case MTK_WDT_STATUS_SWWDT_RST:
            rst_reason="sw_rst";
            break;

        case MTK_WDT_STATUS_IRQWDT_RST:
            rst_reason="irq_rst";
            break;

        case MTK_WDT_STATUS_SECURITY_RST:
            rst_reason="security_rst";
            break;

        case MTK_WDT_STATUS_DEBUGWDT_RST:
            rst_reason="debug_rst";
            break;

        case MTK_WDT_STATUS_THERMAL_DIRECT_RST:
            rst_reason="thermal_ctl_rst";
            break;

        case MTK_WDT_STATUS_SPMWDT_RST:
            rst_reason="spm_rst";
            break;

        case MTK_WDT_STATUS_SPM_THERMAL_RST:
            rst_reason="spm_thermal_rst";
            break;

        default:
            break;
    }

    return rst_reason;
}
void mtk_wdt_init(void)
{
    /* This function will store the reset reason: Time out/ SW trigger */
    dprintf(ALWAYS, "Watchdog Status: %x , boot from %s\n", mtk_wdt_check_status(),parsing_reset_reason(mtk_wdt_check_status()));

    mtk_wdt_mode_config(false, false, false, false, false);

#if (!LK_WDT_DISABLE)
    mtk_wdt_set_time_out_value(10);
    mtk_wdt_mode_config(true, true, true, false, true);
    mtk_wdt_restart();
#endif
}

static bool mtk_is_rgu_trigger_reset(void)
{
    if (mtk_wdt_check_status())
        return true;
    return false;
}

void mtk_arch_reset(char mode)
{
    dprintf(INFO,"UB mtk_arch_reset\n");

    mtk_wdt_reset(mode);

    while (1);
}

static void rgu_swsys_reset(WD_SYS_RST_TYPE reset_type)
{
    if (WD_MD_RST == reset_type) {
        unsigned int wdt_dbg_ctrl;
        wdt_dbg_ctrl = readl(MTK_WDT_SWSYSRST);
        wdt_dbg_ctrl |= MTK_WDT_SWSYS_RST_KEY;
        wdt_dbg_ctrl |= 0x80;// 1<<7
        writel(wdt_dbg_ctrl, MTK_WDT_SWSYSRST);
        spin(1000);
        wdt_dbg_ctrl = readl(MTK_WDT_SWSYSRST);
        wdt_dbg_ctrl |= MTK_WDT_SWSYS_RST_KEY;
        wdt_dbg_ctrl &= (~0x80);// ~(1<<7)
        writel(wdt_dbg_ctrl, MTK_WDT_SWSYSRST);
        dprintf(INFO,"rgu pl md reset\n");
    }
}
#else
void mtk_wdt_init(void)
{
    dprintf(INFO,"UB WDT Dummy init called\n");
}
static bool mtk_is_rgu_trigger_reset()
{
    dprintf(INFO,"UB Dummy mtk_is_rgu_trigger_reset called\n");
    return FALSE;
}
void mtk_arch_reset(char mode)
{
    dprintf(INFO,"UB WDT Dummy arch reset called\n");
}

int mtk_wdt_boot_check(void)
{
    dprintf(INFO,"UB WDT Dummy mtk_wdt_boot_check called\n");
    return WDT_NOT_WDT_REBOOT;
}

void mtk_wdt_disable(void)
{
    dprintf(INFO,"UB WDT Dummy mtk_wdt_disable called\n");
}

static void mtk_wdt_restart(void)
{
    dprintf(INFO,"UB WDT Dummy mtk_wdt_restart called\n");
}
static void mtk_wdt_sw_reset(void)
{
    dprintf(INFO,"UB WDT Dummy mtk_wdt_sw_reset called\n");
}

static void mtk_wdt_hw_reset(void)
{
    dprintf(INFO,"UB WDT Dummy mtk_wdt_hw_reset called\n");
}
static void rgu_swsys_reset(WD_SYS_RST_TYPE reset_type)
{
    dprintf(INFO,"UB WDT Dummy rgu_swsys_reset called\n");
}
#endif
