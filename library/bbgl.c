#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#include "bbgl.h"

#if 0
#define DEBUG(...) \
    fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) do { } while(0)
#endif

/* Singleton representing the black box context */
bbgl_t *bbgl_context = NULL;

/* Receive a file descriptor from the socket */
int read_fd(int socket) {
    struct msghdr msg = { 0 };
    char mbuffer[1];
    struct iovec io = {
        .iov_base = mbuffer,
        .iov_len = sizeof mbuffer
    };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char cbuffer[256];
    msg.msg_control = cbuffer;
    msg.msg_controllen = sizeof cbuffer;

    if (recvmsg(socket, &msg, 0) < 0)
        return -1;

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    int fd = -1;
    memmove(&fd, CMSG_DATA(cmsg), sizeof fd);
    return fd;
}

int bbgl_init(bbgl_t *bbgl) {
    /* Initialization works by sending the server some data on the socket
     * which we open. The server then wakes and creates the shared memory
     * for our message channel, passing along the file descriptor for the
     * shared memory.
     *
     * We communicate the socket pair to the server during execv by
     * passing the socket file descriptor as an argument in argv.
     */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, bbgl->pair) != 0)
        return -1;

    pid_t pid = fork();
    if (pid == 0) {
        /* Close the parent file descriptor on the socket pair */
        close(bbgl->pair[0]);
        /* Format the arguments for the server passing in the child
         * socket file descriptor */
        char fd[28];
        snprintf(fd, sizeof fd, "%d", bbgl->pair[1]);
        char *const argv[] = { "server",  fd, 0 };
        if (execv("./server/server", argv) == -1)
            DEBUG("[bbgl] (client) failed to launch server (%s)\n", strerror(errno));
        exit(1);
    } else {
        /* Close the child socket file descriptor */
        close(bbgl->pair[1]);
        /* Check to make sure the server is running */
        if (kill(pid, 0) == -1 && errno == ESRCH) {
            DEBUG("[bbgl] (client) server is not running\n");
            return 0;
        }
        DEBUG("[bbgl] (client) server is running on `%d'\n", pid);
        bbgl->server = pid;
    }
    DEBUG("[bbgl] (client) running ...\n");
    bbgl_context = bbgl;

    /* Now send the initialization sequence on the socket so the server
     * can create our shared memory channel for the messages.
     */
    write(bbgl->pair[0], "init", 4);

    /* We now block in read waiting for the file descriptor for the shared
     * memory the server has created for us.
     */
    int fd = read_fd(bbgl->pair[0]);

    /* Check to make sure that is a valid file descriptor */
    if (fcntl(fd, F_GETFD) == -1) {
        /* Something didn't go as planned */
        DEBUG("[bbgl] (client) invalid mapping `%d' for message channel\n", fd);
        return 0;
    }

    DEBUG("[bbgl] (client) message channel on shared memory `%d'\n", fd);

    /* We now have the shared memory file descriptor from the server */
    size_t size = BBGL_ROUNDUP(BBGL_PAGESIZE, sizeof *bbgl->message);

    bbgl->message = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bbgl->message == MAP_FAILED) {
        DEBUG("[bbgl] (client) failed to map message channel (%s)\n", strerror(errno));
        goto failure;
    }
    DEBUG("[bbgl] (client) mapped message channel\n");

    /* Initialize the semaphores */
    bbgl_sem_init(&bbgl->message->client);
    bbgl_sem_init(&bbgl->message->server);

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
