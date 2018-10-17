#pragma once

typedef enum {
  // Default log mode, log file size is unlimited
  BTSNOOP_LOG_MODE_DEFAULT,
  // Alone log mode descroption:
  //   1. Hci log is a single file, and its size is limited to 300M;
  //   2. If Log file is larger then 300M, it would override the old log record,
  //    which likes a file ring buffer;
  // Alone log mode and default log mode relationship:
  //   1. If default mode and alone log mode are both open before opening Bluetooth,
  //     default mode should be taken;
  //   2. otherwise, mode log setting should take effect after reopen Bluetooth.
  // How to enable alone log mode
  //   1. Set MTK_BTSNOOPLOG_MODE_SUPPORT to "TRUE" in mdroid_buildcfg.h,
  //   2. Set system property "bt.hcilogalone" to "1"
  BTSNOOP_LOG_MODE_ALONE
} BTSNOOP_LOG_MODE;

typedef void (*SNOOP_WRITE)(const void *data, size_t length);

bool twrite_init(const char * log_path, BTSNOOP_LOG_MODE log_mode, int * fd);
int twrite_deinit();
int twrite_write(const unsigned char *data, int len);

void twrite_write_packet(SNOOP_WRITE btsnoop_write, int type,
    const uint8_t *packet, int length_he_in, int length_in, int flags_in,
    int drops_in, uint32_t time_hi_in, uint32_t time_lo_in);

bool enable_mtk_btsnoop();

