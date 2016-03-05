#ifndef BBGL_SHARED_LIST_HDR
#define BBGL_SHARED_LIST_HDR
#include <stddef.h>

typedef struct bbgl_link_s bbgl_link_t;
typedef struct bbgl_list_s bbgl_list_t;

struct bbgl_link_s {
    bbgl_link_t *prev;
    bbgl_link_t *next;
};

struct bbgl_list_s {
    size_t size;
    bbgl_link_t *head;
    bbgl_link_t *tail;
};

void bbgl_list_init(bbgl_list_t *list);
void bbgl_list_remove(bbgl_list_t *list, bbgl_link_t *link);
void bbgl_list_push_front(bbgl_list_t *list, bbgl_link_t *link);
void bbgl_list_push_back(bbgl_list_t *list, bbgl_link_t *link);
void bbgl_list_insert_before(bbgl_list_t *list, bbgl_link_t *before, bbgl_link_t *link);
void bbgl_list_insert_after(bbgl_list_t *list, bbgl_link_t *after, bbgl_link_t *link);

bbgl_link_t *bbgl_list_pop_front(bbgl_list_t *list);
bbgl_link_t *bbgl_list_pop_back(bbgl_list_t *list);

size_t bbgl_list_size(const bbgl_list_t *const list);

bbgl_link_t *bbgl_list_head(const bbgl_list_t *const list);
bbgl_link_t *bbgl_list_tail(const bbgl_list_t *const list);

bbgl_link_t *bbgl_list_next(const bbgl_link_t *const link);
bbgl_link_t *bbgl_list_prev(const bbgl_link_t *const link);

#define bbgl_list_ref(ELEMENT, TYPE, MEMBER) \
    ((TYPE *)((unsigned char *)(ELEMENT) - offsetof(TYPE, MEMBER)))

#endif
