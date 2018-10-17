#ifndef    _HEADER_PPC_CONTROL_H
#define    _HEADER_PPC_CONTROL_H

// ======
// Version Info
// ======
#define PPC_MAJOR_VERSION    (0)
#define PPC_SUB_VERSION      (0)

#define AUD_PPC_U_DISK_PATH_00    "/mnt/"
#define AUD_PPC_U_DISK_PATH_01    "/data/sda1/"
#define AUD_PPC_DEFAULT_CONFIG_PATH00    "/mnt/ppc_cfg/"
#define AUD_PPC_DEFAULT_CONFIG_PATH01    "/data/sda1/ppc_cfg/"
#define AUD_PPC_TMP_CONFIG_PATH          "/tmp/ppc_cfg/"
#define AUD_PPC_CONFIG_FILE_USED_TEST    "element_pool.cfg"
#define AUD_PPC_DEFAULT_FLAG             "default"
#define AUD_PPC_BUILTIN_FLAG             "built-in"

typedef enum{
    AUD_PPC_SUCCESS               =  0,
    AUD_PPC_GET_PPC_SIZE_FAIL     = -1,
    AUD_PPC_ALLOC_PPC_INFO_FAIL   = -2,
    AUD_PPC_ALLOC_PPC_HANDLE_FAIL = -3,
    AUD_PPC_INVALID_CHANNEL_CONFIG = -4,
    AUD_PPC_CHANNEL_NOT_SUPPORT = -5,
    AUD_PPC_SAMPLERATE_NOT_SUPPORT = -6,
    AUD_PPC_SAMPLERATE_UNKNOWN = -7,
    AUD_PPC_FORMAT_NOT_SUPPORT = -8,
    AUD_PPC_LOAD_FAIL = -9,
    AUD_PPC_CREATE_FAIL = -10,
    AUD_PPC_PROCESS_FAIL = -11,
    AUD_PPC_REINIT_FAIL = -12,
    AUD_PPC_RESET_FAIL = -13,
    AUD_PPC_DESTROY_INFO_FAIL = -14,
    AUD_PPC_DESTROY_HDL_FAIL = -15,
    AUD_PPC_SET_PARA_FAIL = -16,
    AUD_PPC_SET_PARA_CANNOT = -17,
    AUD_PPC_SET_PARA_NEED_REINIT = -18,
    AUD_PPC_DUMP_ELEMENT_FAIL = -19,
    AUD_PPC_DUMP_ELEMENT_CANNOT = -20,
    AUD_PPC_BYPASS_ELEMENT_FAIL = -21,
    AUD_PPC_BYPASS_ELEMENT_CANNOT = -22,
    AUD_PPC_BYPASS_ELEMENT_NEED_RESET = -23,
    AUD_PPC_DUMP_STATUS_FAIL = -24,
    AUD_PPC_DUMP_STATUS_CANNOT = -25,
    AUD_PPC_GET_PARA_FAIL = -26,
    AUD_PPC_GET_PARA_CANNOT = -27,
    AUD_PPC_GET_ELEMENT_BYPASS_STATUS_FAIL = -28,
    AUD_PPC_GET_ELEMENT_BYPASS_STATUS_CANNOT = -29,
    AUD_PPC_SET_CFG_PATH_FAIL = -30,
    AUD_PPC_GET_CFG_PATH_FAIL = -31,
    AUD_PPC_ALLOC_PPC_MAIN_STRUCT_FAIL = -32,
    AUD_PPC_INFO_UNINITED = -33,
    AUD_PPC_DEVICE_NOT_SUPPORT = -34,
    AUD_PPC_MODE_NOT_SUPPORT = -35,
    AUD_PPC_DUMP_PATH_NOT_EXIST = -36,
    AUD_PPC_ALLOC_STRUCT_FAIL = -37,
    AUD_PPC_GET_DELAY_FAIL = -38,
    AUD_PPC_CHECK_VERSION_FAIL = -39,
    AUD_PPC_SET_EQ_MODE_FAIL = -40,
    AUD_PPC_SET_EQ_MODE_CONNOT = -41,
}audio_ppc_result_t;

typedef enum
{
    AUD_PPC_INFO_UNALLOC = 0,
    AUD_PPC_INFO_UNINIT,
    AUD_PPC_INFO_INITING,
    AUD_PPC_INFO_INITED,
    AUD_PPC_INFO_CHANGED
}audio_ppc_info_status_t;

typedef enum
{
    AUD_SYS_INFO_UNALLOC = 0,
    AUD_SYS_INFO_UNINIT,
    AUD_SYS_INFO_INITING,
    AUD_SYS_INFO_INITED,
    AUD_SYS_INFO_CHANGED
}audio_sys_info_status_t;

typedef enum{
    AUD_PPC_DEVICE_SPEAKER               =  1,
    AUD_PPC_DEVICE_HDMI
}audio_ppc_device_t;

typedef enum{
    AUD_PPC_MODE_MOVIE               =  1,
    AUD_PPC_MODE_MUSIC,
    AUD_PPC_MODE_3,
    AUD_PPC_MODE_4,
    AUD_PPC_MODE_5,
    AUD_PPC_MODE_6,
    AUD_PPC_MODE_7,
    AUD_PPC_MODE_8,
    AUD_PPC_MODE_9,
    AUD_PPC_MODE_10,
    AUD_PPC_MODE_11,
    AUD_PPC_MODE_12,
    AUD_PPC_MODE_13,
    AUD_PPC_MODE_14,
    AUD_PPC_MODE_15,
    AUD_PPC_MODE_16,
    AUD_PPC_MODE_17,
    AUD_PPC_MODE_18,
    AUD_PPC_MODE_19,
    AUD_PPC_MODE_20,
    AUD_PPC_MODE_21,
    AUD_PPC_MODE_22,
    AUD_PPC_MODE_23,
    AUD_PPC_MODE_24,
    AUD_PPC_MODE_25,
    AUD_PPC_MODE_26,
    AUD_PPC_MODE_27,
    AUD_PPC_MODE_28,
    AUD_PPC_MODE_29,
    AUD_PPC_MODE_30,
    AUD_PPC_MODE_31,
    AUD_PPC_MODE_32
}audio_ppc_mode_t;


#define AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH    100
#define AUD_PPC_CFG_FILE_PATH_STRING_LENGTH    (AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH - 2)

typedef struct
{
    unsigned int             bypass_mode;
    char*                    ppc_cfg_path;
    void*                    ppc_info;
    void*                    ppc_hdl;
    int                      ppc_info_size;
    int                      ppc_hdl_size;
    audio_ppc_info_status_t  ppc_info_status;
    audio_sys_info_status_t  sys_info_status;
    bool                     ppc_reinit_flag;
    bool                     ppc_reset_flag;
    bool                     ppc_tuning_flag;
    void*                    cur_sys_info;
    void*                    volatile_sys_info;
    void*                    audio_buf_in;
    void*                    audio_buf_out;
    char                     user_cfg_path[AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH];
}audio_ppc_main_struct_t;

#define AUDIO_PPC_CH_NUM 16

typedef enum
{
    AUD_PPC_CHANNEL_FRONT_LEFT    = 0x0001,
    AUD_PPC_CHANNEL_FRONT_RIGHT   = 0x0002,
    AUD_PPC_CHANNEL_FRONT_CENTER  = 0x0004,
    AUD_PPC_CHANNEL_BACK_LEFT     = 0x0008,
    AUD_PPC_CHANNEL_BACK_RIGHT    = 0x0010,
    AUD_PPC_CHANNEL_BACK_CENTER   = 0x0020,
    AUD_PPC_CHANNEL_SIDE_LEFT     = 0x0040,
    AUD_PPC_CHANNEL_SIDE_RIGHT    = 0x0080,
    AUD_PPC_CHANNEL_EXTEND_0      = 0x0100,
    AUD_PPC_CHANNEL_EXTEND_1      = 0x0200,
    AUD_PPC_CHANNEL_EXTEND_2      = 0x0400,
    AUD_PPC_CHANNEL_EXTEND_3      = 0x0800,
    AUD_PPC_CHANNEL_EXTEND_4      = 0x1000,
    AUD_PPC_CHANNEL_EXTEND_5      = 0x2000,
    AUD_PPC_CHANNEL_EXTEND_6      = 0x4000,
    AUD_PPC_CHANNEL_LOW_FREQUENCY = 0x8000
} audio_ppc_channel_t;

typedef struct {
    unsigned int             input_config;
    unsigned int             sample_rate;
    unsigned int             sample_format;
    unsigned int             frame_size;
} audio_playback_info_t;

class AudioPPCControl
{
    public:
        static AudioPPCControl *getInstance();
        static void freeInstance();

        int  set(const char *path, int ppc_mode, int out_device);
        int  open(unsigned int input_config, unsigned int sample_rate, unsigned int frame_size, unsigned int *output_config);
	int  reset(void);
	int  process(void *input, void *output);
	int  process(void *input, void *output, unsigned int frames, int *latency);
	int  setParameter(unsigned int element_id, unsigned int para_id, int para_num, int* value);
	int  getParameter(unsigned int element_id, unsigned int para_id, int para_num, int* value);
	int  setElementBypass(unsigned int element_id, int enable);
	int  getElementBypass(unsigned int element_id, int* enabled);
	int  setCfgFilePath(const char *path);
	int  getCfgFilePath(char *path);
	int  getSystemInfo(void *ppc_sys_info);
	int  setDevice(int out_device);
	int  getDevice(void);
	int  setMode(int ppc_mode);
	int  getMode(void);
	int  dumpElementData(unsigned int element_id, int enabled, char* dump_folder);
	int  dumpConfigStatus(char* config_path, int passwd);
	int  setTuningMode(bool enable);
	int  getDelay(int *delay);
	int  checkVersion(int tool_version);
	int  checkSwipVersion(int type_id, int tool_version);
	int  setEqModePre(unsigned int element_id, int mode);
	int  setEqModePost(unsigned int element_id, int mode);
	static int  getParaValueSize(void);
	static int  getSysInfoSize(void);
	static int  getMaxConfigPathSize(void);

        static audio_ppc_main_struct_t *audio_ppc_main;

    private:

        AudioPPCControl();
        ~AudioPPCControl();

	static AudioPPCControl *unique_ppc_handler;

	int  init(void);
	void deInit(void);
	void ppcGetBuffer(void *input, void *output, unsigned int frames);
	int  ppcChooseConfigPath(void);
	int  ppcCheckFileExist(char *file_name);
	int  ppcSampleRateConvert(unsigned int rate, int *ppc_sample_rate);
	int  ppcFormatConvert(unsigned int format, int *ppc_audio_format);
	int  ppcSetSystemInfo(audio_playback_info_t *playback_info, int *sys_info_changed);
	void ppcSetUserInfo(int *user_info_changed);
	int  ppcStructAlloc(void);
	void ppcStructUnalloc(void);
	int  ppcLoad(void);
	int  ppcCreate(void);
	int  ppcDataProcess(unsigned int frames, int *latency);
	int  ppcSetPara(unsigned int element_id, unsigned int para_id, int num, int* value);
	int  ppcGetPara(unsigned int element_id, unsigned int para_id, int num, int* value);
	int  ppcBypassElement(unsigned int element_id, int enabled);
	int  ppcGetBypassStatus(unsigned int element_id, int *enabled);
	int  ppcChooseDumpPath(char *file_path);
	int  ppcCheckPathExist(char *file_path);
	int  ppcDumpStatus(char* config_path, int passwd);
	int  ppcDumpElement(unsigned int element_id, int enabled, char* dump_folder);

	pthread_mutex_t ppc_mutex;

};


#endif


