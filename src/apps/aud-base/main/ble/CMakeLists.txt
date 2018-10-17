INCLUDE_DIRECTORIES(inc)
INCLUDE_DIRECTORIES(ble_svr)
INCLUDE_DIRECTORIES(ble_client)
INCLUDE_DIRECTORIES(../../../../wifi-mw/1.0-r0/wifi-mw-1.0/library/wifi_mw/wifi)
INCLUDE_DIRECTORIES(./../wifi_setting)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc/config)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc_rpc)

IF(${GATT_TEST_SUPPORT} MATCHES 1)
    set(BLE_CLIENT_TEST_FILE "./ble_client/ble_client_test.c")
    message("add ble_client_test")
ENDIF()

AUX_SOURCE_DIRECTORY(. BLE)

list(APPEND BLE 
			./ble_svr/ble_svr_timer.c
			./ble_svr/ble_svr_wifi_info_get.c
			./ble_svr/ble_svr.c
			./ble_svr/ble_gatts.c
			./ble_client/ble_client.c
			./ble_client/ble_gattc.c
			${BLE_CLIENT_TEST_FILE})

ADD_LIBRARY(ble ${BLE})
