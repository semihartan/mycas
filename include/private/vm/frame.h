#pragma once

#ifndef MC_FRAME_OBJ_H
#define MC_FRAME_OBJ_H

#include <vm/nametab.h>
#include <vm/co.h>

#include <mcstr.h>

typedef struct
{
	mc_string_t name;
	nametab_t* locals;
	nametab_t* args;
	size_t frame_index;
	code_object_t* co;
	uint8_t* returnadr;
}mc_frame_t;

mc_frame_t* frame_alloc(mc_string_t name);
mc_frame_t* frame_copy(mc_frame_t* _frame);

uint8_t* frame_set_return_address(mc_frame_t* _frame, uint8_t* ret_addr);
uint8_t* frame_get_return_address(mc_frame_t* _frame);

nametab_t* frame_locals(mc_frame_t* _frame);
nametab_t* frame_args(mc_frame_t* _frame);


#endif