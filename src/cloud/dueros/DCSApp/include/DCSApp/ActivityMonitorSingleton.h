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

#ifndef DUEROS_DCS_APP_SAMPLEAPP_ACTIVITYMONITORSINGLETON_H_
#define DUEROS_DCS_APP_SAMPLEAPP_ACTIVITYMONITORSINGLETON_H_

#include <pthread.h>
#include <sys/time.h>

namespace duerOSDcsApp {
namespace application {

class ActivityMonitorSingleton {
public:
    static ActivityMonitorSingleton* getInstance();

    static void releaseInstance();

    void updateActiveTimestamp();

    long int getLastestActiveTimestamp() const;

private:
    ActivityMonitorSingleton(const ActivityMonitorSingleton& ths);

    ActivityMonitorSingleton& operator=(const ActivityMonitorSingleton& ths);

    ActivityMonitorSingleton();

    ~ActivityMonitorSingleton();

    static void init();

    static void destory();

private:
    static ActivityMonitorSingleton* s_instance;

    static pthread_once_t s_init_once;

    static pthread_once_t s_destroy_once;

    pthread_mutex_t m_mutex;

    long int m_timestamp;  //the unit is second
};

}  // namespace application
}  // namespace duerOSDcsApp

#endif  // DUEROS_DCS_APP_SAMPLEAPP_ACTIVITYMONITORSINGLETON_H_
