#include <vm/co.h>
#include <vm/opcode.h>

#include <mcdef.h>
#include <adt/sb.h>
#include <error.h>

code_object_t* co_alloc()
{
	code_object_t* self = NEWOBJP(self);
	MC_ERROR_OUT_OF_MEMORY(self);
	self->code = vector_byte_alloc(compare_uint8_t);

	return self;
}

size_t co_get_length(code_object_t* co)
{
	return CO_GET_LENGTH(co);
}

code_object_t* co_append(code_object_t* co, code_object_t* other)
{
	for (size_t i = 0; i < CO_GET_LENGTH(other); i++)
	{
		vector_byte_push_back(co->code, vector_byte_get(other->code, i));
	}
	return co;
} 
static void _emit_byte(code_object_t* co, uint8_t byte);
static void _emit_i32(code_object_t* co, int32_t i32);
static void _emit_u32(code_object_t* co, uint32_t u32);
static void _emit_u64(code_object_t* co, uint64_t u64);
static void _emit_flt64(code_object_t* co, double flt64);

#define xx(a, b)								\
void co_emit_##b(code_object_t* co)				\
{												\
	_emit_byte(co, a);							\
}

#define yy(a, b, c, d, e, f)					\
void co_emit_##e(code_object_t* co, b f)		\
{												\
	_emit_byte(co, a);							\
	_emit_##c(co, f);							\
}

#include <vm/opcodes.h>

#undef yy
#undef xx

void co_insert_byte(code_object_t* co, uint8_t byte, size_t offset)
{
	vector_byte_set(co->code, offset, byte);
}
void co_insert_i32(code_object_t* co, int32_t i32, size_t offset)
{
#define INT32_SZ sizeof(int32_t)
	union
	{
		uint8_t byte[INT32_SZ];
		int32_t i32;
	}u = { i32 };
	for (size_t i = 0; i < INT32_SZ; i++)
	{
		 vector_byte_set(co->code, offset + i, u.byte[i]);
	}
}
void co_insert_u32(code_object_t* co, uint32_t u32, size_t offset)
{
#define UINT32_SZ sizeof(uint32_t)
	union
	{
		uint8_t byte[UINT32_SZ];
		uint32_t u32;
	}u = { u32 };
	for (size_t i = 0; i < UINT32_SZ; i++)
	{
		vector_byte_set(co->code, offset + i, u.byte[i]);
	}
}
void co_insert_u64(code_object_t* co, uint64_t u64, size_t offset)
{
#define UINT64_SZ sizeof(uint64_t)
	union
	{
		uint8_t byte[UINT64_SZ];
		uint64_t u64;
	}u = { u64 };
	for (size_t i = 0; i < UINT64_SZ; i++)
	{
		vector_byte_set(co->code, offset + i, u.byte[i]);
	}
}
void co_insert_flt64(code_object_t* co, double flt64, size_t offset)
{
#define FLT64_SZ sizeof(double)
	union
	{
		uint8_t byte[FLT64_SZ];
		double flt64;
	}u = { flt64 };
	for (size_t i = 0; i < FLT64_SZ; i++)
	{
		vector_byte_set(co->code, offset + i, u.byte[i]);
	}
}

static void _emit_byte(code_object_t* co, uint8_t byte)
{
	vector_byte_push_back(co->code, byte);
}

static void _emit_i32(code_object_t* co, int32_t i32)
{
	_emit_byte(co, (i32 >> 0) & 0xff);
	_emit_byte(co, (i32 >> 8) & 0xff);
	_emit_byte(co, (i32 >> 16) & 0xff);
	_emit_byte(co, (i32 >> 24) & 0xff);
}

static void _emit_u32(code_object_t* co, uint32_t u32)
{
	_emit_byte(co, (u32 >> 0) & 0xff);
	_emit_byte(co, (u32 >> 8) & 0xff);
	_emit_byte(co, (u32 >> 16) & 0xff);
	_emit_byte(co, (u32 >> 24) & 0xff);
}

static void _emit_u64(code_object_t* co, uint64_t u64)
{
	_emit_u32(co, (u64 >> 0 ) & 0xFFFFFFFFllu);
	_emit_u32(co, (u64 >> 32) & 0xFFFFFFFFllu);
}

static void _emit_flt64(code_object_t* co, double flt64)
{
#define FLT64_SZ  sizeof(double)
	union
	{
		uint8_t byte[FLT64_SZ];
		double flt64;
	}u;
	u.flt64 = flt64;
	for (size_t i = 0; i < FLT64_SZ; i++)
		_emit_byte(co, u.byte[i]);
}
