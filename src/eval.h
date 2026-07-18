#ifndef EVAL_H
#define EVAL_H

/***
 * The EVAL module contains SOME of the evaluating functions.
 * Specifically expressions that calculate numbers.
 * That's it. It's used for semantics comptime calculation.
 */

#include "parser/parser.h"
#include "scope.h"
#include "value.h"

typedef struct EvalRes {
    Value value;
    bool  error;
} EvalRes;

EvalRes evaluate_expr_node(Scope* scope, Node* node);

#endif
