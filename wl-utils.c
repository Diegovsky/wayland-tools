#include "wl-utils.h"
#include "malloc.h"
#include <assert.h>
#include <string.h>

typedef uint8_t byte;

// TODO: change dynvec into a vec of pointers

struct _dynvec_t {
    usize capacity, length, type_size; 
    void* buf;
};

dynvec_t* dynvec_new(usize typesize) {
    dynvec_t* vec = calloc(1, sizeof(struct _dynvec_t));
    // Reserves one element upfront.
    vec->buf = malloc(typesize);
    vec->capacity = 1;
    vec->length = 0;
    vec->type_size = typesize;
    return vec;
}

void* dynvec_push(dynvec_t* vec) {
    if(vec->length >= vec->capacity) {
        // Doubles capacity everytime it runs out.
        usize new_capacity = vec->capacity*2;
        vec->buf = realloc(vec->buf, new_capacity*vec->type_size);
        assert(vec->buf != NULL);
        vec->capacity = new_capacity;
    }
    vec->length += 1;
    return dynvec_get(vec, vec->length-1);
}

void* dynvec_next(dynvec_t *vec, void *element) {
    usize index = ( (byte*) element - (byte*) vec->buf )/vec->type_size;
    return dynvec_get(vec, index+1); 
}

usize dynvec_length(const dynvec_t *vec) {
    return vec->length;
}

void* dynvec_get(const dynvec_t *vec, usize index)
{
    if(index >= vec->length) {
        return NULL;
    }
    // Since void pointer arithmetic is a GNU extension, we cast it to a
    // pointer of byte first.
    void* slot = (byte*) vec->buf + vec->type_size * index;
    return slot;
}

void dynvec_free(dynvec_t *vec, dynvec_free_callback_t element_free) {
    dynvec_foreach(vec, void, element) {
        element_free(element);
    }
    free(vec);
}
