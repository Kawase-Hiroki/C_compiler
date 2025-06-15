#ifndef GENERATOR_H
#define GENERATOR_H

#include "parse.h"

extern int count;

void gen_lval(Node *node);
void gen(Node *node);

#endif