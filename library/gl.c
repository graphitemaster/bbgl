#include <string.h>
#include <errno.h>

#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "bbgl.h"

extern bbgl_t *bbgl_context;

extern int read_fd(int socket);

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

    /* Read file descriptor from socket */
    int fd = read_fd(bbgl_context->pair[0]);

    /* Get the shared memory handle */
    mapping_t value;
    value.index = bbgl_context->message->asCreate.index;
    value.address = mmap(0, bbgl_context->message->asCreate.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (value.address == MAP_FAILED) {
        mapping_release(&value);
        close(fd);
        return (mapping_t){ 0, NULL };
    }
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
    bbgl_message_call(bbgl_context->message, mapping.index, BBGL_GENBUFFERS,
        "z*", n, mapping_translate(&mapping, mapping.address));
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    memcpy(buffers, mapping.address, sizeof *buffers * n);
    mapping_release(&mapping);
}

void glBindBuffer(GLenum target, GLuint buffer) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_BINDBUFFER,
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
        bbgl_message_call(bbgl_context->message, mapping.index, BBGL_BUFFERDATA,
            "eZ*e", target, size, mapping_translate(&mapping, mapping.address), usage);
        bbgl_sync(bbgl_context);
        bbgl_flush(bbgl_context);
        mapping_release(&mapping);
    } else {
        bbgl_message_call(bbgl_context->message, 0, BBGL_BUFFERDATA,
            "eZ*e", target, size, NULL, usage);
        bbgl_sync(bbgl_context);
        /* No flush needed in this case */
    }
}

void glEnableVertexAttribArray(GLuint index) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_ENABLEVERTEXATTRIBARRAY,
        "I", index);
    bbgl_sync(bbgl_context);
}

void glDisableVertexAttribArray(GLuint index) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_DISABLEVERTEXATTRIBARRAY,
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
    bbgl_message_call(bbgl_context->message, 0, BBGL_VERTEXATTRIBPOINTER,
        "Iie.z*", index, size, type, normalized, stride, pointer);
    bbgl_sync(bbgl_context);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_DRAWARRAYS,
        "eiz", mode, first, count);
    bbgl_sync(bbgl_context);
}

void glClear(GLbitfield mask) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_CLEAR,
        ":", mask);
    bbgl_sync(bbgl_context);
}

void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_CLEARCOLOR,
        "FFFF", r, g, b, a);
    bbgl_sync(bbgl_context);
}

GLuint glCreateProgram(void) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_CREATEPROGRAM, NULL);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    return bbgl_context->message->value.asUInt;
}

GLuint glCreateShader(GLenum type) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_CREATESHADER, "e",
        type);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    return bbgl_context->message->value.asUInt;
}

void glShaderSource(GLuint shader,
                    GLsizei count,
                    const GLchar **string,
                    const GLint *length)
{
    size_t memory = sizeof *string * count + sizeof *length * count;
    for (GLsizei i = 0; i < count; i++)
        memory += length[i];
    mapping_t mapping = mapping_acquire(memory);

    /* Write the string table */
    char *table = mapping.address;
    for (GLsizei i = 0; i < count; i++) {
        memcpy(table, string[i], length[i]);
        table += length[i];
    }

    /* Write the pointers for each string after the string table */
    char **strings = (char **)table;
    char *current = mapping.address;
    for (GLsizei i = 0; i < count; i++) {
        *strings++ = mapping_translate(&mapping, current);
        current += length[i];
    }

    /* Write the lengths after the array of pointers to strings */
    GLint *lengths = (GLint *)strings;
    for (GLsizei i = 0; i < count; i++)
        lengths[i] = length[i];

    bbgl_message_call(bbgl_context->message,
                      mapping.index,
                      BBGL_SHADERSOURCE,
                      "Iz**",
                      shader,
                      count,
                      mapping_translate(&mapping, table),
                      mapping_translate(&mapping, lengths));
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);

    mapping_release(&mapping);
}

void glCompileShader(GLuint shader) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_COMPILESHADER, "I",
        shader);
    bbgl_sync(bbgl_context);
}

void glAttachShader(GLuint program, GLuint shader) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_ATTACHSHADER, "II",
        program, shader);
    bbgl_sync(bbgl_context);
}

void glLinkProgram(GLuint program) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_LINKPROGRAM, "I",
        program);
    bbgl_sync(bbgl_context);
}

void glValidateProgram(GLuint program) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_VALIDATEPROGRAM, "I",
        program);
    bbgl_sync(bbgl_context);
}

void glUseProgram(GLuint program) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_USEPROGRAM, "I",
        program);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_VIEWPORT, "iizz",
        x, y, width, height);
    bbgl_sync(bbgl_context);
}

void glGenVertexArrays(GLsizei n, GLuint *arrays) {
    mapping_t mapping = mapping_acquire(sizeof *arrays * n);
    bbgl_message_call(bbgl_context->message, mapping.index,
        BBGL_GENVERTEXARRAYS, "z*", n, mapping_translate(&mapping, mapping.address));
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    memcpy(arrays, mapping.address, sizeof *arrays * n);
    mapping_release(&mapping);
}

void glBindVertexArray(GLuint array) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_BINDVERTEXARRAY, "I", array);
    bbgl_sync(bbgl_context);
}

GLint glGetUniformLocation(GLuint program, const GLchar *name) {
    const size_t length = strlen(name) + 1;
    mapping_t mapping = mapping_acquire(length);
    char *copy = mapping.address;
    memcpy(copy, name, length);
    bbgl_message_call(bbgl_context->message, mapping.index,
        BBGL_GETUNIFORMLOCATION, "I*", program, mapping_translate(&mapping, copy));
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    return bbgl_context->message->value.asInt;
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    bbgl_message_call(bbgl_context->message, 0, BBGL_UNIFORM3F,
        "ifff", location, v0, v1, v2);
    bbgl_sync(bbgl_context);
}

GLenum glGetError(void) {
    /* Must flush the recorded commands if we're executing glGetError */
    bbgl_flush(bbgl_context);

    /* Now issue glGetError */
    bbgl_message_call(bbgl_context->message, 0, BBGL_GETERROR, "");
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    return bbgl_context->message->value.asEnum;
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) {
    mapping_t mapping = mapping_acquire(sizeof *params);
    bbgl_message_call(bbgl_context->message, mapping.index, BBGL_GETSHADERIV,
        "Ie*", shader, pname, mapping.address);
    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);
    *params = *((GLint *)mapping.address);
    mapping_release(&mapping);
}

void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {
    mapping_t mapping = mapping_acquire(maxLength + sizeof *length);
    GLsizei *map_length = mapping.address;
    GLchar *map_infoLog = (GLchar *)(map_length + 1);

    bbgl_message_call(bbgl_context->message,
                      mapping.index,
                      BBGL_GETSHADERINFOLOG,
                      "Iz**",
                      shader,
                      maxLength,
                      mapping_translate(&mapping, map_length),
                      mapping_translate(&mapping, map_infoLog));

    bbgl_sync(bbgl_context);
    bbgl_flush(bbgl_context);

    *length = *map_length;
    memcpy(infoLog, map_infoLog, *map_length);

    mapping_release(&mapping);
}
