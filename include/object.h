//
// Created by zhuox on 2025-01-01.
//

#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define AS_FUNCTION(value) ((ObjFunction*) AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*) AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString*) AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj *next;
};

// Function Object
typedef struct {
    // Obj Tab
    Obj obj;
    // Args Count
    int arity;
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

struct ObjString {
    Obj obj;
    // 字符串长度
    int length;
    // 字符串指针
    char *chars;
    // 字符串哈希值
    uint32_t hash;
};

ObjFunction *newFunction();

ObjNative *newNative(NativeFn function);

ObjString *takeString(char *chars, int length);

ObjString *copyString(const char *chars, int length);

void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //clox_object_h
