#include <stdio.h>
#include <string.h>

#include "shared/message.h"

#include "library/gl.h"

extern void *mapping_translate(size_t index, void *address);

void dispatch(bbgl_message_call_t *call) {
    /* This is a little tricky since we need to translate memory mappings
     * on demand for things prepared by the client.
     */
    printf("RUNNING `%s'\n", call->name);
    if (!strcmp(call->name, "GenBuffers")) {
        glGenBuffers(call->params[0].asSizeI,
            mapping_translate(call->mapping, call->params[1].asPointer));
    } else if (!strcmp(call->name, "BindBuffer")) {
        glBindBuffer(call->params[0].asEnum, call->params[1].asUInt);
    } else if (!strcmp(call->name, "BufferData")) {
        if (call->mapping) {
            glBufferData(call->params[0].asEnum,
                         call->params[1].asSizeIPtr,
                         mapping_translate(call->mapping, call->params[2].asPointer),
                         call->params[3].asEnum);
        } else {
            glBufferData(call->params[0].asEnum,
                         call->params[1].asSizeIPtr,
                         NULL,
                         call->params[3].asEnum);
        }
    } else if (!strcmp(call->name, "EnableVertexAttribArray")) {
        glEnableVertexAttribArray(call->params[0].asUInt);
    } else if (!strcmp(call->name, "DisableVertexAttribArray")) {
        glDisableVertexAttribArray(call->params[0].asUInt);
    } else if (!strcmp(call->name, "VertexAttribPointer")) {
        glVertexAttribPointer(call->params[0].asUInt,
                              call->params[1].asInt,
                              call->params[2].asEnum,
                              call->params[3].asBoolean,
                              call->params[4].asSizeI,
                              call->params[5].asPointer);
    } else if (!strcmp(call->name, "DrawArrays")) {
        glDrawArrays(call->params[0].asEnum,
                     call->params[1].asInt,
                     call->params[2].asSizeI);
    } else if (!strcmp(call->name, "Clear")) {
        glClear(call->params[0].asBitfield);
    } else if (!strcmp(call->name, "ClearColor")) {
        glClearColor(call->params[0].asClampF,
                     call->params[1].asClampF,
                     call->params[2].asClampF,
                     call->params[3].asClampF);
    } else if (!strcmp(call->name, "CreateProgram")) {
        call->params[0].asUInt = glCreateProgram();
    } else if (!strcmp(call->name, "CreateShader")) {
        call->params[0].asUInt = glCreateShader(call->params[0].asEnum);
    } else if (!strcmp(call->name, "ShaderSource")) {
        glShaderSource(call->params[0].asUInt,
                       call->params[1].asSizeI,
                       mapping_translate(call->mapping, call->params[2].asPointer),
                       mapping_translate(call->mapping, call->params[3].asPointer));
    } else if (!strcmp(call->name, "CompileShader")) {
        glCompileShader(call->params[0].asUInt);
    } else if (!strcmp(call->name, "AttachShader")) {
        glAttachShader(call->params[0].asUInt,
                       call->params[1].asUInt);
    } else if (!strcmp(call->name, "LinkProgram")) {
        glLinkProgram(call->params[0].asUInt);
    } else if (!strcmp(call->name, "ValidateProgram")) {
        glValidateProgram(call->params[0].asUInt);
    } else if (!strcmp(call->name, "UseProgram")) {
        glUseProgram(call->params[0].asUInt);
    } else if (!strcmp(call->name, "Viewport")) {
        glViewport(call->params[0].asInt,
                   call->params[1].asInt,
                   call->params[2].asSizeI,
                   call->params[3].asSizeI);
    } else if (!strcmp(call->name, "GenVertexArrays")) {
        glGenVertexArrays(call->params[0].asSizeI,
                          (GLuint *)call->params[1].asPointer);
    } else if (!strcmp(call->name, "BindVertexArray")) {
        glBindVertexArray(call->params[0].asUInt);
    } else if (!strcmp(call->name, "GetUniformLocation")) {
        call->params[0].asInt = glGetUniformLocation(call->params[0].asUInt,
                                                     call->params[1].asPointer);
    } else if (!strcmp(call->name, "Uniform3f")) {
        glUniform3f(call->params[0].asInt,
                    call->params[1].asFloat,
                    call->params[2].asFloat,
                    call->params[3].asFloat);
    }
}
