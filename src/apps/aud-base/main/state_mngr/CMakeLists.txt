INCLUDE_DIRECTORIES(../inc)

AUX_SOURCE_DIRECTORY(. STATE_MNGR)

ADD_LIBRARY(state_mngr ${STATE_MNGR})
TARGET_LINK_LIBRARIES(state_mngr NvRAMUtils nvram nvram_custom file_op)
