#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

void glGenBuffers(GLsizei n, GLuint *buffers);
void glBindBuffer(GLenum target, GLuint buffer) ;
void glBufferData(GLenum target,
                  GLsizeiptr size,
                  const GLvoid *data,
                  GLenum usage);
void glEnableVertexAttribArray(GLuint index);
void glDisableVertexAttribArray(GLuint index);
void glVertexAttribPointer(GLuint index,
                           GLint size,
                           GLenum type,
                           GLboolean normalized,
                           GLsizei stride,
                           const GLvoid *pointer);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glClear(GLbitfield mask);
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
GLuint glCreateProgram(void);
GLuint glCreateShader(GLenum type);
void glShaderSource(GLuint shader,
                    GLsizei count,
                    const GLchar **string,
                    const GLint *length);
void glCompileShader(GLuint shader);
void glAttachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);
void glValidateProgram(GLuint program);
void glUseProgram(GLuint program);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glGenVertexArrays(GLsizei n, GLuint *arrays);
void glBindVertexArray(GLuint array);
GLint glGetUniformLocation(GLuint program, const GLchar *name);
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GLenum glGetError(void);
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);

enum {
    BBGL_GENBUFFERS,
    BBGL_BINDBUFFER,
    BBGL_BUFFERDATA,
    BBGL_ENABLEVERTEXATTRIBARRAY,
    BBGL_DISABLEVERTEXATTRIBARRAY,
    BBGL_VERTEXATTRIBPOINTER,
    BBGL_DRAWARRAYS,
    BBGL_CLEAR,
    BBGL_CLEARCOLOR,
    BBGL_CREATEPROGRAM,
    BBGL_CREATESHADER,
    BBGL_SHADERSOURCE,
    BBGL_COMPILESHADER,
    BBGL_ATTACHSHADER,
    BBGL_LINKPROGRAM,
    BBGL_VALIDATEPROGRAM,
    BBGL_USEPROGRAM,
    BBGL_VIEWPORT,
    BBGL_GENVERTEXARRAYS,
    BBGL_BINDVERTEXARRAY,
    BBGL_GETUNIFORMLOCATION,
    BBGL_UNIFORM3F,
    BBGL_GETERROR,
    BBGL_GETSHADERIV,
    BBGL_GETSHADERINFOLOG
};

#ifdef __cplusplus
}
#endif
