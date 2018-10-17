#include <unistd.h>
#include "interface.h"

static int isprepare = 0;
static int setDataSourceDone = 0;
static int isexit = 0;
static pthread_t TTSThread;
#define SIZE (1*1024)
#define DELAY (100*1000)
static char file[128];

void cb(int msg, int ext1, int ext2) {
    printf("cb %d %d %d\n", msg, ext1, ext2);
    switch (msg) {
        case MEDIA_PLAYBACK_COMPLETE:
        {
            break;
        }
		case MEDIA_PREPARED:
		{
			isprepare = 1;
            printf("prepare done\n");
			break;
		}
        case MEDIA_ERROR:
        {
            break;
        }
 
        default:
            break;
    }
}
void updatePos(void* handle) {
	INT64 pos=0;
    sleep(5);
	getCurrentPosition(handle, &pos);
	printf("\n\npos %lld\n\n", pos);
}
#define local_test 1
#ifdef local_test
#define LOOP_COUNT 1000
int main(int argc, char* argv[]) {
    int i = 0;
    void* handle = player_init();
    //setLooping(handle, 1);
    for (i = 0;i< LOOP_COUNT; i++) {
        printf("\n*****************************\n");
        printf("***********  %d times**********", i);
        printf("\n*****************************\n");
        registerCallback(handle, cb);
        setDataSource_url(handle, argv[1]);
        setDataSourceDone=1;
        prepareAsync(handle);
        while (isprepare==0) {
            //printf("waiting prepare done\n");
            usleep(50000);
        }
        seekTo(handle,30);
        updatePos(handle);
        INT64 sec=0;
        getDuration(handle, &sec);
        printf("\n\nduration %lld\n\n", sec);
        start(handle);
#if 1
        updatePos(handle);
        pause_l(handle);
        seekTo(handle,50);
        updatePos(handle);
        updatePos(handle);
        start(handle);
        updatePos(handle);
        seekTo(handle,100);
        updatePos(handle);
#endif
        sleep(30);
        stop(handle);
        setDataSourceDone=0;
        isprepare=0;
        isexit=0;
        reset(handle);
    }
    player_deinit(handle);
    printf("exit\n");
    exit(0);
    return 1;
}
#else

void *writeFunc(void* handle) {
    FILE *fp;
    unsigned char* buffer;
    signed long long avbsize;
    int err = 0;
    int i=0;
    
    buffer = (unsigned char *) malloc(SIZE);
    if (buffer == NULL) {
        printf("alloc fail\n");
        return;
    }
    fp = fopen(file,"r+");
    if (fp == NULL) {
        printf("file can not open \n");
        return;
    }
    else {
        printf("file open ok\n");
    }
    while(1) {
        if (setDataSourceDone == 0) {
            printf("waiting set data source done\n");
			usleep(1000);
            continue;
        }
        if (isexit == 1) {
            if (buffer) {
                free(buffer);
                buffer = NULL;
            }
            if (fp) {
                fclose(fp);
                fp = NULL;
            }
            return;
        }
        avbsize = getAvailiableSize(handle);
        if (avbsize > 0) {
            printf("get avb size %lld\n", avbsize);
            while (avbsize > SIZE) {
                int ret = fread(buffer, 1, SIZE, fp);
                if (feof(fp)) {
                	if (ret > 0) {
	                	printf("eos, write size %d\n", ret);
						writeDatawithEOS(handle, buffer , ret);
                	}
		            if (buffer) {
		                free(buffer);
		                buffer = NULL;
		            }
		            if (fp) {
		                fclose(fp);
		                fp = NULL;
		            }
                    return;
                }
				else if (ret > 0) {
	                printf("write size %d\n", ret);
	                writeData(handle, buffer , ret);
	                avbsize-=ret;
	                i++;
	                //if (30 < i)
	                usleep(DELAY);
				}
				else {

					printf("read error\n");
				}

            }
            int ret = fread(buffer, 1, avbsize, fp);
			if (ret > 0) {
	            writeData(handle, buffer , ret);
	            printf("write size %d\n", ret);
			}
			else {
				printf("read error\n");
			}

            usleep(DELAY);
        }
        else {
            usleep(100000);
        }
    
    }
}

int main(int argc, char* argv[]) {
    int ret, i;

    strncpy(file, argv[1], strlen(argv[1]));
    file[strlen(argv[1])] = '\0';
    void* handle = player_init();
    for (i=0; i< 5; i++) {
        printf("\n\n************%d times******\n\n", i);
        ret = pthread_create(&TTSThread, NULL, writeFunc, (void *)handle);
        if (ret) {
            printf("pthread_create fail\n");
            return -1;
        }
        registerCallback(handle, cb);
        setDataSource(handle);
        setDataSourceDone=1;
        usleep(200000);
        prepareAsync(handle);
        while (isprepare==0) {
            //printf("waiting prepare done\n");
            usleep(50000);
        }
        start(handle);
        sleep(5);
        printf("playback done\n");
        isexit=1;
        pthread_join(TTSThread, NULL);
        reset(handle);

        setDataSourceDone=0;
        isprepare=0;
        isexit=0;
    }
    player_deinit(handle);
    printf("exit\n");
    exit(0);
    return 1;
}
#endif