#if !defined(IOT_CLIENT_H)
#define IOT_CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <ev.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h> // fcntl
#include <unistd.h> // close
#include <arpa/inet.h>

#include "common.h"
#include "bkv.h"

#define FRAME_HEAD 0XFEFE

typedef struct remote_ctx {
    ev_io io;
    struct remote* remote;
} remote_ctx_t;

typedef struct remote {
    int fd;
    
    int connected;

    buffer* read_buffer; 
    struct remote_ctx* read_ctx;

    struct remote_ctx* write_ctx;    
} remote_t;

typedef void iot_client_on_connect();
typedef void iot_client_on_disconnect();
typedef void iot_client_on_message_receive(bkv* b);

typedef struct iot_client_context {
    char* gateway_server_ip;
    int gateway_server_port;

	iot_client_on_connect* on_connect;
	iot_client_on_disconnect* on_disconnect;
	iot_client_on_message_receive* on_message_receive;
} iot_client_context_t;

// #define iot_client_context_initializer { NULL, 0, NULL, NULL, NULL }

int iot_client_boot(iot_client_context_t* ctx);
int iot_client_write(buffer* b);
int iot_client_destroy();

int iot_client_write_login_frame(char* device_id);



#endif