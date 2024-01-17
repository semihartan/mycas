#include <mcmatrix.h>

#include <mc.h>
#include <mcdef.h>
#include <error.h>

#include <mcmath.h>
#include <matrix.double.h>
#include <matrix.bool.h>

#include <adt/sb.h>
#include <mcstr.h>

#include <gc/gc.h>

//#if defined(MC_RUNTIME)
//	#define NORM_INDEX(i)			((i) - 1)
//	#define NORM_BOUND(i)			((i) + 1)
//#else
//	#define NORM_INDEX(i)			(i)
//	#define NORM_BOUND(i)			(i)
//#endif // defined(MC_RUNTIME)

#define CALCELTS(start, increment, end)										\
int nb_of_elts;																\
bool rem;																	\
do																			\
{																			\
	double diff = end - start;                                              \
	rem = mc_fmod(diff, increment) > 0.0;                                   \
	nb_of_elts = (size_t)ceil(diff / (increment)) + (rem ? 0 : 1);			\
}while(0)

#define NORM_INDEX(i)			((i) - 1)
#define NORM_BOUND(i)			((i) - 1)

#define IS_SAME_CLASS(m1, m2)		((m1)->cls		== (m2)->cls)
#define IS_DOUBLE(m)				((m)->cls		== MCLS_DOUBLE)
#define IS_LOGICAL(m)				((m)->cls		== MCLS_LOGICAL)
#define IS_STRING(m)				((m)->cls		== MCLS_STRING)
#define IS_SYMBOLIC(m)				((m)->cls		== MCLS_SYMBOLIC)
#define IS_SCALAR(m)				((m)->row_sz	== 1 && (m)->column_sz == 1)
#define IS_MATRIX(m)				mc_matrix_is_matrix(m)
#define IS_ARRAY(m)					mc_matrix_is_array(m)
#define IS_ROW(m)					mc_matrix_is_row_vector(m)
#define IS_COLUMN(m)				mc_matrix_is_column_vector(m)
#define ARRAY_GET(m, j)				mc_matrix_is_array(m)
#define MATRIX_GET(m, i, j)			mc_matrix_is_array(m)

MC_SINLINE bool _check_matrix_bounds(mc_matrix_t* matrix, size_t i, size_t j)
{
	//#if defined(MC_RUNTIME)
	//	return i >= NORM_BOUND(0) && i < NORM_BOUND(matrix->row_sz) && j >= NORM_BOUND(0) && j < NORM_BOUND(matrix->column_sz);
	//#else
	//	return i >= 0 && i < (matrix->row_sz) && j >= 0 && j < (matrix->column_sz);
	//#endif // defined(MC_RUNTIME)  
	return i >= 1 && i <= (matrix->row_sz) && j >= 1 && j <= (matrix->column_sz);
}
MC_SINLINE bool _check_array_bounds(mc_matrix_t* matrix, size_t i)
{
	//#if defined(MC_RUNTIME)
	//	return i >= NORM_BOUND(0) && i < NORM_BOUND(matrix->length);
	//#else
	//	return i >= 0 && i < matrix->length;
	//#endif // defined(MC_RUNTIME) 
	return i >= 1 && i <= matrix->length;
}

MC_SINLINE double mc_matrix_double_get_u(mc_matrix_t* _matrix, size_t _i, size_t _j);
MC_SINLINE double mc_array_double_get_u(mc_array_t* _matrix, size_t _i);
MC_SINLINE mc_bool_t mc_matrix_logical_get_u(mc_matrix_t* _matrix, size_t _i, size_t _j);
MC_SINLINE mc_bool_t mc_array_logical_get_u(mc_array_t* _array, size_t _i);
MC_SINLINE double mc_matrix_double_set_u(mc_matrix_t* _matrix, size_t _i, size_t _j, double value);
MC_SINLINE double mc_array_double_set_u(mc_array_t* _array, size_t _i, double value);
MC_SINLINE mc_bool_t mc_matrix_logical_set_u(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_bool_t value);
MC_SINLINE mc_bool_t mc_array_logical_set_u(mc_array_t* _array, size_t _i, mc_bool_t value);
MC_SINLINE mc_string_t mc_matrix_str_get_u(mc_matrix_t* _matrix, size_t _i, size_t _j);
MC_SINLINE mc_string_t mc_array_str_get_u(mc_array_t* _array, size_t _i);
MC_SINLINE mc_string_t mc_matrix_str_set_u(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_string_t value);
MC_SINLINE mc_string_t mc_array_str_set_u(mc_array_t* _array, size_t _i, mc_string_t value);

//static mc_matrix_t* mc_matrix_copy(mc_matrix_t* _dst, mc_matrix_t* _dest_pos, mc_matrix_t* _src, mc_matrix_t* _src_pos);
static mc_matrix_t* mc_matrix_realloc(mc_matrix_t* _matrix, size_t _nm, size_t _nn);

mc_matrix_t* mc_matrix_alloc(size_t _m, size_t _n, enum matrix_class _cls, bool _empty)
{
	uint8_t* obj = mc_obj_alloc(MATRIX_SZ + sizeof(uintptr_t), OK_MATRIX, RT_COW);
	*(uintptr_t*)obj = obj - OBJ_HEAD_SZ;
	mc_matrix_t* self = obj + sizeof(uintptr_t);
	MC_ABORT_OUT_OF_MEMORY(self); 
	self->row_sz = _m;
	self->column_sz = _n;
	self->end_m = _m;
	self->end_n = _n; 
	self->nb_of_elements = _m * _n;
	self->cls = _cls;
	if(!_empty)
	{
		switch (_cls)
		{
		case MCLS_DOUBLE:
			self->block = GC_MALLOC(sizeof(double) * self->nb_of_elements);
			break;
		case MCLS_STRING:
			self->block = GC_MALLOC(sizeof(mc_string_t) * self->nb_of_elements);
			break;
		case MCLS_LOGICAL:
			self->block = GC_MALLOC(sizeof(mc_bool_t) * self->nb_of_elements);
			break;
		case MCLS_SYMBOLIC:
			break;
		default:
			break;
		}
		MC_ABORT_OUT_OF_MEMORY(self->block);  
	}
	mc_obj_set_str_func(self, mc_matrix_to_str);
	return self;
}
MC_INLINE mc_matrix_t* mc_array_alloc(size_t _len, enum matrix_class _cls)
{
	return mc_matrix_alloc(1, _len, _cls, false);
}
MC_INLINE mc_matrix_t* mc_matrix_double_alloc(size_t _m, size_t _n)
{
	return mc_matrix_alloc(_m, _n, MCLS_DOUBLE, false);
}
MC_INLINE mc_matrix_t* mc_array_double_alloc(size_t _len)
{
	return mc_array_alloc(_len, MCLS_DOUBLE);
}
MC_INLINE mc_matrix_t* mc_matrix_logical_alloc(size_t _m, size_t _n)
{
	return mc_matrix_alloc(_m, _n, MCLS_LOGICAL, false);
}
MC_INLINE mc_matrix_t* mc_array_logical_alloc(size_t _len) 
{
	return mc_array_alloc(_len, MCLS_LOGICAL);
}
MC_INLINE mc_matrix_t* mc_scalar_double_alloc(double scalar)
{
	mc_matrix_t* _scalar = mc_array_double_alloc(1);
	mc_array_double_set_u(_scalar, 1, scalar);
	return _scalar;
}
MC_INLINE mc_matrix_t* mc_scalar_logical_alloc(mc_bool_t scalar)
{
	mc_matrix_t* _scalar = mc_array_logical_alloc(1);
	mc_array_logical_set_u(_scalar, 1, scalar);
	return _scalar;
}

MC_INLINE mc_matrix_t* mc_matrix_empty_alloc(size_t _m, size_t _n, enum matrix_class _cls)
{
	return mc_matrix_alloc(_m, _n, _cls, true);
}
MC_INLINE mc_matrix_t* mc_array_empty_alloc(size_t _len, enum matrix_class _cls)
{
	return mc_matrix_empty_alloc(1, _len, _cls);
}
MC_INLINE mc_matrix_t* mc_scalar_empty_alloc(enum matrix_class _cls)
{
	return mc_array_empty_alloc(1, _cls);
}

MC_INLINE mc_matrix_t* mc_matrix_empty_double_alloc(size_t _m, size_t _n)
{
	return mc_matrix_alloc(_m, _n, MCLS_DOUBLE, true);
}
MC_INLINE mc_matrix_t* mc_array_empty_double_alloc(size_t _len)
{
	return mc_matrix_empty_alloc(1, _len, MCLS_DOUBLE);
}
MC_INLINE mc_matrix_t* mc_scalar_empty_double_alloc()
{
	return mc_array_empty_alloc(1, MCLS_DOUBLE);
}

MC_INLINE mc_matrix_t* mc_matrix_empty_logical_alloc(size_t _m, size_t _n)
{
	return mc_matrix_alloc(_m, _n, MCLS_LOGICAL, true);
}
MC_INLINE mc_matrix_t* mc_array_empty_logical_alloc(size_t _len)
{
	return mc_matrix_empty_alloc(1, _len, MCLS_LOGICAL);
}
MC_INLINE mc_matrix_t* mc_scalar_empty_logical_alloc()
{
	return mc_array_empty_alloc(1, MCLS_LOGICAL);
}

MC_INLINE mc_matrix_t* mc_matrix_empty_string_alloc(size_t _m, size_t _n)
{
	return mc_matrix_alloc(_m, _n, MCLS_STRING, true);
}
MC_INLINE mc_matrix_t* mc_array_empty_string_alloc(size_t _len)
{
	return mc_matrix_empty_alloc(1, _len, MCLS_STRING);
}
MC_INLINE mc_matrix_t* mc_scalar_empty_string_alloc()
{
	return mc_array_empty_alloc(1, MCLS_STRING);
}

mc_matrix_t* mc_matrix_empty_clone(mc_matrix_t* _matrix)
{
	return mc_matrix_alloc(_matrix->row_sz, _matrix->column_sz, _matrix->cls, false);
}

mc_matrix_t* mc_matrix_make_point_dblock(mc_matrix_t* _matrix, double* block)
{
	_matrix->dblock = block;
	_matrix->cls = MCLS_DOUBLE;
	return _matrix;
}
mc_matrix_t* mc_matrix_make_point_lblock(mc_matrix_t* _matrix, mc_bool_t* block)
{
	_matrix->lblock = block;
	_matrix->cls = MCLS_LOGICAL;
	return _matrix;
}
mc_matrix_t* mc_matrix_make_point_sblock(mc_matrix_t* _matrix, mc_string_t* block)
{
	_matrix->sblock = block;
	_matrix->cls = MCLS_STRING;
	return _matrix;
}
 

static bool _has_two_or_one_el(mc_matrix_t* _matrix)
{
	if (!IS_ARRAY(_matrix))
		MC_ABORT(L"Size must be array.");

	if (_matrix->length > 2 || _matrix->length < 1)
		MC_ABORT(L"Only two dimension is supported in this alpha version.");
}

mc_matrix_t* mc_matrix_copy(mc_matrix_t* _dst, mc_matrix_t* _dst_pos, mc_matrix_t* _src, mc_matrix_t* _src_pos)
{
	if(!IS_SAME_CLASS(_dst, _src))
		MC_ABORT(L"Source and Destinations must be of the same type.");
	if (!_has_two_or_one_el(_src_pos))
		MC_ABORT(L"Position must be an array of two or one element."); 
	if (!_has_two_or_one_el(_dst_pos))
		MC_ABORT(L"Position must be an array of two or one element.");
	
	if (IS_MATRIX(_dst) && IS_MATRIX(_src))
	{
		size_t src_i = NORM_INDEX(mc_array_double_get_u(_src_pos, 1));
		size_t src_j = NORM_INDEX(mc_array_double_get_u(_src_pos, 2));
		size_t dst_i = NORM_INDEX(mc_array_double_get_u(_dst_pos, 1));
		size_t dst_j = NORM_INDEX(mc_array_double_get_u(_dst_pos, 2));

		if (!_check_matrix_bounds(_src, src_i + 1, src_j + 1))
			MC_ABORT(L"Source position is out of bounds.");
		if (!_check_matrix_bounds(_dst, dst_i + 1, dst_j + 1))
			MC_ABORT(L"Destination position is out of bounds.");


		size_t sdi = _src->row_sz - src_i;
		size_t sdj = _src->column_sz - src_j;
		size_t ddi = _dst->row_sz - dst_i;
		size_t ddj = _dst->column_sz - dst_j;

		size_t di = min(ddi, sdi);
		size_t dj = min(ddj, sdj);

		switch (_dst->cls)
		{
			case MCLS_DOUBLE:
				mc_matrix_copy_double_scalar(_dst->dblock, (dst_i), (dst_j), _src->dblock, (src_i), (src_j), di, dj, _src->column_sz, _dst->column_sz);
				break;
			case MCLS_LOGICAL:
				mc_matrix_copy_logical_scalar(_dst->lblock, (dst_i), (dst_j), _src->lblock, (src_i), (src_j), di, dj, _src->column_sz, _dst->column_sz);
				break; 
		}
	}
	else if (IS_MATRIX(_dst) && IS_ARRAY(_src))
	{
		size_t src_j = NORM_INDEX(mc_array_double_get_u(_src_pos, 1));
		size_t dst_i = NORM_INDEX(mc_array_double_get_u(_dst_pos, 1));
		size_t dst_j = NORM_INDEX(mc_array_double_get_u(_dst_pos, 2));
		size_t sdj = _src->length - src_j;
		dst_j = dst_i * _dst->column_sz + dst_j;
		size_t dj = min(_dst->nb_of_elements - dst_j, sdj);

		switch (_dst->cls)
		{
			case MCLS_DOUBLE:
				mc_array_copy_double_scalar(_dst->dblock, (dst_j), _src->dblock, (src_j), (dj));
				break;
			case MCLS_LOGICAL:
				mc_array_copy_logical_scalar(_dst->lblock, (dst_j), _src->lblock, (src_j), (dj));
				break;
		} 
	}
	else if (IS_ARRAY(_dst) && IS_MATRIX(_src))
	{
		size_t src_i = NORM_INDEX(mc_array_double_get_u(_src_pos, 1));
		size_t src_j = NORM_INDEX(mc_array_double_get_u(_src_pos, 2));
		size_t dst_j = NORM_INDEX(mc_array_double_get_u(_dst_pos, 1));
		size_t ddj = _dst->length - dst_j;
		src_j = src_i * _src->column_sz + src_j;
		size_t dj = min(_src->nb_of_elements - src_j, ddj);
		switch (_dst->cls)
		{
			case MCLS_DOUBLE:
				mc_array_copy_double_scalar(_dst->dblock,  (dst_j), _src->dblock, (src_j), dj);
				break;
			case MCLS_LOGICAL:
				mc_array_copy_logical_scalar(_dst->lblock,  (dst_j), _src->lblock, (src_j), dj);
				break;
		} 
	}
	else if (IS_ARRAY(_dst) && IS_ARRAY(_src))
	{ 
		size_t src_j = NORM_INDEX(mc_array_double_get_u(_src_pos, 1));
		size_t dst_j = NORM_INDEX(mc_array_double_get_u(_dst_pos, 1));
		size_t sdj = _src->length - src_j;
		size_t ddj = _dst->length - dst_j; 
		size_t dj = min(ddj, sdj);
		switch (_dst->cls)
		{
			case MCLS_DOUBLE:
				mc_array_copy_double_scalar(_dst->dblock, (dst_j), _src->dblock, (src_j), dj);
				break;
			case MCLS_LOGICAL:
				mc_array_copy_logical_scalar(_dst->lblock, (dst_j), _src->lblock, (src_j), dj);
				break;
		} 
	}
	return _dst;
}

static mc_matrix_t* mc_matrix_realloc(mc_matrix_t* _matrix, size_t _nm, size_t _nn)
{
	
}

double mc_matrix_double_get(mc_matrix_t* _matrix, size_t _i, size_t _j)
{
	MC_ERROR_ABORT(_check_matrix_bounds(_matrix, _i, _j), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	double* block = _matrix->block; 
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)];
}
double mc_array_double_get(mc_array_t* _matrix, size_t _i)
{
	MC_ERROR_ABORT(_check_array_bounds(_matrix, _i), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	double* block = _matrix->block;
	return block[NORM_INDEX(_i)];
}
mc_bool_t mc_matrix_logical_get(mc_matrix_t* _matrix, size_t _i, size_t _j)
{
	MC_ERROR_ABORT(_check_matrix_bounds(_matrix, _i, _j), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_bool_t* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)];
}
mc_bool_t mc_array_logical_get(mc_array_t* _array, size_t _i)
{
	MC_ERROR_ABORT(_check_array_bounds(_array, _i), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_bool_t* block = _array->block;
	return block[NORM_INDEX(_i)];
}
double mc_matrix_double_set(mc_matrix_t* _matrix, size_t _i, size_t _j, double value)
{
	MC_ERROR_ABORT(_check_matrix_bounds(_matrix, _i, _j), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	double* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)] = value;
}
double mc_array_double_set(mc_array_t* _array, size_t _i, double value)
{
	MC_ERROR_ABORT(_check_array_bounds(_array, _i), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	double* block = _array->block;
	return block[NORM_INDEX(_i)] = value;
}
mc_bool_t mc_matrix_logical_set(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_bool_t value)
{
	MC_ERROR_ABORT(_check_matrix_bounds(_matrix, _i, _j), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_bool_t* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)] = value;
}
mc_bool_t mc_array_logical_set(mc_array_t* _array, size_t _i, mc_bool_t value)
{
	MC_ERROR_ABORT(_check_array_bounds(_array, _i), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_bool_t* block = _array->block;
	return block[NORM_INDEX(_i)] = value;
}

mc_string_t mc_matrix_str_get(mc_matrix_t* _matrix, size_t _i, size_t _j)
{
	MC_ERROR_ABORT(_check_matrix_bounds(_matrix, _i, _j), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_string_t* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)];
}
mc_string_t mc_array_str_get(mc_array_t* _array, size_t _i)
{
	MC_ERROR_ABORT(_check_array_bounds(_array, _i), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_string_t* block = _array->block;
	return block[NORM_INDEX(_i)];
}
mc_string_t mc_matrix_str_set(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_string_t value)
{
	MC_ERROR_ABORT(_check_matrix_bounds(_matrix, _i, _j), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_string_t* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)] = value;
}
mc_string_t mc_array_str_set(mc_array_t* _array, size_t _i, mc_string_t value)
{
	MC_ERROR_ABORT(_check_array_bounds(_array, _i), L"Indices are out of bounds.", MC_ERROR_FAILURE);
	mc_string_t* block = _array->block;
	return block[NORM_INDEX(_i)]= value;
}

MC_SINLINE double mc_matrix_double_get_u(mc_matrix_t* _matrix, size_t _i, size_t _j)
{
	double* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)];
}
MC_SINLINE double mc_array_double_get_u(mc_array_t* _matrix, size_t _i)
{
	double* block = _matrix->block;
	return block[NORM_INDEX(_i)];
}
MC_SINLINE mc_bool_t mc_matrix_logical_get_u(mc_matrix_t* _matrix, size_t _i, size_t _j)
{ 
	mc_bool_t** block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)];
}
MC_SINLINE mc_bool_t mc_array_logical_get_u(mc_array_t* _array, size_t _i)
{
	mc_bool_t* block = _array->block;
	return block[NORM_INDEX(_i)];
}
MC_SINLINE double mc_matrix_double_set_u(mc_matrix_t* _matrix, size_t _i, size_t _j, double value)
{
	double* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)] = value;
}
MC_SINLINE double mc_array_double_set_u(mc_array_t* _array, size_t _i, double value)
{
	double* block = _array->block;
	return block[NORM_INDEX(_i)] = value;
}
MC_SINLINE mc_bool_t mc_matrix_logical_set_u(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_bool_t value)
{
	mc_bool_t* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)] = value;
}
MC_SINLINE mc_bool_t mc_array_logical_set_u(mc_array_t* _array, size_t _i, mc_bool_t value)
{
	mc_bool_t* block = _array->block;
	return block[NORM_INDEX(_i)] = value;
}
MC_SINLINE mc_string_t mc_matrix_str_get_u(mc_matrix_t* _matrix, size_t _i, size_t _j)
{
	mc_string_t* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)];
}
MC_SINLINE mc_string_t mc_array_str_get_u(mc_array_t* _array, size_t _i)
{
	mc_string_t* block = _array->block;
	return block[NORM_INDEX(_i)];
}
MC_SINLINE mc_string_t mc_matrix_str_set_u(mc_matrix_t* _matrix, size_t _i, size_t _j, mc_string_t value)
{
	mc_string_t* block = _matrix->block;
	return block[NORM_INDEX(_i) * _matrix->column_sz + NORM_INDEX(_j)] = value;
}
MC_SINLINE mc_string_t mc_array_str_set_u(mc_array_t* _array, size_t _i, mc_string_t value)
{
	mc_string_t* block = _array->block;
	return block[NORM_INDEX(_i)] = value;
}

static mc_matrix_t* riter = NULL; 
static mc_matrix_t* witer = NULL; 
static mc_matrix_iter_t* mc_matrix_iter_alloc(mc_matrix_t* _matrix)
{
	mc_matrix_iter_t* iter = NEWOBJP(iter);
	MC_ABORT_OUT_OF_MEMORY(iter);
	iter->nb_of_elements = _matrix->nb_of_elements;
	iter->iter_pos = 1;
	iter->writer = _matrix->block;
	iter->matrix = _matrix;
	return iter;
}

static mc_bool_t mc_matrix_double_riter(mc_matrix_iter_t* _iter)
{  
	if (_iter->iter_pos++ <= _iter->nb_of_elements)
	{
		mc_array_double_set_u(_iter->iter, 1, *_iter->driter++);
		return true;
	}
	else
		return false;
}
static mc_bool_t mc_matrix_logical_riter(mc_matrix_iter_t* _iter)
{
	if (_iter->iter_pos++ <= _iter->nb_of_elements)
	{
		mc_array_double_set_u(_iter->iter, 1, *_iter->lriter++);
		return true;
	}
	else
		return false;
} 
static mc_bool_t mc_matrix_double_witer(mc_matrix_iter_t* _iter)
{
	if (_iter->iter_pos++ <= _iter->nb_of_elements)
	{
		mc_matrix_make_point_dblock(_iter->iter, _iter->dwiter++); 
		return true;
	}
	else
		return false;
}
static mc_bool_t mc_matrix_logical_witer(mc_matrix_iter_t* _iter)
{
	if (_iter->iter_pos++ <= _iter->nb_of_elements)
	{
		mc_matrix_make_point_dblock(_iter->iter, _iter->dwiter++);
		return true;
	}
	else
		return false;
}
mc_matrix_iter_t* mc_matrix_riter(mc_matrix_t* _matrix)
{
	mc_matrix_iter_t* riter = mc_matrix_iter_alloc(_matrix);
	riter->iter = mc_array_double_alloc(1);
	return riter;
}
mc_matrix_iter_t* mc_matrix_witer(mc_matrix_t* _matrix)
{
	mc_matrix_iter_t* witer = mc_matrix_iter_alloc(_matrix);
	witer->iter = mc_scalar_empty_double_alloc();
	return witer;
}

mc_matrix_iter_t* mc_matrix_reset(mc_matrix_iter_t* _iter)
{
	_iter->nb_of_elements = _iter->matrix->nb_of_elements;
	_iter->iter_pos = 1;
	_iter->writer = _iter->matrix->block;
	return _iter;
}
mc_bool_t mc_matrix_riter_next(mc_matrix_iter_t* _iter)
{
	if (_iter->matrix->cls != MCLS_DOUBLE && _iter->matrix->cls != MCLS_LOGICAL)
	return false;
	mc_bool_t ret = false;
	switch (_iter->matrix->cls)
	{
	case MCLS_DOUBLE:
		ret = mc_matrix_double_riter(_iter);
		break;
	case MCLS_LOGICAL:
		ret = mc_matrix_logical_riter(_iter);
		break;
	}
	if (!ret)
		_iter->iter_pos = 1;
	return ret;
}
mc_bool_t mc_matrix_witer_next(mc_matrix_iter_t* _iter)
{
	if(_iter->matrix->cls != MCLS_DOUBLE && _iter->matrix->cls != MCLS_LOGICAL)
		return false; 
	mc_bool_t ret = false;
	switch (_iter->matrix->cls)
	{
		case MCLS_DOUBLE:
			ret = mc_matrix_double_witer(_iter);
			break;
		case MCLS_LOGICAL:
			ret = mc_matrix_logical_witer(_iter);
			break;
	}
	if (!ret)
		_iter->iter_pos = 1;
		return ret;
}
MC_INLINE bool mc_matrix_is_array(mc_matrix_t* _matrix)
{
	return _matrix->row_sz == 1;
}

MC_INLINE bool mc_matrix_is_scalar(mc_matrix_t* _matrix)
{
	return IS_SCALAR(_matrix);
}

MC_INLINE bool mc_matrix_is_vector(mc_matrix_t* _matrix)
{
	return _matrix->row_sz == 1 || _matrix->column_sz == 1;
}

MC_INLINE bool mc_matrix_is_matrix(mc_matrix_t* _matrix)
{
	return !mc_matrix_is_array(_matrix);
}

MC_INLINE bool mc_matrix_is_column_vector(mc_matrix_t* _matrix)
{
	return _matrix->column_sz == 1;
}

mc_matrix_t* mc_matrix_double(mc_matrix_t* _matrix)
{
	if (_matrix->cls == MCLS_DOUBLE)
		return _matrix;
	mc_matrix_t* _double = mc_matrix_double_alloc(_matrix->row_sz, _matrix->column_sz);
	
	switch (_matrix->cls)
	{
		case MCLS_LOGICAL:
			mc_matrix_logical_to_double_scalar(_double->dblock, _matrix->lblock, _matrix->nb_of_elements);
			break;
		case MCLS_STRING:
			mc_matrix_str_to_double_scalar(_double->dblock, _matrix->sblock, _matrix->nb_of_elements);
			break;
		default:
			MC_ERROR_ABORT(0, L"Unsupported conversion src type.", MC_ERROR_FAILURE);
			break;
	}
	return _double;
}
mc_matrix_t* mc_matrix_logical(mc_matrix_t* _matrix)
{
	if (_matrix->cls == MCLS_LOGICAL)
		return _matrix;
	mc_matrix_t* _logical = mc_matrix_logical_alloc(_matrix->row_sz, _matrix->column_sz);

	switch (_matrix->cls)
	{
	case MCLS_DOUBLE:
		mc_matrix_double_to_logical_scalar(_logical->lblock, _matrix->dblock, _matrix->nb_of_elements);
		break;
	case MCLS_STRING:
		mc_matrix_str_to_logical_scalar(_logical->lblock, _matrix->sblock, _matrix->nb_of_elements);
		break;
	default:
		MC_ERROR_ABORT(0, L"Unsupported conversion src type.", MC_ERROR_FAILURE);
		break;
	}
	return _logical;
}

mc_matrix_t* mc_matrix_size(mc_matrix_t* _matrix)
{
	mc_matrix_t* matrix = mc_matrix_alloc(1, 2, MCLS_DOUBLE, false);
	mc_matrix_double_set(matrix, 1, 1, _matrix->row_sz);
	mc_matrix_double_set(matrix, 1, 2, _matrix->column_sz);
	return matrix;
}
mc_matrix_t* mc_matrix_numel(mc_matrix_t* _matrix)
{
	return _matrix->nb_of_elements;
}
mc_matrix_t* mc_array_length(mc_matrix_t* _matrix)
{
	mc_matrix_t* matrix = mc_array_alloc(1, MCLS_DOUBLE);
	mc_array_double_set(matrix, 1, _matrix->length);
	return matrix;
}
mc_matrix_t* mc_matrix_rowsz(mc_matrix_t* _matrix)
{
	mc_matrix_t* rowsz = mc_array_double_alloc(1);
	mc_array_double_set_u(rowsz, 1, _matrix->row_sz);
	return rowsz;
}
mc_matrix_t* mc_matrix_colsz(mc_matrix_t* _matrix)
{
	mc_matrix_t* colsz = mc_array_double_alloc(1);
	mc_array_double_set_u(colsz, 1, _matrix->column_sz);
	return colsz;
}

mc_matrix_t* mc_matrix_build_2dsubscript(mc_matrix_t* _matrix, mc_matrix_t* _rows, mc_matrix_t* _columns)
{
	if (!IS_ROW(_rows))
		_rows = mc_matrix_transp(_rows);
	if (!IS_ROW(_columns))
		_rows = mc_matrix_transp(_columns);
	mc_matrix_t* _subscript = mc_matrix_double_alloc(_rows->length, _columns->length);

	for (size_t i = 1; i <= _subscript->row_sz; i++)
	{
		for (size_t j = 1; j <= _subscript->column_sz; j++)
		{
			size_t row = mc_array_double_get_u(_rows, i);
			size_t col = mc_array_double_get_u(_columns, j);

			mc_matrix_double_set_u(_subscript, i, j, (i - 1) * _matrix->column_sz + j);
		}
	}
	return _subscript;
}
mc_matrix_t* mc_matrix_build_1dsubscript(mc_matrix_t* _linear)
{

}
mc_matrix_t* mc_matrix_subscript_get(mc_matrix_t* _matrix, mc_matrix_t* _subscript)
{ 
	mc_matrix_t* els = mc_matrix_empty_clone(_subscript);
	mc_matrix_iter_t* riter = mc_matrix_riter(_subscript);
	mc_matrix_iter_t* witer = mc_matrix_witer(els);
	while (mc_matrix_witer_next(witer) && mc_matrix_riter_next(riter))
	{
		size_t lin_pos = mc_array_double_get_u(riter->iter, 1);
		if (lin_pos < 1 || lin_pos > _matrix->nb_of_elements)
			MC_ABORT(L"Indices are out of bound.");
		double e = mc_array_double_get_u(_matrix, lin_pos);
		mc_array_double_set_u(witer->iter, 1, e);
	}
	return els;
}
mc_matrix_t* mc_matrix_subscript_set(mc_matrix_t* _matrix, mc_matrix_t* _subscript, mc_matrix_t* _value)
{ 
	mc_matrix_iter_t* riter0 = mc_matrix_riter(_subscript);
	mc_matrix_iter_t* riter1 = mc_matrix_riter(_value);
	//mc_matrix_iter_t* witer = mc_matrix_witer(_matrix);
	while (/*mc_matrix_witer_next(witer) && */mc_matrix_riter_next(riter0) && mc_matrix_riter_next(riter1))
	{
		size_t lin_pos = mc_array_double_get_u(riter0->iter, 1);
		double value = mc_array_double_get_u(riter1->iter, 1);
		if (lin_pos < 1 || lin_pos > _matrix->nb_of_elements)
			MC_ABORT(L"Indices are out of bound."); 
		mc_array_double_set_u(_matrix, lin_pos, value);
	}
	return _matrix;
}


mc_matrix_t* mc_matrix_equals(mc_matrix_t* _left, mc_matrix_t* _right)
{
	mc_matrix_t* logical = mc_array_logical_alloc(1);
	if(REFERENCE_EQ(_left, _right))
		return mc_array_logical_set(logical, 1, true), logical;
	else if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		return mc_array_logical_set(logical, 1, false), logical;
	else if (_left->cls != _right->cls)
		return mc_array_logical_set(logical, 1, false), logical;

	logical = mc_matrix_logical_alloc(_left->row_sz, _left->column_sz);
	
	switch (_left->cls)
	{
		case MCLS_DOUBLE:
			for (size_t i = 0; i < logical->nb_of_elements; i++)
			{
				double d1 = mc_array_double_get_u(_left, i + 1);
				double d2 = mc_array_double_get_u(_right, i + 1);
				logical->lblock[i] = d1 == d2;
			}
			break;
		case MCLS_LOGICAL:
			for (size_t i = 0; i < logical->nb_of_elements; i++)
				logical->lblock[i] = ~(mc_array_logical_get_u(_left, i + 1) ^ mc_array_logical_get_u(_right, i + 1));
			break;
	}
	return logical;
}
mc_matrix_t* mc_matrix_nequals(mc_matrix_t* _left, mc_matrix_t* _right)
{
	return mc_matrix_lneg(mc_matrix_equals(_left, _right));
}

mc_matrix_t* mc_array_gen_d(double _start, double _increment, double _end)
{
	int nb_of_elts; 
	bool rem;
	do {
		double diff = _end - _start; 
		rem = mc_fmod(diff, _increment) > 0.0; 
		nb_of_elts = (size_t)fabs(ceil(diff / _increment)) + (rem ? 0 : 1);
	} while (0);

	mc_array_t* array = mc_array_double_alloc(nb_of_elts);

	for (size_t i = 1; i <= nb_of_elts; i++)
	{
		mc_array_double_set_u(array, i, _start);
		_start += _increment;
	} 
	if (!rem)
		mc_array_double_set_u(array, array->end, _end);
	return array;
}
mc_matrix_t* mc_array_gen_m(mc_matrix_t* _start, mc_matrix_t* _increment, mc_matrix_t* _end)
{
	return mc_array_gen_d(mc_array_double_get_u(_start, 1), mc_array_double_get_u(_increment, 1), mc_array_double_get_u(_end, 1));
}

mc_matrix_t* mc_matrix_resize(mc_matrix_t* _matrix, mc_matrix_t* size)
{
	if (!IS_ARRAY(size))
		MC_ABORT(L"Size must be array.");
	size_t m = mc_array_double_get_u(size, 1);
	size_t n = mc_array_double_get_u(size, 2);
	if (_matrix->row_sz		== m && 
		_matrix->column_sz	== n)
		return _matrix;
	mc_matrix_t* __matrix = mc_matrix_alloc(m, n, _matrix->cls, false);
	mc_matrix_t* dstpos = mc_matrix_double_alloc(1, 2);
	mc_array_double_set_u(dstpos, 1, 1);
	mc_array_double_set_u(dstpos, 2, 1);

	mc_matrix_t* srcpos = mc_matrix_double_alloc(1, 2);
	mc_array_double_set_u(srcpos, 1, 1);
	mc_array_double_set_u(srcpos, 2, 1);

	mc_matrix_copy(__matrix, dstpos, _matrix, srcpos);
	return __matrix;
}

static mc_matrix_t* mc_matrix_linmap_pos(mc_matrix_t* _matrix, mc_matrix_t* pos)
{
	if (!IS_ARRAY(pos) || pos->length > 2)
		MC_ABORT(L"Position must be array of size 2.");
	size_t posi = mc_array_double_get_u(pos, 1);
	size_t posj = mc_array_double_get_u(pos, 2);

	double mapped = (posi - 1) * _matrix->column_sz + posj;
	if (mapped > _matrix->nb_of_elements)
		mapped = _matrix->nb_of_elements;
	pos = mc_scalar_double_alloc(mapped);
	return pos;
}

mc_matrix_t* mc_matrix_horzcat(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_SAME_CLASS(_left, _right))
		MC_ABORT(L"Source and Destinations must be of the same type.");
	 
	size_t m = max(_left->row_sz, _right->row_sz);
	size_t n = _left->column_sz + _right->column_sz;
	mc_matrix_t* size = mc_array_double_alloc(2);
	mc_array_double_set_u(size, 1, m);
	mc_array_double_set_u(size, 2, n);

	size_t pre_col_sz = _left->column_sz;
	_left = mc_matrix_resize(_left, size);

	mc_matrix_t* dstpos = mc_matrix_double_alloc(1, 2);
	mc_array_double_set_u(dstpos, 1, 1);
	mc_array_double_set_u(dstpos, 2,  pre_col_sz + 1);

	if (IS_ARRAY(_left))
		dstpos = mc_matrix_linmap_pos(_left, dstpos);

	mc_matrix_t* srcpos = mc_matrix_double_alloc(1, 2);
	mc_array_double_set_u(srcpos, 1, 1);
	mc_array_double_set_u(srcpos, 2, 1);

	if (IS_ARRAY(_right))
		srcpos = mc_matrix_linmap_pos(_right, srcpos);

	return mc_matrix_copy(_left, dstpos, _right, srcpos);
}

mc_matrix_t* mc_matrix_vercat(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_SAME_CLASS(_left, _right))
		MC_ABORT(L"Source and Destinations must be of the same type.");
	
	mc_matrix_t* dstpos = mc_array_double_alloc(2);
	
	// Find the required size.
	size_t m = _left->row_sz + _right->row_sz;
	size_t n = max(_left->column_sz, _right->column_sz);
	mc_matrix_t* size = mc_array_double_alloc(2);

	mc_array_double_set_u(size, 1, m);
	mc_array_double_set_u(size, 2, n);
	// Create the target matrix.
	size_t pre_row_sz = _left->row_sz;
	_left = mc_matrix_resize(_left, size);

	mc_array_double_set_u(dstpos, 1, pre_row_sz + 1);
	mc_array_double_set_u(dstpos, 2, 1);
	if (IS_ARRAY(_left))
		dstpos = mc_matrix_linmap_pos(_left, dstpos);

	mc_matrix_t* srcpos = mc_array_double_alloc(2);
	mc_array_double_set_u(srcpos, 1, 1);
	mc_array_double_set_u(srcpos, 2, 1);
	if (IS_ARRAY(_right))
		srcpos = mc_matrix_linmap_pos(_right, srcpos);

	//wprintf(L"dstpos = %s\nsrcpos = %s\n", mc_matrix_to_str(dstpos), mc_matrix_to_str(srcpos));

	return mc_matrix_copy(_left, dstpos, _right, srcpos);
}

mc_matrix_t* mc_matrix_zeros(mc_matrix_t* size)
{
	if (!IS_ARRAY(size))
		MC_ABORT(L"Size must be array.");

	if(size->length > 2)
		MC_ABORT(L"Only two dimension is supported in this alpha version.");

	if (size->length == 1)
		return mc_array_double_alloc(size->column_sz);
	else
		return mc_matrix_double_alloc(size->row_sz, size->column_sz);
}
mc_matrix_t* mc_matrix_ones(mc_matrix_t* size)
{
	if (!IS_ARRAY(size))
		MC_ABORT(L"Size must be array.");

	if (size->length > 2)
		MC_ABORT(L"Only two dimension is supported in this alpha version.");
	mc_matrix_t* ones = NULL;
	if (size->length == 1)
		ones = mc_array_double_alloc(size->column_sz);
	else
		ones = mc_matrix_double_alloc(size->row_sz, size->column_sz);
	return ones;
}

static void _check_compliance(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
}

mc_matrix_t* mc_matrix_transp(mc_matrix_t* _matrix)
{
	size_t m = _matrix->column_sz;
	size_t n = _matrix->row_sz;
	mc_matrix_t* _transpose = mc_matrix_alloc(m, n, _matrix->cls, false);
	
	switch (_matrix->cls)
	{
		case MCLS_DOUBLE:
			mc_matrix_transpose_double_scalar(_transpose->dblock, _matrix->dblock, m, n);
			break;
		case MCLS_LOGICAL:
			mc_matrix_transpose_double_scalar(_transpose->lblock, _matrix->lblock, m, n);
			break;
	}
	return _transpose;
}
mc_matrix_t* mc_matrix_neg(mc_matrix_t* _matrix)
{
	if (IS_LOGICAL(_matrix))
		return mc_matrix_lneg(_matrix);
	mc_matrix_t* _negated = mc_matrix_empty_clone(_matrix);
	mc_matrix_neg_double_scalar(_negated->dblock, _matrix, _negated->nb_of_elements);
	return _negated;
}
mc_matrix_t* mc_matrix_add(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if(_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _sum = mc_matrix_empty_clone(_left);
	mc_matrix_add_double_scalar(_sum->dblock, _left->dblock, _right->dblock, _sum->nb_of_elements);
	return _sum;
}
mc_matrix_t* mc_matrix_sub(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _subs = mc_matrix_empty_clone(_left);
	mc_matrix_sub_double_scalar(_subs->dblock, _left->dblock, _right->dblock, _subs->nb_of_elements);
	return _subs;
}
mc_matrix_t* mc_matrix_div(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (!IS_SCALAR(_left) || !IS_SCALAR(_right))
		MC_ABORT(L"Both operand must be scalar too be divided.");
	return mc_matrix_dotdiv(_left, _right);
}
mc_matrix_t* mc_matrix_mul(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->column_sz != _right->row_sz)
		MC_ABORT(L"Incorrect dimensions for matrix multiplication. Check that the number of columns in the first matrix matches the number of rows in the second matrix.To operate on each element of the matrix individually, use TIMES(.*) for elementwise multiplication.");
	mc_matrix_t* _mul = mc_matrix_double_alloc(_left->row_sz, _right->column_sz);
	mc_matrix_mul_double_scalar(_mul->dblock, _left->dblock, _right->dblock, _left->row_sz, _left->column_sz, _right->column_sz);
	return _mul;
}
mc_matrix_t* mc_matrix_pow(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (!IS_SCALAR(_left) || !IS_SCALAR(_right))
		MC_ABORT(L"Both operand must be scalar too be divided.");
	return mc_matrix_dotpow(_left, _right);
}
mc_matrix_t* mc_matrix_dotmul(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _mul = mc_matrix_empty_clone(_left);
	mc_matrix_dotmul_double_scalar(_mul->dblock, _left->dblock, _right->dblock, _mul->nb_of_elements);
	return _mul;
}
mc_matrix_t* mc_matrix_dotdiv(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _div = mc_matrix_empty_clone(_left);
	mc_matrix_dotdiv_double_scalar(_div->dblock, _left->dblock, _right->dblock, _div->nb_of_elements);
	return _div;
}
mc_matrix_t* mc_matrix_dotpow(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _pow = mc_matrix_empty_clone(_left);
	mc_matrix_dotpow_double_scalar(_pow->dblock, _left->dblock, _right->dblock, _pow->nb_of_elements);
	return _pow;
}

mc_matrix_t* mc_matrix_all(mc_matrix_t* _matrix)
{
	mc_matrix_t* result = mc_scalar_logical_alloc(true);
	if (IS_LOGICAL(_matrix))
		mc_matrix_all_logical_scalar(result->lblock, _matrix->lblock, _matrix->nb_of_elements);
	else if(IS_DOUBLE(_matrix))
		mc_matrix_all_double_scalar(result->lblock, _matrix->dblock, _matrix->nb_of_elements);
	return result;
}
mc_bool_t mc_matrix_is_true(mc_matrix_t* _matrix)
{
	return mc_array_logical_get_u(mc_matrix_all(_matrix), 1);
}
mc_bool_t mc_matrix_is_false(mc_matrix_t* _matrix)
{
	return !mc_matrix_is_true(_matrix);
}
mc_matrix_t* mc_matrix_land(mc_matrix_t* _left, mc_matrix_t* _right) 
{
	if (!IS_LOGICAL(_left))
		_left = mc_matrix_logical(_left);
	if (!IS_LOGICAL(_right))
		_right = mc_matrix_logical(_right);
	mc_matrix_t* _and = mc_matrix_empty_clone(_left);
	mc_matrix_and_logical_scalar(_and->lblock, _left->lblock, _right->lblock, _and->nb_of_elements);
	return _and;
}
mc_matrix_t* mc_matrix_lor(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_LOGICAL(_left))
		_left = mc_matrix_logical(_left);
	if (!IS_LOGICAL(_right))
		_right = mc_matrix_logical(_right);
	mc_matrix_t* _or = mc_matrix_empty_clone(_left);
	mc_matrix_or_logical_scalar(_or->lblock, _left->lblock, _right->lblock, _or->nb_of_elements);
	return _or;
}
mc_matrix_t* mc_matrix_lneg(mc_matrix_t* _matrix)
{
	if (!IS_LOGICAL(_matrix))
		_matrix = mc_matrix_logical(_matrix);
	mc_matrix_t* _negated = mc_matrix_empty_clone(_matrix);
	mc_matrix_neg_logical_scalar(_negated->lblock, _negated->nb_of_elements);
	return _negated;
}

mc_matrix_t* mc_matrix_lt(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _lt = mc_matrix_logical_alloc(_left->row_sz, _left->column_sz);
	mc_matrix_lt_double_scalar(_lt->dblock, _left->dblock, _right->dblock, _lt->nb_of_elements);
	return _lt;
}
mc_matrix_t* mc_matrix_lte(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _lte = mc_matrix_logical_alloc(_left->row_sz, _left->column_sz);
	mc_matrix_lte_double_scalar(_lte->dblock, _left->dblock, _right->dblock, _lte->nb_of_elements);
	return _lte;
}
mc_matrix_t* mc_matrix_gt(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _gt = mc_matrix_logical_alloc(_left->row_sz, _left->column_sz);
	mc_matrix_gt_double_scalar(_gt->dblock, _left->dblock, _right->dblock, _gt->nb_of_elements);
	return _gt;
}
mc_matrix_t* mc_matrix_gte(mc_matrix_t* _left, mc_matrix_t* _right)
{
	if (!IS_DOUBLE(_left) || !IS_DOUBLE(_right))
		MC_ABORT(L"Both operand must be type of double");
	if (_left->row_sz != _right->row_sz || _left->column_sz != _right->column_sz)
		MC_ABORT(L"Both operand must be of the same size");
	mc_matrix_t* _gte = mc_matrix_logical_alloc(_left->row_sz, _left->column_sz);
	mc_matrix_gte_double_scalar(_gte->dblock, _left->dblock, _right->dblock, _gte->nb_of_elements);
	return _gte;
}


static mc_string_t mc_matrix_double_matrix_to_str(mc_matrix_t* self, sb_t* sb)
{
	size_t i = 0;
	size_t nb_of_chunks = self->column_sz / 16;
	size_t start_col = 1;
	size_t end_col = 1;
	for(; i < nb_of_chunks; i++)
	{
		start_col = i * 16 + 1;
		end_col = start_col + 16;
		sb_appendf(sb, Str("\nColumns %u through %u.\n"), start_col, end_col);
		for (size_t m = 1; m <= self->row_sz; m++)
		{
			for (size_t n = start_col; n <= end_col; n++)
			{
				sb_appendf(sb, Str("%.lf "), mc_matrix_double_get(self, m, n));
			}
			sb_append(sb, Str("\n"));
		}
	}
	
	sb_appendf(sb, Str("\nColumns %u through %u.\n"), end_col, self->column_sz);
	for (size_t m = 1; m <= self->row_sz; m++)
	{
		for (size_t n = end_col; n <= self->column_sz; n++)
		{
			sb_appendf(sb, Str("%.lf "), mc_matrix_double_get(self, m, n));
		}
		sb_append(sb, Str("\n")); 
	}
	return sb_to_str(sb);
}
static mc_string_t mc_matrix_double_array_to_str(mc_matrix_t* self, sb_t* sb)
{
	if (self->length > 16)
	{
		sb_appendf(sb, Str("[%lf %lf %lf ... %lf %lf %lf]\n"),
			mc_array_double_get(self, 0),
			mc_array_double_get(self, 1),
			mc_array_double_get(self, 2),
			mc_array_double_get(self, self->length - 3),
			mc_array_double_get(self, self->length - 2),
			mc_array_double_get(self, self->length - 1));
		return;
	}
	if(self->length > 1)
		sb_append(sb, Str("["));
	for (size_t i = 1; i < self->length; i++)
	{
		sb_appendf(sb, Str("%lf "), mc_array_double_get(self, i));
	}
	sb_appendf(sb, Str("%lf"), mc_array_double_get(self, self->end_n));
	if (self->length > 1)
		sb_append(sb, Str("]")); 
	return sb_to_str(sb);
}

static mc_string_t mc_matrix_logical_matrix_to_str(mc_matrix_t* self, sb_t* sb)
{
	size_t i = 0;
	size_t nb_of_chunks = self->column_sz / 16;
	size_t start_col = 1;
	size_t end_col = 1;
	for (; i < nb_of_chunks; i++)
	{
		start_col = i * 16 + 1;
		end_col = start_col + 16;
		sb_appendf(sb, Str("\nColumns %u through %u.\n"), start_col, end_col);
		for (size_t m = 1; m <= self->row_sz; m++)
		{
			for (size_t n = start_col; n <= end_col; n++)
			{
				sb_appendf(sb, Str("%u "), mc_matrix_logical_get(self, m, n));
			}
			sb_append(sb, Str("\n"));
		}
	}

	sb_appendf(sb, Str("\nColumns %u through %u.\n"), end_col, self->column_sz);
	for (size_t m = 1; m <= self->row_sz; m++)
	{
		for (size_t n = end_col; n <= self->column_sz; n++)
		{
			sb_appendf(sb, Str("%u "), mc_matrix_logical_get(self, m, n));
		}
		sb_append(sb, Str("\n"));
	}
	return sb_to_str(sb);
}
static mc_string_t mc_matrix_logical_array_to_str(mc_matrix_t* self, sb_t* sb)
{
	if (self->length > 16)
	{
		sb_appendf(sb, Str("[%u %u %u ... %u %u %u]\n"),
			mc_array_logical_get(self, 0),
			mc_array_logical_get(self, 1),
			mc_array_logical_get(self, 2),
			mc_array_logical_get(self, self->length - 3),
			mc_array_logical_get(self, self->length - 2),
			mc_array_logical_get(self, self->length - 1));
		return;
	}
	if (self->length > 1)
		sb_append(sb, Str("["));
	for (size_t i = 1; i < self->length; i++)
	{
		sb_appendf(sb, Str("%u "), mc_array_logical_get(self, i));
	}
	sb_appendf(sb, Str("%u"), mc_array_logical_get(self, self->end_n));
	if (self->length > 1)
		sb_append(sb, Str("]"));
	return sb_to_str(sb);
}

static mc_string_t mc_matrix_str_matrix_to_str(mc_matrix_t* self, sb_t* sb)
{
	size_t i = 0;
	size_t nb_of_chunks = self->column_sz / 16;
	size_t start_col = 1;
	size_t end_col = 1;
	for (; i < nb_of_chunks; i++)
	{
		start_col = i * 16 + 1;
		end_col = start_col + 16;
		sb_appendf(sb, Str("\nColumns %u through %u.\n"), start_col, end_col);
		for (size_t m = 1; m <= self->row_sz; m++)
		{
			for (size_t n = start_col; n <= end_col; n++)
			{
				sb_appendf(sb, Str("%s "), mc_matrix_str_get(self, m, n));
			}
			sb_append(sb, Str("\n"));
		}
	}

	sb_appendf(sb, Str("\nColumns %u through %u.\n"), end_col, self->column_sz);
	for (size_t m = 1; m <= self->row_sz; m++)
	{
		for (size_t n = end_col; n <= self->column_sz; n++)
		{
			sb_appendf(sb, Str("%s "), mc_matrix_str_get(self, m, n));
		}
		sb_append(sb, Str("\n"));
	}
	return sb_to_str(sb);
}
static mc_string_t mc_matrix_str_array_to_str(mc_matrix_t* self, sb_t* sb)
{
	if (self->length > 16)
	{
		sb_appendf(sb, Str("[%s %s %s ... %s %s %s]\n"),
			mc_array_str_get(self, 0),
			mc_array_str_get(self, 1),
			mc_array_str_get(self, 2),
			mc_array_str_get(self, self->length - 3),
			mc_array_str_get(self, self->length - 2),
			mc_array_str_get(self, self->length - 1));
		return;
	}
	if (self->length > 1)
		sb_append(sb, Str("["));
	for (size_t i = 1; i < self->length; i++)
	{
		sb_appendf(sb, Str("%s "), mc_array_str_get(self, i));
	}
	sb_appendf(sb, Str("%s"), mc_array_str_get(self, self->end_n));
	if (self->length > 1)
		sb_append(sb, Str("]"));
	return sb_to_str(sb);
}

mc_string_t mc_matrix_to_str(mc_matrix_t* self)
{
	mc_string_t str = NULL;
	sb_t* sb = sb_alloc();
	if (mc_matrix_is_matrix(self))
		switch (self->cls)
		{
			case MCLS_DOUBLE:
				str = mc_matrix_double_matrix_to_str(self, sb);
				break;
			case MCLS_LOGICAL:
				str = mc_matrix_logical_matrix_to_str(self, sb);
				break;
			case MCLS_STRING:
				str = mc_matrix_str_matrix_to_str(self, sb);
				break;
		}
	else if (mc_matrix_is_array(self))
		switch (self->cls)
		{
			case MCLS_DOUBLE:
				str = mc_matrix_double_array_to_str(self, sb);
				break;
			case MCLS_LOGICAL:
				str = mc_matrix_logical_array_to_str(self, sb);
				break;
			case MCLS_STRING:
				str = mc_matrix_str_array_to_str(self, sb);
				break;
		}
	

	return str;
}