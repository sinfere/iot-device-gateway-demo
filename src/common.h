#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_DEFAULT_SIZE 64

typedef struct {
    u_int8_t* buf;
    size_t size;
    size_t capacity;
} buffer;

buffer* buffer_new(u_int8_t* buf, size_t size);
buffer* buffer_alloc();
buffer* buffer_clone(buffer* b);
int buffer_grow(buffer* b, size_t capacity);
int buffer_append(buffer* b, u_int8_t* buf, size_t size);