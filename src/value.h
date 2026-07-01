#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <stddef.h>

typedef enum ValueType {
    VT_NONE,
    VT_INT8,
    VT_INT16,
    VT_INT32,
    VT_INT64,
    VT_UINT8,
    VT_UINT16,
    VT_UINT32,
    VT_UINT64,
    VT_FLOAT32,
    VT_FLOAT64,
    VT_STRUCT
} ValueType;

extern const char* ValueTypeNames[];

typedef struct Value Value;

typedef struct Struct {
    char** fields;
    Value* values;
    size_t count;
} Struct;

typedef struct Value {
    ValueType type;
    union {
        int8_t   i8;
        int16_t  i16;
        int32_t  i32;
        int64_t  i64;
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float    f32;
        double   f64;
        Struct   struct_;
    } value;
} Value;

ValueType get_value_type_from_string(const char* string);
Value cast_value(Value value, ValueType cast_type);
#endif
