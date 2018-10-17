#pragma once

void cmd_continue_boot(const char *arg, void *data, unsigned sz);
void cmd_download(const char *arg, void *data, unsigned sz);
void cmd_download_boot(const char *arg, void *data, unsigned sz);
void cmd_download_tz(const char *arg, void *data, unsigned sz);
void cmd_erase(const char *arg, void *data, unsigned sz);
void cmd_flash(const char *arg, void *data, unsigned sz);
void cmd_flash_img(const char *arg, void *data, unsigned sz);
void cmd_flash_sparse_img(const char *arg, void *data, unsigned sz);
void cmd_getvar(const char *arg, void *data, unsigned sz);
void cmd_reboot(const char *arg, void *data, unsigned sz);
void cmd_reboot_bootloader(const char *arg, void *data, unsigned sz);
void cmd_reboot_recovery(const char *arg, void *data, unsigned sz);
