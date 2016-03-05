#ifndef BBGL_SERVER_CONTEXT_HDR
#define BBGL_SERVER_CONTEXT_HDR
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef struct bbgl_context_s bbgl_context_t;

struct bbgl_context_s {
    Display *display;
    Window window;
    pid_t pid;
};

int bbgl_context_init(bbgl_context_t *context);
void bbgl_context_flush(bbgl_context_t *context);

#endif
