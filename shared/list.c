#include "list.h"

void bbgl_list_init(bbgl_list_t *list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void bbgl_list_push_front(bbgl_list_t *list, bbgl_link_t *link) {
    if (list->head) {
        list->head->prev = link;
        link->prev = NULL;
        link->next = list->head;
        list->head = link;
    } else {
        list->head = link;
        list->tail = link;
        link->next = NULL;
        link->prev = NULL;
    }
    list->size++;
}

void bbgl_list_push_back(bbgl_list_t *list, bbgl_link_t *link) {
    if (list->tail) {
        list->tail->next = link;
        link->prev = list->tail;
        link->next = NULL;
        list->tail = link;
    } else {
        list->head = link;
        list->tail = link;
        link->next = NULL;
        link->prev = NULL;
    }
    list->size++;
}

void bbgl_list_insert_after(bbgl_list_t *list, bbgl_link_t *after, bbgl_link_t *link) {
    link->next = after->next;
    link->prev = after;
    if (after->next)
        after->next->prev = link;
    else
        list->tail = link;
    after->next = link;
    list->size++;
}

void bbgl_list_insert_before(bbgl_list_t *list, bbgl_link_t *before, bbgl_link_t *link) {
    link->prev = before->prev;
    link->next = before;
    if (before->prev)
        before->prev->next = link;
    else
        list->head = link;
    before->prev = link;
    list->size++;
}

bbgl_link_t *bbgl_list_pop_front(bbgl_list_t *list) {
    bbgl_link_t *link = list->head;
    if (!link)
        return NULL;
    if (link->next)
        link->next->prev = link->prev;
    if (link->prev)
        link->prev->next = link->next;
    if (list->head == link)
        list->head = link->next;
    if (list->tail == link)
        list->tail = link->prev;
    list->size--;
    return link;
}

bbgl_link_t *bbgl_list_pop_back(bbgl_list_t *list) {
    bbgl_link_t *link = list->tail;
    if (!link)
        return NULL;
    if (link->next)
        link->next->prev = link->prev;
    if (link->prev)
        link->prev->next = link->next;
    if (list->head == link)
        list->head = link->next;
    if (list->tail == link)
        list->tail = link->prev;
    list->size--;
    return link;
}

void bbgl_list_remove(bbgl_list_t *list, bbgl_link_t *link) {
    if (!link)
        return;
    if (link->next)
        link->next->prev = link->prev;
    if (link->prev)
        link->prev->next = link->next;
    if (list->head == link)
        list->head = link->next;
    if (list->tail == link)
        list->tail = link->prev;
    list->size--;
}

size_t bbgl_list_size(const bbgl_list_t *const list) {
    return list->size;
}

bbgl_link_t *bbgl_list_head(const bbgl_list_t *const list) {
    return list->head;
}

bbgl_link_t *bbgl_list_tail(const bbgl_list_t *const list) {
    return list->tail;
}

bbgl_link_t *bbgl_list_next(const bbgl_link_t *const link) {
    return link->next;
}

bbgl_link_t *bbgl_list_prev(const bbgl_link_t *const link) {
    return link->prev;
}
