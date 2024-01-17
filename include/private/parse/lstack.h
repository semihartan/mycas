#pragma once

#ifndef LEXER_STACK_H
#define LEXER_STACK_H

#include <parse/parser.h> 

lexer_t* push_lexer(parser_t* self, lexer_t* lexer);
lexer_t* pop_lexer(parser_t* self);

#endif