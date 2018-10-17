#include <reg.h>
#include <platform/mt8516.h>
#include <platform/spm.h>
#include <platform/spm_mtcmos.h>
#include <platform/spm_mtcmos_internal.h>
#include <platform/pll.h>

#define USE_SPIN    0

#define spm_mtcmos_cpu_lock(x)      (*x = 0)
#define spm_mtcmos_cpu_unlock(x)    (*x = 0)
#define spm_mtcmos_noncpu_lock(x)   (*(&x) = 0)
#define spm_mtcmos_noncpu_unlock(x) (*(&x) = 0)

#if USE_SPIN

#define udelay(x)       spin(x)
#define mdelay(x)       udelay((x) * 1000)

#else /* !USE_SPIN */

#define my_delay(count) \
    do { \
        volatile unsigned int i = count * 26; \
        for (; i != 0; i--); \
    } while (0)

#define udelay(x)       my_delay(x)
#define mdelay(x)       udelay((x) * 1000)

#endif              /* USE_SPIN */

typedef int (*spm_cpu_mtcmos_ctrl_func) (int state, int chkWfiBeforePdn);

static void wait_reg_bits(addr_t reg, u32 mask, u32 value)
{
#ifndef CFG_FPGA_PLATFORM
    while ((spm_read(reg) & mask) != value) ;
#endif
}

static void wait_2reg_bits(addr_t reg1, addr_t reg2, u32 mask, u32 value)
{
#ifndef CFG_FPGA_PLATFORM
    while ((spm_read(reg1) & mask) != value ||
            (spm_read(reg2) & mask) != value) ;
#endif
}

static void wait_pwr_status(u32 mask, u32 value)
{
    wait_2reg_bits(SPM_PWR_STATUS, SPM_PWR_STATUS_2ND, mask, value);
}

static spm_cpu_mtcmos_ctrl_func spm_cpu_mtcmos_ctrl_funcs[] = {

    spm_mtcmos_ctrl_cpu0,
    spm_mtcmos_ctrl_cpu1,
    spm_mtcmos_ctrl_cpu2,
    spm_mtcmos_ctrl_cpu3,
    spm_mtcmos_ctrl_cpu4,
    spm_mtcmos_ctrl_cpu5,
    spm_mtcmos_ctrl_cpu6,
    spm_mtcmos_ctrl_cpu7
};

int spm_mtcmos_ctrl_cpu(unsigned int cpu, int state, int chkWfiBeforePdn)
{
    return (*spm_cpu_mtcmos_ctrl_funcs[cpu]) (state, chkWfiBeforePdn);
}

int spm_mtcmos_ctrl_cpu0(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA7_CPU0_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU0_L1_PDN,
                  spm_read(SPM_CA7_CPU0_L1_PDN) | L1_PDN);

        wait_reg_bits(SPM_CA7_CPU0_L1_PDN, L1_PDN_ACK, L1_PDN_ACK);

        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA7_CPU0, 0);

        spm_mtcmos_cpu_unlock(&flags);
    } else {    /* STA_POWER_ON */

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA7_CPU0, CA7_CPU0);

        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA7_CPU0_L1_PDN,
                  spm_read(SPM_CA7_CPU0_L1_PDN) & ~L1_PDN);

        wait_reg_bits(SPM_CA7_CPU0_L1_PDN, L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA7_CPU0_PWR_CON,
                  spm_read(SPM_CA7_CPU0_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu1(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA7_CPU1_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU1_L1_PDN,
                  spm_read(SPM_CA7_CPU1_L1_PDN) | L1_PDN);

        wait_reg_bits(SPM_CA7_CPU1_L1_PDN, L1_PDN_ACK, L1_PDN_ACK);

        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA7_CPU1, 0);

        spm_mtcmos_cpu_unlock(&flags);
    } else {    /* STA_POWER_ON */

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA7_CPU1, CA7_CPU1);

        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA7_CPU1_L1_PDN,
                  spm_read(SPM_CA7_CPU1_L1_PDN) & ~L1_PDN);

        wait_reg_bits(SPM_CA7_CPU1_L1_PDN, L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA7_CPU1_PWR_CON,
                  spm_read(SPM_CA7_CPU1_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu2(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA7_CPU2_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU2_L1_PDN,
                  spm_read(SPM_CA7_CPU2_L1_PDN) | L1_PDN);

        wait_reg_bits(SPM_CA7_CPU2_L1_PDN, L1_PDN_ACK, L1_PDN_ACK);

        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA7_CPU2, 0);

        spm_mtcmos_cpu_unlock(&flags);
    } else {    /* STA_POWER_ON */

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA7_CPU2, CA7_CPU2);

        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA7_CPU2_L1_PDN,
                  spm_read(SPM_CA7_CPU2_L1_PDN) & ~L1_PDN);

        wait_reg_bits(SPM_CA7_CPU2_L1_PDN, L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA7_CPU2_PWR_CON,
                  spm_read(SPM_CA7_CPU2_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu3(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA7_CPU3_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU3_L1_PDN,
                  spm_read(SPM_CA7_CPU3_L1_PDN) | L1_PDN);

        wait_reg_bits(SPM_CA7_CPU3_L1_PDN, L1_PDN_ACK, L1_PDN_ACK);

        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA7_CPU3, 0);

        spm_mtcmos_cpu_unlock(&flags);
    } else {    /* STA_POWER_ON */

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA7_CPU3, CA7_CPU3);

        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA7_CPU3_L1_PDN,
                  spm_read(SPM_CA7_CPU3_L1_PDN) & ~L1_PDN);

        wait_reg_bits(SPM_CA7_CPU3_L1_PDN, L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA7_CPU3_PWR_CON,
                  spm_read(SPM_CA7_CPU3_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu4(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            mdelay(500);

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU0_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU0_CA15_L1_PDN_ACK, CPU0_CA15_L1_PDN_ACK);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU0_CA15_L1_PDN_ISO);

        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA15_CPU0, 0);

        spm_mtcmos_cpu_unlock(&flags);

        if (!
                (spm_read(SPM_PWR_STATUS) &
                 (CA15_CPU1 | CA15_CPU2 | CA15_CPU3))
                && !(spm_read(SPM_PWR_STATUS_2ND) &
                     (CA15_CPU1 | CA15_CPU2 | CA15_CPU3)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } else {    /* STA_POWER_ON */

        if (!(spm_read(SPM_PWR_STATUS) & CA15_CPUTOP) &&
                !(spm_read(SPM_PWR_STATUS_2ND) & CA15_CPUTOP))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA15_CPU0, CA15_CPU0);

        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) &
                  ~CPU0_CA15_L1_PDN_ISO);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) & ~CPU0_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU0_CA15_L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA15_CPU0_PWR_CON,
                  spm_read(SPM_CA15_CPU0_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu5(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA15_CPU1_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU1_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU1_CA15_L1_PDN_ACK, CPU1_CA15_L1_PDN_ACK);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU1_CA15_L1_PDN_ISO);

        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA15_CPU1, 0);

        spm_mtcmos_cpu_unlock(&flags);

        if (!
                (spm_read(SPM_PWR_STATUS) &
                 (CA15_CPU1 | CA15_CPU2 | CA15_CPU3))
                && !(spm_read(SPM_PWR_STATUS_2ND) &
                     (CA15_CPU1 | CA15_CPU2 | CA15_CPU3)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } else {    /* STA_POWER_ON */

        if (!(spm_read(SPM_PWR_STATUS) & CA15_CPUTOP) &&
                !(spm_read(SPM_PWR_STATUS_2ND) & CA15_CPUTOP))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA15_CPU1, CA15_CPU1);

        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) &
                  ~CPU1_CA15_L1_PDN_ISO);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) & ~CPU1_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU1_CA15_L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA15_CPU1_PWR_CON,
                  spm_read(SPM_CA15_CPU1_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu6(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA15_CPU2_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU2_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU2_CA15_L1_PDN_ACK, CPU2_CA15_L1_PDN_ACK);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU2_CA15_L1_PDN_ISO);

        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA15_CPU2, 0);

        spm_mtcmos_cpu_unlock(&flags);

        if (!
                (spm_read(SPM_PWR_STATUS) &
                 (CA15_CPU1 | CA15_CPU2 | CA15_CPU3))
                && !(spm_read(SPM_PWR_STATUS_2ND) &
                     (CA15_CPU1 | CA15_CPU2 | CA15_CPU3)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } else {    /* STA_POWER_ON */

        if (!(spm_read(SPM_PWR_STATUS) & CA15_CPUTOP) &&
                !(spm_read(SPM_PWR_STATUS_2ND) & CA15_CPUTOP))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA15_CPU2, CA15_CPU2);

        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) &
                  ~CPU2_CA15_L1_PDN_ISO);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) & ~CPU2_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU2_CA15_L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA15_CPU2_PWR_CON,
                  spm_read(SPM_CA15_CPU2_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu7(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA15_CPU3_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU3_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU3_CA15_L1_PDN_ACK, CPU3_CA15_L1_PDN_ACK);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) | CPU3_CA15_L1_PDN_ISO);

        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA15_CPU3, 0);

        spm_mtcmos_cpu_unlock(&flags);

        if (!
                (spm_read(SPM_PWR_STATUS) &
                 (CA15_CPU1 | CA15_CPU2 | CA15_CPU3))
                && !(spm_read(SPM_PWR_STATUS_2ND) &
                     (CA15_CPU1 | CA15_CPU2 | CA15_CPU3)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } else {    /* STA_POWER_ON */

        if (!(spm_read(SPM_PWR_STATUS) & CA15_CPUTOP) &&
                !(spm_read(SPM_PWR_STATUS_2ND) & CA15_CPUTOP))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA15_CPU3, CA15_CPU3);

        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) &
                  ~CPU3_CA15_L1_PDN_ISO);
        spm_write(SPM_CA15_L1_PWR_CON,
                  spm_read(SPM_CA15_L1_PWR_CON) & ~CPU3_CA15_L1_PDN);

        wait_reg_bits(SPM_CA15_L1_PWR_CON, CPU3_CA15_L1_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA15_CPU3_PWR_CON,
                  spm_read(SPM_CA15_CPU3_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpusys0(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) &
                    CA7_CPUTOP_STANDBYWFI) == 0) ;

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) | SRAM_CKISO);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_CA7_CPUTOP_L2_PDN,
                  spm_read(SPM_CA7_CPUTOP_L2_PDN) | L2_SRAM_PDN);

        wait_reg_bits(SPM_CA7_CPUTOP_L2_PDN, L2_SRAM_PDN_ACK, L2_SRAM_PDN_ACK);

        udelay(2);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA7_CPUTOP, 0);

        spm_mtcmos_cpu_unlock(&flags);
    } else {    /* STA_POWER_ON */

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA7_CPUTOP, CA7_CPUTOP);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA7_CPUTOP_L2_PDN,
                  spm_read(SPM_CA7_CPUTOP_L2_PDN) & ~L2_SRAM_PDN);

        wait_reg_bits(SPM_CA7_CPUTOP_L2_PDN, L2_SRAM_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) | SRAM_ISOINT_B);
        udelay(1);
        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA7_CPUTOP_PWR_CON,
                  spm_read(SPM_CA7_CPUTOP_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

int spm_mtcmos_ctrl_cpusys1(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN) {
        if (chkWfiBeforePdn)
            mdelay(500);

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) | PWR_ISO);

        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) & ~SRAM_ISOINT_B);

        spm_write(SPM_CA15_L2_PWR_CON,
                  spm_read(SPM_CA15_L2_PWR_CON) | CA15_L2_PDN);

        wait_reg_bits(SPM_CA15_L2_PWR_CON, CA15_L2_PDN_ACK, CA15_L2_PDN_ACK);

        spm_write(SPM_CA15_L2_PWR_CON,
                  spm_read(SPM_CA15_L2_PWR_CON) | CA15_L2_PDN_ISO);
        udelay(2);

        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) & ~PWR_RST_B);
        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) | PWR_CLK_DIS);

        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) & ~PWR_ON);
        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) & ~PWR_ON_2ND);

        wait_pwr_status(CA15_CPUTOP, 0);

        spm_mtcmos_cpu_unlock(&flags);
    } else {    /* STA_POWER_ON */

        spm_mtcmos_cpu_lock(&flags);

        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) | PWR_ON_2ND);

        wait_pwr_status(CA15_CPUTOP, CA15_CPUTOP);

        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) & ~PWR_ISO);

        spm_write(SPM_CA15_L2_PWR_CON,
                  spm_read(SPM_CA15_L2_PWR_CON) & ~CA15_L2_PDN_ISO);
        spm_write(SPM_CA15_L2_PWR_CON,
                  spm_read(SPM_CA15_L2_PWR_CON) & ~CA15_L2_PDN);

        wait_reg_bits(SPM_CA15_L2_PWR_CON, CA15_L2_PDN_ACK, 0);

        udelay(1);
        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) | SRAM_ISOINT_B);
        udelay(1);
        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) & ~SRAM_CKISO);

        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CA15_CPUTOP_PWR_CON,
                  spm_read(SPM_CA15_CPUTOP_PWR_CON) | PWR_RST_B);

        spm_mtcmos_cpu_unlock(&flags);
    }

    return 0;
}

void spm_mtcmos_ctrl_cpusys1_init_1st_bring_up(int state)
{

    if (state == STA_POWER_DOWN) {
        spm_mtcmos_ctrl_cpu7(STA_POWER_DOWN, 0);
        spm_mtcmos_ctrl_cpu6(STA_POWER_DOWN, 0);
        spm_mtcmos_ctrl_cpu5(STA_POWER_DOWN, 0);
        spm_mtcmos_ctrl_cpu4(STA_POWER_DOWN, 0);
    } else {    /* STA_POWER_ON */

        spm_mtcmos_ctrl_cpu4(STA_POWER_ON, 1);
        spm_mtcmos_ctrl_cpu5(STA_POWER_ON, 1);
        spm_mtcmos_ctrl_cpu6(STA_POWER_ON, 1);
        spm_mtcmos_ctrl_cpu7(STA_POWER_ON, 1);
    }
}

bool spm_cpusys0_can_power_down(void)
{
    return !(spm_read(SPM_PWR_STATUS) &
             (CA15_CPU0 | CA15_CPU1 | CA15_CPU2 | CA15_CPU3 | CA15_CPUTOP |
              CA7_CPU1 | CA7_CPU2 | CA7_CPU3))
           && !(spm_read(SPM_PWR_STATUS_2ND) &
                (CA15_CPU0 | CA15_CPU1 | CA15_CPU2 | CA15_CPU3 | CA15_CPUTOP |
                 CA7_CPU1 | CA7_CPU2 | CA7_CPU3));
}

bool spm_cpusys1_can_power_down(void)
{
    return !(spm_read(SPM_PWR_STATUS) &
             (CA7_CPU0 | CA7_CPU1 | CA7_CPU2 | CA7_CPU3 | CA7_CPUTOP |
              CA15_CPU1 | CA15_CPU2 | CA15_CPU3))
           && !(spm_read(SPM_PWR_STATUS_2ND) &
                (CA7_CPU0 | CA7_CPU1 | CA7_CPU2 | CA7_CPU3 | CA7_CPUTOP |
                 CA15_CPU1 | CA15_CPU2 | CA15_CPU3));
}


/**************************************
 * for non-CPU MTCMOS
 **************************************/
#define MD2_PWR_STA_MASK    (0x1 << 22)
#define VEN_PWR_STA_MASK    (0x1 << 8)
#define VDE_PWR_STA_MASK    (0x1 << 7)
#define ISP_PWR_STA_MASK    (0x1 << 5)
#define MFG_PWR_STA_MASK    (0x1 << 4)
#define DIS_PWR_STA_MASK    (0x1 << 3)
#define CONN_PWR_STA_MASK   (0x1 << 1)
#define MD1_PWR_STA_MASK    (0x1 << 0)

#define SRAM_PDN            (0xf << 8)  /* VDEC, VENC, ISP, DISP */
#define MFG_SRAM_PDN        (0x3f << 8)
#define MD_SRAM_PDN         (0x1 << 8)  /* MD1, C2K */
#define CONN_SRAM_PDN       (0x1 << 8)

#define VDE_SRAM_ACK        (0x1 << 12)
#define VEN_SRAM_ACK        (0xf << 12)
#define ISP_SRAM_ACK        (0x3 << 12)
#define DIS_SRAM_ACK        (0x1 << 12)
#define MFG_SRAM_ACK        (0x1 << 16)

#define MD1_PROT_MASK        ((0x1U<<24) | (0x1<<25) | (0x1<<26) | (0x1<<27) | (0x1<<28))   /* bit 24,25,26,27,28 */
#define MD2_PROT_MASK        ((0x1U<<29) | (0x1<<30) | (0x1<<31))   /* bit 29, 30, 31 */
/* bit16 is GCE, Dram dummy read use cqdma, and it is in GCE. */
#define DISP_PROT_MASK       ((0x1 << 1))   /* bit 1, 6, 16; if bit6 set, MMSYS PDN, access reg will hang, */
#define MFG_PROT_MASK        ((0x1 << 14))  /* bit 14 */
#define CONN_PROT_MASK       ((0x1 << 2) | (0x1 << 8))  /* bit 2, 8 */



int spm_mtcmos_ctrl_vdec(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_VDE_PWR_CON,
                  spm_read(SPM_VDE_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_VDE_PWR_CON) & VDE_SRAM_ACK) !=
                VDE_SRAM_ACK) {
        }

        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_VDE_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_VDE_PWR_CON, val);

        spm_write(SPM_VDE_PWR_CON,
                  spm_read(SPM_VDE_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & VDE_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & VDE_PWR_STA_MASK)) {
        }
    } else {    /* STA_POWER_ON */
        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) | PWR_ON);
        spm_write(SPM_VDE_PWR_CON,
                  spm_read(SPM_VDE_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & VDE_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_2ND) & VDE_PWR_STA_MASK)) {
        }

        spm_write(SPM_VDE_PWR_CON,
                  spm_read(SPM_VDE_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_VDE_PWR_CON,
                  spm_read(SPM_VDE_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_VDE_PWR_CON,
                  spm_read(SPM_VDE_PWR_CON) | PWR_RST_B);

        spm_write(SPM_VDE_PWR_CON,
                  spm_read(SPM_VDE_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_VDE_PWR_CON) & VDE_SRAM_ACK)) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_venc(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_VEN_PWR_CON,
                  spm_read(SPM_VEN_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_VEN_PWR_CON) & VEN_SRAM_ACK) !=
                VEN_SRAM_ACK) {
        }

        spm_write(SPM_VEN_PWR_CON, spm_read(SPM_VEN_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_VEN_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_VEN_PWR_CON, val);

        spm_write(SPM_VEN_PWR_CON,
                  spm_read(SPM_VEN_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & VEN_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & VEN_PWR_STA_MASK)) {
        }
    } else {    /* STA_POWER_ON */
        spm_write(SPM_VEN_PWR_CON, spm_read(SPM_VEN_PWR_CON) | PWR_ON);
        spm_write(SPM_VEN_PWR_CON,
                  spm_read(SPM_VEN_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & VEN_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_2ND) & VEN_PWR_STA_MASK)) {
        }

        spm_write(SPM_VEN_PWR_CON,
                  spm_read(SPM_VEN_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_VEN_PWR_CON,
                  spm_read(SPM_VEN_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_VEN_PWR_CON,
                  spm_read(SPM_VEN_PWR_CON) | PWR_RST_B);

        spm_write(SPM_VEN_PWR_CON,
                  spm_read(SPM_VEN_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_VEN_PWR_CON) & VEN_SRAM_ACK)) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_isp(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_ISP_PWR_CON,
                  spm_read(SPM_ISP_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_ISP_PWR_CON) & ISP_SRAM_ACK) !=
                ISP_SRAM_ACK) {
        }

        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_ISP_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_ISP_PWR_CON, val);

        spm_write(SPM_ISP_PWR_CON,
                  spm_read(SPM_ISP_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & ISP_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & ISP_PWR_STA_MASK)) {
        }
    } else {    /* STA_POWER_ON */
        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) | PWR_ON);
        spm_write(SPM_ISP_PWR_CON,
                  spm_read(SPM_ISP_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & ISP_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_2ND) & ISP_PWR_STA_MASK)) {
        }

        spm_write(SPM_ISP_PWR_CON,
                  spm_read(SPM_ISP_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_ISP_PWR_CON,
                  spm_read(SPM_ISP_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_ISP_PWR_CON,
                  spm_read(SPM_ISP_PWR_CON) | PWR_RST_B);

        spm_write(SPM_ISP_PWR_CON,
                  spm_read(SPM_ISP_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_ISP_PWR_CON) & ISP_SRAM_ACK)) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}


int spm_mtcmos_ctrl_disp(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) | DISP_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & DISP_PROT_MASK) !=
                DISP_PROT_MASK) {
        }

        spm_write(SPM_DIS_PWR_CON,
                  spm_read(SPM_DIS_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK) !=
                DIS_SRAM_ACK) {
        }

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_DIS_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_DIS_PWR_CON, val);

        spm_write(SPM_DIS_PWR_CON,
                  spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & DIS_PWR_STA_MASK)) {
        }
    } else {    /* STA_POWER_ON */
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON);
        spm_write(SPM_DIS_PWR_CON,
                  spm_read(SPM_DIS_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_2ND) & DIS_PWR_STA_MASK)) {
        }

        spm_write(SPM_DIS_PWR_CON,
                  spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_DIS_PWR_CON,
                  spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_DIS_PWR_CON,
                  spm_read(SPM_DIS_PWR_CON) | PWR_RST_B);

        spm_write(SPM_DIS_PWR_CON,
                  spm_read(SPM_DIS_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK)) {
        }

        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) & ~DISP_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & DISP_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}


int spm_mtcmos_ctrl_mfg(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) | MFG_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & MFG_PROT_MASK) !=
                MFG_PROT_MASK) {
        }

        spm_write(SPM_MFG_PWR_CON,
                  spm_read(SPM_MFG_PWR_CON) | MFG_SRAM_PDN);

        while ((spm_read(SPM_MFG_PWR_CON) & MFG_SRAM_ACK) !=
                MFG_SRAM_ACK) {
        }

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_MFG_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_MFG_PWR_CON, val);

        spm_write(SPM_MFG_PWR_CON,
                  spm_read(SPM_MFG_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & MFG_PWR_STA_MASK)) {
        }
    } else {    /* STA_POWER_ON */
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON);
        spm_write(SPM_MFG_PWR_CON,
                  spm_read(SPM_MFG_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK) ||
                !(spm_read(SPM_PWR_STATUS_2ND) & MFG_PWR_STA_MASK)) {
        }

        spm_write(SPM_MFG_PWR_CON,
                  spm_read(SPM_MFG_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MFG_PWR_CON,
                  spm_read(SPM_MFG_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MFG_PWR_CON,
                  spm_read(SPM_MFG_PWR_CON) | PWR_RST_B);

        spm_write(SPM_MFG_PWR_CON,
                  spm_read(SPM_MFG_PWR_CON) & ~MFG_SRAM_PDN);

        while ((spm_read(SPM_MFG_PWR_CON) & MFG_SRAM_ACK)) {
        }

        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) & ~MFG_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & MFG_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_mdsys1(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) | MD1_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & MD1_PROT_MASK) !=
                MD1_PROT_MASK) {
        }

        spm_write(SPM_MD_PWR_CON,
                  spm_read(SPM_MD_PWR_CON) | MD_SRAM_PDN);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_MD_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_MD_PWR_CON, val);

        spm_write(SPM_MD_PWR_CON,
                  spm_read(SPM_MD_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & MD1_PWR_STA_MASK)) {
        }

    } else {    /* STA_POWER_ON */

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ON);
        spm_write(SPM_MD_PWR_CON,
                  spm_read(SPM_MD_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_2ND) & MD1_PWR_STA_MASK)) {
        }

        spm_write(SPM_MD_PWR_CON,
                  spm_read(SPM_MD_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_RST_B);

        spm_write(SPM_MD_PWR_CON,
                  spm_read(SPM_MD_PWR_CON) & ~MD_SRAM_PDN);

        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) & ~MD1_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & MD1_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_mdsys2(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) | MD2_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & MD2_PROT_MASK) !=
                MD2_PROT_MASK) {
        }

        spm_write(SPM_C2K_PWR_CON,
                  spm_read(SPM_C2K_PWR_CON) | MD_SRAM_PDN);

        spm_write(SPM_C2K_PWR_CON, spm_read(SPM_C2K_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_C2K_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_C2K_PWR_CON, val);

        spm_write(SPM_C2K_PWR_CON,
                  spm_read(SPM_C2K_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & MD2_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & MD2_PWR_STA_MASK)) {
        }

    } else {    /* STA_POWER_ON */

        spm_write(SPM_C2K_PWR_CON, spm_read(SPM_C2K_PWR_CON) | PWR_ON);
        spm_write(SPM_C2K_PWR_CON,
                  spm_read(SPM_C2K_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & MD2_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_2ND) & MD2_PWR_STA_MASK)) {
        }

        spm_write(SPM_C2K_PWR_CON,
                  spm_read(SPM_C2K_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_C2K_PWR_CON,
                  spm_read(SPM_C2K_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_C2K_PWR_CON,
                  spm_read(SPM_C2K_PWR_CON) | PWR_RST_B);

        spm_write(SPM_C2K_PWR_CON,
                  spm_read(SPM_C2K_PWR_CON) & ~MD_SRAM_PDN);

        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) & ~MD2_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & MD2_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_connsys(int state)
{
    int err = 0;
    unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {

        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) | CONN_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & CONN_PROT_MASK) !=
                CONN_PROT_MASK) {
        }

        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) | CONN_SRAM_PDN);

        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_CONN_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_CONN_PWR_CON, val);

        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) & ~(PWR_ON | PWR_ON_2ND));

        while ((spm_read(SPM_PWR_STATUS) & CONN_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_2ND) & CONN_PWR_STA_MASK)) {
        }
    } else {
        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) | PWR_ON);
        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) | PWR_ON_2ND);

        while (!(spm_read(SPM_PWR_STATUS) & CONN_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_2ND) & CONN_PWR_STA_MASK)) {
        }

        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) | PWR_RST_B);

        spm_write(SPM_CONN_PWR_CON,
                  spm_read(SPM_CONN_PWR_CON) & ~CONN_SRAM_PDN);

        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) & ~CONN_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & CONN_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_topaxi_prot(int bit, int en)
{
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (en == 1) {
        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) | (1U << bit));
        while ((spm_read(TOPAXI_PROT_STA1) & (1U << bit)) !=
                (1U << bit)) {
        }
    } else {
        spm_write(TOPAXI_PROT_EN,
                  spm_read(TOPAXI_PROT_EN) & ~(1U << bit));
        while (spm_read(TOPAXI_PROT_STA1) & (1U << bit)) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return 0;
}
