//
// 测试厂商平台上移植的libwebsockets，例子里面调用libwebsockets连接云端、发送测试消息、接收云端返回；
// 打印如下信息，说明libwebsockets与云端连通了
//
/*
------ connect established success ------
------ try to send msg to server ------
------ send 8 ------
------ receive msg from server: {"service_data":{"ttsUrl":[{"tts_text":"服务忙，请稍后重试","tts_url":"http://intellitts.alicdn.com/9a7a9d4e-32f0-4d8e-a7c5-f0e29398fc6b.mp3?Expires=1597985683&OSSAccessKeyId=15tm9tL7BfwyxAJd&Signature=zEbhb5kyZpBKpoPIMfyqXW3PHCw%3D"}]},"status_code":"500","finish":"1","status":"0","ctrl_dev":"true"} ------
------ connection closed ------
*/

#include <stdio.h>
#include <string.h>
#include "libwebsockets.h"

#define CA_PATH ("ca.pem")
#define PROTOCOL_WSS_ENABLE  ("wss")
#define PROTOCOL_WS_ENABLE ("ws")
#define SERVER ("mlabs.taobao.com")
#define USE_SSL 1
#define PORT 443
#define TEST_MSG "test msg"

static struct lws_context_creation_info * info = NULL;
struct lws_client_connect_info *conn = NULL;
struct lws_context *lws_ctx = NULL;
struct lws *g_lws = NULL;


enum connect_status
{
    STATUS_NONE,
    STATUS_ESTABLISED,
    STATUS_HANDSHAKE,
    STATUS_RECEIVE,
    STATUS_CLOSED
};

static int status = STATUS_NONE;
static int callback_handler(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    int ret = 0;
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
        {
            printf("------ connect established success ------\n");
            status = STATUS_ESTABLISED;
            lws_callback_on_writable(wsi);
            break;
        }
            
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        {
            printf("------ connection error ------\n");
            break;
        }
            
        case LWS_CALLBACK_CLOSED:
        {
            printf("------ connection closed ------\n");
            status = STATUS_CLOSED;
            break;
        }
            
        case LWS_CALLBACK_CLIENT_RECEIVE:
        {
            status = STATUS_RECEIVE;
            printf("------ receive msg from server: %s ------\n", in);
            break;
        }
            
        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            if (status == STATUS_ESTABLISED) {
                printf("------ try to send msg to server ------\n");
                char buffer[1024] = {0};
                strcpy(buffer + LWS_PRE, TEST_MSG);
                int send = lws_write(wsi, (unsigned char *)(buffer + LWS_PRE), strlen(TEST_MSG), LWS_WRITE_TEXT);
                printf("------ send %d ------\n", send);
                status = STATUS_HANDSHAKE;
            }
            break;
        }
            
        default:
            break;
    }
    return ret;
}

static struct lws_protocols protocols[] =
{
    {
        "dumb-increment-protocol,fake-nonexistant-protocol",
        callback_handler,
        0,
        1024 * 2
    },
    {
        NULL, NULL, 0, 0 /* end */
    }
};

static const struct lws_extension exts[] = {
    {
        "permessage-deflate",
        lws_extension_callback_pm_deflate,
        "permessage-deflate; client_max_window_bits"
    },
    {
        "deflate-frame",
        lws_extension_callback_pm_deflate,
        "deflate_frame"
    },
    {
        NULL, NULL, NULL /* terminator */
    }
};

int libwebsockets_test() {
    int ietf_version = -1;
    const char *prot;
    
    int use_ssl = USE_SSL;
    int port = PORT;
    const char *host = SERVER;
    
    if (use_ssl) {
        prot = PROTOCOL_WSS_ENABLE;
    } else {
        prot = PROTOCOL_WS_ENABLE;
    }
    
    int debug_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG | LLL_PARSER | LLL_HEADER | LLL_EXT | LLL_CLIENT | LLL_LATENCY;
    
    lws_set_log_level(debug_level, lwsl_emit_syslog);
    
    info = malloc(sizeof(struct lws_context_creation_info));
    memset(info, 0, sizeof(struct lws_context_creation_info));
    info->port = CONTEXT_PORT_NO_LISTEN;
    info->protocols = protocols;
    info->gid = -1;
    info->uid = -1;
    info->max_http_header_pool = 1;
    info->timeout_secs = 3;
    info->options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info->ssl_ca_filepath = CA_PATH;
    printf("Create lws context\n");
    lws_ctx = lws_create_context(info);
    if (lws_ctx == NULL) {
        printf("Creating libwebsocket context failed\n");
        if (info) {
            printf("Free lws_ctx_crt_info\n");
            free(info);
            info = NULL;
        }
        return -1;
    }
    printf("Create lws context success\n");
    conn = malloc(sizeof(struct lws_client_connect_info));
    memset(conn, 0, sizeof(struct lws_client_connect_info));
    conn->context = lws_ctx;
    conn->ssl_connection = use_ssl;
    conn->port = port;
    conn->path = "/websocket";
    conn->address = host;
    conn->host = conn->address;
    conn->origin = conn->address;
    conn->ietf_version_or_minus_one = ietf_version;
    conn->client_exts = exts;
    if (!strcmp(prot, "http") || !strcmp(prot, "https")) {
        conn->method = "GET";
    } else {
        
    }
    while (1) {
        
        if (g_lws == NULL) {
            conn->protocol = protocols[0].name;
            g_lws = lws_client_connect_via_info(conn);
            if (g_lws == NULL) {
                printf("------ connect error ------\n");
                break;
            }
        }
        lws_service(lws_ctx, 50);
        usleep(10 * 1000);
    }
    return 0;
}

int main()
{
    libwebsockets_test();
    while(1) {
        sleep(3);
    }
    return 0;
}
