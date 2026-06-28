#ifndef VALUE_TYPES_H
#define VALUE_TYPES_H

#include <parser.h>
#include <value.h>

typedef struct ValueFunction {
    char params_names[128];
    ValueType params_types[128];
    ValueType return_type;
    Node node;
} ValueFunction;

#endif
