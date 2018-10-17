/******************************************************************************
 *
 *  Copyright (C) 2016 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#include <string.h>

#include "osi/include/properties.h"

#ifdef MTK_BLUEDROID_PATCH
#include "mdroid_buildcfg.h"
#endif

#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
#include <fcntl.h>
#include "osi/include/log.h"

#define LOG_TAG "bt_property"
#define PROPERTY_FILE BT_MISC_PATH"bt_property"
#endif

int osi_property_get(const char *key, char *value, const char *default_value) {
#if defined(OS_GENERIC)
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    int fd = 0, len = 0, h_flag = 0, value_len = 0;
    char *rbuf = NULL;
    char *location = NULL;
    char *p_key = NULL;
    char *p_value = value;

    if (!default_value)
    {
        return -1;
    }

    value_len = strlen(default_value);
    if (value_len >= PROPERTY_VALUE_MAX)
        value_len = PROPERTY_VALUE_MAX - 1;

    memcpy(value, default_value, value_len);
    value[value_len] = '\0';

    fd = open(PROPERTY_FILE, O_RDONLY | O_CREAT , 0777);
    if (fd < 0)
    {
        LOG_ERROR(LOG_TAG, "%s open property file failed %d", __func__, fd);
        return value_len;
    }

    len = lseek(fd, 0, SEEK_END);
    rbuf = malloc(len + 1);
    if (NULL == rbuf)
    {
        LOG_ERROR(LOG_TAG, "%s malloc buf failed", __func__);
        close(fd);
        return value_len;
    }
    memset(rbuf, 0, len + 1);
    lseek(fd, 0, SEEK_SET);
    read(fd, rbuf, len);

    p_key = malloc(strlen(key) + 3);
    if (NULL == p_key)
    {
        LOG_ERROR(LOG_TAG, "%s malloc p_key failed", __func__);
        free(rbuf);
        close(fd);
        return value_len;
    }
    memset(p_key, 0, strlen(key) + 3);
    sprintf(p_key, "<%s:", key);

    location = strstr(rbuf, p_key);
    if (NULL != location)
    {
        for (value_len = 0; location[value_len + strlen(p_key)] != '>'; value_len++)
        {
            if ((0 == h_flag)
                && (location[value_len + strlen(p_key)] != ':')
                && (location[value_len + strlen(p_key)] != ' '))
            {
                h_flag = 1;
            }

            if (1 == h_flag)
            {
                *p_value = location[value_len + strlen(p_key)];
                p_value++;
            }
        }
        *p_value = '\0';
    }

    free(p_key);
    free(rbuf);
    close(fd);
    return value_len;
#else
    /* For linux right now just return default value, if present */
    int len = -1;
    if (!default_value)
        return len;

    len = strlen(default_value);
    if (len >= PROPERTY_VALUE_MAX)
        len = PROPERTY_VALUE_MAX - 1;

    memcpy(value, default_value, len);
    value[len] = '\0';
    return len;
#endif
#else
    return property_get(key, value, default_value);
#endif  // defined(OS_GENERIC)
}

int osi_property_set(const char *key, const char *value) {
#if defined(OS_GENERIC)
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    int fd = 0, i = 0, len = 0;
    char *rbuf = NULL;
    char *wbuf = NULL;
    char *p_wbuf = NULL;
    char *p_key = NULL;
    char *location = NULL;

    fd = open(PROPERTY_FILE, O_RDWR | O_CREAT, 0777);
    if (fd < 0)
    {
        LOG_ERROR(LOG_TAG, "%s open property file failed %d", __func__, fd);
        return -1;
    }

    len = lseek(fd, 0, SEEK_END);
    rbuf = malloc(len + 1);
    if (NULL == rbuf)
    {
        LOG_ERROR(LOG_TAG, "%s malloc rbuf failed", __func__);
        close(fd);
        return -1;
    }
    memset(rbuf, 0, len + 1);

    wbuf = malloc(len + strlen(key) + strlen(value) + 16);
    if (NULL == wbuf)
    {
        LOG_ERROR(LOG_TAG, "%s malloc wbuf failed", __func__);
        free(rbuf);
        close(fd);
        return -1;
    }
    memset(wbuf, 0, len + strlen(key) + strlen(value) + 16);
    p_wbuf = wbuf;

    lseek(fd, 0, SEEK_SET);
    read(fd, rbuf, len);

    p_key = malloc(strlen(key) + 3);
    if (NULL == p_key)
    {
        LOG_ERROR(LOG_TAG, "%s malloc p_key failed", __func__);
        free(rbuf);
        free(wbuf);
        close(fd);
        return -1;
    }
    memset(p_key, 0, strlen(key) + 3);
    sprintf(p_key, "<%s:", key);

    location = strstr(rbuf, p_key);
    if (NULL == location)
    {
        memcpy(wbuf, rbuf, len);
        p_wbuf += len;
        p_wbuf += sprintf(p_wbuf, "<%s: %s>\n", key, value);
    }
    else
    {
        memcpy(wbuf, rbuf, location - rbuf);
        p_wbuf += (location - rbuf);
        p_wbuf += sprintf(p_wbuf, "<%s: %s>", key, value);
        while (*location != '>')
        {
            location++;
        }
        location++;
        p_wbuf += sprintf(p_wbuf, "%s", location);
    }

    p_wbuf = '\0';

    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    write(fd, wbuf, strlen(wbuf));

    free(p_key);
    free(rbuf);
    free(wbuf);
    close(fd);
    return 0;
#else
    return -1;
#endif
#else
    return property_set(key, value);
#endif  // defined(OS_GENERIC)
}
