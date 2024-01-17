#pragma once

#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include <adt/vector.byte.h>
#include <mc.h>
#include <mcstr.h>

typedef vector_byte_t code_t;

#define CO_GET_LENGTH(co)			(co->code->count)

typedef struct
{
	code_t* code;
}code_object_t;

code_object_t* co_alloc();
size_t co_get_length(code_object_t* co);
mc_string_t co_disassemble(code_object_t* co);
code_object_t* co_append(code_object_t* co, code_object_t* other);

void co_insert_byte(code_object_t* co, uint8_t byte, size_t offset);
void co_insert_i32(code_object_t* co, int32_t i32, size_t offset);
void co_insert_u32(code_object_t* co, uint32_t u32, size_t offset);
void co_insert_u64(code_object_t* co, uint64_t u64, size_t offset);
void co_insert_flt64(code_object_t* co, double flt64, size_t offset);

#define xx(a, b)								\
void co_emit_##b(code_object_t* co);

#define yy(a, b, c, d, e, f)					\
void co_emit_##e(code_object_t* co, b f);

#include <vm/opcodes.h>

#undef yy
#undef xx


//void co_emit_ldglob(code_object_t* co,	int32_t index);
//void co_emit_pow(code_object_t* co);


#endif