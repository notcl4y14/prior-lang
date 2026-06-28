#ifndef VALUE_TYPES_H
#define VALUE_TYPES_H

#include <parser.h>
#include <value.h>

typedef struct ValueFunctionParam {
    char*     name;
    ValueType type;
} ValueFunctionParam;

typedef struct ValueFunction {
    ValueFunctionParam params[256];
    ValueType return_type;
    Node node;
} ValueFunction;

#endif
