#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "lexer/token.h"
#include "parser/parser.h"
#include "semantics/scope.h"
#include "error.h"

#define SEMANTICS_ERROR_SIZE 256

typedef struct Semantics {
    ErrorList error_list;
    Scope*    scope;
} Semantics;

Semantics create_semantics(Scope* scope);
void semantics_add_error(Semantics* s, const char* errmsg, TokenPosition position);
void process_semantics(Semantics* s, Node* ast);

#endif
