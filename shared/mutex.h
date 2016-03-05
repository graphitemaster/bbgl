#ifndef BBGL_SHARED_MUTEX_HDR
#define BBGL_SHARED_MUTEX_HDR

typedef int bbgl_mutex_t;

void bbgl_mutex_init(bbgl_mutex_t *m);
void bbgl_mutex_destroy(bbgl_mutex_t *m);
int bbgl_mutex_lock(bbgl_mutex_t *m);
int bbgl_mutex_unlock(bbgl_mutex_t *m);


#endif
