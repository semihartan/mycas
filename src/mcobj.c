#include <mc.h>
#include <error.h>

#include <gc/gc.h>


#define DEF_OBJ_HEAD(o)													\
uint8_t* _obj_head_address = (uint8_t*)o - sizeof(uintptr_t);			\
uintptr_t obj_head_address = *(uintptr_t*)_obj_head_address;			\
struct obj_head* obj_head = obj_head_address


mc_obj_t mc_obj_alloc(size_t sz, enum obj_type type, enum ref_type ref)
{
	sz += OBJ_HEAD_SZ;
	uint8_t* obj = GC_MALLOC(sz);
	MC_ERROR_RETVAL(obj != NULL, L"Failed to allocate object.", NULL);

	struct obj_head* head = obj;
	head->type = type;
	head->ref = ref;
	head->str_func = mc_obj_str;
	return obj + OBJ_HEAD_SZ;
}

MC_INLINE mc_obj_t mc_obj_make_simple(mc_obj_t _obj)
{
	DEF_OBJ_HEAD(_obj);
	obj_head->ref = RT_SIMPLE;
	return _obj;
}
MC_INLINE mc_obj_t mc_obj_make_cow(mc_obj_t _obj)
{
	DEF_OBJ_HEAD(_obj);
	obj_head->ref = RT_COW;
	return _obj;
}
MC_INLINE enum obj_type mc_obj_get_obj_type(mc_obj_t _obj)
{
	DEF_OBJ_HEAD(_obj);
	return obj_head->type;
}
MC_INLINE enum ref_type mc_obj_get_ref_type(mc_obj_t _obj)
{
	DEF_OBJ_HEAD(_obj);
	return obj_head->ref;
}

mc_string_t mc_obj_str(mc_obj_t self)
{
	DEF_OBJ_HEAD(self);
	return obj_head->str_func(self);
}

void mc_obj_set_str_func(mc_obj_t self, str_func_t str_func)
{
	DEF_OBJ_HEAD(self);
	obj_head->str_func = str_func;
}