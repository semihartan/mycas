#pragma once

#ifndef MC_STRING_H
#define MC_STRING_H

#include <mc.h>  

#include <stddef.h>

#define MC_CAST_TO_STR(obj)		(wchar_t*)obj
#define MC_CAST_TO_CSTR(obj)	(const wchar_t*)obj
#define Str(s)					mc_str_init(L##s)
 
typedef mc_obj_t mc_string_t;

MC_INLINE mc_string_t mc_str_alloc(size_t _sz);

mc_string_t mc_str_init(const wchar_t* _wstr);

size_t mc_str_len(mc_string_t _str);

size_t mc_str_sz(mc_string_t _str);

mc_string_t mc_str_erase(mc_string_t _str);

MC_INLINE mc_string_t mc_str_empty(size_t _len);

mc_string_t mc_str_duplicate(mc_string_t _str);

mc_string_t mc_str_prepend(mc_string_t _str1, mc_string_t _str2);

mc_string_t mc_str_append(mc_string_t _str1, mc_string_t _str2);

mc_string_t mc_str_obj_str(mc_string_t str);

#endif