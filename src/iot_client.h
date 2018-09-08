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

#define REMOTE_IP       "127.0.0.1"
#define REMOTE_PORT     31000

#define DEVICE_ID       "test"

typedef struct remote_ctx {
    ev_io io;
    int connected;
    struct remote* remote;
} remote_ctx_t;

typedef struct remote {
    int fd;
    
    buffer* read_buffer; 
    struct remote_ctx* read_ctx;

    struct remote_ctx* write_ctx;    
} remote_t;

typedef void iot_client_on_connect();
typedef void iot_client_on_disconnect();
typedef void iot_client_on_message_receive(bkv* b);

typedef struct iot_client_context {
	iot_client_on_connect* on_connect;
	iot_client_on_disconnect* on_disconnect;
	iot_client_on_message_receive* on_message_receive;
} iot_client_context_t;

#define iot_client_context_initializer { NULL, NULL, NULL }

int iot_client_boot(iot_client_context_t* ctx);
int iot_client_write(buffer* b);
int iot_client_destroy();