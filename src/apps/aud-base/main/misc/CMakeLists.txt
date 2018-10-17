INCLUDE_DIRECTORIES(../inc)
INCLUDE_DIRECTORIES(../../../../wifi-mw/1.0-r0/wifi-mw-1.0/library/wifi_mw/wifi)

AUX_SOURCE_DIRECTORY(. MISC)

IF(${ADAPTOR_SOURCE_ADD} MATCHES 1)
	message("ADAPTOR_SOURCE_ADD is defined in wifi_setting")
  list(APPEND MISC 
			${ADAPTOR_APP_CTRL_SOURCE}/misc/app_ctrl_misc.c)
ENDIF()

ADD_LIBRARY(misc ${MISC})
TARGET_LINK_LIBRARIES(misc wlanMtk wlanCtrl wlanMon)
