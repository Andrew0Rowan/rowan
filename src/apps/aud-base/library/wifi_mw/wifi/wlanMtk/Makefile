LOCAL_DIR := $(shell pwd)

INCLUDE := \
		   -I$(LOCAL_DIR)/.. \
		   -I$(LOCAL_DIR)/../wlanCtrl \
		   -I$(LOCAL_DIR)/../../../../main/inc

LIBS := -lwlanCtrl
LDFLAGS := $(LIBS) \
		  -L$(LOCAL_DIR)/../wlanCtrl
DEFINES := -DOS_TYPE_LINUX -DPLATFORM_IS_BDP
CFLAGS  := -MMD -g -Wall -shared -fPIC -O2 -Wno-strict-aliasing \
		   $(DEFINES) $(INCLUDE) $(LDFLAGS)
CXXFLAGS:= $(CFLAGS)

TARGET := libwlanMtk.so
SOURCE := c_net_p2p_api.c c_net_wlan_api.c
OBJS := $(patsubst %.c,%.o,$(SOURCE))

.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(TARGET) *.o *.d *.so
