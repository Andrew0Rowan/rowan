
/******************************************************************************
*                         C O M P I L E R   F L A G S
*******************************************************************************
*/

/******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
*******************************************************************************
*/
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
/*#include <syslog.h>*/
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>



/******************************************************************************
*                              C O N S T A N T S
*******************************************************************************
*/
/* !defined(ANDROID) */
#ifndef ALOGI
#define ALOGI printf
#endif
#ifndef ALOGE
#define ALOGE printf
#endif
#ifndef ALOGD
#define ALOGD printf
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "6620_launcher"

#if TARGET_PLATFORM == mt2635
#define CUST_COMBO_WIFI_DEV "/dev/wmtWifi"
#define POWER_ON_WIFI "AP"
#define POWER_ON_WIFI_LEN 2
#else
#define CUST_COMBO_WIFI_DEV "/dev/wmtWifi"
#define POWER_ON_WIFI "1"
#define POWER_ON_WIFI_LEN 1
#endif

/******************************************************************************
*                             D A T A   T Y P E S
*******************************************************************************
*/

/******************************************************************************
*                                 M A C R O S
*******************************************************************************
*/

/******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
*******************************************************************************
*/

/******************************************************************************
*                            P U B L I C   D A T A
*******************************************************************************
*/

/******************************************************************************
*                           P R I V A T E   D A T A
*******************************************************************************
*/

static int gWifiFd = -1;

/******************************************************************************
*                              F U N C T I O N S
*******************************************************************************
*/
int main(int argc, char *argv[]) {
        int retryCounter = 0;
        int i_ret = -1;

        gWifiFd = open(CUST_COMBO_WIFI_DEV, O_RDWR | O_NOCTTY);
        do {
                if (gWifiFd < 0) {
                        ALOGI("Can't open device node(%s) error:%d \n", CUST_COMBO_WIFI_DEV, gWifiFd);
                        usleep(300000);
                } else {
                       break;
                }
                retryCounter++;
        } while (retryCounter < 20);

        if (gWifiFd > 0) {
                do {
                        i_ret = write(gWifiFd, POWER_ON_WIFI, POWER_ON_WIFI_LEN);
                        if (i_ret == 1) {
                                ALOGI("Power on device node(%s) gWifiFd:%d succeed !!\n", CUST_COMBO_WIFI_DEV, gWifiFd);
                                break;
                        } else {
                               ALOGI("Power on device node(%s) gWifiFd:%d Fail (%d) and retry\n", CUST_COMBO_WIFI_DEV, gWifiFd, i_ret);
                        }
                        retryCounter++;
                        usleep(1000000);
                } while (retryCounter < 20);
        }
        close(gWifiFd);

    return 0;
}
