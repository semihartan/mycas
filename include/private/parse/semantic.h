#pragma once

#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <parse/ast.h>

void sem_array_must_be_identifier(anode_t* array);
void sem_subscript_must_be_2D(anode_t* subscript);
void semantic_analyze(anode_t* root);

#endif