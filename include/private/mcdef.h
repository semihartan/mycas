#pragma once

#ifndef MCDEF_H
#define MCDEF_H

#include <config.h>

#include <stdint.h>
#include <stdbool.h> 
#include <wchar.h> 

 
#define MC_INLINE			extern inline
#define MC_SINLINE			static inline

#define NEWOBJ(type)		GC_MALLOC(sizeof(type))
#define NEWOBJP(type)		GC_MALLOC(sizeof(*type))

#if !defined(MC_BOOL)

#define MC_BOOL				mc_bool_t

typedef uint8_t				mc_bool_t;

#define false				0u
#define true				1u

#endif // !MC_BOOL


#endif