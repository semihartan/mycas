#pragma once

#ifndef LEXER_STACK_H
#define LEXER_STACK_H

typedef struct
{
	int array[64];
	int top;
}lexer_stack_t;

#endif