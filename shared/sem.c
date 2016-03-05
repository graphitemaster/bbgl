#include "sem.h"

void bbgl_sem_init(bbgl_sem_t *sem) {
    sem->val = 0;
    bbgl_cond_init(&sem->cond);
    bbgl_mutex_init(&sem->mutex);
}

void bbgl_sem_destroy(bbgl_sem_t *sem) {
    bbgl_cond_destroy(&sem->cond);
    bbgl_mutex_destroy(&sem->mutex);
}

void bbgl_sem_wait(bbgl_sem_t *sem) {
    bbgl_mutex_lock(&sem->mutex);
    while (sem->val == 0)
        bbgl_cond_wait(&sem->cond, &sem->mutex);
    sem->val--;
    bbgl_mutex_unlock(&sem->mutex);
}

void bbgl_sem_post(bbgl_sem_t *sem) {
    bbgl_mutex_lock(&sem->mutex);
    sem->val++;
    bbgl_cond_signal(&sem->cond);
    bbgl_mutex_unlock(&sem->mutex);
}
