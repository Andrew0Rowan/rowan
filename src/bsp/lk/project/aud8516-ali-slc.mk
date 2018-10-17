# main project for aud8516-slc
ARCH := arm64
ARM_CPU := cortex-a53
TARGET := aud8516-ali-slc
WITH_KERNEL_VM ?= 1
WITH_LINKER_GC := 1

MODULES += \
        app/fitboot \

LK_HEAP_IMPLEMENTATION=miniheap
