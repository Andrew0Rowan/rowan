LOCAL_DIR := $(shell pwd)

INCLUDE := \
		   -I${LOCAL_DIR}/../../../../main/inc \
		   -I$(LOCAL_DIR)/inc \
		   -I$(LOCAL_DIR)/..
		   
LIBS := -lpthread -lc
LDFLAGS := $(LIBS)

DEFINES := -DOS_TYPE_LINUX -DPLATFORM_IS_BDP
CFLAGS  := -MMD -g -Wall -shared -fPIC -O2 -Wno-strict-aliasing \
			$(DEFINES) $(INCLUDE) $(LDFLAGS)

CXXFLAGS:= $(CFLAGS)

TARGET := libwlanCtrl.so
SOURCE := $(wildcard *.c)

OBJS := $(patsubst %.c,%.o,$(SOURCE))

.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(TARGET) *.o *.d *.so