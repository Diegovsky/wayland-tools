#ifndef WL_UTILS_H
#define WL_UTILS_H

#include <stdint.h>
/* Change this if you need more than 255 elements. */
typedef uint8_t usize;

/* A dynamic array type that can only add elements to it's end */
typedef struct _dynvec_t dynvec_t;
typedef void (*dynvec_free_callback_t)(void *element);

dynvec_t *dynvec_new(usize typesize);
/* Returns an allocated memory address at the end of the dynvec */
void *dynvec_push(dynvec_t *vec);
void *dynvec_get(const dynvec_t *vec, usize index);
usize dynvec_length(const dynvec_t *vec);
void dynvec_free(dynvec_t *vec, dynvec_free_callback_t);
void *dynvec_next(dynvec_t *vec, void *element);

/* A more conventional wrapper aroudn `dynvec_push` */
#define dynvec_insert(vec, type, value) *(type *)dynvec_push(vec) = value;

#define dynvec_foreach(vec, element_type, element_name)                        \
    for (element_type *element_name = dynvec_get(vec, 0);                      \
         element_name != NULL; element_name = dynvec_next(vec, element_name))

#endif // WL_UTILS_H
