/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <signal.h>
#include <cstdlib>
#include <string>
#include <execinfo.h>
#include "LoggerUtils/DcsSdkLogger.h"
#include "DCSApp/DCSApplication.h"
#include "DCSApp/DeviceIoWrapper.h"
#include <DeviceTools/SingleApplication.h>
#include <DeviceTools/PrintTickCount.h>

#ifndef __SAMPLEAPP_VERSION__
#define __SAMPLEAPP_VERSION__ "Unknown SampleApp Version"
#endif

#ifndef __DCSSDK_VERSION__
#define __DCSSDK_VERSION__ "Unknown DcsSdk Version"
#endif

#ifndef __DUER_LINK_VERSION__
#define __DUER_LINK_VERSION__ "Unknown DuerLink Version"
#endif

int main(int argc, char** argv) {
    pid_t status = 0;
    int i = 200;

    deviceCommonLib::deviceTools::printTickCount("duer_linux main begin");
#ifdef MTK8516
    if (geteuid() != 0) {
        APP_ERROR("This program must run as root, such as \"sudo ./duer_linux\"");
        return 1;
    }
    if (deviceCommonLib::deviceTools::SingleApplication::is_running()) {
        APP_ERROR("duer_linux is already running");
        return 1;
    }
#endif

    /*enable wlan0 interface*/
    while(i > 0) {
        status = system("ifconfig wlan0 up");
        if (status == -1) {
            APP_INFO("ifconfig wlan0 up first fail");
            usleep(50);
            i--;
            continue;
        } else {
            APP_INFO("ifconfig wlan0 up first successfully.status = 0x%x, i = %d",status,i);

            if (WIFEXITED(status)) {
                APP_INFO("ifconfig wlan0 up second successfully.status = 0x%x,i=%d",status,i);
                if ( WEXITSTATUS(status)) {
                    APP_INFO("ifconfig wlan0 up second fail.error = %d", WEXITSTATUS(status));
                    usleep(50);
                    i--;
                    continue;
                } else {
                    APP_INFO("ifconfig wlan0 up third successfully.i = %d",i);
                    break;
                }
            } else {
                APP_INFO("ifconfig wlan0 up second fail. error=%d",WEXITSTATUS(status));
                usleep(50);
                i--;
                continue;
            }
        }
    }

    status = system("sh /usr/bin/dueros/generator_device_id.sh");
    if (-1 == status) {
        APP_ERROR("system failed!");
    } else {
        if (WIFEXITED(status)) {
            if (0 == WEXITSTATUS(status)) {
            } else {
                APP_ERROR("System shell script failed:[%d].", WEXITSTATUS(status));
            }
        } else {
            APP_INFO("System status = [%d]", WEXITSTATUS(status));
        }
    }

    /// print current version
    APP_INFO("SampleApp Version: [%s]", __SAMPLEAPP_VERSION__);
    APP_INFO("DcsSdk Version: [%s]", __DCSSDK_VERSION__);
    APP_INFO("DuerLink Version: [%s]", __DUER_LINK_VERSION__);

    auto dcsApplication = duerOSDcsApp::application::DCSApplication::create();

    if (!dcsApplication) {
        APP_ERROR("Failed to create to SampleApplication!");
        duerOSDcsApp::application::SoundController::getInstance()->release();
        duerOSDcsApp::application::DeviceIoWrapper::getInstance()->release();
        return EXIT_FAILURE;
    }

    // This will run until application quit.
    dcsApplication->run();

    return EXIT_SUCCESS;
}
