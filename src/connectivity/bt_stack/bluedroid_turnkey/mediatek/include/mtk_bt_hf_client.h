#ifndef MTK_INCLUDE_BT_HF_CLIENT_H
#define MTK_INCLUDE_BT_HF_CLIENT_H

#include <hardware/bt_hf_client.h>

__BEGIN_DECLS

typedef enum
{
    BTHF_CLIENT_ENABLE,
    BTHF_CLIENT_DISABLE,
}bthf_client_ability_status_t;

typedef enum
{
    BTHF_CLIENT_DISABLE_NONE = 0,
    BTHF_CLIENT_DISABLE_START,
    BTHF_CLIENT_DISABLE_AUDIO_DISCONNECT,
    BTHF_CLIENT_DISABLE_DISCONNECT,
    BTHF_CLIENT_DISABLE_CLEANUP,
    BTHF_CLIENT_DISABLE_COMPLETE,
}bthf_client_disable_status_t;

/**
 * Callback for sending HFP enable/disable status to app
 */
typedef void (* bthf_client_ability_callback) (bthf_client_ability_status_t status);

/**
 * Callback for sending phonebook storage memory to app
 */
typedef void (* bthf_client_cpbs_callback) (int *storage);


/**
 * Callback for sending phonebook entries to app
 */
typedef void (* bthf_client_cpbr_entry_callback) (int index,  const char *number, int type,  const char *name);


/**
 * Callback for sending phonebook entry count to app
 */
typedef void (* bthf_client_cpbr_count_callback) (int index_max);


/**
 * Callback for sending phonebook entry complete event to app
 */
typedef void (* bthf_client_cpbr_complete_callback) ();


/** BT-HF callback structure. */
typedef struct {
    /** set to sizeof(bthf_ex_client_callbacks_t) */
    size_t      size;
    bthf_client_ability_callback           ability_cb;
    bthf_client_cpbs_callback              cpbs_cb;
    bthf_client_cpbr_count_callback        cpbr_count_cb;
    bthf_client_cpbr_entry_callback        cpbr_entry_cb;
    bthf_client_cpbr_complete_callback     cpbr_complete_cb;
} bthf_client_ex_callbacks_t;


/** Represents the standard BT-HF interface. */
typedef struct {

    /** set to sizeof(bthf_ex_client_interface_t) */
    size_t size;
    /**
     * Register the BtHf Ex callbacks
     */
    bt_status_t (*init_ex)(bthf_client_ex_callbacks_t* callbacks);

    /** Closes HFP Client Ex interface. */
    void (*cleanup_ex)(void);

    /** Select phonebook memory storage */
    bt_status_t (*select_pb_storage) ();

    /** Set phonebook memory storage */
    bt_status_t (*set_pb_storage) (int storage_idx);

    /** Set TE charset */
    bt_status_t (*set_charset) (const char* charset);

    /** Read phonebook entries */
    bt_status_t (*read_pb_entry) (int idx_min, int idx_max);

    /** Enable HFP Client Ex interface*/
    bt_status_t (*enable_ex)(bthf_client_ex_callbacks_t* callbacks);

    /** Enable HFP Client interface*/
    bt_status_t (*enable)(bthf_client_callbacks_t* callbacks);

    /** Disable HFP Client interface*/
    bt_status_t (*disable)(void);
} bthf_client_ex_interface_t;

__END_DECLS

#endif /* MTK_INCLUDE_BT_HF_CLIENT_H */

