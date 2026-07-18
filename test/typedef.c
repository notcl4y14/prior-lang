#include "type.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

/***
 * TODO: Make this test an actual testing test that tests with asserts.
 */

int main() {
    DefTable def_table = create_def_table();

    /* Assigning core types */
    def_table_assign_def( &def_table, "i8",  create_type_def("i8",  TYPE_VALUE_INT8)   );
    def_table_assign_def( &def_table, "i16", create_type_def("i16", TYPE_VALUE_INT16)  );
    def_table_assign_def( &def_table, "i32", create_type_def("i32", TYPE_VALUE_INT32)  );
    def_table_assign_def( &def_table, "i64", create_type_def("i64", TYPE_VALUE_INT64)  );
    def_table_assign_def( &def_table, "u8",  create_type_def("u8",  TYPE_VALUE_UINT8)  );
    def_table_assign_def( &def_table, "u16", create_type_def("u16", TYPE_VALUE_UINT16) );
    def_table_assign_def( &def_table, "u32", create_type_def("u32", TYPE_VALUE_UINT32) );
    def_table_assign_def( &def_table, "u64", create_type_def("u64", TYPE_VALUE_UINT64) );

    /* Assigning alias types */
    def_table_assign_def( &def_table, "uint32_t", create_alias_def("uint32_t", "u32") );
    def_table_assign_def( &def_table, "float",    create_alias_def("float",    "f32") );

    /* Assigning struct types */
    StructDefData struct_data = create_struct_def_data();

    {
        struct_data.fields_idents[0] = "x";
        struct_data.fields_types[0] = "float";
        struct_data.fields_idents[1] = "y";
        struct_data.fields_types[1] = "float";
        struct_data.count = 2;

        def_table_assign_def( &def_table, "Vector2f", create_struct_def("Vector2f", struct_data) );
    }

    for (int32_t i = 0; i < def_table.count; ++i) {
        const char* ident     = def_table.defs_idents[i];
        const Definition* def = &def_table.defs_data[i];

        printf("%s", ident);

        switch (def->type) {
            case DEF_TYPE_ALIASTYPE:  printf("\t: alias "); break;
            case DEF_TYPE_VALUETYPE:  printf("\t: value "); break;
            case DEF_TYPE_STRUCT:     printf("\t: struct"); break;
            default: assert(false);
        }

        switch (def->type) {
            case DEF_TYPE_ALIASTYPE:  printf(" = %s", def->data.data_alias);                break;
            case DEF_TYPE_VALUETYPE:  printf(" = %s", ValueTypeNames[def->data.data_type]); break;
            case DEF_TYPE_STRUCT:
                printf(" = {");

                for (int32_t j = 0; j < def->data.data_struct.count; ++j) {
                    const char* name = def->data.data_struct.fields_idents[j];
                    const char* type = def->data.data_struct.fields_types[j];
                    printf(" %s: %s,", name, type);
                }

                printf(" }");
                break;

            default: assert(false);
        }

        printf("\n");
    }

    free_def_table(&def_table);
}
