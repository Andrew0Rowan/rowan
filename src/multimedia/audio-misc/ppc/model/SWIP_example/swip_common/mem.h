#ifndef MEM_H
#define MEM_H

typedef struct
{
    long mem_start; // use long because address length is 32 bit/64 bit in 32/64 architecture
    unsigned int mem_size;
    long mem_now;
} MTK_MEM_HANDLE;

typedef enum
{
    MEM_SUCCESS,
    MEM_FAIL
} MTK_MEM_RES;

MTK_MEM_RES mem_init(void* mem_p, char *buffer, unsigned int size);
MTK_MEM_RES mem_alloc(MTK_MEM_HANDLE *mem_handle, long *buffer, unsigned int size);
MTK_MEM_RES mem_calloc(MTK_MEM_HANDLE *mem_handle, long *buffer, unsigned int size);
MTK_MEM_RES mem_free(MTK_MEM_HANDLE *mem_handle);

#endif
