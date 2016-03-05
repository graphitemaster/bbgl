#ifndef BBGL_SHARED_COND_HDR
#define BBGL_SHARED_COND_HDR
typedef int bbgl_mutex_t;

typedef struct bbgl_cond_s bbgl_cond_t;

struct bbgl_cond_s {
    bbgl_mutex_t *m;
    int seq;
    int pad;
};

void bbgl_cond_init(bbgl_cond_t *cond);
void bbgl_cond_destroy(bbgl_cond_t *cond);
void bbgl_cond_signal(bbgl_cond_t *cond);
int bbgl_cond_wait(bbgl_cond_t *cond, bbgl_mutex_t *mutex);

#endif
