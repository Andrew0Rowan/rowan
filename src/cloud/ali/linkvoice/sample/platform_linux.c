//
//  platform_linux.c
//  pal_sdk
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "pal_platform.h"

int pal_thread_create(void **thread, const char *name, void *(*start_routine)(void *), void *arg, unsigned int stack_size)
{
    pthread_t *t = (pthread_t*)pal_malloc(sizeof(pthread_t)); //FIXME memory leak if there is no thread destroy
    if (t == NULL) {
        *thread = NULL;
        return -1;
    }
    int ret = pthread_create(t, NULL, start_routine, arg);
    *thread = t;
    return ret;
}

void pal_thread_exit(void *thread)
{
    pthread_exit(0);
}

int pal_thread_join(void *thread)
{
    pthread_t *t = (pthread_t *)thread;
    return pthread_join(*t, NULL);
}

void pal_thread_destroy(void *thread)
{
    pal_free((pthread_t*)thread);
}

int pal_mutex_init(void **mutex)
{
    pthread_mutex_t *m = (pthread_mutex_t *)pal_malloc(sizeof(pthread_mutex_t));
    if (NULL == m) {
        *mutex = NULL;
        return -1;
    }
    int ret = pthread_mutex_init(m, NULL);
    if (ret != 0) {
        pal_free(m);
        *mutex = NULL;
        return ret;
    }
    *mutex = m;
    return ret;
}

int pal_mutex_destroy(void *mutex)
{
    int ret = pthread_mutex_destroy((pthread_mutex_t*)mutex);
    pal_free(mutex);
    return ret;
}

int pal_mutex_lock(void *mutex)
{
    return pthread_mutex_lock((pthread_mutex_t*)mutex);
}

int pal_mutex_unlock(void *mutex)
{
    return pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

void *pal_semaphore_init()
{
    sem_t *sem = pal_malloc(sizeof(sem_t));
    if (0 != sem_init(sem, 0, 0)) {
        pal_free(sem);
        return NULL;
    }
    return sem;
}

void pal_semaphore_destroy(void *sem)
{
    sem_destroy(sem);
    pal_free(sem);
}

int pal_semaphore_wait(void *sem, int timeout_ms)
{
    if (-1 == timeout_ms) {
        sem_wait(sem);
        return 0;
    } else {
        struct timespec ts;
		struct timeval t;
		gettimeofday(&t, NULL);        
		printf("===== t.tv_sec %d, t.tv_usec %ld\n", t.tv_sec, t.tv_usec);
		ts.tv_sec  = t.tv_sec + timeout_ms / 1000;
        ts.tv_nsec = t.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
        ts.tv_sec += ts.tv_nsec / 1000000000;
        ts.tv_nsec = ts.tv_nsec % 1000000000;
        printf("===== ts.tv_sec %d, ts.tv_nsec %ld\n", ts.tv_sec, ts.tv_nsec);
        return sem_timedwait(sem, &ts);
    }
}

void pal_semaphore_post(void *sem)
{
    sem_post(sem);
}

void pal_msleep(int millisecond)
{
    usleep(millisecond * 1000);
}

void* pal_malloc(int size)
{
    if (size <= 0) {
        return NULL;
    }
    return malloc(size);
}

void pal_free(void *mem)
{
    if (!mem) {
        return;
    }
    free(mem);
}

uint64_t pal_get_utc_time_ms()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    uint64_t ms = (uint64_t)t.tv_sec * 1000;
    ms += t.tv_usec / 1000;
    return ms;
}
