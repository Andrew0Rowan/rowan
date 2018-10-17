#ifndef MTK_PLATFORM
#include "stdio.h"
#include <stdlib.h>
#include "string.h"
#endif
#include "mem.h"
#include "ppc_common.h"

MTK_MEM_RES mem_init(void* swip_hdl, char *buffer, unsigned int size)
{
    MTK_MEM_HANDLE *mem_hdl;
	MTK_MEM_HANDLE **mem_p = swip_hdl;

    if (buffer == NULL)
    {
        print_log("[mem_init] buffer is NULL\n");
        return MEM_FAIL;
    }


    if (size < sizeof(MTK_MEM_HANDLE))
    {
        print_log("[mem_init] buffer size is too small\n");
        return MEM_FAIL;
    }

    mem_hdl = (MTK_MEM_HANDLE *) buffer;
    mem_hdl->mem_start = (long)(buffer + sizeof(MTK_MEM_HANDLE));
    mem_hdl->mem_now = mem_hdl->mem_start;
    mem_hdl->mem_size = size - sizeof(MTK_MEM_HANDLE);
    *mem_p = mem_hdl;

    return MEM_SUCCESS;
}

MTK_MEM_RES mem_alloc(MTK_MEM_HANDLE *mem_handle, long *buffer, unsigned int size)
{
    if (mem_handle->mem_size < size)
    {
        print_log("[mam_calloc] memory not enough: current size(%d) required size(%d)\n", mem_handle->mem_size, size);
        return MEM_FAIL;
    }
    *buffer = mem_handle->mem_now;
    mem_handle->mem_now += size;
    mem_handle->mem_size -= size;
    return MEM_SUCCESS;
}

MTK_MEM_RES mem_calloc(MTK_MEM_HANDLE *mem_handle, long *buffer, unsigned int size)
{
    if (mem_handle->mem_size < size)
    {
        print_log("[mam_calloc] memory not enough: current size(%d) required size(%d)\n", mem_handle->mem_size, size);
        return MEM_FAIL;
    }
    *buffer = mem_handle->mem_now;
    memset((void *)*buffer, 0, size);
    mem_handle->mem_now += size;
    mem_handle->mem_size -= size;
    return MEM_SUCCESS;
}

MTK_MEM_RES mem_free(MTK_MEM_HANDLE *mem_handle)
{
    mem_handle->mem_now = 0;
    mem_handle->mem_size = 0;
    mem_handle->mem_start = 0;
    return MEM_SUCCESS;
}
