#ifndef VALUE_TYPES_H
#define VALUE_TYPES_H

#include "parser.h"
#include "type.h"
#include <value.h>

typedef struct ValueFunctionParam {
    char* name;
    Type  type;
} ValueFunctionParam;

typedef struct ValueFunction {
    ValueFunctionParam params[256];
    Type  return_type;
    Node* node;
} ValueFunction;

#endif
