//
// Created by zhuox on 2025-01-01.
//

#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define AS_CLOSURE(value) ((ObjClosure*) AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*) AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*) AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString*) AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE,
} ObjType;

struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj *next;
};

// Function Object
typedef struct {
    // Obj Tab
    Obj obj;
    // Args Count
    int arity;
    // Upvalue Count
    int upvalueCount;
    // Function Code
    Chunk chunk;
    // Function Name
    ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

typedef struct {
    Obj obj;
    Value *location;
    Value closed;
    struct ObjUpvalue *next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct ObjString {
    Obj obj;
    // 字符串长度
    int length;
    // 字符串指针
    char *chars;
    // 字符串哈希值
    uint32_t hash;
} ObjString;

ObjClosure *newClosure(ObjFunction *function);

ObjFunction *newFunction();

ObjNative *newNative(NativeFn function);

ObjString *takeString(char *chars, int length);

ObjString *copyString(const char *chars, int length);

ObjUpvalue *newUpvalue(Value *slot);

void printObject(Value value);

static inline bool isObjType(const Value value, const ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //clox_object_h
