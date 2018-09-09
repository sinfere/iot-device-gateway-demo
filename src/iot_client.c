#include "iot_client.h"

#include <pthread.h>

static pthread_t ntid;
static remote_t* remote;
static iot_client_context_t* client_ctx;

// handler
static int setnonblock(int fd);
static void on_remote_read(EV_P_ ev_io *w, int revents);
static void on_remote_write(EV_P_ ev_io *w, int revents);

// connect & disconnect
static remote_t* new_remote();
static void connect_to_remote(EV_P);
static void close_remote(EV_P);

// frame pack 
static u_int16_t crc16(u_int8_t *p, u_int16_t length);
static buffer* frame_pack(buffer* b);

// parse related
#define PARSE_RESULT_CODE_OK             0
#define PARSE_RESULT_CODE_INVALID        1
#define PARSE_RESULT_CODE_INCOMPLETE     2
typedef struct parse_result {
    int code;
    bkv* bkv;
    buffer* pending_parse_buffer;
} parse_result_t;
static parse_result_t* parse(buffer* b);
static int search_frame_head(buffer* b);
static u_int16_t get_uint16_number(u_int8_t* p);

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
        LOGE("[iot] can't create thread: %s\n", strerror(err));
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

static void close_remote(EV_P) {
    remote->connected = -1;
    ev_io_stop(EV_A_ &remote->read_ctx->io);
}

static int setnonblock(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

static void on_remote_read(EV_P_ ev_io *w, int revents)
{
    LOGI("[iot] on_remote_read");

    buffer* b = remote->read_buffer;
    int r = recv(remote->fd, b->buf + b->size, b->capacity - b->size, 0);
    if (r == 0) {
        // connection closed
        LOGI("[iot] remote connnection closed");
        close_remote(EV_A);
        return;
    } else if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // no data
            // continue to wait for recv
            return;
        } else {
            perror("recv");
            close_remote(EV_A);
            return;
        }
    } 

    bkv* mb = NULL;

    LOGI("[iot] read %d", r);

    b->size = r;

    parse_result_t *pr = parse(b);
    if (pr->code == PARSE_RESULT_CODE_OK) {
        mb = pr->bkv;
    } else {
        free(b);
        if (pr->code == PARSE_RESULT_CODE_INCOMPLETE) {
            remote->read_buffer = pr->pending_parse_buffer;
        } else {
            b->size = 0;
            memset(b->buf, 0, b->capacity * sizeof(u_int8_t));
        }
    }
    free(pr);

    if (client_ctx->on_message_receive != NULL && mb != NULL) {
        client_ctx->on_message_receive(mb);
    }  
}



static void on_remote_write(EV_P_ ev_io *w, int revents)
{
    remote_ctx_t *write_ctx = (remote_ctx_t *)w;
    remote_t* remote = write_ctx->remote;

    ev_io_stop(EV_A_ &remote->write_ctx->io);      

    if (remote->connected < 0) {
        return;
    }

    remote->connected = 1;

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
    inet_pton(AF_INET, client_ctx->gateway_server_ip, &remote_addr.sin_addr);
    remote_addr.sin_port = htons(client_ctx->gateway_server_port);
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

    remote->read_buffer = buffer_alloc(2048);
    remote->read_ctx->remote = remote;

    remote->write_ctx = malloc(sizeof(remote_ctx_t));
    memset(remote->write_ctx, 0, sizeof(remote_ctx_t));    

    remote->write_ctx->remote = remote;
  
    return remote;
}




int iot_client_write_login_frame(char* device_id) {
    bkv* mb = bkv_new();
    u_int8_t type[1] = {1};
    bkv_add_by_string_key(mb, "type", type, 1);
    bkv_add_by_string_key(mb, "device_id", (u_int8_t*)device_id, strlen(device_id));
    buffer* b = bkv_pack(mb);
    buffer* f = frame_pack(b);
    
    iot_client_write(f);

    bkv_free(mb);
    buffer_free(b);
    buffer_free(f);
    
    return 0;
}








static int search_frame_head(buffer* b) {
    int i = 0;

    u_int8_t h1 = (FRAME_HEAD >> 8) & 0xFF;
    u_int8_t h2 = FRAME_HEAD & 0xFF;
    for (i = 0; i < b->size - 1; i++) {
        u_int8_t b1 = *(b->buf + i);
        u_int8_t b2 = *(b->buf + i + 1);
        if (b1 == h1 && b2 == h2) {
            return i;
        }
    }

    return -1;
}

static u_int16_t get_uint16_number(u_int8_t* p) {
    u_int16_t n = 0;

    n = *p;
    n <<= 8;
    n |= *(p + 1);

    return n;
}

static parse_result_t* parse(buffer* b) {
    parse_result_t *r = malloc(sizeof(parse_result_t));
    memset(r, 0, sizeof(parse_result_t));

    LOGI("[iot] parsing buffer");
    dump_buffer("p", b);

    int h = search_frame_head(b);
    LOGI("[iot] parse: head: %d", h);
    if (b < 0) {
        LOGE("[iot] parse error: head not found");
        r->code = PARSE_RESULT_CODE_INVALID;
        return r;
    }

    if (b->size < h + 6) {
        LOGE("[iot] parse error: too short < 6");
        r->code = PARSE_RESULT_CODE_INCOMPLETE;
        r->pending_parse_buffer = buffer_clone(b);
        return r;
    }

    int len = get_uint16_number(b->buf + h + 2);
    LOGI("[iot] parse: len: %d", len);
    if (b->size < h + 4 + len) {
        LOGI("[iot] parse: len not enough");
        r->code = PARSE_RESULT_CODE_INCOMPLETE;
        r->pending_parse_buffer = buffer_clone(b);
        return r;
    }

    u_int16_t checksum = get_uint16_number(b->buf + h + 4);
    u_int16_t calculated_checsum = crc16(b->buf + h + 6, len - 2);
    if (checksum == calculated_checsum) {
        bkv_unpack_result* bkr = bkv_unpack(b->buf + h + 6, len - 2);
        if (bkr->code == 0) {
            LOGI("[iot] parse: ok");
            r->code = PARSE_RESULT_CODE_OK;
            r->bkv = bkr->bkv;
            free(bkr);
            return r;
        } else {
            LOGI("[iot] parse: unpack bkv fail: %d", bkr->code);
            r->code = PARSE_RESULT_CODE_INVALID;
            free(bkr);
            return r;
        }
    } else {
        LOGI("[iot] parse: checksum not equal: %d - %d", checksum, calculated_checsum);
        r->code = PARSE_RESULT_CODE_INVALID;
        return r;
    }
}

static buffer* frame_pack(buffer* b) {
    size_t total_len = b->size + 2 + 2 + 2;
    u_int8_t* buf = malloc(total_len * sizeof(u_int8_t));
    buf[0] = ((FRAME_HEAD & 0xFF00) >> 8) & 0xFF;
    buf[1] = FRAME_HEAD & 0xFF;

    size_t len = b->size + 2;
    buf[2] = ((len & 0xFF00) >> 8) & 0xFF;
    buf[3] = len & 0xFF;

    u_int16_t crc = crc16(b->buf, b->size);
    // LOGI("[iot] checksum = %x", crc);
    buf[4] = ((crc & 0xFF00) >> 8) & 0xFF;
    buf[5] = crc & 0xFF;
    
    memcpy(buf + 6, b->buf, b->size);
    return buffer_new(buf, total_len);
}

static u_int16_t crc16(u_int8_t *p, u_int16_t length)
{
    u_int8_t i;
    u_int16_t data;
    u_int16_t crc = 0xffff;
    u_int16_t POLY = 0x8408;

    if (length == 0) {
        return (~crc);
    }

    do {
        data = 0xff & *p++;
        for (i = 0 ; i < 8; i++) {
            if ((crc & 0x0001) ^ (data & 0x0001)) {
                crc = (crc >> 1) ^ POLY;
            } else {
                crc >>= 1;
            }
            data >>= 1;
        }
    } while (--length);

    return ~crc;
}