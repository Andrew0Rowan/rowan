INCLUDE_DIRECTORIES(../inc)
INCLUDE_DIRECTORIES(../utils/inc)
INCLUDE_DIRECTORIES(../../../../../../../../src/apps/aud-base/library/wifi_mw/wifi)

AUX_SOURCE_DIRECTORY(. ACFG)

ADD_LIBRARY(acfg ${ACFG})
