#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glext.h>

#include <stdio.h>

#include "context.h"

static int attributes[] = {
    GLX_RGBA,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    GLX_DOUBLEBUFFER,
    GLX_DEPTH_SIZE, 1,
    None
};

/* Only supports Core Profile GL 3.2 for now */
static int glattributes[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    None
};

static void bbgl_context_search(Window *found,
                                pid_t pid,
                                Atom atom,
                                Display *display,
                                Window window)
{
    Atom type;
    int format;
    unsigned long items;
    unsigned long after;
    unsigned char *id = 0;
    if (XGetWindowProperty(display,
                           window,
                           atom,
                           0,
                           1,
                           False,
                           XA_CARDINAL,
                           &type,
                           &format,
                           &items,
                           &after,
                           &id) == Success)
    {
        if (id) {
            if ((unsigned long)pid == *((unsigned long *)id))
                *found = window;
            XFree(id);
        }
    }

    /* Search for child windows */
    Window root;
    Window parent;
    Window *child;
    unsigned int children = 0;
    if (XQueryTree(display, window, &root, &parent, &child, &children) != 0) {
        for (unsigned int i = 0; i < children; i++)
            bbgl_context_search(found, pid, atom, display, child[i]);
    }
}

int bbgl_context_init(bbgl_context_t *context) {
    Display *display = XOpenDisplay(0);
    Window window = 0;
    Atom atom = XInternAtom(display, "_NET_WM_PID", True);
    bbgl_context_search(&window, context->pid, atom, display, XDefaultRootWindow(display));
    if (window == 0)
        return 0;

    int element = -1;
    GLXFBConfig *config = glXChooseFBConfig(display, DefaultScreen(display), NULL, &element);
    if (!config)
        return 0;
    XVisualInfo *info = glXChooseVisual(display, DefaultScreen(display), attributes);
    if (!info)
        return 0;
    GLXContext gl = glXCreateContextAttribsARB(display, config[0], NULL, GL_TRUE, glattributes);
    if (!context)
        return 0;

    glXMakeCurrent(display, window, gl);

    /* Disable VSync */
    PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
    PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA = NULL;
    PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI = NULL;
    glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
        glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(display, window, 0);
    } else {
        glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
            glXGetProcAddress((const GLubyte*)"glXSwapIntervalMESA");
        if (glXSwapIntervalMESA) {
            glXSwapIntervalMESA(0);
        } else {
            glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
                glXGetProcAddress((const GLubyte*)"glXSwapIntervalSGI");
            if (glXSwapIntervalSGI)
                glXSwapIntervalSGI(0);
        }
    }

    context->display = display;
    context->window = window;

    return 1;
}

void bbgl_context_flush(bbgl_context_t *context) {
    glXSwapBuffers(context->display, context->window);
}
