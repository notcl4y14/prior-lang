#include "type.h"
#include "mem.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>



TypeStructData create_type_struct_data() {
    TypeStructData type_struct_data = (TypeStructData) { 0 };

    type_struct_data.fields_names = calloc(256, sizeof(char*));
    assert(type_struct_data.fields_names != NULL && "Failed to allocate (TypeStructData).fields_names");

    type_struct_data.fields_types = calloc(256, sizeof(Type*));
    assert(type_struct_data.fields_types != NULL && "Failed to allocate (TypeStructData).fields_types");

    return type_struct_data;
}

void free_type_struct_data(TypeStructData* type_struct_data) {
    free(type_struct_data->fields_types);
    type_struct_data->fields_types = NULL;

    free(type_struct_data->fields_names);
    type_struct_data->fields_names = NULL;
}



Type create_alias_typedef(char* ident, char* type_name) {
    Type type = (Type) { 0 };

    type.ident = str_alloc_copy(ident);
    assert(type.ident != NULL);

    type.type = TYPE_TYPE_ALIAS;

    type.data.data_alias = str_alloc_copy(type_name);
    assert(type.data.data_alias != NULL);

    return type;
}

Type create_value_typedef(char* ident, ValueType value_type) {
    Type type = (Type) { 0 };

    type.ident = str_alloc_copy(ident);
    assert(type.ident != NULL);

    type.type = TYPE_TYPE_VALUE;

    type.data.data_value = value_type;

    return type;
}

Type create_struct_typedef(char* ident, TypeStructData struct_type) {
    Type type = (Type) { 0 };

    type.ident = str_alloc_copy(ident);
    assert(type.ident != NULL);

    type.type = TYPE_TYPE_STRUCT;

    type.data.data_struct = struct_type;

    return type;
}



TypeTable create_type_table() {
    TypeTable type_table = (TypeTable) { 0 };

    type_table.types_idents = calloc(256, sizeof(char*));
    assert(type_table.types_idents != NULL && "Failed to allocate (TypeTable).types_idents");

    type_table.types_values = calloc(256, sizeof(Type));
    assert(type_table.types_values != NULL && "Failed to allocate (TypeTable).types_values");

    type_table.count = 0;

    return type_table;
}

void free_type_table(TypeTable* tt) {
    free(tt->types_values);
    tt->types_values = NULL;

    free(tt->types_idents);
    tt->types_idents = NULL;
}

void type_table_assign_type(TypeTable* tt, Type type) {
    const size_t index = tt->count++;

    tt->types_idents[index] = str_alloc_copy(type.ident);
    tt->types_values[index] = type;
}

Type type_table_get_type(TypeTable* tt, char* ident) {
    for (int32_t i = 0; i < tt->count; ++i) {
        const char* type_ident = tt->types_idents[i];

        if (strcmp(type_ident, ident) == 0) {
            return tt->types_values[i];
        }
    }

    return (Type) { .type = TYPE_TYPE_NONE, {} };
}
