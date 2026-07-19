#include "error.h"

#include <stdio.h>
#include <string.h>

Error create_error(const char* errmsg, TokenPosition errpos) {
    Error error = (Error) { 0 };

    strncpy(error.errmsg, errmsg, ERROR_MESSAGE_SIZE);
    error.position = errpos;

    return error;
}



ErrorList create_error_list() {
    return (ErrorList) {
        .errors = { {} },
        .count = 0,
    };
}

void add_to_error_list(ErrorList* errlist, Error error) {
    errlist->errors[errlist->count++] = error;
}

void print_error_list(ErrorList* errlist) {
    for (int32_t i = 0; i < errlist->count; ++i) {
        const Error* error = &errlist->errors[i];
        printf("%ld:%ld: %s\n", error->position.line + 1, error->position.column + 1, error->errmsg);
    }
}
