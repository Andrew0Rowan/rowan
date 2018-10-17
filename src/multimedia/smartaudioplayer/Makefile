###########################################################################
# libsmartaudioplayer.so
###########################################################################
PLAYER_SRCS :=	src/SmartAudioPlayer.cpp \
				src/GenericSource.cpp \
				src/SmartAudioPlayerDecoder.cpp \
				src/SmartAudioPlayerDriver.cpp \
				src/SmartAudioPlayerRenderer.cpp \
				src/FFStreamer.cpp \
				src/FFDemuxer.cpp \
				src/FFDecoder.cpp \
				src/MASRender.cpp \
				src/FFmpeg.cpp \
				src/AVPacketSource.cpp \
				src/Interface.cpp

UTILS_SRCS	:=	src/utils/ABuffer.cpp \
				src/utils/AMessageQueue.cpp \
				src/utils/pp_msg_q.cpp \
				src/utils/Timers.cpp \
				src/utils/Log.cpp

SOURCE_FILES := $(PLAYER_SRCS) $(UTILS_SRCS)

PLAYER_INC := 	-I . \
				-I include/ \
				-I src/utils/inc

PLAYER_CFLAGS  := -w -g -funwind-tables -std=c++11 -fPIC

PLAYER_LDFLAGS := -fPIC -shared 

PLAYER_LIBS :=  -lpthread \
				-lasound \
				#-lffmpeg

PLAYER_OBJS := $(patsubst %.cpp,%.o,$(SOURCE_FILES))

PLAYER_TARGET := libsmartaudioplayer.so

.PHONY : everything objs clean veryclean rebuild
  
all : $(PLAYER_TARGET) 

install:
	install -d ${DESTDIR}/${libdir}
	install -m 0644 libsmartaudioplayer.so ${DESTDIR}/${libdir}/libsmartaudioplayer.so
	install -d ${DESTDIR}/${includedir}
	install -m 0644 include/* ${DESTDIR}/${includedir}/
clean :
	find . -name "*.o" | xargs rm -f
	rm -fr $(PLAYER_TARGET)  

$(PLAYER_OBJS):%.o:%.cpp
	${CXX} ${PLAYER_CFLAGS} ${CFLAGS} $(PLAYER_INC) -c $< -o $@
$(PLAYER_TARGET):$(PLAYER_OBJS) 
	$(CXX) -o $@ $(PLAYER_OBJS) $(PLAYER_LDFLAGS) $(PLAYER_LIBS)

