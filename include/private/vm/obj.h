#pragma once

#ifndef OBJ_H
#define OBJ_H

#include "ht.h"

#include <stdint.h>



enum func_type
{
	FUNC_TYPE_INTERNAL,
	FUNC_TYPE_NATIVE, // Reserved for future
	FUNC_TYPE_RUTIME,
};

typedef struct
{
	const char* name;
	uint32_t inargc;
	enum func_type func_type;
	void* address;
}func_info_t;

enum atrr_type
{
	OBJ_REF,
	OBJ_VAL,
};

typedef struct
{
	const char* name;
	enum atrr_type atrr_type;
	uint64_t type_hash;
}attr_info_t;

typedef struct
{
	const char* name;
	uint64_t type_hash;
	htab_t* pv_attrs;
	htab_t* pb_attrs;
	htab_t* func_table;
}mc_type_t;

typedef struct
{ 
	mc_type_t* type;
}mc_object;

enum class
{
	CLS_DOUBLE,
	CLS_LOGICAL,
	CLS_SYM,
};

//typedef struct
//{
//
//}mc_class;


#endif