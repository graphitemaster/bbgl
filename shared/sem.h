#ifndef BBGL_SHARED_SEM_HDR
#define BBGL_SHARED_SEM_HDR
#include "cond.h"
#include "mutex.h"

typedef struct bbgl_sem_s bbgl_sem_t;

struct bbgl_sem_s {
    int val;
    bbgl_mutex_t mutex;
    bbgl_cond_t cond;
};

void bbgl_sem_init(bbgl_sem_t *sem);
void bbgl_sem_destroy(bbgl_sem_t *sem);
void bbgl_sem_post(bbgl_sem_t *sem);
void bbgl_sem_wait(bbgl_sem_t *sem);

#endif
