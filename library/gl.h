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

#ifdef __cplusplus
}
#endif
