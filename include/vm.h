#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "table.h"
#include "value.h"

// Max call deep
#define FRAMES_MAX 64
// Max stack size, every call frame has 255 slots to store local value
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/**
 * Call Frame
 * behead of function call
 */
typedef struct {
    // current function
    ObjClosure *closure;
    // current instruction pointer
    uint8_t *ip;
    // current stack
    Value *slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    // top of stack. point the next free slot
    Value *stackTop;
    // global variables
    Table globals;
    // interned strings
    Table strings;
    // open upvalues
    ObjUpvalue *openUpvalues;
    // all objects
    Obj *objects;
    // gray objects
    int grayCount;
    int grayCapacity;
    // self adjust gc
    size_t bytesAllocated;
    size_t nextGC;
    Obj **grayStack;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();

void freeVM();

InterpretResult interpret(const char *source);

void push(const Value value);

Value pop();

#endif
