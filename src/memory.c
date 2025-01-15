#include <stdlib.h>

#include "compiler.h"
#include "memory.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

static void freeObject(Obj *object);

void *reallocate(void *pointer, const size_t oldSize, const size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
        if (vm.bytesAllocated > vm.nextGC) {
            collectGarbage();
        }
    }

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void *result = realloc(pointer, newSize);
    if (result == NULL) {
        exit(1);
    }

    return result;
}

void markValue(const Value value) {
    if (IS_OBJ(value)) {
        markObject(AS_OBJ(value));
    }
}

static void markArray(const ValueArray *array) {
    for (int i = 0; i < array->count; i++) {
        markValue(array->values[i]);
    }
}

static void blackenObject(Obj *object) {
#ifdef DEBUG_LOG_GC
    printf("%p mark object ", (void *) object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
        case OBJ_BOUND_METHOD: {
            const ObjBoundMethod *method = (ObjBoundMethod *) object;
            markValue(method->receiver);
            markObject((Obj *) method->method);
            break;
        }
        case OBJ_CLASS: {
            const ObjClass *class = (ObjClass *) object;
            markObject((Obj *) class->name);
            markTable(&class->methods);
            break;
        }
        case OBJ_CLOSURE: {
            const ObjClosure *closure = (ObjClosure *) object;
            markObject((Obj *) closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject((Obj *) closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            const ObjFunction *function = (ObjFunction *) object;
            markObject((Obj *) function->name);
            markArray(&function->chunk.constants);
            break;
        }
        case OBJ_INSTANCE: {
            const ObjInstance *instance = (ObjInstance *) object;
            markObject((Obj *) instance->klass);
            markTable(&instance->fields);
            break;
        }
        case OBJ_UPVALUE:
            markValue(((ObjUpvalue *) object)->closed);
            break;
    }
}

void markObject(Obj *object) {
    if (object == NULL) {
        return;
    }
    if (object->isMarked) {
        return;
    }
#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void *) object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    if (vm.grayCapacity < vm.grayCapacity + 1) {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = (Obj **) realloc(vm.grayStack, vm.grayCapacity * sizeof(Obj *));
        if (vm.grayStack == NULL) {
            exit(1);
        }
    }
}

static void markRoots() {
    // mark the stack
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
        markValue(*slot);
    }
    // mark the call frames
    for (int i = 0; i < vm.frameCount; i++) {
        markObject((Obj *) vm.frames[i].closure);
    }
    // mark open upvalues
    for (const ObjUpvalue *upvalue = vm.openUpvalues;
         upvalue != NULL; upvalue = upvalue->next) {
        markObject((Obj *) upvalue);
    }
    // mark the globals
    markTable(&vm.globals);
    // mark the compiler state
    markCompilerRoots();
    markObject((Obj *) vm.initString);
}

static void traceReferences() {
    while (vm.grayCount > 0) {
        Obj *object = vm.grayStack[--vm.grayCount];
        blackenObject(object);
    }
}

static void sweep() {
    Obj *previous = NULL;
    Obj *object = vm.objects;
    while (object != NULL) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj *unreached = object;
            object = object->next;
            if (previous != NULL) {
                previous->next = object;
            } else {
                vm.objects = object;
            }
            freeObject(unreached);
        }
    }
}

void collectGarbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    const size_t before = vm.bytesAllocated;
#endif
    markRoots();
    traceReferences();
    tableRemoveWhite(&vm.strings);
    sweep();
    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;
#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}

static const char *translateType(const ObjType type) {
    switch (type) {
        case OBJ_BOUND_METHOD:
            return "bound method";
        case OBJ_CLASS:
            return "class";
        case OBJ_CLOSURE:
            return "closure";
        case OBJ_FUNCTION:
            return "function";
        case OBJ_INSTANCE:
            return "instance";
        case OBJ_NATIVE:
            return "native function";
        case OBJ_STRING:
            return "string";
        case OBJ_UPVALUE:
            return "upvalue";
        default:
            return "unknown type";
    }
}

static void freeObject(Obj *object) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %s\n", (void *) object, translateType(object->type));
#endif
    switch (object->type) {
        case OBJ_BOUND_METHOD: {
            FREE(ObjBoundMethod, object);
            break;
        }
        case OBJ_CLASS: {
            const ObjClass *class = (ObjClass *) object;
            freeTable(&class->methods);
            FREE(ObjClass, object);
            break;
        }
        case OBJ_CLOSURE: {
            const ObjClosure *closure = (ObjClosure *) object;
            FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
            FREE(ObjClosure, object);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction *function = (ObjFunction *) object;
            freeChunk(&function->chunk);
            FREE(ObjFunction, object);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance *instance = (ObjInstance *) object;
            freeTable(&instance->fields);
            FREE(ObjInstance, object);
            break;
        }
        case OBJ_NATIVE: {
            FREE(ObjNative, object);
            break;
        }
        case OBJ_STRING: {
            const ObjString *string = (ObjString *) object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, object);
            break;
        }
        case OBJ_UPVALUE: {
            FREE(ObjUpvalue, object);
            break;
        }
    }
}

void freeObjects() {
    Obj *object = vm.objects;
    while (object != NULL) {
        Obj *next = object->next;
        freeObject(object);
        object = next;
    }
    free(vm.grayStack);
}
