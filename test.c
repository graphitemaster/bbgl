#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include "library/bbgl.h"

static GLuint gVBO;
static GLint gColorLocation;
static float gColor[3];
SDL_Window *gWindow;
bbgl_t bbgl;

static volatile int gRunning = 1;

static void signal_handler(int signal) {
    (void)signal;
    gRunning = 0;
}

float random_float() {
    return (float)rand() / (RAND_MAX + 1.0f);
}

static void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform3f(gColorLocation, gColor[0], gColor[1], gColor[2]);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    //SDL_GL_SwapWindow(gWindow);
    bbgl_flush(&bbgl);
}

static void add_shader(GLuint program, const char *text, GLenum type) {
    GLuint object = glCreateShader(type);
    assert(object);

    const GLchar *p[1] = { text };
    GLint lengths[1] = { strlen(text) };
    glShaderSource(object, 1, p, lengths);

    glCompileShader(object);

    glAttachShader(program, object);
}

void create_vertex_array() {
    /* this is the global vertex array */
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

void create_vertex_buffer() {
    static const float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f
    };
    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
}

void compile_shaders() {
    static const char *vs =
    "#version 330\n"
    "layout (location = 0) in vec3 position;\n"
    "void main() {\n"
    "    gl_Position = vec4(0.5 * position.x,\n"
    "                       0.5 * position.y,\n"
    "                       position.z,\n"
    "                       1.0);\n"
    "}\n";

    static const char *fs =
    "#version 330\n"
    "out vec4 frag;\n"
    "uniform vec3 color;\n"
    "void main() {\n"
    "   frag = vec4(color, 1.0);\n"
    "}\n";

    GLuint program = glCreateProgram();
    assert(program);

    add_shader(program, vs, GL_VERTEX_SHADER);
    add_shader(program, fs, GL_FRAGMENT_SHADER);

    glLinkProgram(program);
    glValidateProgram(program);
    glUseProgram(program);

    gColorLocation = glGetUniformLocation(program, "color");
    printf("%d\n", gColorLocation);
}

int main() {
    srand(time(0));

    for (int i = 0; i < 3; i++)
        gColor[i] = random_float();

    signal(SIGINT, signal_handler);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    /* Create our window centered at 512x512 resolution */
    gWindow = SDL_CreateWindow("Hello Triangle",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               800,
                               600,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GL_CreateContext(gWindow);
    SDL_GL_SetSwapInterval(1);

    /* create a black box context */
    if (!bbgl_init(&bbgl)) {
        fprintf(stderr, "[bbgl] (client) failed to connect to black box\n");
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    //const char *vendor = (const char *)glGetString(GL_VENDOR);
    //const char *renderer = (const char *)glGetString(GL_RENDERER);
    //const char *version = (const char *)glGetString(GL_VERSION);
    //const char *shading = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

    //printf("vendor:     %s\n", vendor);
    //printf("renderer:   %s\n", renderer);
    //printf("version:    %s\n", version);
    //printf("shading:    %s\n", shading);

    create_vertex_array();
    create_vertex_buffer();
    compile_shaders();

    int dirs[3] = { +1, +1, +1 };
    while (gRunning) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                gRunning = 0;
                break;
            }
        }
        render();
        for (int i = 0; i < 3; i++) {
            int value = ((int)(gColor[i] * 255.0f)+dirs[i]);
            if (value > 255 || value <= 0)
                dirs[i] = -dirs[i];
            gColor[i] = value/255.0f;
        }
    }

    bbgl_destroy(&bbgl);
    return 0;
}
