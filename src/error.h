#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>
#include <token.h>

#define ERROR_MESSAGE_SIZE 256
#define ERROR_LIST_MAX 128

typedef struct Error {
    char          errmsg[ERROR_MESSAGE_SIZE];
    TokenPosition position;
} Error;

Error create_error(const char* errmsg, TokenPosition errpos);

typedef struct ErrorList {
    Error    errors[ERROR_LIST_MAX];
    uint32_t count;
} ErrorList;

ErrorList create_error_list();
void add_to_error_list(ErrorList* errlist, Error error);

#endif
