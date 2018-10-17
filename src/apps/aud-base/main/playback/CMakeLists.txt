INCLUDE_DIRECTORIES(../inc)
INCLUDE_DIRECTORIES(../utils/inc)
INCLUDE_DIRECTORIES(../../../../smartaudioplayer/1.0-r0/smartaudioplayer-1.0/include)

IF(${DLNA_SUPPORT} MATCHES 1)
	message("dlna is defined in playback")
	SET(PLAYBACK playback_uri.cpp playback_uri_cli.cpp playback_prompt.c playback_prompt_cli.c playback_tts.c  playback_tts_cli.c playback_dlna.c playback_dlna_cli.c)
ELSE()
	message("dlna is not defined in playback")
	SET(PLAYBACK playback_uri.cpp playback_uri_cli.cpp playback_prompt.c playback_prompt_cli.c playback_tts.c  playback_tts_cli.c)		
ENDIF()

ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE)

ADD_LIBRARY(playback ${PLAYBACK})
TARGET_LINK_LIBRARIES(playback smartaudioplayer utils)
