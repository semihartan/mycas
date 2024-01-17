#pragma once

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <vm/opcode.h>
  
/*
* Variable size structure for instruction type.
*/
typedef struct
{
	opcode_t	op_byte : 6;
	uint8_t		iat		: 2;
	uint8_t*		operand;
}instr_t;


#endif