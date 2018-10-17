#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "Format.h"
#include "Errors.h"

/**
 * Create a player instance.
 *
 * @return  player instance.
 */
void* player_init(void);
status_t setPCMMode(void* handle, int id);
status_t setDataSource_url(void* handle, const char *url, int id);
status_t setDataSource(void* handle, int id);
/* the OutFileName must be name.(name.ext), the . after name is needed.
the player will add the ext name. such as  OutFileName="dump.",the player
will dump the mp3 source to dump.mp3 file.*/
status_t setOutFileName(void* handle, const char *OutFileName);
status_t prepare(void* handle);
status_t prepareAsync(void* handle);
status_t start(void* handle);
status_t stop(void* handle);
status_t pause_l(void* handle);
void setPlaybackSettings(void* handle, const AudioFormat* format);
status_t seekTo(void* handle, off64_t usec);
status_t reset(void* handle);
status_t setLooping(void* handle, int loop);
status_t getDuration(void* handle, off64_t *usec);
status_t getCurrentPosition(void* handle, off64_t *usec);
void player_deinit(void* handle);
status_t writeData(void* handle, unsigned char* buffer, int size);
status_t writeDatawithEOS(void* handle, unsigned char* buffer, int size);
status_t writePCMData(void* handle, unsigned char* buffer, int size);
status_t writePCMDatawithEOS(void* handle, unsigned char* buffer, int size);
long long int getAvailiableSize(void* handle);
void registerCallback(void* handle, callback *fn);
#endif