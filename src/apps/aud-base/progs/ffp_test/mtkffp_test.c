#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>
#include "mtkffp.h"

#define SAMPLE_RATE (16000)
#define PERIOD_SIZE (10/*ms*/ * SAMPLE_RATE / 1000)
#define INPUT_CHANNELS (6)
#define OUTPUT_CHANNELS (2)

static int32_t input[PERIOD_SIZE * INPUT_CHANNELS];
static int32_t output[PERIOD_SIZE * OUTPUT_CHANNELS];
static FILE *fin;
static FILE *fout;
static void *ffp;
static volatile int write_finish;

static void *thread_writer(void *arg)
{
	while (fread(input, PERIOD_SIZE * INPUT_CHANNELS, sizeof(int32_t), fin)) {
		mtkffp_write(ffp, input, sizeof(input));
	}
	write_finish = 1;
	mtkffp_ctl(ffp, MTKFFP_INTERRUPT_READER, NULL);
	return NULL;
}

static void *thread_reader(void *arg)
{
	int i = 0;

	while (!write_finish) {
		int doa;

		mtkffp_read(ffp, output, sizeof(output));
		mtkffp_ctl(ffp, MTKFFP_GET_DOA, &doa);
		//printf("doa:%d\n", doa);
		fwrite(output, PERIOD_SIZE * OUTPUT_CHANNELS, sizeof(int32_t), fout);
		if (i == 2000) {//simulate the recognizer
			struct mtkffp_recognition recog = { 0, 0, 100, 1 };

			mtkffp_ctl(ffp, MTKFFP_SET_RECOGNITION, &recog);
			printf("wake test!\n");
		}
		i++;
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int err;
	const char *file_path = "/etc/holidayModel";
	enum mtkffp_working_mode mode = MTKFFP_WORKING_MODE_ASYNC;

	if (argc != 2) {
		printf("Usage: ffptestmtkffp MODE\n"
			"  MODE=s(sync mode)\n"
			"  MODE=a(async mode)\n");
		return 0;
	}
	if (strcmp(argv[1], "s") == 0)
		mode = MTKFFP_WORKING_MODE_SYNC;

	fin = fopen("/tmp/16000_32_6.pcm", "rb");
	if (!fin) {
		printf("error: can't open fin\n");
		return 0;
	}
	fout = fopen("/tmp/16000_32_2.pcm", "wb");
	if (!fout) {
		printf("error: can't open fout\n");
		return 0;
	}

	ffp = mtkffp_create();
	if (!ffp) {
		printf("error: can't mtkffp_create\n");
		return 0;
	}

	err = mtkffp_ctl(ffp, MTKFFP_SET_CONFIG_FILE_PATH, (void *)file_path);
	if (err < 0) {
		printf("error: mtkffp_ctl fail, err=%d\n", err);
		return 0;
	}
	err = mtkffp_prepare(ffp, mode);
	if (err < 0) {
		printf("error: can't mtkffp_prepare err=%d\n", err);
		return 0;
	}

	if (mode == MTKFFP_WORKING_MODE_SYNC) {
		printf("sync processing...\n");
		while (fread(input, PERIOD_SIZE * INPUT_CHANNELS, sizeof(int32_t), fin)) {
			mtkffp_process(ffp, input, sizeof(input), output, sizeof(output));
			fwrite(output, PERIOD_SIZE * OUTPUT_CHANNELS, sizeof(int32_t), fout);
		}
	} else if (mode == MTKFFP_WORKING_MODE_ASYNC) {
		pthread_t thread1, thread2;
		pthread_attr_t attr1;
		pthread_attr_init(&attr1);
		cpu_set_t cpu_set;

		CPU_ZERO(&cpu_set);
		CPU_SET(3, &cpu_set);
		if (pthread_attr_setaffinity_np(&attr1, sizeof(cpu_set_t), &cpu_set) != 0) {
			printf("pthread_attr_setaffinity_np fail\n");
		}

		printf("async processing...\n");
		if (pthread_create(&thread1, &attr1, thread_writer, NULL) != 0) {
			printf("pthread_create thread_writer fail\n");
			return 0;
		}

		if (pthread_create(&thread2, NULL, thread_reader, NULL) != 0) {
			printf("pthread_create thread_reader fail\n");
			return 0;
		}
		pthread_join(thread1, NULL);
		pthread_join(thread2, NULL);
	}
	mtkffp_unprepare(ffp);
	mtkffp_destroy(ffp);
	fclose(fin);
	fclose(fout);
	return 0;
}

