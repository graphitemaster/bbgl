#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "bbgl.h"

extern bbgl_t *bbgl_context;

/* TODO: a better memory allocator aroung this mapping system */
typedef struct {
    size_t index;
    void *address;
} mapping_t;

static void mapping_release(const mapping_t *const mapping) {
    bbgl_context->message->type = BBGL_MESSAGE_MAPPING_DESTROY;
    bbgl_context->message->asDestroy.index = mapping->index;
    bbgl_sync(bbgl_context);
}

mapping_t mapping_acquire(size_t size) {
    /* Request a new mapping */
    bbgl_context->message->type = BBGL_MESSAGE_MAPPING_CREATE;
    bbgl_context->message->asCreate.size = size;
    bbgl_sync(bbgl_context);

    mapping_t value;
    /* Get the shared memory handle */
    value.index = bbgl_context->message->asCreate.index;
    char buffer[1024];
    snprintf(buffer, sizeof buffer, "/bbgl%zu", value.index);
    int fd = shm_open(buffer, O_RDWR, 0666);
    if (fd == -1) {
        fprintf(stderr, "[bbgl] (client) failed to open memory mapping `%s'\n", buffer);
        mapping_release(&value);
        return (mapping_t){ 0, NULL };
    }

    value.address = mmap(0, bbgl_context->message->asCreate.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (value.address == MAP_FAILED) {
        fprintf(stderr, "[bbgl] (client) failed to map memory `%s'n", buffer);
        mapping_release(&value);
        close(fd);
        return (mapping_t){ 0, NULL };
    }

    printf("[bbgl] (client) opened shared memory mapping `%s'\n", buffer);
    return value;
}

void *mapping_translate(const mapping_t *const mapping, void *address) {
    /* Translates memory mapping into an offset which is then applied to
     * the address at the server level.
     */
    uintptr_t base = (uintptr_t)mapping->address;
    uintptr_t where = (uintptr_t)address;
    return (void *)(where - base);
}

void glGenBuffers(GLsizei n, GLuint *buffers) {
    mapping_t mapping = mapping_acquire(sizeof *buffers * n);
    bbgl_message_call(bbgl_context->message, mapping.index, "GenBuffers",
        "z*", n, mapping_translate(&mapping, mapping.address));
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    memcpy(buffers, mapping.address, sizeof *buffers * n);
    mapping_release(&mapping);
}

void glBindBuffer(GLenum target, GLuint buffer) {
    bbgl_message_call(bbgl_context->message, 0, "BindBuffer",
        "eI", target, buffer);
    bbgl_sync(bbgl_context);
}

void glBufferData(GLenum target,
                  GLsizeiptr size,
                  const GLvoid *data,
                  GLenum usage)
{
    if (data) {
        mapping_t mapping = mapping_acquire(size);
        memcpy(mapping.address, data, size);
        bbgl_message_call(bbgl_context->message, mapping.index, "BufferData",
            "eZ*e", target, size, mapping_translate(&mapping, mapping.address), usage);
        bbgl_sync(bbgl_context);
        bbgl_flush(bbgl_context);
        mapping_release(&mapping);
    } else {
        bbgl_message_call(bbgl_context->message, 0, "BufferData",
            "eZ*e", target, size, NULL, usage);
        bbgl_sync(bbgl_context);
        /* No flush needed in this case */
    }
}

void glEnableVertexAttribArray(GLuint index) {
    bbgl_message_call(bbgl_context->message, 0, "EnableVertexAttribArray",
        "I", index);
    bbgl_sync(bbgl_context);
}

void glDisableVertexAttribArray(GLuint index) {
    bbgl_message_call(bbgl_context->message, 0, "DisableVertexAttribArray",
        "I", index);
    bbgl_sync(bbgl_context);
}

void glVertexAttribPointer(GLuint index,
                           GLint size,
                           GLenum type,
                           GLboolean normalized,
                           GLsizei stride,
                           const GLvoid *pointer)
{
    bbgl_message_call(bbgl_context->message, 0, "VertexAttribPointer",
        "Iie.z*", index, size, type, normalized, stride, pointer);
    bbgl_sync(bbgl_context);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    bbgl_message_call(bbgl_context->message, 0, "DrawArrays",
        "eiz", mode, first, count);
    bbgl_sync(bbgl_context);
}

void glClear(GLbitfield mask) {
    bbgl_message_call(bbgl_context->message, 0, "Clear",
        ":", mask);
    bbgl_sync(bbgl_context);
}

void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
    bbgl_message_call(bbgl_context->message, 0, "ClearColor",
        "FFFF", r, g, b, a);
    bbgl_sync(bbgl_context);
}

GLuint glCreateProgram(void) {
    bbgl_message_call(bbgl_context->message, 0, "CreateProgram", "");
    bbgl_sync(bbgl_context);
    /* Force the recorded things to be flushed, we need the result
     * immediately */
    bbgl_flush(bbgl_context);
    return bbgl_context->message->asCall.params[0].asUInt;
}

GLuint glCreateShader(GLenum type) {
    bbgl_message_call(bbgl_context->message, 0, "CreateShader", "e",
        type);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    return bbgl_context->message->asCall.params[0].asUInt;
}

void glShaderSource(GLuint shader,
                    GLsizei count,
                    const GLchar **string,
                    const GLint *length)
{
    /* TODO: figure out */
}

void glCompileShader(GLuint shader) {
    bbgl_message_call(bbgl_context->message, 0, "CompileShader", "I",
        shader);
    bbgl_sync(bbgl_context);
    /* Don't need to flush here since shader compilation can be deferred
     * until link program */
}

void glAttachShader(GLuint program, GLuint shader) {
    bbgl_message_call(bbgl_context->message, 0, "AttachShader", "II",
        program, shader);
    bbgl_sync(bbgl_context);
    /* Dont need to flush here since shader attachment can be deferred
     * until link program */
}

void glLinkProgram(GLuint program) {
    bbgl_message_call(bbgl_context->message, 0, "LinkProgram", "I",
        program);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
}

void glValidateProgram(GLuint program) {
    bbgl_message_call(bbgl_context->message, 0, "ValidateProgram", "I",
        program);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
}

void glUseProgram(GLuint program) {
    bbgl_message_call(bbgl_context->message, 0, "UseProgram", "I",
        program);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    bbgl_message_call(bbgl_context->message, 0, "Viewport", "iizz",
        x, y, width, height);
    bbgl_sync(bbgl_context);
}

void glGenVertexArrays(GLsizei n, GLuint *arrays) {
    mapping_t mapping = mapping_acquire(sizeof *arrays * n);
    bbgl_message_call(bbgl_context->message, mapping.index,
        "GenVertexArrays", "z*", n, mapping_translate(&mapping, mapping.address));
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    memcpy(arrays, mapping.address, sizeof *arrays * n);
    mapping_release(&mapping);
}

void glBindVertexArray(GLuint array) {
    bbgl_message_call(bbgl_context->message, 0, "BindVertexArray", "I", array);
    bbgl_sync(bbgl_context);
}

GLint glGetUniformLocation(GLuint program, const GLchar *name) {
    const size_t length = strlen(name) + 1;
    mapping_t mapping = mapping_acquire(length);
    char *copy = mapping.address;
    memcpy(copy, name, length);
    bbgl_message_call(bbgl_context->message, mapping.index, "GetUniformName", "I*",
        program, mapping_translate(&mapping, copy));
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    return bbgl_context->message->asCall.params[0].asInt;
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    bbgl_message_call(bbgl_context->message, 0, "Uniform3f",
        "ifff", location, v0, v1, v2);
    bbgl_sync(bbgl_context);
}
