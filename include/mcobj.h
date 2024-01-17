#pragma once

#ifndef MC_OBJECT_H
#define MC_OBJECT_H

#include <mcdef.h>

#define OBJ_HEAD_SZ						sizeof(struct obj_head)

#define DEF_OBJ_HEAD_PTR()				uintptr_t obj_head_ptr

#define CALCULATE_OBJ_HEAD(x)			x->obj_head_ptr = (uint8_t*)x - OBJ_HEAD_SZ

#define REFERENCE_EQ(x, y)				((mc_obj_t)x == (mc_obj_t)y)

enum obj_type
{
	OK_MATRIX,
	OK_MATRIX_VIEW,
	OK_STRING,
	OK_SYM,
	OK_FUNC,
};

enum ref_type
{
	RT_SIMPLE,
	RT_COW,
};

#define MC_OBJECT_SZ		sizeof(mc_obj_t)

typedef uintptr_t mc_obj_t;

typedef mc_obj_t mc_string_t; 

typedef mc_string_t (*str_func_t)(mc_obj_t self);

struct obj_head
{
	uint8_t type : 2;
	uint8_t ref : 1;
	str_func_t str_func;
};

MC_INLINE mc_obj_t mc_obj_make_simple(mc_obj_t _obj);
MC_INLINE mc_obj_t mc_obj_make_cow(mc_obj_t _obj);
MC_INLINE enum obj_type mc_obj_get_obj_type(mc_obj_t _obj);
MC_INLINE enum ref_type mc_obj_get_ref_type(mc_obj_t _obj);
MC_INLINE enum ref_type mc_obj_get_is_scalar(mc_obj_t _obj);

mc_obj_t mc_obj_alloc(size_t sz, enum obj_type type, enum ref_type ref);
 
mc_string_t mc_obj_str(mc_obj_t self);

void mc_obj_set_str_func(mc_obj_t obj, str_func_t str_func);


#endif