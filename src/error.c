#include <error.h>
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
