#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    u_int8_t* buf;
    size_t size;
} buffer;

typedef struct {
    int is_string_key;
    buffer* key;
    buffer* value;
} kv;

kv* kv_new_from_number_key(u_int64_t key, u_int8_t* value, size_t value_size);
kv* kv_new_from_string_key(char* key, u_int8_t* value, size_t value_size);

buffer* kv_pack(kv* t);

#define KV_UNPACK_RESULT_CODE_EMPTY_BUF             1
#define KV_UNPACK_RESULT_CODE_DECODE_LENGTH_FAIL    -1
#define KV_UNPACK_RESULT_CODE_BUF_NOT_ENOUGH        -2
#define KV_UNPACK_RESULT_CODE_WRONG_KEY_SIZE        -3

typedef struct {
    int code;
    kv* kv;
    size_t size;
} kv_unpack_result;

kv_unpack_result* kv_unpack(u_int8_t* buf, size_t buf_size);

void kv_free(kv* t);
void kv_free_buffer(buffer* b);
void kv_free_unpack_result(kv_unpack_result *r);

char* kv_get_string_key(kv* t);
u_int64_t kv_get_number_key(kv* t);








typedef struct {
    kv** kvs;
    size_t size;
} bkv;

bkv* bkv_new();
void bkv_add(bkv* b, kv* t);
void bkv_add_by_number_key(bkv* b, u_int64_t key, u_int8_t* value, size_t value_size);
void bkv_add_by_string_key(bkv* b, char* key, u_int8_t* value, size_t value_size);

buffer* bkv_pack(bkv* b);

typedef struct {
    int code;
    bkv* bkv;
    size_t size;
} bkv_unpack_result;

bkv_unpack_result* bkv_unpack(u_int8_t* buf, size_t buf_size);

void bkv_free(bkv* b);
void bkv_free_unpack_result(bkv_unpack_result *r);







void dump_buffer(char* name, buffer* b);
void dump_kv(kv* t);
void dump_bkv(bkv* b);
