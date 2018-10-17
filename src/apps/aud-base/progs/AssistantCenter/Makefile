TARGET := libAssistantCenter.so

CMD_HUB_PATH := CmdHub/
UNIVERSAL_CMD_PATH := UniversalCmd/
JSON_CMD_PATH := $(UNIVERSAL_CMD_PATH)/JsonCmd/
COMMUNICATION_HUB_PATH := CommunicationHub/
SOCKET_HUB_PATH := $(COMMUNICATION_HUB_PATH)/SocketHub/
#CJSON_PATH := cJson/
#----------------------c++------------------------#
CMD_HUB_SOURCE := $(CMD_HUB_PATH)/CmdHubApi.cpp $(CMD_HUB_PATH)/CmdHub.cpp $(CMD_HUB_PATH)/CmdDebug.cpp

UNIVERSAL_CMD_SOURCE := $(UNIVERSAL_CMD_PATH)/UniversalCmdFactory.cpp
JSON_CMD_SOURCE := $(JSON_CMD_PATH)/JsonCmd.cpp
COMMUNICATION_HUB_SOURCE := $(COMMUNICATION_HUB_PATH)/CommunicationHubFactory.cpp
SOCKET_HUB_SOURCE := $(SOCKET_HUB_PATH)/SocketHub.cpp
#CJSON_SOURCE := $(CJSON_PATH)/cJSON.c

CPP_SOURCE := $(CMD_HUB_SOURCE) $(UNIVERSAL_CMD_SOURCE) $(JSON_CMD_SOURCE) $(COMMUNICATION_HUB_SOURCE) $(SOCKET_HUB_SOURCE)
CPP_SOURCE +=  
CPP_OBJS := $(patsubst %.cpp,%.o,$(CPP_SOURCE))

INCLUDE := 	-I Include/ \
			-I $(CMD_HUB_PATH) \
			-I $(UNIVERSAL_CMD_PATH) \
			-I $(JSON_CMD_PATH) \
			-I $(COMMUNICATION_HUB_PATH) \
			-I $(SOCKET_HUB_PATH)
#			-I $(CJSON_PATH)

LIBS := -lm  -lpthread -lcjson
LDFLAGS += -L../../../cjson/1.6.0-r0/build 

C_FLAGS := -Wall -Os
CPP_FLAGS := $(C_FLAGS) -std=c++11

all:$(TARGET)

$(TARGET):$(CPP_OBJS)
	$(CXX) -shared -fPIC -o $@ $^ $(INCLUDE) $(LDFLAGS) $(LIBS) $(CPP_FLAGS)
$(CPP_OBJS):%o:%cpp
	$(CXX) -shared -fPIC -o $@ -c $<  $(CPP_FLAGS) $(INCLUDE)
$(C_OBJS):%o:%c
	$(CC) -shared -fPIC -o $@ -c $<  $(C_FLAGS) $(INCLUDE)

clean:
	rm -fr $(CPP_OBJS) $(C_OBJS) $(TARGET)
