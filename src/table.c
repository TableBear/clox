//
// Created by zhuox on 2025-01-02.
//
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table *table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    // 所有字段归零
    initTable(table);
}

static Entry *findEntry(Entry *entries, const int capacity, const ObjString *key) {
    uint32_t index = key->hash % capacity;
    Entry *tombstone = NULL;
    for (;;) {
        Entry *entry = &entries[index];
        if (entry->key == NULL) {
            // 如果找到空槽，那么就返回第一个墓碑（如果有的话）
            if (IS_NIL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                // 保存第一个找到的墓碑
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table *table, const int capacity) {
    Entry *entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }
    // re-hash
    // if entry is deleted, it won't be count
    table->count = 0;
    for (int i = 0; i < table->capacity; ++i) {
        Entry *entry = &table->entries[i];
        if (entry->key == NULL) continue;
        Entry *dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    // 释放旧的内存
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableGet(const Table *table, const ObjString *key, Value *value) {
    if (table->count == 0) return false;
    const Entry *entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;
    *value = entry->value;
    return true;
}

bool tableSet(Table *table, ObjString *key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        const int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }
    Entry *entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    // 新插入的键（也即不是替换）且插入位置不是墓碑位置，则计数加一
    if (isNewKey && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(const Table *table, const ObjString *key) {
    if (table->count == 0) return false;
    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;
    // 标记为删除
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void tableAddAll(const Table *from, Table *to) {
    for (int i = 0; i < from->capacity; ++i) {
        const Entry *entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjString *tableFindString(const Table *table, const char *chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;
    uint32_t index = hash % table->capacity;
    for (;;) {
        Entry *entry = &table->entries[index];
        if (entry->key == NULL) {
            // 遇到空槽，意味着没有找到
            if (IS_NIL(entry->value)) {
                // 没有找到
                return NULL;
            }
        } else if (entry->key->length == length &&
                   entry->key->hash == hash &&
                   memcmp(entry->key->chars, chars, length) == 0
        ) {
            return entry->key;
        }
        index = (index + 1) % table->capacity;
    }
}

void tableRemoveWhite(const Table* table) {
    for (int i = 0; i < table->capacity; ++i) {
        const Entry *entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked) {
            tableDelete(table, entry->key);
        }
    }
}

void markTable(const Table *table) {
    for (int i = 0; i < table->capacity; ++i) {
        const Entry *entry = &table->entries[i];
        markObject((Obj *) entry->key);
        markValue(entry->value);
    }
}
