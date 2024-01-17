#pragma once

#ifndef SYM_H
#define SYM_H

#include <mcdef.h>
#include <mcstr.h>
#include <vm/frame.h>

#define HAS_LOCAL(f)				(((f) & SYM_LOCAL)		!= 0) 
#define HAS_GLOBAL(f)				(((f) & SYM_GLOBAL)		!= 0) 
#define HAS_VAR(f)					(((f) & SYM_VARIABLE)	!= 0)
#define HAS_FUNC(f)					(((f) & SYM_FUNCTION)	!= 0) 
#define HAS_STRING(f)				(((f) & SYM_STRING)		!= 0) 

enum sym_flag
{
	SYM_FUNCTION	= 1u << 0,
	SYM_VARIABLE	= 1u << 1,
	SYM_STRING		= 1u << 2,
	SYM_LOCAL		= 1u << 3,
	SYM_GLOBAL		= 1u << 4,
};

typedef struct
{
	mc_string_t name;
	enum sym_flag flags;
	size_t tab_index;
	union
	{
		mc_frame_t* frame;
		mc_obj_t obj;
	};
	int inargc;
	int outargc;
}sym_t;

mc_string_t sym_to_str(sym_t* sym);

#endif