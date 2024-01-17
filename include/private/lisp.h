#pragma once
#ifndef LISP_H
#define LISP_H

#include <stdint.h>

enum cell_type
{
	CELL_TYPE_CONS,
	CELL_TYPE_SYMBOL,
	CELL_TYPE_NUMBER
};

typedef void* cell_t;

cell_t lisp_alloc_number(double value);
cell_t lisp_alloc_symbol(char* const symbol);
//cell_t lisp_alloc_number(cell);


#endif