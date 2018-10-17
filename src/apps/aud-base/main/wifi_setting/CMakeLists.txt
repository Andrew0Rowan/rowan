INCLUDE_DIRECTORIES(../inc)
INCLUDE_DIRECTORIES(../../../../wifi-mw/1.0-r0/wifi-mw-1.0/library/wifi_mw/wifi)

AUX_SOURCE_DIRECTORY(. WIFI_SETTING)

IF(${ADAPTOR_SOURCE_ADD} MATCHES 1)
	message("ADAPTOR_SOURCE_ADD is defined in wifi_setting")
  list(APPEND WIFI_SETTING 
			${ADAPTOR_APP_CTRL_SOURCE}/wifi_setting/app_ctrl_wifi.c)
ENDIF()

ADD_LIBRARY(wifi_setting ${WIFI_SETTING})
TARGET_LINK_LIBRARIES(wifi_setting smtcn)