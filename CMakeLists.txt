cmake_minimum_required(VERSION 3.10)

project(priorlang VERSION 1.0 LANGUAGES C)

add_library(prior_core STATIC
    src/ast.c
    src/error.c
    src/interp.c
    src/lexer.c
    src/mem.c
    src/parser.c
    src/printast.c
    src/semantics.c
    src/value.c
    src/utils.c
)
target_include_directories(prior_core PUBLIC src)

# commandline executable
add_executable(prior src/main.c)
target_link_libraries(prior PRIVATE prior_core)

enable_testing()

# testing
add_executable(test_lexer EXCLUDE_FROM_ALL test/lexer.c)
target_link_libraries(test_lexer PRIVATE prior_core)
target_compile_definitions(test_lexer PRIVATE
    ROOT_DIR="${CMAKE_SOURCE_DIR}"
)
add_test(NAME lexer_test COMMAND test_lexer)

add_executable(test_main EXCLUDE_FROM_ALL test/main.c)
target_link_libraries(test_main PRIVATE prior_core)
add_test(NAME main_test COMMAND test_main)

add_custom_target(build_tests
    DEPENDS test_lexer test_main
)