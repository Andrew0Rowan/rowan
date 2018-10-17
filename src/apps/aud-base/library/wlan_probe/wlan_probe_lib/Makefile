LOCAL_DIR := $(shell pwd)

INCLUDE := \
		   
LIBS := -lpthread
LDFLAGS := $(LIBS) \

DEFINES := -DOS_TYPE_LINUX
CFLAGS  := -MMD -g -Wall -shared -fPIC -O2 -Wno-strict-aliasing \
			$(DEFINES) $(INCLUDE) $(LDFLAGS)

CXXFLAGS:= $(CFLAGS)

TARGET := libwlanProbe.so
SOURCE := $(wildcard *.c)

OBJS := $(patsubst %.c,%.o,$(SOURCE))

.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(TARGET) *.o *.d *.so