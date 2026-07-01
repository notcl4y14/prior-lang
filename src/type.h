#ifndef TYPE_H
#define TYPE_H

#include "value.h"
#include <stddef.h>

typedef struct Type Type;

/***
 * TypeStructData is a semantical type definition type data
 * that represents a struct. Couldn't have guessed it.
 */
typedef struct TypeStructData {
    char** fields_names;
    char** fields_types;
    size_t count;
} TypeStructData;

TypeStructData create_type_struct_data();
void free_type_struct_data(TypeStructData* type_struct_data);

/***
 * TypeType assigns a semantical type instance's type:
 *     - ALIAS: The type definition is an alias to another type.
 *     - VALUE: The type definition is a type of a primitive value. (u8, i32)
 *     - STRUCT: The type defines a struct
 * There is a better way to name this. Like TypeDefType, which is less confusing
 * but still is.
 */
typedef enum TypeType {
    TYPE_TYPE_NONE,
    TYPE_TYPE_ALIAS,
    TYPE_TYPE_VALUE,
    TYPE_TYPE_STRUCT,
} TypeType;

/***
 * Type is a semantical instance that represents
 * a data type. It can be used to define a type
 * or an alias to a type.
 * Example:
 *     core type: "u8" -> VT_UINT8
 *     custom type: "MyStruct" -> TypeStruct
 *     alias: "uint8_t" -> "u8" -> VT_UINT8
 */
typedef struct Type {
    TypeType type;
    union {
        char*          data_alias;
        ValueType      data_value;
        TypeStructData data_struct;
    } data;
} Type;

Type create_alias_typedef(const char* type_name);
Type create_value_typedef(ValueType value_type);
Type create_struct_typedef(TypeStructData struct_type);

/***
 * TypeTable is a table that stores types with their types
 * and datas that represent their types. If you don't get it,
 * ignore this comment.
 */
typedef struct TypeTable {
    char** types_idents;
    Type*  types_values;
    size_t count;
} TypeTable;

/***
 * Creates a new TypeTable instance.
 */
TypeTable create_type_table();

/***
 * Frees TypeTable instance.
 */
void free_type_table(TypeTable* tt);

/***
 * Creates a new type definition in the Type Table.
 * The `Type type` argument has to be fully initialized.
 */
void type_table_assign_type(TypeTable* tt, const char* ident, Type type);

/***
 * Returns the type from the TypeTable by identifier.
 * Returns Type with TYPE_TYPE_NONE if not found.
 */
Type type_table_get_type(TypeTable* tt, const char* ident);

#endif
