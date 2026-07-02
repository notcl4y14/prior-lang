#include "type.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

/***
 * TODO: Make this test an actual testing test that tests with asserts.
 */

int main() {
    TypeTable type_table = create_type_table();

    /* Assigning core types */
    type_table_assign_type( &type_table, "i8",  create_value_typedef(VT_INT8)   );
    type_table_assign_type( &type_table, "i16", create_value_typedef(VT_INT16)  );
    type_table_assign_type( &type_table, "i32", create_value_typedef(VT_INT32)  );
    type_table_assign_type( &type_table, "i64", create_value_typedef(VT_INT64)  );
    type_table_assign_type( &type_table, "u8",  create_value_typedef(VT_UINT8)  );
    type_table_assign_type( &type_table, "u16", create_value_typedef(VT_UINT16) );
    type_table_assign_type( &type_table, "u32", create_value_typedef(VT_UINT32) );
    type_table_assign_type( &type_table, "u64", create_value_typedef(VT_UINT64) );

    /* Assigning alias types */
    type_table_assign_type( &type_table, "uint32_t", create_alias_typedef("u32") );
    type_table_assign_type( &type_table, "float",    create_alias_typedef("f32") );

    /* Assigning struct types */
    TypeStructData struct_data = create_type_struct_data();

    {
        struct_data.fields_names[0] = "x";
        struct_data.fields_types[0] = "float";
        struct_data.fields_names[1] = "y";
        struct_data.fields_types[1] = "float";
        struct_data.count = 2;

        type_table_assign_type( &type_table, "Vector2f", create_struct_typedef(struct_data) );
    }

    for (int32_t i = 0; i < type_table.count; ++i) {
        const char* ident = type_table.types_idents[i];
        const Type* tdef = &type_table.types_values[i];

        printf("%s", ident);

        switch (tdef->type) {
            case TYPE_TYPE_ALIAS:  printf("\t: alias "); break;
            case TYPE_TYPE_VALUE:  printf("\t: value "); break;
            case TYPE_TYPE_STRUCT: printf("\t: struct"); break;
            default: assert(false);
        }

        switch (tdef->type) {
            case TYPE_TYPE_ALIAS:  printf(" = %s", tdef->data.data_alias);                 break;
            case TYPE_TYPE_VALUE:  printf(" = %s", ValueTypeNames[tdef->data.data_value]); break;
            case TYPE_TYPE_STRUCT:
                printf(" = {");

                for (int32_t j = 0; j < tdef->data.data_struct.count; ++j) {
                    const char* name = tdef->data.data_struct.fields_names[j];
                    const char* type = tdef->data.data_struct.fields_types[j];
                    printf(" %s: %s,", name, type);
                }

                printf(" }");
                break;

            default: assert(false);
        }

        printf("\n");
    }

    free_type_struct_data(&struct_data);
    free_type_table(&type_table);
}
