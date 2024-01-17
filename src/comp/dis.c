#include <compiler/dis.h>
#include <vm/opcode.h>

#include <mcdef.h>
#include <adt/sb.h>


#define DISASSEMBLE0(op)					\
case op:									\
	sb_appendl(sb, Str(#op));				\
	++i;									\
	break

static double	_decode_flt64(code_t * code, size_t src); 
static uint8_t	_decode_byte(code_t * code, size_t src);
static int32_t	_decode_i32(code_t * code, size_t src);
static uint32_t _decode_u32(code_t * code, size_t src);
static uint64_t _decode_u64(code_t * code, size_t src);

mc_string_t co_disassemble(code_object_t* co)
{
	sb_t* sb = sb_alloc();
	for (size_t i = 0; i < co->code->count;)
	{
		opcode_t bytecode = vector_byte_get(co->code, i);
		sb_appendf(sb, Str("%.3u    "), i);
		switch (bytecode)
		{
			#define xx(a, b)							\
			case a:										\
				sb_appendl(sb, Str(#a));				\
				++i;									\
				break;

			#define yy(a, b, c, d, e, f)				\
			case a:										\
			{											\
				sb_append(sb, Str(#a));					\
				++i;									\
				b c = _decode_##c(co->code, i);			\
				sb_appendf(sb, Str(" " L##d L"\n"), c);	\
				i += sizeof(b);							\
				break;									\
			}

				#include <vm/opcodes.h>
			
			#undef yy
			#undef xx 
			default:
				sb_appendl(sb, Str("Unknown bytecode."));
				goto finish;
		}
	}
finish:
	return sb_to_str(sb);
}

static double _decode_flt64(code_t* code, size_t src)
{
#define FLT64_SZ  sizeof(double)
	union
	{
		uint8_t byte[FLT64_SZ];
		double flt64;
	}u;
	for (size_t i = 0; i < FLT64_SZ; i++)
	{
		u.byte[i] = vector_byte_get(code, src + i);
	}
	return u.flt64;
}
static uint8_t _decode_byte(code_t* code, size_t src)
{
	return vector_byte_get(code, src);
}
static int32_t _decode_i32(code_t* code, size_t src)
{
#define INT32_SZ sizeof(int32_t)
	union
	{
		uint8_t byte[INT32_SZ];
		int32_t i32;
	}u;
	for (size_t i = 0; i < INT32_SZ; i++)
	{
		u.byte[i] = vector_byte_get(code, src + i);
	}
	return u.i32;
}
static uint32_t _decode_u32(code_t* code, size_t src)
{
#define UINT32_SZ sizeof(uint32_t)
	union
	{
		uint8_t byte[UINT32_SZ];
		uint32_t u32;
	}u;
	for (size_t i = 0; i < UINT32_SZ; i++)
	{
		u.byte[i] = vector_byte_get(code, src + i);
	}
	return u.u32;
}
static uint64_t _decode_u64(code_t* code, size_t src)
{
#define UINT64_SZ sizeof(uint64_t)
	union
	{
		uint8_t byte[UINT64_SZ];
		uint64_t u64;
	}u;
	for (size_t i = 0; i < UINT64_SZ; i++)
	{
		u.byte[i] = vector_byte_get(code, src + i);
	}
	return u.u64;
}