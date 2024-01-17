#pragma once

#ifndef MC_HEADER_H
#define MC_HEADER_H

#include <mcdef.h>
#include <mcobj.h>
#include <mcmatrix.h>
#include <mcstr.h>

///*
//*
//* We use a tagged pointer for all mclang object for now.
//* The format of the pointer tag is as follows.
//*
//* Since we support only x64 platform, all the adresses are multiples of 8. And this allows
//* us to use the least significant 3 bits for our own purposes.
//*
//*         ╔═════════════════════════╦══════════════════════════╗
//*         ║ Object Type (Two Bits)  ║ Reference Type (One Bit) ║
//* ╔═══════╬════════════╦════════════╬══════════════════════════╣
//* ║  Bits ║      2     ║     1      ║             0            ║
//* ╚═══════╩════════════╩════════════╩══════════════════════════╝
//*/
//
//#define MC_OBJ_TAG_BITS 3
//
//#define MC_OBJ_TYPE_BITS 2
//#define MC_OBJ_REFTYPE_BITS 1
//
//#define MC_OBJ_TAG_MASK					((1 << (MC_OBJ_TAG_BITS + 1)) - 1)
//#define MC_OBJ_TYPE_MASK				(((1 << (MC_OBJ_TYPE_BITS + 1)) - 1) << (MC_OBJ_TAG_BITS - MC_OBJ_TYPE_BITS)) 
//#define MC_OBJ_REFTYPE_MASK				(((1 << (MC_OBJ_REFTYPE_BITS + 1)) - 1) << (MC_OBJ_TAG_BITS - MC_OBJ_REFTYPE_BITS))
//
//#define MC_OBJ_TYPE_MATRIX				00u
//#define MC_OBJ_TYPE_STRING				01u
//#define MC_OBJ_TYPE_SYM					02u
//#define MC_OBJ_TYPE_FUNC				03u
//
//#define MC_OBJ_REFTYPE_SIMPLE			0u
//#define MC_OBJ_REFTYPE_COW				1u
//
//#define MC_OBJ_GET_TYPE(obj)			((obj & MC_OBJ_TYPE_MASK) >> MC_OBJ_REFTYPE_BITS)
//#define MC_OBJ_GET_REFTYPE(obj)			(obj & MC_OBJ_TYPE_MASK)
//#define MC_OBJ_SET_TYPE(obj, type)		(obj = (obj | (type << MC_OBJ_REFTYPE_BITS)))
//#define MC_OBJ_SET_REFTYPE(obj, type)	(obj = (obj | type))
//#define MC_OBJ_MAKE_COW(obj)			MC_OBJ_SET_REFTYPE(obj, MC_OBJ_REFTYPE_COW)
//
//#define MC_OBJ_GET_PTR(obj)				(obj = (obj & ~MC_OBJ_TAG_MASK))							



void mc_disp(mc_obj_t obj);
void mc_print(mc_string_t format, mc_obj_t obj);

enum matrix_class
{
	MCLS_DOUBLE,
	MCLS_STRING,
	MCLS_LOGICAL,
	MCLS_SYMBOLIC,
};

#define MATRIX_SZ				sizeof(mc_matrix_t)

/*
* A generic matrix type.
*/
typedef struct
{
	size_t row_sz;
	union
	{
		size_t length;
		size_t column_sz;
	};
	size_t end_m;
	union 
	{	
		size_t end_n;
		size_t end;
	};
	size_t nb_of_elements;
	
	
	enum matrix_class cls;
	union
	{
		void* block;
		union
		{
			double* dblock;
		};
		union
		{
			mc_bool_t* lblock;
		};
		mc_string_t* sblock;
	};
}mc_matrix_t;

typedef struct
{
	size_t iter_pos;
	size_t nb_of_elements;
	union
	{
		void* writer;
		double* driter;
		double* dwiter;
		mc_bool_t* lriter;
		mc_bool_t* lwiter;
		mc_string_t* sriter;
		mc_string_t* switer;
	};
	mc_matrix_t* iter;
	mc_matrix_t* matrix;
}mc_matrix_iter_t;

mc_matrix_iter_t* mc_matrix_riter(mc_matrix_t* _matrix);
mc_matrix_iter_t* mc_matrix_witer(mc_matrix_t* _matrix);
mc_matrix_iter_t* mc_matrix_reset(mc_matrix_iter_t* _iter);
mc_bool_t mc_matrix_riter_next(mc_matrix_iter_t* _iter);
mc_bool_t mc_matrix_witer_next(mc_matrix_iter_t* _iter);

typedef mc_matrix_t mc_array_t;

typedef mc_array_t mc_vector_t;

typedef mc_array_t mc_logical_t;

mc_matrix_t* mc_matrix_alloc(size_t _m, size_t _n, enum matrix_class _cls, bool _empty);
mc_matrix_t* mc_array_alloc(size_t _len, enum matrix_class _cls);

MC_INLINE mc_matrix_t* mc_array_alloc(size_t _len, enum matrix_class _cls);
MC_INLINE mc_matrix_t* mc_matrix_double_alloc(size_t _m, size_t _n);
MC_INLINE mc_matrix_t* mc_array_double_alloc(size_t _len);
MC_INLINE mc_matrix_t* mc_scalar_double_alloc(double scalar);

MC_INLINE mc_matrix_t* mc_matrix_logical_alloc(size_t _m, size_t _n);
MC_INLINE mc_matrix_t* mc_array_logical_alloc(size_t _len);
MC_INLINE mc_matrix_t* mc_scalar_logical_alloc(mc_bool_t scalar);

MC_INLINE mc_matrix_t* mc_matrix_empty_alloc(size_t _m, size_t _n, enum matrix_class _cls);
MC_INLINE mc_matrix_t* mc_array_empty_alloc(size_t _len, enum matrix_class _cls);
MC_INLINE mc_matrix_t* mc_scalar_empty_alloc(enum matrix_class _cls);

MC_INLINE mc_matrix_t* mc_matrix_empty_double_alloc(size_t _m, size_t _n);
MC_INLINE mc_matrix_t* mc_array_empty_double_alloc(size_t _len);
MC_INLINE mc_matrix_t* mc_scalar_empty_double_alloc();

MC_INLINE mc_matrix_t* mc_matrix_empty_logical_alloc(size_t _m, size_t _n);
MC_INLINE mc_matrix_t* mc_array_empty_logical_alloc(size_t _len);
MC_INLINE mc_matrix_t* mc_scalar_empty_logical_alloc();

MC_INLINE mc_matrix_t* mc_matrix_empty_string_alloc(size_t _m, size_t _n);
MC_INLINE mc_matrix_t* mc_array_empty_string_alloc(size_t _len);
MC_INLINE mc_matrix_t* mc_scalar_empty_string_alloc();
 
mc_matrix_t* mc_matrix_empty_clone(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_make_point_dblock(mc_matrix_t* _matrix, double* block);
mc_matrix_t* mc_matrix_make_point_lblock(mc_matrix_t* _matrix, mc_bool_t* block);
mc_matrix_t* mc_matrix_make_point_sblock(mc_matrix_t* _matrix, mc_string_t* block);

mc_matrix_t* mc_matrix_build_2dsubscript(mc_matrix_t* _matrix, mc_matrix_t* _rows, mc_matrix_t* _columns);
mc_matrix_t* mc_matrix_build_1dsubscript(mc_matrix_t* _linear);

mc_matrix_t* mc_matrix_subscript_get(mc_matrix_t* _matrix, mc_matrix_t* _subscript);
mc_matrix_t* mc_matrix_subscript_set(mc_matrix_t* _matrix, mc_matrix_t* _subscript, mc_matrix_t* value);


mc_matrix_t* mc_matrix_copy(mc_matrix_t* _dst, mc_matrix_t* _dest_pos, mc_matrix_t* _src, mc_matrix_t* _src_pos);

MC_INLINE bool mc_matrix_is_array(mc_matrix_t* _matrix);

#define mc_matrix_is_row_vector mc_matrix_is_array

MC_INLINE bool mc_matrix_is_scalar(mc_matrix_t* _matrix);
MC_INLINE bool mc_matrix_is_vector(mc_matrix_t* _matrix);
MC_INLINE bool mc_matrix_is_matrix(mc_matrix_t* _matrix);
MC_INLINE bool mc_matrix_is_column_vector(mc_matrix_t* _matrix); 

/**
 * @brief Conversion functions between logical and double classes.
 * @param _matrix 
 * @return 
*/
mc_matrix_t* mc_matrix_double(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_logical(mc_matrix_t* _matrix);

double mc_matrix_double_get(mc_matrix_t* _matrix, size_t _i, size_t _j);
double mc_array_double_get(mc_array_t* _array, size_t _i);

mc_bool_t mc_matrix_logical_get(mc_matrix_t* _matrix, size_t _i, size_t _j);
mc_bool_t mc_array_logical_get(mc_array_t* _array, size_t _i);

mc_string_t mc_matrix_str_get(mc_matrix_t* _matrix, size_t _i, size_t _j);
mc_string_t mc_array_str_get(mc_array_t* _array, size_t _i);

double mc_matrix_double_set(mc_matrix_t* _matrix, size_t _i, size_t _j, double value);
double mc_array_double_set(mc_array_t* _array, size_t _i, double value);

mc_bool_t mc_matrix_logical_set(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_bool_t value);
mc_bool_t mc_array_logical_set(mc_array_t* _array, size_t _i, mc_bool_t value);

mc_string_t mc_matrix_str_set(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_string_t value);
mc_string_t mc_array_str_set(mc_array_t* _array, size_t _i, mc_string_t value);

double mc_matrix_double_pushs(mc_matrix_t* _matrix, double scalar);
mc_matrix_t* mc_matrix_double_pushm(mc_matrix_t* _matrix, mc_matrix_t* __matrix);
double mc_array_double_get(mc_array_t* _array, size_t _i);

mc_bool_t mc_matrix_logical_get(mc_matrix_t* _matrix, size_t _i, size_t _j);
mc_bool_t mc_array_logical_get(mc_array_t* _array, size_t _i);

mc_string_t mc_matrix_str_get(mc_matrix_t* _matrix, size_t _i, size_t _j);
mc_string_t mc_array_str_get(mc_array_t* _array, size_t _i);

double mc_matrix_double_set(mc_matrix_t* _matrix, size_t _i, size_t _j, double value);
double mc_array_double_set(mc_array_t* _array, size_t _i, double value);

mc_bool_t mc_matrix_logical_set(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_bool_t value);
mc_bool_t mc_array_logical_set(mc_array_t* _array, size_t _i, mc_bool_t value);

mc_string_t mc_matrix_str_set(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_string_t value);
mc_string_t mc_array_str_set(mc_array_t* _array, size_t _i, mc_string_t value);
 
mc_matrix_t* mc_matrix_equals(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_nequals(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_size(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_numel(mc_matrix_t* _matrix);
mc_matrix_t* mc_array_length(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_rowsz(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_colsz(mc_matrix_t* _matrix);

mc_matrix_t* mc_array_gen_d(double _start, double _increment, double _end);
mc_matrix_t* mc_array_gen_m(mc_matrix_t* _start, mc_matrix_t* _increment, mc_matrix_t* _end);

mc_matrix_t* mc_matrix_resize(mc_matrix_t* _matrix, mc_matrix_t* size);
mc_matrix_t* mc_matrix_horzcat(mc_matrix_t* _left, mc_matrix_t* _right);  
mc_matrix_t* mc_matrix_vercat(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_zeros(mc_matrix_t* size);
mc_matrix_t* mc_matrix_ones(mc_matrix_t* size);

mc_matrix_t* mc_matrix_transp(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_neg(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_add(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_sub(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_div(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_mul(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_pow(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_dotmul(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_dotdiv(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_dotpow(mc_matrix_t* _left, mc_matrix_t* _right);

mc_bool_t    mc_matrix_is_true(mc_matrix_t* _matrix);
mc_bool_t    mc_matrix_is_false(mc_matrix_t* _matrix);
mc_matrix_t* mc_matrix_land(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_lor(mc_matrix_t* _left, mc_matrix_t* _right);
mc_matrix_t* mc_matrix_lneg(mc_matrix_t* _left);

mc_matrix_t* mc_matrix_lt(mc_matrix_t* _left, mc_matrix_t* _right); 
mc_matrix_t* mc_matrix_lte(mc_matrix_t* _left, mc_matrix_t* _right); 
mc_matrix_t* mc_matrix_gt(mc_matrix_t* _left, mc_matrix_t* _right); 
mc_matrix_t* mc_matrix_gte(mc_matrix_t* _left, mc_matrix_t* _right); 

mc_string_t mc_matrix_to_str(mc_matrix_t* _matrix);

#endif