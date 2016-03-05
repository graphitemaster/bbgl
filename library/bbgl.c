#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/wait.h>

#include <unistd.h>

#include "bbgl.h"

/* Singleton representing the black box context */
bbgl_t *bbgl_context = NULL;

int bbgl_init(bbgl_t *bbgl) {
    /* Create the shared memory region */
    int fd = shm_open("/bbgl", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        fprintf(stderr, "[bbgl] (client) failed to open shared memory\n");
        return -1;
    }

    size_t size = BBGL_ROUNDUP(BBGL_PAGESIZE, sizeof *bbgl->message);
    if (ftruncate(fd, size) == -1) {
        fprintf(stderr, "[bbgl] (client) failed to truncate shared memory\n");
        goto failure;
    }

    bbgl->message = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bbgl->message == MAP_FAILED) {
        fprintf(stderr, "[bbgl] (client) failed to map shared memory\n");
        goto failure;
    }

    bbgl_sem_init(&bbgl->message->client);
    bbgl_sem_init(&bbgl->message->server);

    pid_t pid = fork();
    if (pid == 0) {
        char *const argv[] = { "server", 0 };
        if (execv("./server/server", argv) == -1)
            fprintf(stderr, "[bbgl] (client) failed to launch server (%s)\n", strerror(errno));
        exit(1);
    } else {
        /* Check to make sure the black box is running */
        if (kill(pid, 0) == -1 && errno == ESRCH) {
            fprintf(stderr, "[bbgl] (client) server is not running\n");
            return 0;
        }
        printf("[bbgl] (client) server is running on `%d'\n", pid);
        bbgl->server = pid;
    }
    printf("[bbgl] (client) running ...\n");
    bbgl_context = bbgl;

    /* Send a context create message to find the current GLX context and
     * steal it so it becomes usable by the server.
     */
    bbgl->message->type = BBGL_MESSAGE_CONTEXT_CREATE;
    bbgl->message->asContextCreate.pid = getpid();
    bbgl_sync(bbgl);

    /* To indicate error we set pid to zero on failure */
    if (bbgl->message->asContextCreate.pid != getpid()) {
        /* Wait for server to shutdown */
        int status;
        pid_t terminate = waitpid(pid, &status, WUNTRACED);
        assert(terminate == pid);
        goto failure;
    }

    return 1;
failure:
    close(fd);
    return 0;
}

void bbgl_destroy(bbgl_t *bbgl) {
    /* Send the shutdown signal */
    bbgl->message->type = BBGL_MESSAGE_SHUTDOWN;
    bbgl_sync(bbgl);

    /* Destroy the semaphores */
    bbgl_sem_destroy(&bbgl->message->client);
    bbgl_sem_destroy(&bbgl->message->server);
}

void bbgl_sync(bbgl_t *bbgl) {
    bbgl_sem_post(&bbgl->message->client);
    bbgl_sem_wait(&bbgl->message->server);
}

void bbgl_flush(bbgl_t *bbgl) {
    bbgl->message->type = BBGL_MESSAGE_FLUSH;
    bbgl_sync(bbgl);
}
