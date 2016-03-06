#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/socket.h>

#include "shared/list.h"
#include "shared/message.h"

#include "context.h"

/* The display server context */
static bbgl_context_t context;

extern void dispatch(bbgl_message_call_t *call);

typedef struct {
    char name[PATH_MAX];
    void *address;
    size_t size;
    int fd;
    bbgl_link_t link;
} mapping_t;

/* Linked list of all memory mappings */
static bbgl_list_t mappings;

int mapping_create(size_t size) {
    mapping_t *mapping = calloc(1, sizeof *mapping);
    if (!mapping)
        goto failure;
    mapping->size = BBGL_ROUNDUP(BBGL_PAGESIZE, size);
    if (snprintf(mapping->name, sizeof mapping->name, "/bbgl%zu", bbgl_list_size(&mappings) + 1) == -1)
        goto failure;
    mapping->fd = shm_open(mapping->name, O_CREAT | O_RDWR, 0666);
    if (mapping->fd == -1)
        goto failure;
    if (ftruncate(mapping->fd, mapping->size) == -1)
        goto failure;
    mapping->address = mmap(0, mapping->size, PROT_READ | PROT_WRITE, MAP_SHARED, mapping->fd, 0);
    if (mapping->address == MAP_FAILED)
        goto failure;
    bbgl_list_push_back(&mappings, &mapping->link);
    printf("[bbgl] (server) created memory mapping `%s'\n", mapping->name);
    return 0;

failure:
    if (mapping->fd)
        close(mapping->fd);
    free(mapping);
    return -1;
}

void mapping_destroy(mapping_t *mapping) {
    /* Close the shared memory handle and unmap the memory */
    close(mapping->fd);
    munmap(mapping->address, mapping->size);

    /* Remove from the linked list */
    bbgl_list_remove(&mappings, &mapping->link);

    /* Reclaim the memory for this mapping */
    free(mapping);
}

void *mapping_translate(size_t index, void *address) {
    /* Get the mapping for the given index */
    bbgl_link_t *link = bbgl_list_head(&mappings);
    for (size_t i = 0; i < index - 1; i++)
        link = bbgl_list_next(link);
    mapping_t *mapping = bbgl_list_ref(link, mapping_t, link);
    uintptr_t where = (uintptr_t)address;
    uintptr_t base = (uintptr_t)mapping->address;
    return (void *)(base + where);
}

typedef struct {
    bbgl_message_call_t call;
    bbgl_link_t link;
} record_t;

/* Linked list of all recorded calls */
static bbgl_list_t records;

int record_create(bbgl_message_call_t *call) {
    record_t *record = calloc(1, sizeof *record);
    if (!record)
        return -1;
    memcpy(&record->call, call, sizeof *call);
    bbgl_list_push_back(&records, &record->link);
    return 0;
}

void record_destroy(record_t *record) {
    free(record);
}

/* We can't send shared memory file descriptors as integers on the socket.
 * They need a level of translation since they don't mean the same in
 * different processes.
 */
static int send_fd(int socket, int fd) {
    struct msghdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof fd)];
    memset(buf, '\0', sizeof buf);

    struct iovec io = {
        .iov_base = "",
        .iov_len = 1
    };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof buf;

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof fd);

    memmove(CMSG_DATA(cmsg), &fd, sizeof fd);

    msg.msg_controllen = cmsg->cmsg_len;

    return sendmsg(socket, &msg, 0) >= 0;
}


int main(int argc, char **argv) {
    argc--;
    argv++;

    /* Expecting a file descriptor for the socket */
    if (argc == 0)
        return -1;

    int fd = atoi(argv[0]);

    /* Check if valid file descriptor */
    if (fcntl(fd, F_GETFD) == -1)
        return -1;

    /* Read the initialization message from the socket */
    char buf[4];
    read(fd, buf, sizeof buf);
    if (memcmp(buf, "init", 4)) {
        /* Not a valid message */
        return -1;
    }

    bbgl_list_init(&mappings);
    bbgl_list_init(&records);

    bbgl_message_t *message = NULL;

    /* Create shared memory mapping */
    size_t size = BBGL_ROUNDUP(BBGL_PAGESIZE, sizeof *message);
    char name[PATH_MAX] = "/bbglXXXXXX";
    mktemp(name);
    int shm = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm == -1) {
        fprintf(stderr, "[bbgl] (server) message channel open failure `%s' (%s)\n",
            name, strerror(errno));
        return -1;
    } else {
        printf("[bbgl] (server) message channel on shared memory `%d'\n", shm);
    }

    /* Allocate the backing memory for the shared memory mapping */
    if (ftruncate(shm, size) == -1) {
        fprintf(stderr, "[bbgl] (server) message channel allocation failed (%s)\n",
            strerror(errno));
        close(shm);
        return -1;
    }

    /* Now map it into the servers address space */
    message = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (message == MAP_FAILED) {
        fprintf(stderr, "[bbgl] (server) message channel mapping failed (%s)\n",
            strerror(errno));
        close(shm);
        return -1;
    }

    /* Inform the client of our shared memory file descriptor */
    send_fd(fd, shm);

    /* The server can now begin its' thing */
    printf("[bbgl] (server) running ...\n");
    for (;;) {
        /* Wait for the client */
        printf("[bbgl] (server) waiting for client ...\n");
        bbgl_sem_wait(&message->client);
        printf("[bbgl] (server) got request from client `%s'\n",
            bbgl_message_name(message));

        if (message->type == BBGL_MESSAGE_SHUTDOWN) {
            printf("[bbgl] (server) shutting down ...\n");
            bbgl_sem_post(&message->server);
            break;
        } else if (message->type == BBGL_MESSAGE_CALL) {
            if (record_create(&message->asCall) == -1) {
                fprintf(stderr, "[bbgl] (server) failed to record call `%s'\n",
                    message->asCall.name);
                bbgl_sem_post(&message->server);
                break;
            } else {
                printf("[bbgl] (server) recorded call `%s'\n",
                    message->asCall.name);
            }
        } else if (message->type == BBGL_MESSAGE_FLUSH) {
            for (bbgl_link_t *link = bbgl_list_head(&records); link; ) {
                record_t *record = bbgl_list_ref(link, record_t, link);
                bbgl_link_t *next = bbgl_list_next(link);
                bbgl_list_remove(&records, link);
                dispatch(&record->call);
                record_destroy(record);
                link = next;
            }
            bbgl_context_flush(&context);
        } else if (message->type == BBGL_MESSAGE_MAPPING_CREATE) {
            if (mapping_create(message->asCreate.size) == -1) {
                fprintf(stderr, "[bbgl] (server) failed to create mapping\n");
                bbgl_sem_post(&message->server);
                break;
            } else {
                bbgl_link_t *link = bbgl_list_tail(&mappings);
                mapping_t *mapping = bbgl_list_ref(link, mapping_t, link);
                message->asCreate.size = mapping->size;
                message->asCreate.index = bbgl_list_size(&mappings);
                /* Send the file descriptor through the socket to get the
                 * correct translation */
                send_fd(fd, mapping->fd);
                //bbgl_sem_post(&message->server);
                //break;
            }
        } else if (message->type == BBGL_MESSAGE_MAPPING_DESTROY) {
            size_t index = message->asDestroy.index;
            if (index < bbgl_list_size(&mappings)) {
                bbgl_link_t *link = bbgl_list_head(&mappings);
                for (size_t i = 0; i < index; i++)
                    link = bbgl_list_next(link);
                mapping_t *mapping = bbgl_list_ref(link, mapping_t, link);
                bbgl_list_remove(&mappings, link);
                mapping_destroy(mapping);
            }
        } else if (message->type == BBGL_MESSAGE_CONTEXT_CREATE) {
            context.pid = message->asContextCreate.pid;
            if (!bbgl_context_init(&context)) {
                fprintf(stderr, "[bbgl] (server) failed to find client window\n");
                message->asContextCreate.pid = 0;
                bbgl_sem_post(&message->server);
                break;
            } else {
                printf("[bbgl] (server) found client window\n");
            }
        }

        /* Let the client know we're done */
        bbgl_sem_post(&message->server);
    }

    /* Free resources */
    munmap(message, size);
    close(fd);

    /* Tear down in reverse order */
    for (bbgl_link_t *link = bbgl_list_tail(&mappings); link; link = bbgl_list_prev(link)) {
        mapping_t *mapping = bbgl_list_ref(link, mapping_t, link);
        mapping_destroy(mapping);
    }

    /* Tear down recordings in reverse order as well */
    for (bbgl_link_t *link = bbgl_list_tail(&records); link; link = bbgl_list_prev(link)) {
        record_t *record = bbgl_list_ref(link, record_t, link);
        record_destroy(record);
    }

    return 0;
}
