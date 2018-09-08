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

#define REMOTE_IP       "127.0.0.1"
#define REMOTE_PORT     31000

typedef struct {
    ev_io io;
    int connected;
    struct remote* remote;
} remote_ctx;

typedef struct remote {
    int fd;
    
    buffer* read_buffer; 
    remote_ctx* read_ctx;

    remote_ctx* write_ctx;    
} remote_t;

int iot_client_boot();
int iot_client_write(buffer* b);