#ifndef BBGL_SHARED_MESSAGE_HDR
#define BBGL_SHARED_MESSAGE_HDR
#include <stdint.h>
#include <stddef.h>

#include <sys/types.h>

#include "sem.h"

typedef enum bbgl_message_type_e bbgl_message_type_t;
typedef union bbgl_message_param_u bbgl_message_param_t;
typedef struct bbgl_message_call_s bbgl_message_call_t;
typedef struct bbgl_message_s bbgl_message_t;

enum bbgl_message_type_e {
    BBGL_MESSAGE_MAPPING_CREATE,
    BBGL_MESSAGE_MAPPING_DESTROY,
    BBGL_MESSAGE_CONTEXT_CREATE,
    BBGL_MESSAGE_CONTEXT_DESTROY,
    BBGL_MESSAGE_CALL,
    BBGL_MESSAGE_FLUSH,
    BBGL_MESSAGE_SHUTDOWN
};

union bbgl_message_param_u {
    unsigned char asBoolean;    /* . */
    char asByte;                /* b */
    unsigned char asUByte;      /* B */
    short asShort;              /* s */
    unsigned short asUShort;    /* S */
    int asInt;                  /* i */
    unsigned int asUInt;        /* U */
    int64_t asInt64;            /* l */
    uint64_t asUInt64;          /* L */
    unsigned int asSizeI;       /* z */
    unsigned int asEnum;        /* e */
    intptr_t asIntPtr;          /* p */
    uintptr_t asSizeIPtr;       /* Z */
    uintptr_t asSync;           /* O */
    unsigned int asBitfield;    /* : */
    unsigned short asHalf;      /* h */
    float asFloat;              /* f */
    float asClampF;             /* F */
    double asDouble;            /* d */
    double asClampD;            /* D */
    void *asPointer;            /* * */
};

struct bbgl_message_call_s {
    char name[255];
    char spec[17];
    /* For functions which we expect immediate return values from;
     * e.g: glCreateProgram, glCreateShader the return value is stored
     * in params[0].
     */
    bbgl_message_param_t params[16];
    /* The mapping index for auxiliary data */
    size_t mapping;
};

struct bbgl_message_s {
    bbgl_sem_t client;
    bbgl_sem_t server;
    bbgl_message_type_t type;
    union {
        /* BBGL_MESSAGE_CALL */
        bbgl_message_call_t asCall;
        /* BBGL_MESSAGE_MAPPING_DESTROY */
        struct {
            size_t index;
        } asDestroy;
        /* BBGL_MESSAGE_MAPPING_CREATE */
        struct {
            size_t size;
            /* After server has created mapping it stores the mapping
             * index here so the client library can use it.
             */
            size_t index;
        } asCreate;
        /* BBGL_MESSAGE_CONTEXT_CREATE */
        struct {
            pid_t pid;
        } asContextCreate;
    };
};

/* Returns a human-readable description of a message */
const char *bbgl_message_name(const bbgl_message_t *const message);

/* Fill out `message' with the appropriate information to record a
 * GL function call.
 *
 * Parameters:
 *  - mapping   which memory mapping to expect auxiliary data in
 *  - name      name of the GL function to record
 *  - spec      a string-literal of the function call specification
 *  - ...       arguments for the function
 */
void bbgl_message_call(bbgl_message_t *message,
                       size_t mapping,
                       const char *name,
                       const char *spec,
                       ...);

#define BBGL_PAGESIZE \
    4096

#define BBGL_ROUNDUP(N, S) \
    ((((N) + (S) - 1) / (S)) * (S))

#endif
