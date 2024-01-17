#pragma once

#ifndef ADT_ARRAY_H
#define ADT_ARRAY_H
 
#include <error.h>
#include <mcdef.h>
#include <gc/gc.h>

#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#define CREATE_ARRAY_INTER(name, type)													\
typedef struct																			\
{																						\
	size_t length;																		\
	type* inner;																		\
}array_##name##_t;																		\
array_##name##_t* array_##name##_alloc(size_t length);									\
void array_##name##_free(array_##name##_t* self);										\
type array_##name##_get(array_##name##_t* self, size_t index);							\
type array_##name##_set(array_##name##_t* self, size_t index, type value);				\
void array_##name##_clear_from_size(array_##name##_t* self, size_t from, size_t size);	\
void array_##name##_clear_from(array_##name##_t* self, size_t from);					\
void array_##name##_clear(array_##name##_t* self);										\
void array_##name##_copy(array_##name##_t* destination, size_t destinationIndex, array_##name##_t* source, size_t sourceIndex, size_t size);

#define CREATE_ARRAY_IMPL(name, type)													\
																						\
array_##name##_t* array_##name##_alloc(size_t length)									\
{																						\
	array_##name##_t* self = NEWOBJP(self);												\
	MC_ERROR_OUT_OF_MEMORY(self);														\
	self->length = length;																\
	self->inner = (type*)GC_MALLOC(sizeof(*self->inner) * self->length);				\
	MC_ERROR_OUT_OF_MEMORY(self->inner);												\
	memset(self->inner, 0, self->length * sizeof(type));								\
	return self;																		\
}																						\
void array_##name##_free(array_##name##_t* self)										\
{																						\
	if (self)																			\
	{																					\
		if (self->inner)																\
			GC_FREE(self->inner);														\
		GC_FREE(self);																	\
	}																					\
}																						\
type array_##name##_get(array_##name##_t* self, size_t index)							\
{																						\
	if (index < 0 || index >= self->length)												\
		return NULL;																	\
	return self->inner[index];															\
}																						\
type array_##name##_set(array_##name##_t* self, size_t index, type value)				\
{																						\
	if (index < 0 || index >= self->length)												\
		return;																			\
	return self->inner[index] = value;													\
}																						\
void array_##name##_clear_from_size(array_##name##_t* self, size_t from, size_t size)	\
{																						\
	for(size_t i = from; i < size; i++)													\
	{																					\
		if(self->inner[i])																\
			GC_FREE(self->inner[i]);													\
		self->inner[i] = 0;																\
	}																					\
}																						\
void array_##name##_clear_from(array_##name##_t* self, size_t from)						\
{																						\
	array_##name##_clear_from_size(self, from, self->length - from);					\
}																						\
void array_##name##_clear(array_##name##_t* self)										\
{																						\
	array_##name##_clear_from(self, 0);													\
}																						\
void array_##name##_copy(array_##name##_t* destination, size_t destinationIndex, array_##name##_t* source, size_t sourceIndex, size_t size)																				  \
{																										\
	if ((destination->length - destinationIndex) < size || (source->length - sourceIndex) < size)		\
		return;																							\
	memcpy(destination->inner + destinationIndex, source->inner + sourceIndex, size * sizeof(type));	\
} 

#endif