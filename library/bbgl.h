#ifndef BBGL_LIBRARY_HDR
#define BBGL_LIBRARY_HDR
#include <sys/types.h> /* pid_t */

#include "shared/message.h"
#include "gl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bbgl_s bbgl_t;

struct bbgl_s {
    bbgl_message_t *message;
    pid_t server;
    int pair[2];
};

int bbgl_init(bbgl_t *bbgl);
void bbgl_destroy(bbgl_t *bbgl);

void bbgl_sync(bbgl_t *bbgl);
void bbgl_flush(bbgl_t *bbgl);


#ifdef __cplusplus
}
#endif

#endif
