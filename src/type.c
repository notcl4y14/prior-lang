#include "util/mem.h"
#include "type.h"
#include "value.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



ValueType get_type_value_type(Type type) {
    switch (type) {
        case TYPE_VALUE_INT8:    return VT_INT8;
        case TYPE_VALUE_INT16:   return VT_INT16;
        case TYPE_VALUE_INT32:   return VT_INT32;
        case TYPE_VALUE_INT64:   return VT_INT64;
        case TYPE_VALUE_UINT8:   return VT_UINT8;
        case TYPE_VALUE_UINT16:  return VT_UINT16;
        case TYPE_VALUE_UINT32:  return VT_UINT32;
        case TYPE_VALUE_UINT64:  return VT_UINT64;
        case TYPE_VALUE_FLOAT32: return VT_FLOAT32;
        case TYPE_VALUE_FLOAT64: return VT_FLOAT64;
        case TYPE_ENUM:          return VT_ENUM;
        case TYPE_STRUCT:        return VT_STRUCT;
        case TYPE_FUNCTION:      return VT_FUNCTION;
        default: assert(false);  return VT_NONE;
    }
}



EnumDefData create_enum_def_data() {
    EnumDefData data = (EnumDefData) { 0 };

    data.entries_idents = calloc(256, sizeof(char*));
    assert(data.entries_idents != NULL && "Failed to allocate (EnumDefData).entries_idents");

    data.entries_values = calloc(256, sizeof(Value));
    assert(data.entries_values != NULL && "Failed to allocate (EnumDefData).entries_values");

    data.count = 0;

    return data;
}

void free_enum_def_data(EnumDefData* data) {
    free(data->entries_values);
    data->entries_values = NULL;

    free(data->entries_idents);
    data->entries_idents = NULL;
}

StructDefData create_struct_def_data() {
    StructDefData data = (StructDefData) { 0 };

    data.fields_idents = calloc(256, sizeof(char*));
    assert(data.fields_idents != NULL && "Failed to allocate (StructDefData).fields_idents");

    data.fields_types = calloc(256, sizeof(char*));
    assert(data.fields_types != NULL && "Failed to allocate (StructDefData).fields_types");

    data.count = 0;

    return data;
}

void free_struct_def_data(StructDefData* data) {
    free(data->fields_types);
    data->fields_types = NULL;

    free(data->fields_idents);
    data->fields_idents = NULL;
}

FuncDefData create_func_def_data() {
    FuncDefData data = (FuncDefData) { 0 };

    data.params_idents = calloc(256, sizeof(char*));
    assert(data.params_idents != NULL && "Failed to allocate (FuncDefData).params_idents");

    data.params_types = calloc(256, sizeof(char*));
    assert(data.params_types != NULL && "Failed to allocate (FuncDefData).params_types");

    data.count = 0;

    return data;
}

void free_func_def_data(FuncDefData* data) {
    free(data->params_types);
    data->params_types = NULL;

    free(data->params_idents);
    data->params_idents = NULL;
}



Definition create_type_def(const char* ident, Type type) {
    return (Definition) {
        .type = DEF_TYPE_VALUETYPE,
        .data.data_type = type,
        .ident = (char*) ident,
    };
};

Definition create_alias_def(const char* ident, const char* alias) {
    return (Definition) {
        .type = DEF_TYPE_ALIASTYPE,
        .data.data_alias = (char*) alias,
        .ident = (char*) ident,
    };
}

Definition create_enum_def(const char* ident, EnumDefData data) {
    return (Definition) {
        .type = DEF_TYPE_ENUM,
        .data.data_enum = data,
        .ident = (char*) ident,
    };
}

Definition create_struct_def(const char* ident, StructDefData data) {
    return (Definition) {
        .type = DEF_TYPE_STRUCT,
        .data.data_struct = data,
        .ident = (char*) ident,
    };
}

Definition create_func_def(const char* ident, FuncDefData data) {
    return (Definition) {
        .type = DEF_TYPE_FUNCTION,
        .data.data_function = data,
        .ident = (char*) ident,
    };
}



DefTable create_def_table() {
    DefTable def_table = (DefTable) { 0 };

    def_table.defs_idents = calloc(256, sizeof(char*));
    assert(def_table.defs_idents != NULL && "Failed to allocate (DefTable).defs_idents");

    def_table.defs_data = calloc(256, sizeof(Type));
    assert(def_table.defs_data != NULL && "Failed to allocate (DefTable).defs_data");

    def_table.count = 0;

    return def_table;
}

void free_def_table(DefTable* dt) {
    for (int32_t i = 0; i < dt->count; ++i) {
        Definition* def = &dt->defs_data[i];

        if (def->type == DEF_TYPE_ENUM) {
            free_enum_def_data(&def->data.data_enum);
        } else if (def->type == DEF_TYPE_STRUCT) {
            free_struct_def_data(&def->data.data_struct);
        } else if (def->type == DEF_TYPE_FUNCTION) {
            free_func_def_data(&def->data.data_function);
        }
    }

    free(dt->defs_data);
    dt->defs_data = NULL;

    free(dt->defs_idents);
    dt->defs_idents = NULL;
}

void def_table_assign_def(DefTable* dt, const char* ident, Definition def) {
    const size_t index = dt->count++;

    dt->defs_idents[index] = str_alloc_copy(ident);
    dt->defs_data[index] = def;
}

Definition def_table_get_def(DefTable* dt, const char* ident) {
    for (int32_t i = 0; i < dt->count; ++i) {
        const char* def_ident = dt->defs_idents[i];

        if (strcmp(def_ident, ident) == 0) {
            const Definition* def = &dt->defs_data[i];

            /* Find the source of alias */
            if (def->type == DEF_TYPE_ALIASTYPE) {
                return def_table_get_def(dt, def->data.data_alias);
            }

            return *def;
        }
    }

    return (Definition) { .type = DEF_TYPE_NONE, {} };
}
