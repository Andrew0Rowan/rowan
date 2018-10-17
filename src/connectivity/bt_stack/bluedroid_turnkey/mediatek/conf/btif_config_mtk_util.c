/******************************************************************************
 *
 *  Copyright (C) 2010 MediaTek Inc.
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

/************************************************************************************
 *
 *  Filename:      btif_config_mtk_util.c
 *
 *  Description:   Reads the local BT adapter and remote device properties from
 *                 Blueangel database and stores into NVRAM storage, typically as
 *                 conf file in mobile's filesystem
 *
 ***********************************************************************************/
#define LOG_TAG "btif_config_mtk_util"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <hardware/bluetooth.h>

#include "osi/include/allocator.h"
#include "osi/include/config.h"
#include "osi/include/log.h"

#include "btif_config_mtk_util.h"

/*******************************************************************************
 ** Constants & Macros
 ********************************************************************************/
#ifndef FALSE
#define TRUE 1
#define FALSE 0
#endif

#define BLUEANGEL_PATH "/data/@btmtk/"
#define BLUEANGEL_PATH_BAK "/data/@btmtk_bak"
#define BLUEANGEL_DEV_CACHE_PATH "/data/@btmtk/dev_cache"  /* paired device */
#define BLUEANGEL_HOST_CACHE_PATH  "/data/@btmtk/host_cache"  /* host setting, ex. name, mode */
#define HID_REMOTE_INFO_DATABASE_FOLDER "/data/@btmtk/HidRemoteDb/" /* BREDR peer HID db*/
#define BLE_REMOTE_SERVICE_DATABASE_FOLDER "/data/@btmtk/bleRemoteDb/" /* HOGP db*/
#define BLUEANGEL_DEV_DB_PATH "/data/@btmtk/devdb" /*ssp and smp info*/

#define BTBM_ADP_MAX_NAME_LEN 249  /* 248 byte + 1byte end*/
#define BTBM_ADP_MAX_SDAP_UUID_NO 30
#define BTBM_ADP_MAX_SDAP_APP_UUID_NO 5
#define BTBM_ADP_MAX_PAIRED_LIST_NO 40
#define HID_MAX_DESC_SIZE 884
#define BD_ADDR_SIZE 6
#define LINK_KEY_SIZE 16
#define DDB_MAX_ENTRIES  40

#define HID_APP_ID_MOUSE      1
#define HID_APP_ID_KYB      2

/* Group: Minor Device Class - Peripheral Major class */
#define COD_MINOR_PERIPH_KEYBOARD     0x40
#define COD_MINOR_PERIPH_POINTING     0x80
#define COD_MINOR_PERIPH_COMBOKEY     0x000000C0

/* Group: Minor Device Class - ORed with Peripheral Minor Device class */
#define COD_MINOR_PERIPH_UNCLASSIFIED 0x00000000
#define COD_MINOR_PERIPH_JOYSTICK     0x00000004
#define COD_MINOR_PERIPH_GAMEPAD      0x00000008
#define COD_MINOR_PERIPH_REMOTECTRL   0x0000000C
#define COD_MINOR_PERIPH_SENSING      0x00000010
#define COD_MINOR_PERIPH_DIGITIZER    0x00000014
#define COD_MINOR_PERIPH_CARD_RDR     0x00000018
#define COD_MINOR_PERIPH_DIGITAL_PAN  0x0000001C
#define COD_MINOR_PERIPH_HAND_SCANNER 0x00000020
#define COD_MINOR_PERIPH_HAND_GESTURAL_INPUT 0x00000024

#define BT_LINK_KEY_GAP_TYPE_NO_KEY             0x00
#define BT_LINK_KEY_GAP_TYPE_COMBINATION_NONE16 0x01
#define BT_LINK_KEY_GAP_TYPE_COMBINATION_16     0x02
#define BT_LINK_KEY_GAP_TYPE_UNAUTHENTICATED    0x03
#define BT_LINK_KEY_GAP_TYPE_AUTHENTICATED      0x04

#define SMP_SEC_NONE                 0
#define SMP_SEC_UNAUTHENTICATE      (1 << 0)
#define SMP_SEC_AUTHENTICATED       (1 << 2)

#define BT_DEVICE_TYPE_BREDR   0x01
#define BT_DEVICE_TYPE_BLE     0x02
#define BT_DEVICE_TYPE_DUMO    0x03

#define BTA_HH_NV_LOAD_MAX 16

typedef enum
{
    BTMTK_BOND_STATE_UNBOND,
    BTMTK_BOND_STATE_BONDED,
    BTMTK_BOND_STATE_BONDING,
} btmtk_bond_state;

typedef enum
{
    BTBM_DEVICE_TYPE_LE,
    BTBM_DEVICE_TYPE_BR_EDR,
    BTBM_DEVICE_TYPE_BR_EDR_LE
} btmtk_device_type;

typedef enum
{
    SM_KEY_LTK  = 0x01,
    SM_KEY_EDIV = 0x02,
    SM_KEY_RAND = 0x04,
    SM_KEY_IRK  = 0x08,
    SM_KEY_ADDR = 0x10,
    SM_KEY_CSRK = 0x20
} SmKeyType;

typedef enum
{
    BT_DEV_TYPE_UNKNOWN,
    BT_DEV_TYPE_LE,
    BT_DEV_TYPE_BR_EDR,
    BT_DEV_TYPE_BR_EDR_LE,
    NUM_OF_BT_DEV_TYPE,
} BtDevType;

/************************************************************************************
 **  Local type definitions
 ************************************************************************************/
typedef char bdstr_t[18];

typedef struct config_t config_t;

typedef struct
{
    uint32_t lap;
    uint8_t uap;
    uint16_t nap;
} bt_addr_struct;

typedef struct
{
    unsigned char scan_mode;
    char name[BTBM_ADP_MAX_NAME_LEN];
    unsigned int scan_mode_timeout;
    bt_addr_struct local_addr;
} btmtk_host_cache_struct;

typedef struct
{
    bt_addr_struct addr;
    char name[BTBM_ADP_MAX_NAME_LEN];
    char nickname[BTBM_ADP_MAX_NAME_LEN];
    unsigned int cod;
    struct
    {
        unsigned char uuid_no;
        unsigned short uuid[BTBM_ADP_MAX_SDAP_UUID_NO];
        unsigned short channel[BTBM_ADP_MAX_SDAP_UUID_NO];  /* map to uuid */

        unsigned char app_uuid_no;
        unsigned char app_uuid[BTBM_ADP_MAX_SDAP_APP_UUID_NO][16];
        unsigned short app_channel[BTBM_ADP_MAX_SDAP_APP_UUID_NO];  /* map to uuid */
    } sdp;
    btmtk_bond_state paired;
    uint8_t connected;
    uint8_t trusted;
    uint8_t legacy_pairing;
    short rssi;
    btmtk_device_type device_type;
} btmtk_device_entry_struct;

#pragma pack(1)
/*
   typedef struct
   {
   uint16_t vendor_id;
   uint16_t product_id;
   uint16_t version;
   unsigned char countrycode;
   uint16_t desc_len;
   char desc_list[HID_MAX_DESC_SIZE];
   }HidRemoteInfoType;*/
typedef struct
{
    uint16_t     queryFlags;
    /* Defines which query field contains
     * valid data.
     */
    uint16_t               deviceRelease;
    /* Vendor specified device release
     * version.
     */
    uint16_t               parserVersion;
    /* HID parser version for which this
     * device is designed.
     */
    uint8_t                deviceSubclass;
    /* Device subclass (minor Class of
     * Device).
     */
    uint8_t                countrycode;
    /*countryCode;  */
    uint8_t                virtualCable;
    /* Virtual Cable relationship is
     * supported.
     */
    uint8_t                reconnect;
    /* Device initiates reconnect.
    */
    uint8_t                sdpDisable;
    /* When TRUE, the device cannot accept
     * an SDP query when the control/interrupt
     * channels are connected.
     */
    uint8_t                batteryPower;
    /* The device runs on battery power.
    */
    uint8_t                padd;

    uint8_t                remoteWakeup;
    /* The device can be awakened remotely.
    */
    uint16_t               profileVersion;
    /* Version of the HID profile.
    */
    uint16_t               supervTimeout;
    /* Suggested supervision timeout value
     * for LMP connections.
     */
    uint8_t                normConnectable;
    /* The device is connectable when no
     * connection exists.
     */
    uint8_t                bootDevice;
    /* Boot protocol support is provided.
    */
    uint32_t               desc_len;
    /*descriptorLen; */
    uint8_t               *descriptorList;
    /* A list of HID descriptors (report or
     * physical) associated with the device.
     * Each element of the list is an SDP
     * data element sequence, and therefore
     * has a header of two bytes (0x35, len)
     * which precedes each descriptor.
     */
    uint32_t               descriptorLenOffset;
    /* Offset length  of the HID descriptor list*/
    uint16_t               vendor_id;
    /*vendorID*/
    uint16_t               product_id;
    /*productID*/
    char                desc_list[];
} HidRemoteInfoType;
typedef struct
{
    uint16_t handle;
    uint8_t desc_uuid_size;
    uint8_t desc_uuid[2];
    uint8_t desc_size;
    uint8_t desc_value[2];
}descriptor;

typedef  struct
{
    uint16_t char_size;
    uint16_t handle;
    uint8_t valid;
    uint8_t properties;
    uint16_t value_handle;
    uint8_t value_uuid_size;
    uint8_t value_uuid[2];
    uint8_t padding;
    uint16_t value_size;
    descriptor desc;
}characteristic;

typedef struct
{
    uint16_t service_size;
    uint16_t start_handle;
    uint16_t end_handle;
    uint8_t uuid_size;
    uint8_t uuid[2];

    uint16_t include_size;
    characteristic charc;
}ble_remote_gattservice;

typedef struct
{
    uint8_t rpt_uuid[2];
    uint8_t rpt_value[2];
    uint8_t inst_id;
    uint8_t prop;
}hid_report_t;

typedef struct
{
    char bdAddr[BD_ADDR_SIZE];
    uint8_t trusted;
    uint8_t linkKey[LINK_KEY_SIZE];  /* BT4.0 as LTK */
    uint8_t gampKey[32];
    uint8_t dampKey[32];
    uint8_t linkkeyType;

    uint8_t gapKeyType;
    uint8_t  threebyte_padding[3];
    BtDevType devType;
    uint32_t addrType; // LE_ADDR_PUBLIC=0x00, LE_ADDR_RANDOM=0x01

    /* slave key storage when local as master */
    SmKeyType smKeyType;
    uint16_t ediv;
    uint8_t sixbyte_pading[6];
    uint64_t rand;
    uint8_t csrk[16];
    uint8_t irk[16];
    uint8_t privacy_supported;
    uint16_t twobyte_padd;
    uint8_t  onebyte_pad;
    /* key database when local as slave */
    SmKeyType distSmKeyType;
    uint16_t distEDIV;
    uint8_t sixbyte_pad[6];
    uint64_t distRAND;
    uint8_t distLTK[16];
} BtDeviceRecord;

typedef struct {
    uint16_t numDdbRecs;
    uint16_t recordSize;
    uint32_t padding;
    BtDeviceRecord devDb[DDB_MAX_ENTRIES];
} bt_ddb_data;

typedef struct
{
    uint8_t ltk[16];
    uint64_t rand;
    uint16_t ediv;
    uint8_t sec_level;
    uint8_t key_size;
}penc_keys_t;

typedef struct
{
    uint8_t irk[16];
    uint8_t addr_type;
    char bdAddr[BD_ADDR_SIZE];
}pid_keys_t;

/******************************************************************************
 **  Static functions
 ******************************************************************************/
static char* convert_bdaddr2string(const bt_addr_struct *bdaddr, bdstr_t *bdstr);
static void uuid16_to_uuid128(uint16_t uuid16, bt_uuid_t* uuid128);
static void uuid_to_string(bt_uuid_t *p_uuid, char *str);
static void convert_bdaddr2filename (const bt_addr_struct *bdaddr, bdstr_t *bdaddr_str );
static int load_blueangel_host_info(config_t *config);
static int load_blueangel_dev_info(config_t *config);
static void load_blueangel_gatt_info(config_t *config, const bt_addr_struct *bdaddr, bdstr_t bdstr);
static void load_blueangel_hid_info (config_t *config, const bt_addr_struct *bdaddr, bdstr_t bdstr);
static void load_blueangel_devdb_info(config_t *config);
static void load_blueangel_cod_info (config_t *config, unsigned int cod, bdstr_t bdstr);
static bool btmtk_config_set_bin(config_t *config, const char *section, const char *key, const uint8_t *value, size_t length);

/********************************************************************************
 **
 ** Function         open_file_map
 **
 ** Description      Opens the required file and maps files into memory.
 **
 ** Returns          void
 **
 ********************************************************************************/
static int open_file_map(const char *pathname, const char**map, int* size)
{
    struct stat st;
    st.st_size = 0;
    int fd;
    if((fd = open(pathname, O_RDONLY)) >= 0)
    {
        if(fstat(fd, &st) == 0 && st.st_size)
        {
            *size = st.st_size;
            *map = (const char*)mmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);
            if(*map && *map != MAP_FAILED)
            {
                return fd;
            }
        }
        close(fd);
    }
    return -1;
}

/********************************************************************************
 **
 ** Function         close_file_map
 **
 ** Description      Unmap files into memory.
 **
 ** Returns          void
 **
 ********************************************************************************/
static void close_file_map(int fd, const char* map, int size)
{
    munmap((void*)map, size);
    close(fd);
}

/********************************************************************************
 **
 ** Function         load_blueangel_dev_info
 **
 ** Description      Load the paired device settings like Remote device BD Address,
 **                  remote Name,device type,COD,.... from the path
 **                  /data/@btmtk/dev_cache and write into bt_config.conf.
 **
 ** Returns          void
 **
 ********************************************************************************/
static int load_blueangel_dev_info(config_t *config)
{
    char path[25];
    int size = 0;
    const char* map = NULL;
    int num_paired_dev = 0;
    snprintf(path, sizeof(path), "%s", BLUEANGEL_DEV_CACHE_PATH);
    int fd = open_file_map(path, &map, &size);
    if(fd < 0 || size == 0)
    {
        LOG_ERROR(LOG_TAG, "open_file_map fail, fd:%d, path:%s, size:%d", fd, path, size);
        if (fd >= 0)
            close(fd);
        return FALSE;
    }
    btmtk_device_entry_struct *paired_dev_cache[BTBM_ADP_MAX_PAIRED_LIST_NO];
    for (int i = 0; i < BTBM_ADP_MAX_PAIRED_LIST_NO; i++)
    {
        paired_dev_cache[i] = (btmtk_device_entry_struct *)map;
        if (paired_dev_cache[i]->addr.lap == 0 && paired_dev_cache[i]->addr.uap == 0 &&
                paired_dev_cache[i]->addr.nap == 0)
        {
            break;
        }
        map = map + sizeof(btmtk_device_entry_struct);
        num_paired_dev++;
    }
    bdstr_t bdstr;
    for (int i =0; i < num_paired_dev; i++)
    {
        convert_bdaddr2string(&(paired_dev_cache[i]->addr),&bdstr);
        config_set_string(config, bdstr, "Name", paired_dev_cache[i]->name);
        config_set_string(config, bdstr, "Aliase", paired_dev_cache[i]->nickname);
        config_set_int(config, bdstr, "DevClass", paired_dev_cache[i]->cod);
        char sdp_uuid[1200] = {0};
        memset(sdp_uuid, 0, sizeof(sdp_uuid));
        for (int j =0; j< paired_dev_cache[i]->sdp.uuid_no; j++)
        {
            bt_uuid_t remote_uuid;
            char buf[64] = {0};
            uuid16_to_uuid128(paired_dev_cache[i]->sdp.uuid[j], &remote_uuid);
            memset(buf, 0, sizeof(buf));
            uuid_to_string(&remote_uuid, buf);
            //debug("[CN]uuid.no: %d, buf: %s", j, buf);
            sprintf(&sdp_uuid[j * 37], "%s ", buf);
        }
        config_set_string(config, bdstr, "Service", sdp_uuid);
        if (paired_dev_cache[i]->device_type == BTBM_DEVICE_TYPE_LE)
        {
            config_set_int(config, bdstr, "DevType", BT_DEVICE_TYPE_BLE);
            load_blueangel_cod_info (config, paired_dev_cache[i]->cod, bdstr);
            /* Bluedroid requires HidAttrMask node to load the HidDescriptor but in the Sakura blueangel struct
             ** there is no mechanism to save the remote HidAttrMask in db. Hence adding the attr mask to 0 since
             ** bluedroid also set the remote HidAttrMask to 0 for LE devices.
             */
            config_set_int(config, bdstr, "HidAttrMask", 0);
            load_blueangel_gatt_info(config, &(paired_dev_cache[i]->addr), bdstr);
        }
        else if (paired_dev_cache[i]->device_type == BTBM_DEVICE_TYPE_BR_EDR)
        {
            config_set_int(config, bdstr, "DevType", BT_DEVICE_TYPE_BREDR);
            if (((paired_dev_cache[i]->cod & 0x1f00) >> 8 == 0x5)) //MajorDeviceClass:Peripheral(mouse,keyboards,.)
            {
                load_blueangel_cod_info (config, paired_dev_cache[i]->cod, bdstr);
                load_blueangel_hid_info (config, &(paired_dev_cache[i]->addr), bdstr);
            }

        }
        else if (paired_dev_cache[i]->device_type == BTBM_DEVICE_TYPE_BR_EDR_LE)
        {
            config_set_int(config, bdstr, "DevType", BT_DEVICE_TYPE_DUMO);
            /* To do*/
        }
    }
    load_blueangel_devdb_info(config);
    close_file_map(fd, map, size);
    return TRUE;
}

/********************************************************************************
 **
 ** Function         load_blueangel_devdb_info
 **
 ** Description      Load the peer ssp and smp settings like linkkey/LTK,
 Linkey Type, Address Type,Security level,CSRK,IRK,...from
 **                  the path /data/@btmtk/devdb and write into bt_config.conf.
 **
 ** Returns          void
 **
 ********************************************************************************/
static void load_blueangel_devdb_info(config_t *config)
{
    char path[22];
    int size = 0;
    const char* map = NULL;
    snprintf(path, sizeof(path), "%s", BLUEANGEL_DEV_DB_PATH);

    int fd = open_file_map(path, &map, &size);
    if(fd < 0 || size == 0)
    {
        LOG_ERROR(LOG_TAG, "open_file_map fail, fd:%d, path:%s, size:%d", fd, path, size);
        if (fd >= 0)
            close(fd);
        return;
    }

    bt_ddb_data *ddb = (bt_ddb_data*)map;
    BtDeviceRecord *record;
    uint16_t count = ddb->numDdbRecs;
    record = &ddb->devDb[0];
    while(count > 0)
    {
        char bdstr[18];
        uint8_t link_key[LINK_KEY_SIZE];
        sprintf(bdstr, "%02x:%02x:%02x:%02x:%02x:%02x",
                record->bdAddr[5],
                record->bdAddr[4],
                record->bdAddr[3],
                record->bdAddr[2],
                record->bdAddr[1],
                record->bdAddr[0]);
        config_set_int(config, bdstr, "AddrType", record->addrType);
        if (record->devType == BT_DEV_TYPE_BR_EDR)
        {
            config_set_int(config, bdstr, "LinkKeyType", record->linkkeyType);
            /*reverse the linkkey*/
            for (int i = 0; i < LINK_KEY_SIZE; i++)
            {
                link_key[LINK_KEY_SIZE -1 -i] = record->linkKey[i];
            }
            btmtk_config_set_bin(config, bdstr, "LinkKey", (const uint8_t*)link_key,
                    sizeof(link_key));
        }
        else if (record->devType == BT_DEV_TYPE_LE)
        {
            penc_keys_t* penc = (penc_keys_t*) malloc(sizeof(penc_keys_t));
            if (penc != NULL)
            {
                memset(penc, 0, sizeof(penc_keys_t));
                memcpy(penc->ltk, record->linkKey, sizeof(record->linkKey));
                penc->rand = record->rand;
                penc->ediv = record->ediv;
                penc->sec_level = record->gapKeyType;
                switch (record->gapKeyType)
                {
                    case BT_LINK_KEY_GAP_TYPE_UNAUTHENTICATED:
                        penc->sec_level = SMP_SEC_UNAUTHENTICATE;
                        break;
                    case BT_LINK_KEY_GAP_TYPE_AUTHENTICATED:
                        penc->sec_level = SMP_SEC_AUTHENTICATED;
                        break;
                    case BT_LINK_KEY_GAP_TYPE_NO_KEY:
                        penc->sec_level = SMP_SEC_NONE;
                        break;
                    default:
                        break;
                }
                btmtk_config_set_bin(config, bdstr, "LE_KEY_PENC", (const uint8_t*)penc, sizeof(penc_keys_t));
                btmtk_config_set_bin(config, bdstr, "LE_KEY_LCSRK", (const uint8_t*)record->csrk,
                        sizeof(record->csrk));
                free(penc);
                penc = NULL;
                if (record->addrType == 0x000001) //check for random addr
                {
                    pid_keys_t* pid = (pid_keys_t* )malloc(sizeof(pid_keys_t));
                    if (pid == NULL) return;
                    memset(pid, 0, sizeof(pid_keys_t));
                    memcpy(pid->irk, record->irk, sizeof(record->irk));
                    memcpy(pid->bdAddr, record->bdAddr, 6);
                    pid->addr_type = 0x01; //random addr*/
                    /* Reverse the BD Address */
                    for (int i = 0; i < BD_ADDR_SIZE; i++)
                    {
                        pid->bdAddr[BD_ADDR_SIZE -1 -i] = record->bdAddr[i];
                    }
                    btmtk_config_set_bin(config, bdstr, "LE_KEY_PID", (const uint8_t*)pid, sizeof(pid_keys_t));
                    free(pid);
                    pid = NULL;
                }
            }
        }
        count--;
        record++;
    }
    close_file_map(fd, map, size);
}

/********************************************************************************
 **
 ** Function         load_blueangel_gatt_info
 **
 ** Description      Load the Remote HOGP records like HID Report,Descriptor from
 **                  the path /data/@btmtk/bleRemoteDb/<remote BD Addr> and write
 **                  into bt_config.conf.
 ** Returns          void
 **
 ********************************************************************************/
static void load_blueangel_gatt_info(config_t *config, const bt_addr_struct *bdaddr, bdstr_t bdstr)
{
    bdstr_t bdaddr_str;
    struct dirent *dptr;
    DIR *dirp;
    convert_bdaddr2filename (bdaddr, &bdaddr_str);
    if((dirp = opendir(BLE_REMOTE_SERVICE_DATABASE_FOLDER)) != NULL)
    {
        while((dptr = readdir(dirp)) != NULL)
        {
            if(strcmp(dptr->d_name, bdaddr_str) == 0)
            {
                char path[45];
                int size = 0;
                const char* map = NULL;
                const char* pos_cur = NULL;
                const char* pos_file_end = NULL;
                snprintf (path, sizeof(path), "%s%s", BLE_REMOTE_SERVICE_DATABASE_FOLDER, dptr->d_name);
                int fd = open_file_map(path, &map, &size);
                if(fd < 0 || size == 0)
                {
                    LOG_ERROR(LOG_TAG, "open_file_map fail, fd:%d, path:%s, size:%d", fd, path, size);
                    if (fd >= 0)
                        close(fd);
                    return;
                }
                pos_cur = map;
                pos_file_end = map + size;
                char* buf = (char*) malloc(BTA_HH_NV_LOAD_MAX * sizeof(hid_report_t));
                if(!buf)return;
                uint8_t srvc_inst_id = 0x00;
                uint8_t inst_id = 0x00;
                uint8_t rpt_count = 0;

                while (pos_cur < pos_file_end)
                {
                    ble_remote_gattservice* gattservice = (ble_remote_gattservice*)map;
                    uint8_t rpt_inst_id = 0x00;
                    uint16_t service_size = gattservice->service_size;

                    /* Decrement service_size to the first service char_size pointer
                     ** i.e. (sizeof(start_handle)+sizeof(end_handle)+sizeof(uuid_size)+uuid_size
                     ** +sizeof(include_size)+include_size)
                     */
                    uint16_t* include_size_p = &gattservice->include_size;
                    if(gattservice->uuid_size != 2)
                        include_size_p = (uint16_t*)(map+ 7 + gattservice->uuid_size);
                    service_size -= ((5 + gattservice->uuid_size) + (2 + *include_size_p));

                    /* Move the map pointer till the first char_size
                     ** i.e.(sizeof(service_size)+sizeof(start_handle)+sizeof(end_handle)+
                     ** sizeof(uuid_size)+uuid_size+sizeof(include_size)+include_size)
                     */
                    map = map + (7 + gattservice->uuid_size) + (2 + *include_size_p);
                    while (service_size > 0)
                    {
                        characteristic* charc = (characteristic*)map;
                        uint16_t char_size = charc->char_size;
                        /* Move the pointer till the charc value pointer
                         ** i.e. (sizeof(char_size)+sizeof(handle)+sizeof(valid)+sizeof(properties)
                         ** +sizeof(value_handle)+sizeof(value_uuid_size)+value_uuid_size+
                         ** sizeof(padding)+sizeof(value_size)+1)
                         */
                        uint16_t* value_size_p = &charc->value_size;
                        if(charc->value_uuid_size != 2)
                            value_size_p = (uint16_t*)(map+10+charc->value_uuid_size);
                        map = map +12+charc->value_uuid_size;

                        /* Check for the Characteristic UUID: HID Report MAP = 0x2a4b*/
                        if (((charc->value_uuid[0] == 0x4b)&& (charc->value_uuid[1] == 0x2a))
                                && *value_size_p > 0)
                        {
                            const char *value_str = config_get_string(config, bdstr, "HidDescriptor", NULL);
                            if (!value_str)
                                btmtk_config_set_bin(config, bdstr, "HidDescriptor", (const uint8_t*)map,
                                        *value_size_p);
                        }
                        map = map + *value_size_p;
                        /*Decrement to char_size+sizeof(char_size)*/
                        service_size -= (char_size + 2);
                        /*Move to the descriptor handle*/
                        char_size -=
                            (10 + charc->value_uuid_size + *value_size_p);
                        while (char_size > 0)
                        {
                            descriptor* remote_desc = (descriptor*)map;
                            uint8_t* desc_size_p = &remote_desc->desc_size;
                            if(remote_desc->desc_uuid_size !=2)
                                desc_size_p = (uint8_t*)(map+3+remote_desc->desc_uuid_size);
                            char_size -=
                                (4 + remote_desc->desc_uuid_size
                                 + *desc_size_p);
                            map = map + (4 + remote_desc->desc_uuid_size
                                 + *desc_size_p);
                            /*Check for Report Reference descriptor UUID = 0x2908 */
                            if (((remote_desc->desc_uuid[0] == 0x08) && (remote_desc->desc_uuid[1] == 0x29)) &&
                                    (*desc_size_p > 0))
                            {
                                hid_report_t* hid_rpt = (hid_report_t*) malloc(sizeof(hid_report_t));
                                if(!hid_rpt)return;
                                inst_id = ((srvc_inst_id << 4) | rpt_inst_id);
                                memcpy(hid_rpt->rpt_uuid, charc->value_uuid, charc->value_uuid_size);
                                memcpy(hid_rpt->rpt_value, remote_desc->desc_value, 2);
                                hid_rpt->inst_id = inst_id;
                                hid_rpt->prop = charc->properties;

                                rpt_inst_id++;
                                memcpy (buf, hid_rpt, sizeof(hid_report_t));
                                buf = buf + sizeof(hid_report_t);
                                free(hid_rpt);
                                hid_rpt = NULL;
                                rpt_count++;
                                map += char_size;
                                break;
                            }
                        }
                    }
                    pos_cur = map;
                    srvc_inst_id++;
                }
                buf = buf - (rpt_count * sizeof(hid_report_t));
                btmtk_config_set_bin(config, bdstr, "HidReport", (const uint8_t*)buf,
                        (rpt_count * sizeof(hid_report_t)));
                free(buf);
                buf = NULL;
                close_file_map(fd, map, size);
                break;
            }
        }
        closedir(dirp);
    }
    return;
}

/********************************************************************************
 **
 ** Function         load_blueangel_hid_info
 **
 ** Description      Load the Remote HID records like HID Report,Descriptor,..
 **                  from the path /data/@btmtk/HidRemoteDb/<remote BD ADDR> and
 **                  write into bt_config.conf.
 **
 ** Returns          void
 **
 ********************************************************************************/
static void load_blueangel_hid_info (config_t *config, const bt_addr_struct *bdaddr, bdstr_t bdstr)
{
    bdstr_t bdaddr_str;
    struct dirent *dptr;
    DIR *dirp;

    convert_bdaddr2filename (bdaddr, &bdaddr_str);
    if((dirp = opendir(HID_REMOTE_INFO_DATABASE_FOLDER)) != NULL)
    {
        while((dptr = readdir(dirp)) != NULL)
        {
            if(strcmp(dptr->d_name, bdaddr_str) == 0)
            {
                char path[45];
                int size = 0;
                const char* map = NULL;
                snprintf(path, sizeof(path), "%s%s", HID_REMOTE_INFO_DATABASE_FOLDER, dptr->d_name);
                int fd = open_file_map(path, &map, &size);
                if(fd < 0 || size == 0)
                {
                    LOG_ERROR(LOG_TAG, "open_file_map fail, fd:%d, path:%s, size:%d", fd, path, size);
                    if (fd >= 0)
                        close(fd);
                    return;
                }
                HidRemoteInfoType* HidRemoteInfo = (HidRemoteInfoType*)map;
                config_set_int(config, bdstr, "HidVendorId", HidRemoteInfo->vendor_id);
                config_set_int(config, bdstr, "HidProductId", HidRemoteInfo->product_id);
                config_set_int(config, bdstr, "HidCountryCode", HidRemoteInfo->countrycode);
                if(HidRemoteInfo->desc_len > 0)
                    btmtk_config_set_bin(config, bdstr, "HidDescriptor", (const uint8_t*)HidRemoteInfo->desc_list,
                            HidRemoteInfo->desc_len);
                close_file_map(fd, map, size);
                break;
            }
        }
        closedir(dirp);
    }
}

/********************************************************************************
 **
 ** Function         load_blueangel_cod_info
 **
 ** Description      Set the Hid Sub calss and App Id.
 **
 ** Returns          void
 **
 ********************************************************************************/
static void load_blueangel_cod_info (config_t *config, unsigned int cod, bdstr_t bdstr)
{
    uint8_t dev_sub_class = (cod & 0x00f0);
    uint8_t app_id = 0xff; //Bluedroid sets app_id to BTA_ALL_APP_ID for LE devices.
    config_set_int(config, bdstr, "HidSubClass", dev_sub_class);
    if(dev_sub_class)
    {
        if ((dev_sub_class & COD_MINOR_PERIPH_POINTING) == COD_MINOR_PERIPH_POINTING)
            app_id = HID_APP_ID_MOUSE;
        else
            app_id = HID_APP_ID_KYB;
    }
    config_set_int(config, bdstr, "HidAppId", app_id);
}

/*******************************************************************************
 **
 ** Function         load_blueangel_host_info
 **
 ** Description      Load the host settings like BD Address, Name, Scantimeout,..
 **                  from the path /data/@btmtk/host_cache and write into
 **                  bt_config.conf.
 **
 ** Returns          TRUE/FALSE
 **
 *******************************************************************************/

static int load_blueangel_host_info(config_t *config)
{
    DIR *dirp;
    int ret = FALSE;
    if((dirp = opendir(BLUEANGEL_PATH)) != NULL)
    {
        char path[25];
        int size = 0;
        const char* map = NULL;
        snprintf(path, sizeof(path), "%s", BLUEANGEL_HOST_CACHE_PATH);
        int fd = open_file_map(path, &map, &size);
        if(fd < 0 || size == 0)
        {
            LOG_ERROR(LOG_TAG, "open_file_map fail, fd:%d, path:%s, size:%d", fd, path, size);
            if (fd >= 0)
                close(fd);
            return FALSE;
        }
        btmtk_host_cache_struct *host_cache = (btmtk_host_cache_struct*)map;
        /* Convert the BD ADDR into string */
        bdstr_t bdstr;
        convert_bdaddr2string(&(host_cache->local_addr),&bdstr);

        config_set_string(config, "Adapter", "Address", bdstr);
        config_set_int(config, "Adapter", "ScanMode", host_cache->scan_mode);
        config_set_int(config, "Adapter", "DiscoveryTimeout", host_cache->scan_mode_timeout);
        config_set_string(config, "Adapter", "Name", host_cache->name);

        //To Do Blueangel supported LE features
        close_file_map(fd, map, size);
        closedir(dirp);
        ret = TRUE;
    }
    return ret;
}

config_t *load_blueangel_cfg()
{
    config_t *config = config_new_empty();

    if (!config)
    {
        LOG_ERROR(LOG_TAG, "%s unable to allocate config object.", __func__);
        return NULL;
    }

    if (load_blueangel_host_info(config))
    {
        if (FALSE == load_blueangel_dev_info(config))
            LOG_WARN(LOG_TAG, "%s: load_blueangel_dev_info fail.", __func__);

        return config;
    }

    return NULL;
}

/*******************************************************************************
 **
 ** Function         uuid_to_string
 **
 ** Description      Convert UUID bytes into string format.
 **                  Example: 00001812-0000-1000-8000-00805f9b34fb
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uuid_to_string(bt_uuid_t *p_uuid, char *str)
{
    uint32_t uuid0, uuid4;
    uint16_t uuid1, uuid2, uuid3, uuid5;

    memcpy(&uuid0, &(p_uuid->uu[0]), 4);
    memcpy(&uuid1, &(p_uuid->uu[4]), 2);
    memcpy(&uuid2, &(p_uuid->uu[6]), 2);
    memcpy(&uuid3, &(p_uuid->uu[8]), 2);
    memcpy(&uuid4, &(p_uuid->uu[10]), 4);
    memcpy(&uuid5, &(p_uuid->uu[14]), 2);

    sprintf((char *)str, "%.8x-%.4x-%.4x-%.4x-%.8x%.4x",
            ntohl(uuid0), ntohs(uuid1),
            ntohs(uuid2), ntohs(uuid3),
            ntohl(uuid4), ntohs(uuid5));
    return;
}

/*******************************************************************************
 **
 ** Function         convert_bdaddr2filename
 **
 ** Description      Convert BD Address into filename.
 **                  Example: 0123456789ab >> 01_23_45_67_89_ab
 **
 ** Returns          void
 **
 *******************************************************************************/
static void convert_bdaddr2filename (const bt_addr_struct *bdaddr, bdstr_t *bdaddr_str )
{
    uint8_t addr[6];
    addr[2] = (uint8_t) ((bdaddr->lap & 0x00FF0000) >> 16);
    addr[1] = (uint8_t) ((bdaddr->lap & 0x0000FF00) >> 8);
    addr[0] = (uint8_t) (bdaddr->lap & 0x000000FF);
    addr[3] = (uint8_t) bdaddr->uap;
    addr[5] = (uint8_t) ((bdaddr->nap & 0xFF00) >> 8);
    addr[4] = (uint8_t) (bdaddr->nap & 0x00FF);
    sprintf(*bdaddr_str, "%02X_%02X_%02X_%02X_%02X_%02X",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

/*******************************************************************************
 **
 ** Function         uuid16_to_uuid128
 **
 ** Description      Convert UUID16 to UUID128.
 **                  Example: 1812 >> 00001812-0000-1000-8000-00805f9b34fb
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uuid16_to_uuid128(uint16_t uuid16, bt_uuid_t* uuid128)
{
    uint16_t uuid16_bo;
    static const uint8_t  sdp_base_uuid[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
    memset(uuid128, 0, sizeof(bt_uuid_t));

    memcpy(uuid128->uu, sdp_base_uuid, 16);
    uuid16_bo = ntohs(uuid16);
    memcpy(uuid128->uu + 2, &uuid16_bo, sizeof(uint16_t));
}

/*******************************************************************************
 **
 ** Function         convert_bdaddr2string
 **
 ** Description      Convert BD Address into string format
 **                  Example: 0123456789ab >> 01:23:45:67:89:ab
 **
 ** Returns          string
 **
 *******************************************************************************/
static char* convert_bdaddr2string(const bt_addr_struct *bdaddr, bdstr_t *bdstr)
{
    uint8_t addr[6];
    /* Change the addr from [lap][uap][nap] to [nap][uap][lap] */
    addr[0] = (uint8_t) ((bdaddr->nap & 0xFF00) >> 8);
    addr[1] = (uint8_t) (bdaddr->nap & 0x00FF);
    addr[2] = (uint8_t) bdaddr->uap;
    addr[3] = (uint8_t) ((bdaddr->lap & 0x00FF0000) >> 16);
    addr[4] = (uint8_t) ((bdaddr->lap & 0x0000FF00) >> 8);
    addr[5] = (uint8_t) (bdaddr->lap & 0x000000FF);

    sprintf(*bdstr, "%02x:%02x:%02x:%02x:%02x:%02x",
            addr[0], addr[1], addr[2],
            addr[3], addr[4], addr[5]);

    return *bdstr;
}

/*******************************************************************************
 **
 ** Function         btmtk_config_set_bin
 **
 ** Description      Convert hex to binary
 **
 ** Returns          TRUE/FALSE
 **
 *******************************************************************************/
static bool btmtk_config_set_bin(config_t *config, const char *section, const char *key, const uint8_t *value, size_t length)
{
    const char *lookup = "0123456789abcdef";

    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);

    if (length > 0)
        assert(value != NULL);

    char *str = (char *)osi_calloc(length * 2 + 1);
    if (!str)
        return FALSE;

    for (size_t i = 0; i < length; ++i) {
        str[(i * 2) + 0] = lookup[(value[i] >> 4) & 0x0F];
        str[(i * 2) + 1] = lookup[value[i] & 0x0F];
    }

    config_set_string(config, section, key, str);

    osi_free(str);
    return TRUE;
}
