TARGET := upgrade_app
SOURCES := \
	src/crc.c \
	src/debug.c \
	src/file_ops.c \
	src/gpt.c \

ifeq ($(BOOTDEV_TYPE),emmc)
	SOURCES += src/emmc_ops.c \
	src/main_emmc.c
else
	SOURCES += src/nand_ops.c \
	src/main_nand.c
endif

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

GET_LOCAL_DIR = $(shell pwd)
LOCAL_DIR := $(GET_LOCAL_DIR)
INCLUDES :=\
	-I $(LOCAL_DIR)/inc \
	-I $(LOCAL_DIR)/../../library/upg_trigger \

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
%.o: %.c 
	$(CC) $(INCLUDES) -c -o $@ $< 

clean:
	$(RM) $(OBJECTS) $(TARGET)

install: $(TARGET)
	echo $(DESTDIR)
	echo $(PREFIX)
	echo $(shell pwd)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/
