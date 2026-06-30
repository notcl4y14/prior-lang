#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "parser.h"
#include "interp.h"

#define SEMANTICS_ERROR_SIZE 256

typedef struct Semantics {
    char errmsg[SEMANTICS_ERROR_SIZE];
    bool error;
    Scope* scope;
} Semantics;

void set_semantics_error(Semantics* s, const char* errmsg);
const char* get_semantics_error(Semantics* s);
Semantics create_semantics(Scope* scope);
void process_semantics(Semantics* s, Node* ast);

#endif
