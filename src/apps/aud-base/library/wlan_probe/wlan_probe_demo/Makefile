LOCAL_DIR := $(shell pwd)

#source file
SOURCE  := $(wildcard *.c)
OBJS 	:= $(patsubst %.c,%.o,$(SOURCE))
  
#target you can change test to what you want
TARGET  := wlan_probe_demo
  
#compile and lib parameter
#CC      := gcc
LIBS    := -lwlanProbe
LDFLAGS := $(LIBS) \
			-L$(LOCAL_DIR)/../wlan_probe_lib

DEFINES := 
INCLUDE := \
			-I$(LOCAL_DIR)/../wlan_probe_lib \
           
CFLAGS  := -g -pthread -Wall -O3 $(DEFINES) $(INCLUDE) $(LDFLAGS)
CXXFLAGS:= $(CFLAGS)
  
#i think you should do anything here
.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean :
	rm -fr *.so
	rm -fr *.o
    
veryclean : clean
	rm -fr $(TARGET)
  
$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS) 

