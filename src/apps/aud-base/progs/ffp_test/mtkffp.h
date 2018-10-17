#ifndef _MTKFFP_H_
#define _MTKFFP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MTKFFP_SET_CONFIG_FILE_PATH (1)
#define MTKFFP_SET_RECOGNITION      (2)
#define MTKFFP_GET_DOA              (3)
#define MTKFFP_INTERRUPT_READER     (4)

struct mtkffp_recognition {
	int confidence1;
	int delay1;
	int confidence2;
	int delay2;
};

enum mtkffp_working_mode {
	MTKFFP_WORKING_MODE_SYNC,
	MTKFFP_WORKING_MODE_ASYNC,
};

void *mtkffp_create(void);

int mtkffp_ctl(void *handle, int id, void *param);

int mtkffp_prepare(void *handle, enum mtkffp_working_mode mode);

int mtkffp_process(void *handle,
	const void *input_pcm, unsigned int input_size,
	void *output_pcm, unsigned int output_size);

int mtkffp_write(void *handle, const void *input_pcm, int input_size);

int mtkffp_read(void *handle, void *output_pcm, int output_size);

int mtkffp_unprepare(void *handle);

void mtkffp_destroy(void *handle);

#ifdef __cplusplus
}
#endif

#endif
