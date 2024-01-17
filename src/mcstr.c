#include <mcstr.h>
#include <error.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>
#include <memory.h>

#include <gc/gc.h>

 
#define STR_HEAD_SZ				sizeof(struct string_head)
#define STR_HEAD(str)			(struct string_head*)((uint8_t*)str - STR_HEAD_SZ)

#define DEF_STR_HEAD(str)		struct string_head* str##_head = STR_HEAD(str)

#define STR_FUNC_BEGIN(str)						\
DEF_STR_HEAD(str);								\
wchar_t* str##_start = (wchar_t*)str;			\

struct string_head
{
	size_t len;
	size_t capacity;
	size_t sz;
	DEF_OBJ_HEAD_PTR();
};

mc_string_t mc_str_alloc(size_t _len)
{
	size_t sz = _len;
	sz *= sizeof(wchar_t);

	uint8_t* str = mc_obj_alloc(sz + STR_HEAD_SZ, OK_STRING, RT_SIMPLE);
	MC_ERROR_RETVAL(str != NULL, L"Failed to allocate string.", NULL);
	  
	struct string_head* str_head = str;
	str_head->len = _len - 1;
	str_head->capacity = _len - 1;
	str_head->sz = sz; 
	CALCULATE_OBJ_HEAD(str_head);

	str += STR_HEAD_SZ;
	 
	mc_obj_set_str_func(str, mc_str_obj_str);
	return str;
}
mc_string_t mc_str_erase(mc_string_t _str)
{
	STR_FUNC_BEGIN(_str);
	_str_head->len = 0;
	_str_start[0] = L'\0';
	return _str;
}
 
MC_INLINE mc_string_t mc_str_empty(size_t _len)
{
	return mc_str_alloc(_len + 1);
}

mc_string_t mc_str_ncopy(mc_string_t _str1, mc_string_t _str2, size_t _sz)
{
	STR_FUNC_BEGIN(_str1);
}

mc_string_t mc_str_init(const wchar_t* _wstr)
{
	size_t len = wcslen(_wstr);
	mc_string_t str = mc_str_empty(len);
	DEF_STR_HEAD(str);
	return memcpy(str, _wstr, str_head->sz);
}

size_t mc_str_len(mc_string_t _str)
{
	STR_FUNC_BEGIN(_str);
	return _str_head->len;
}

size_t mc_str_sz(mc_string_t _str)
{
	STR_FUNC_BEGIN(_str);
	return _str_head->sz;
}

mc_string_t mc_str_duplicate(mc_string_t _str)
{
	STR_FUNC_BEGIN(_str);
	mc_string_t duplicate = mc_str_empty(_str_head->len);
	MC_ERROR_RETVAL(duplicate != NULL, L"Failed to allocate string.", NULL); 
	return memcpy(duplicate, _str, _str_head->sz);
}
mc_string_t mc_str_prepend(mc_string_t _str1, mc_string_t _str2)
{
	STR_FUNC_BEGIN(_str1);
	STR_FUNC_BEGIN(_str2);

	mc_string_t new = mc_str_empty(_str1_head->len + _str2_head->len);
	STR_FUNC_BEGIN(new);
	wmemcpy(new_start, _str2, _str2_head->len);
	wmemcpy(new_start + _str2_head->len, _str1, _str1_head->len); 
	return new;
}

mc_string_t mc_str_append(mc_string_t _str1, mc_string_t _str2)
{
	STR_FUNC_BEGIN(_str1);
	STR_FUNC_BEGIN(_str2);

	mc_string_t new = mc_str_empty(_str1_head->len + _str2_head->len);
	STR_FUNC_BEGIN(new);
	wmemcpy(new_start, _str1, _str1_head->len);
	wmemcpy(new_start + _str1_head->len, _str2, _str2_head->len);
	return new;
}
mc_string_t mc_str_obj_str(mc_string_t str)
{
	return str;
}
//mc_string_t mc_str_scoop(const mc_string_t _str, mc_string_t _end)
//{
//	STR_FUNC_BEGIN(_str);
//	wchar_t* end_ptr; 
//	size_t sz;
//	mc_string_t duplicate = NULL;
//	if (head->unicode)
//	{
//		wend = _end;
//		sz = wcstr - wend;
//		duplicate = mc_wstr_empty(head->len);
//	} 
//	else
//	{
//		end = _end;
//		sz = cstr - end;
//		duplicate = mc_str_empty(head->len);
//	} 
//	MCAS_ERROR_RETVAL(duplicate != NULL, L"Failed to allocate string.", NULL);
//	return mc_str_duplicate(;
//}