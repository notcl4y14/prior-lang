#ifndef TYPE_H
#define TYPE_H

#include "value.h"

#include <stddef.h>

// typedef enum Type Type;

/***
 * TypeStructData is a semantical type definition type data
 * that represents a struct. Couldn't have guessed it.
 */
// typedef struct TypeStructData {
//     char** fields_names;
//     char** fields_types;
//     size_t count;
// } TypeStructData;

// TypeStructData create_type_struct_data();
// void free_type_struct_data(TypeStructData* type_struct_data);

/***
 * TypeStructData is a semantical type definition type data
 * that represents a function. Couldn't have guessed it either.
 */
// typedef struct TypeFunctionData {
//     char** params_names;
//     char** params_types;
//     size_t count;
// } TypeFunctionData;

// TypeFunctionData create_type_function_data();
// void free_type_function_data(TypeFunctionData* type_function_data);

/***
 * TypeEnumData is a semantical type definition type data
 * that represents an enum. This is so hard to know isn't it.
 */
// typedef struct TypeEnumData {
//     char** entries_names;
//     Value* entries_values;
//     size_t count;
// } TypeEnumData;

// TypeEnumData create_type_enum_data();
// void free_type_enum_data(TypeEnumData* type_enum_data);

/***
 * TypeType assigns a semantical type instance's type:
 *     - ALIAS: The type definition is an alias to another type.
 *     - VALUE: The type definition is a type of a primitive value. (u8, i32)
 *     - ENUM: The type defines an enum.
 *     - STRUCT: The type defines a struct.
 *     - FUNCTION: The type defines a function.
 * There is a better way to name this. Like TypeDefType, which is less confusing
 * but still is.
 */
// typedef enum TypeType {
//     TYPE_TYPE_NONE,
//     TYPE_TYPE_ALIAS,
//     TYPE_TYPE_VALUE,
//     TYPE_TYPE_ENUM,
//     TYPE_TYPE_STRUCT,
//     TYPE_TYPE_FUNCTION,
// } TypeType;

/***
 * Type is a semantical instance that represents
 * a data type. It can be used to define a type
 * or an alias to a type.
 * Example:
 *     core type: "u8" -> VT_UINT8
 *     custom type: "MyStruct" -> TypeStruct
 *     alias: "uint8_t" -> "u8" -> VT_UINT8
 */
// typedef struct Type {
//     TypeType type;
//     union {
//         char*            data_alias;
//         ValueType        data_value;
//         TypeEnumData     data_enum;
//         TypeStructData   data_struct;
//         TypeFunctionData data_function;
//     } data;
// } Type;

// Type create_alias_typedef(const char* type_name);
// Type create_value_typedef(ValueType value_type);
// Type create_enum_typedef(TypeEnumData enum_type);
// Type create_struct_typedef(TypeStructData struct_type);
// Type create_function_typedef(TypeFunctionData function_type);
// ValueType get_typedef_value_type(Type type);

typedef enum Type {
    TYPE_NONE,
    TYPE_VALUE_NULL,
    TYPE_VALUE_INT8,
    TYPE_VALUE_INT16,
    TYPE_VALUE_INT32,
    TYPE_VALUE_INT64,
    TYPE_VALUE_UINT8,
    TYPE_VALUE_UINT16,
    TYPE_VALUE_UINT32,
    TYPE_VALUE_UINT64,
    TYPE_VALUE_FLOAT32,
    TYPE_VALUE_FLOAT64,
    TYPE_ENUM,
    TYPE_STRUCT,
    TYPE_FUNCTION,
} Type;

ValueType get_type_value_type(Type type);

typedef struct EnumDefData {
    char** entries_idents;
    Value* entries_values;
    size_t count;
} EnumDefData;

EnumDefData create_enum_def_data();
void free_enum_def_data(EnumDefData* enum_def_data);

typedef struct StructDefData {
    char** fields_idents;
    char** fields_types;
    size_t count;
} StructDefData;

StructDefData create_struct_def_data();
void free_struct_def_data(StructDefData* struct_def_data);

typedef struct FuncDefData {
    char** params_idents;
    char** params_types;
    // TODO: Default values
    size_t count;
} FuncDefData;

FuncDefData create_func_def_data();
void free_func_def_data(FuncDefData* func_def_data);

/***
 * Definition Type - type of the definition.
 */
typedef enum DefinitionType {
    DEF_TYPE_NONE,
    DEF_TYPE_VALUETYPE,
    DEF_TYPE_ALIASTYPE,
    DEF_TYPE_ENUM,
    DEF_TYPE_STRUCT,
    DEF_TYPE_FUNCTION,
} DefinitionType;

/***
 * Definition is an instance that holds type and value.
 * It's responsible for storing semantical definitions
 * such as enums, structs, types, aliases, etc.
 * It is not responsible for evaluation types like variables
 * and values. (However, some definitions include values like
 * enums)
 *
 * DefinitionType type : type of definition;
 * union {} data       : data of the definition, matches with (Definition).type;
 * char* ident         : identifier (name) of the definition.
 */
typedef struct Definition {
    DefinitionType type;
    union {
        Type          data_type;
        char*         data_alias;
        EnumDefData   data_enum;
        StructDefData data_struct;
        FuncDefData   data_function;
    } data;
    char* ident;
} Definition;

Definition create_type_def(const char* ident, Type type);
Definition create_alias_def(const char* ident, const char* alias);
Definition create_enum_def(const char* ident, EnumDefData data);
Definition create_struct_def(const char* ident, StructDefData data);
Definition create_func_def(const char* ident, FuncDefData data);

/***
 * DefTable is a table that stores definitions.
 * Global and local types are implemented through Scopes.
 */
typedef struct DefTable {
    char**      defs_idents;
    Definition* defs_data;
    size_t      count;
} DefTable;

/***
 * Creates a new DefTable instance.
 */
DefTable create_def_table();

/***
 * Frees DefTable instance.
 */
void free_def_table(DefTable* dt);

/***
 * Creates a new type definition in the Type Table.
 * The `Type type` argument has to be fully initialized.
 */
void def_table_assign_def(DefTable* dt, const char* ident, Definition def);

/***
 * Returns the type from the DefTable by identifier.
 * Returns Type with TYPE_TYPE_NONE if not found.
 */
Definition def_table_get_def(DefTable* dt, const char* ident);

#endif
