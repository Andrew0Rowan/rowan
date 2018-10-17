CC := ${CROSS_COMPILE}gcc
AR := ${CROSS_COMPILE}ar
LD := ${CROSS_COMPILE}ld
OBJCOPY := ${CROSS_COMPILE}objcopy

LDS = tllink.lds

DIR_INC = ./include
DIR_SRC = ./src
DIR_PREBUILT = ./prebuild
DIR_OBJ = ${TL_RAW_OUT}/obj
DIR_BIN = ${TL_RAW_OUT}/bin

ASRCS = $(wildcard $(DIR_SRC)/*.s)
CSRCS = $(wildcard $(DIR_SRC)/*.c)
CSRCS += \
	$(DIR_SRC)/drivers/tz_emi_mpu.c

SRCS = $(ASRCS) $(CSRCS)
AOBJS = $(patsubst %.s, $(DIR_OBJ)/%.o, $(notdir $(ASRCS)))
COBJS = $(patsubst %.c, $(DIR_OBJ)/%.o, $(notdir $(CSRCS)))
SOBJS = $(wildcard $(DIR_PREBUILT)/*.a)
OBJS = $(AOBJS) $(COBJS) $(SOBJS)

TARGET = teeloader
BIN_TARGET = $(DIR_BIN)/$(TARGET)

all: $(OBJS)
	@if [ ! -d `dirname $(BIN_TARGET).elf` ] ; then \
		mkdir -p `dirname $(BIN_TARGET).elf`; \
	fi
	sed "s/%BASE_ADDR%/${BASE_ADDR}/g" $(LDS) > $(DIR_OBJ)/$(LDS)
	$(LD) --start-group $^ --end-group -T$(DIR_OBJ)/$(LDS) -o $(BIN_TARGET).elf
	-echo "teeloader binary created"
	$(OBJCOPY) -O binary $(BIN_TARGET).elf $(BIN_TARGET).bin
	./zero_padding.sh $(BIN_TARGET).bin ${TL_ALIGN_SIZE}

$(COBJS): $(CSRCS)
	@if [ ! -d `dirname $@` ] ; then \
		mkdir -p `dirname $@`; \
	fi
	$(CC) -I$(DIR_INC) -DBASE_ADDR=${BASE_ADDR} -DTL_ALIGN_SIZE=${TL_ALIGN_SIZE} -DTRUSTEDOS_ENTRYPOINT=${TRUSTEDOS_ENTRYPOINT} -c $(filter %$(patsubst %.o,%.c,$(notdir $@)),$(CSRCS)) -o $@

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.s
	@if [ ! -d `dirname $@` ] ; then \
		mkdir -p `dirname $@`; \
	fi
	$(CC) -c $^ -o $@

.PHONY: clean
clean:
	-@rm -rf $(DIR_OBJ)/* $(DIR_BIN)/*
