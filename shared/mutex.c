#include "mutex.h"
#include "futex.h"

void bbgl_mutex_init(bbgl_mutex_t *mutex) {
    *mutex = 0;
}

void bbgl_mutex_destroy(bbgl_mutex_t *mutex) {
    (void)mutex;
}

int bbgl_mutex_lock(bbgl_mutex_t *mutex) {
    int c;
    for (int i = 0; i < 100; i++) {
        c = cmpxchg(mutex, 0, 1);
        if (c == 0)
            return 0;
        __asm__ __volatile__("pause" ::: "memory");
    }
    if (c == 1)
        c = xchg(mutex, 2);
    while (c) {
        futex(mutex, FUTEX_WAIT, 2, NULL, NULL, 0);
        c = xchg(mutex, 2);
    }
    return 0;
}

int bbgl_mutex_unlock(bbgl_mutex_t *mutex) {
    if (*mutex == 2)
        *mutex = 0;
    else if (xchg(mutex, 0) == 1)
        return 0;
    for (int i = 0; i < 200; i++) {
        if (*mutex)
            if (cmpxchg(mutex, 1, 2))
                return 0;
        __asm__ __volatile__("pause" ::: "memory");
    }
    futex(mutex, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}
