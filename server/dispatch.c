#include <stdio.h>
#include <string.h>

#include "shared/message.h"

#include "library/gl.h"

extern void *mapping_translate(size_t index, const void *address);

#define USE_COMPUTED_GOTO

/*
 * DISPATCH_BEGIN(call) {
 *   DISPATCH_CASE(A) { ... DISPATCH_NEXT(); }
 *   DISPATCH_CASE(B) { ... DISPATCH_NEXT(); }
 * }
 *
 * if USE_COMPUTED_GOTO is defined translates to:
 *
 * goto *table[call->name];
 * for (;;) {
 *   A: { ... return; }
 *   B: { ... return; }
 * }
 *
 * otherwise translates to switch table:
 *
 * switch (call->name) {
 *   case BBGL_A: { ... break; }
 *   case BBGL_B: { ... break; }
 * }
 *
 *
 */
#ifdef USE_COMPUTED_GOTO
#define DISPATCH_BEGIN(CALL) \
    goto *table[(CALL)->name]; \
    for (;;)
#define DISPATCH_CASE(NAME) \
    NAME:
#define DISPATCH_NEXT() \
    return
#else
#define DISPATCH_BEGIN(CALL) \
    switch ((CALL)->name)
#define DISPATCH_CASE(NAME) \
    case BBGL_##NAME:
#define DISPATCH_NEXT() \
    break
#endif


void dispatch(const bbgl_message_call_t *const call,
             bbgl_message_param_t *value)
{
#ifdef USE_COMPUTED_GOTO
    /* Use computed goto for dispatching function calls if supported. It gains
     * us some additional performance.
     */
    static void *table[] = {
        [BBGL_GENBUFFERS] = &&GENBUFFERS,
        [BBGL_BINDBUFFER] = &&BINDBUFFER,
        [BBGL_BUFFERDATA] = &&BUFFERDATA,
        [BBGL_ENABLEVERTEXATTRIBARRAY] = &&ENABLEVERTEXATTRIBARRAY,
        [BBGL_DISABLEVERTEXATTRIBARRAY] = &&DISABLEVERTEXATTRIBARRAY,
        [BBGL_VERTEXATTRIBPOINTER] = &&VERTEXATTRIBPOINTER,
        [BBGL_DRAWARRAYS] = &&DRAWARRAYS,
        [BBGL_CLEAR] = &&CLEAR,
        [BBGL_CLEARCOLOR] = &&CLEARCOLOR,
        [BBGL_CREATEPROGRAM] = &&CREATEPROGRAM,
        [BBGL_CREATESHADER] = &&CREATESHADER,
        [BBGL_SHADERSOURCE] = &&SHADERSOURCE,
        [BBGL_COMPILESHADER] = &&COMPILESHADER,
        [BBGL_ATTACHSHADER] = &&ATTACHSHADER,
        [BBGL_LINKPROGRAM] = &&LINKPROGRAM,
        [BBGL_VALIDATEPROGRAM] = &&VALIDATEPROGRAM,
        [BBGL_USEPROGRAM] = &&USEPROGRAM,
        [BBGL_VIEWPORT] = &&VIEWPORT,
        [BBGL_GENVERTEXARRAYS] = &&GENVERTEXARRAYS,
        [BBGL_BINDVERTEXARRAY] = &&BINDVERTEXARRAY,
        [BBGL_GETUNIFORMLOCATION] = &&GETUNIFORMLOCATION,
        [BBGL_UNIFORM3F] = &&UNIFORM3F,
        [BBGL_GETERROR] = &&GETERROR,
        [BBGL_GETSHADERIV] = &&GETSHADERIV,
        [BBGL_GETSHADERINFOLOG] = &&GETSHADERINFOLOG
    };
#endif

    /* This is a little tricky since we need to translate memory mappings
     * on demand for things prepared by the client.
     */
    DISPATCH_BEGIN(call) {
        DISPATCH_CASE(GENBUFFERS) {
            glGenBuffers(call->params[0].asSizeI,
                mapping_translate(call->mapping, call->params[1].asPointer));
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(BINDBUFFER) {
            glBindBuffer(call->params[0].asEnum, call->params[1].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(BUFFERDATA) {
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
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(ENABLEVERTEXATTRIBARRAY) {
            glEnableVertexAttribArray(call->params[0].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(DISABLEVERTEXATTRIBARRAY) {
            glDisableVertexAttribArray(call->params[0].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(VERTEXATTRIBPOINTER) {
            glVertexAttribPointer(call->params[0].asUInt,
                                  call->params[1].asInt,
                                  call->params[2].asEnum,
                                  call->params[3].asBoolean,
                                  call->params[4].asSizeI,
                                  call->params[5].asPointer);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(DRAWARRAYS) {
            glDrawArrays(call->params[0].asEnum,
                         call->params[1].asInt,
                         call->params[2].asSizeI);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(CLEAR) {
            glClear(call->params[0].asBitfield);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(CLEARCOLOR) {
            glClearColor(call->params[0].asClampF,
                         call->params[1].asClampF,
                         call->params[2].asClampF,
                         call->params[3].asClampF);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(CREATEPROGRAM) {
            value->asUInt = glCreateProgram();
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(CREATESHADER) {
            value->asUInt = glCreateShader(call->params[0].asEnum);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(SHADERSOURCE) {
            const GLchar **strings = mapping_translate(call->mapping,
                                                       call->params[2].asPointer);
            GLint *lengths = mapping_translate(call->mapping,
                                               call->params[3].asPointer);
            /* We need to translate incoming strings in the array since
             * they're encoded as offsets.
             */
            for (GLsizei i = 0; i < call->params[1].asSizeI; i++)
                strings[i] = mapping_translate(call->mapping, strings[i]);
            glShaderSource(call->params[0].asUInt,
                           call->params[1].asSizeI,
                           strings,
                           lengths);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(COMPILESHADER) {
            glCompileShader(call->params[0].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(ATTACHSHADER) {
            glAttachShader(call->params[0].asUInt,
                           call->params[1].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(LINKPROGRAM) {
            glLinkProgram(call->params[0].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(VALIDATEPROGRAM) {
            glValidateProgram(call->params[0].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(USEPROGRAM) {
            glUseProgram(call->params[0].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(VIEWPORT) {
            glViewport(call->params[0].asInt,
                       call->params[1].asInt,
                       call->params[2].asSizeI,
                       call->params[3].asSizeI);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(GENVERTEXARRAYS) {
            glGenVertexArrays(call->params[0].asSizeI,
                              mapping_translate(call->mapping, call->params[1].asPointer));
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(BINDVERTEXARRAY) {
            glBindVertexArray(call->params[0].asUInt);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(GETUNIFORMLOCATION) {
            value->asInt = glGetUniformLocation(call->params[0].asUInt,
                                                mapping_translate(call->mapping, call->params[1].asPointer));
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(UNIFORM3F) {
            glUniform3f(call->params[0].asInt,
                        call->params[1].asFloat,
                        call->params[2].asFloat,
                        call->params[3].asFloat);
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(GETERROR) {
            value->asEnum = glGetError();
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(GETSHADERIV) {
            glGetShaderiv(call->params[0].asUInt,
                          call->params[1].asEnum,
                          mapping_translate(call->mapping, call->params[2].asPointer));
            DISPATCH_NEXT();
        }
        DISPATCH_CASE(GETSHADERINFOLOG) {
            glGetShaderInfoLog(call->params[0].asUInt,
                               call->params[1].asSizeI,
                               mapping_translate(call->mapping, call->params[2].asPointer),
                               mapping_translate(call->mapping, call->params[2].asPointer));
            DISPATCH_NEXT();
        }
    }
}
