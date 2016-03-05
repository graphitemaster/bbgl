#include <limits.h>
#include <stddef.h>

#include "cond.h"
#include "mutex.h"
#include "futex.h"

void bbgl_cond_init(bbgl_cond_t *cond) {
    cond->m = NULL;
    cond->seq = 0;
}

void bbgl_cond_destroy(bbgl_cond_t *cond) {
    (void)cond;
}

void bbgl_cond_signal(bbgl_cond_t *cond) {
    atomic_add(&cond->seq, 1);
    futex(&cond->seq, FUTEX_WAKE, 1, NULL, NULL, 0);
}

int bbgl_cond_wait(bbgl_cond_t *cond, bbgl_mutex_t *mutex) {
    int seq = cond->seq;
    if (cond->m != mutex) {
        if (cond->m)
            return -1;
        (void)cmpxchg(&cond->m, NULL, mutex);
        if (cond->m != mutex)
            return -1;
    }
    bbgl_mutex_unlock(mutex);
    futex(&cond->seq, FUTEX_WAIT, seq, NULL, NULL, 0);
    while (xchg(mutex, 2))
        futex(mutex, FUTEX_WAIT, 2, NULL, NULL, 0);
    return 0;
}
