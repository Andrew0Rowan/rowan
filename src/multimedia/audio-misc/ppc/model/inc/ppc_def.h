#ifndef PPC_DEF_H
#define PPC_DEF_H

#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

/* for boolean */
typedef enum boolean
{
    PPC_FALSE = 0,
    PPC_TRUE = 1
} PPC_BOOL;

/* for para data type */
typedef enum data_type
{
    TYPE_INT    = 0,
    TYPE_SHORT  = 1,
    TYPE_CHAR   = 2,
    TYPE_FLOAT  = 3,
    TYPE_DOUBLE = 4
} DATA_TYPE;

/* element parameter information*/
typedef struct element_para_info
{
    char name[100];
    unsigned int id;
    char desc[100];
    DATA_TYPE data_type;
    int index;
    int num;
} PARA_INFO;

/* element type structure */
typedef struct element_type
{
    char name[100];
    unsigned int type_id;
    char desc[100];
    PPC_BOOL multi_instance;
    PPC_BOOL shared_buf;
    int mcps;
    int memory;
    int max_input_channel;
    int max_output_channel;
    int sample_rate;
    int para_num;
    PARA_INFO *para_info;
    int data_num;
    int32_t *default_para;
} ELEMENT_TYPE;

/* parameter */
typedef struct para PARA;
struct para
{
    unsigned int id;
    int num;
    int32_t *data;
    PARA *next;
};

/* codition structure */
typedef struct condition
{
    int in_channel_num;
    int out_channel_num;
    int *in_channel_list;
    int *out_channel_list;
    int mode;
    int device;
} CONDITION;

/* element structure */
typedef struct element
{
    char name[100];
    unsigned int element_id;
    unsigned int in_channel;
    unsigned int out_channel;
    PPC_BOOL bypass;
    PPC_BOOL dump;
    // for header file save default parameter
    //int *para;
    // for parameter list
    int para_list_num;
    PARA *para_head;
    PARA *para_tail;
    char dump_folder[100];
} ELEMENT;

/* link structure */
typedef struct link
{
    unsigned int front_element_id;
    unsigned int front_channel;
    unsigned int rear_element_id;
    unsigned int rear_channel;
} LINK;

/* page structure */
typedef struct page
{
    char name[100];
    unsigned int page_id;
    int frame_size;
    PPC_BOOL default_page;
    int cod_num;
    int ele_num;
    int link_num;
    CONDITION *cod_list;
    ELEMENT *ele_list;
    LINK *link_list;
} PAGE;

#endif
