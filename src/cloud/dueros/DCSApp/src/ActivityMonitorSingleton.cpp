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

#include "DCSApp/ActivityMonitorSingleton.h"

namespace duerOSDcsApp {
namespace application {

ActivityMonitorSingleton* ActivityMonitorSingleton::s_instance = nullptr;
pthread_once_t ActivityMonitorSingleton::s_init_once = PTHREAD_ONCE_INIT;
pthread_once_t ActivityMonitorSingleton::s_destroy_once = PTHREAD_ONCE_INIT;

ActivityMonitorSingleton* ActivityMonitorSingleton::getInstance() {
    pthread_once(&s_init_once, &ActivityMonitorSingleton::init);
    return s_instance;
}

void ActivityMonitorSingleton::releaseInstance() {
    pthread_once(&s_destroy_once, ActivityMonitorSingleton::destory);
}

void ActivityMonitorSingleton::updateActiveTimestamp() {
    pthread_mutex_lock(&m_mutex);
    timeval tv;
    gettimeofday(&tv, NULL);
    m_timestamp = tv.tv_sec;
    pthread_mutex_unlock(&m_mutex);
}

long int ActivityMonitorSingleton::getLastestActiveTimestamp() const {
    return m_timestamp;
}

ActivityMonitorSingleton::ActivityMonitorSingleton() : m_timestamp(0L) {
    pthread_mutex_init(&m_mutex, NULL);
}

ActivityMonitorSingleton::~ActivityMonitorSingleton() {
    pthread_mutex_destroy(&m_mutex);
}

void ActivityMonitorSingleton::init() {
    if (nullptr == s_instance) {
        s_instance = new ActivityMonitorSingleton();
    }
}

void ActivityMonitorSingleton::destory() {
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

}  // namespace application
}  // namespace duerOSDcsApp