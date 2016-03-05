#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "message.h"

const char *bbgl_message_name(const bbgl_message_t *const message) {
    switch (message->type) {
    case BBGL_MESSAGE_MAPPING_CREATE:
        return "Create Memory Mapping";
    case BBGL_MESSAGE_MAPPING_DESTROY:
        return "Destroy Memory Mapping";
    case BBGL_MESSAGE_CALL:
        return "Record Call";
    case BBGL_MESSAGE_FLUSH:
        return "Flush Calls";
    case BBGL_MESSAGE_SHUTDOWN:
        return "Shutdown";
    case BBGL_MESSAGE_CONTEXT_CREATE:
        return "Create Context";
    case BBGL_MESSAGE_CONTEXT_DESTROY:
        return "Destroy Context";
    }
    return "Unknown";
}

/* Used only by the client */
extern void *mapping_address_translate(size_t index, void *address);
void bbgl_message_call(bbgl_message_t *message, size_t mapping, const char *name, const char *spec, ...) {
    message->type = BBGL_MESSAGE_CALL;
    strncpy(message->asCall.name, name, sizeof message->asCall.name);
    message->asCall.mapping = mapping;

    va_list va;
    va_start(va, spec);
    bbgl_message_param_t *p = message->asCall.params;
    /* Due to the way C promotion rules work for variable argument functions,
     * this may look strange.
     */
    for (const char *s = spec; *s; s++, p++) {
        switch (*s) {
        case '.':
            p->asBoolean = !!va_arg(va, int);
            break;
        case 'b':
            p->asByte = (char)va_arg(va, int);
            break;
        case 'B':
            p->asUByte = (unsigned char)va_arg(va, unsigned int);
            break;
        case 's':
            p->asShort = (short)va_arg(va, int);
            break;
        case 'S':
            p->asUShort = (unsigned short)va_arg(va, unsigned int);
            break;
        case 'i':
            p->asInt = va_arg(va, int);
            break;
        case 'I':
            p->asUInt = va_arg(va, unsigned int);
            break;
        case 'l':
            p->asInt64 = va_arg(va, int64_t);
            break;
        case 'L':
            p->asUInt64 = va_arg(va, uint64_t);
            break;
        case 'z':
            p->asSizeI = va_arg(va, unsigned int);
            break;
        case 'e':
            p->asEnum = va_arg(va, unsigned int);
            break;
        case 'p':
            /* TODO: make offset */
            p->asIntPtr = va_arg(va, intptr_t);
            break;
        case 'Z':
            /* TODO: make offset */
            p->asSizeIPtr = va_arg(va, uintptr_t);
            break;
        case 'O':
            /* TODO: make offset */
            p->asSync = va_arg(va, uintptr_t);
            break;
        case ':':
            p->asBitfield = va_arg(va, unsigned int);
            break;
        case 'h':
            p->asHalf = (unsigned short)va_arg(va, unsigned int);
            break;
        case 'f':
            p->asFloat = (float)va_arg(va, double);
            break;
        case 'F':
            p->asClampF = (float)va_arg(va, double);
            break;
        case 'd':
            p->asDouble = va_arg(va, double);
            break;
        case 'D':
            p->asClampD = va_arg(va, double);
            break;
        case '*':
            p->asPointer = va_arg(va, void *);
            break;
        }
    }
    va_end(va);
}
