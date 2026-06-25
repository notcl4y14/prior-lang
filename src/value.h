#ifndef VALUE_H
#define VALUE_H

typedef enum ValueType {
    VT_NONE,
    VT_INT32,
    VT_UINT32,
    VT_FLOAT32,
} ValueType;

extern const char* ValueTypeNames[];

#endif
