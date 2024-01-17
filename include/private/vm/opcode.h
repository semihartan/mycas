#pragma once

#ifndef OPCODE_H
#define OPCODE_H

#include <stdint.h>
#include <mcstr.h>

#define OP_ARGC_MASK		(1u << 5)
#define OP_MOD_MASK			(1u << 7)

enum opcode
{
	#define xx(a, b) a,

	#define yy(a, b, c, d, e, f) a,

		#include <vm/opcodes.h>
	
	#undef yy
	#undef xx
};

typedef uint8_t opcode_t;

mc_string_t op_to_str(enum opcode op);

#endif