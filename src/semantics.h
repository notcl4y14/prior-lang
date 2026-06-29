#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "parser.h"

#define SEMANTICS_ERROR_SIZE 256

typedef struct Semantics {
    char errmsg[SEMANTICS_ERROR_SIZE];
    bool error;
} Semantics;

void set_semantics_error(Semantics* s, const char* errmsg);
const char* get_semantics_error(Semantics* s);
Semantics create_semantics();
void process_semantics(Semantics* s, Node* ast);

#endif
