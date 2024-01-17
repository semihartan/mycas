#include <vm/opcode.h>

#include <util.h>

mc_string_t op_to_str(enum opcode op)
{
	switch (op)
	{
		#define xx(a, b)						\
		case a:									\
			return Str(#a);						\

		#define yy(a, b, c, d, e, f)			\
		case a:									\
			return Str(#a);						\

			#include <vm/opcodes.h>
		
		#undef yy
		#undef xx

		default:
			return mc_str_init(L"Unknown opcode.");
	}
}