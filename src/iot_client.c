#include "iot_client.h"

#include <pthread.h>

static pthread_t ntid;
static remote_t* remote;
static iot_client_context_t* client_ctx;

int setnonblock(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

static void on_remote_read(EV_P_ ev_io *w, int revents)
{
    printf("on_remote_read\r\n");
}

static void on_remote_write(EV_P_ ev_io *w, int revents)
{
    remote_ctx_t *write_ctx = (remote_ctx_t *)w;
    remote_t* remote = write_ctx->remote;

    ev_io_stop(EV_A_ &remote->write_ctx->io);    

    LOGI("[iot] on_remote_write: connected");
    if (client_ctx->on_connect != NULL) {
        client_ctx->on_connect();
    }    
}

static void connect_to_remote(EV_P) 
{
    int remote_fd;
  
    if (-1 == (remote_fd = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("socket");
        exit(1);
    }

    if (-1 == setnonblock(remote_fd)) {
        perror("setnonblock");
        exit(EXIT_FAILURE);
    }

    remote->fd = remote_fd;

    ev_io_init(&remote->read_ctx->io, on_remote_read, remote->fd, EV_READ);
    ev_io_start(EV_A_ &remote->read_ctx->io);

    ev_io_init(&remote->write_ctx->io, on_remote_write, remote->fd, EV_WRITE);
    ev_io_start(EV_A_ &remote->write_ctx->io);

    struct sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;  
    inet_pton(AF_INET, REMOTE_IP, &remote_addr.sin_addr);
    remote_addr.sin_port = htons(REMOTE_PORT);
    // remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int r = connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if (r == -1 && errno != EINPROGRESS) {
        perror("connect");
        return;
    }

    LOGI("[iot] connecting");
}

static remote_t* new_remote() 
{
    remote_t* remote = malloc(sizeof(remote_t));
    memset(remote, 0, sizeof(remote_t));

    remote->read_ctx = malloc(sizeof(remote_ctx_t));
    memset(remote->read_ctx, 0, sizeof(remote_ctx_t));

    remote->read_buffer = buffer_alloc();
    remote->read_ctx->remote = remote;

    remote->write_ctx = malloc(sizeof(remote_ctx_t));
    memset(remote->write_ctx, 0, sizeof(remote_ctx_t));    

    remote->write_ctx->remote = remote;
  
    return remote;
}

void* start (void* arg)
{
    setnonblock(0);
    remote = new_remote();

    struct ev_loop *loop = ev_loop_new(0);
    connect_to_remote(loop);
    ev_loop(loop, 0);

    return NULL;
}

int iot_client_boot(iot_client_context_t* ctx) 
{
    client_ctx = ctx;

    int err;
    err = pthread_create(&ntid, NULL, start, NULL);
    if (err != 0) {
        LOGE("can't create thread: %s\n", strerror(err));
        return err;
    }
    return 0;
}

int iot_client_write(buffer* b) 
{
    int ret = send(remote->fd, b->buf, b->size, 0);
    if (ret == -1) {
        perror("send");
        return ret;
    }

    return 0;
}

int iot_client_destroy() {
    return 0;
}