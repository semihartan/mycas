#pragma once

#ifndef VM_H
#define VM_H

#include <mcobj.h>
#include <vm/frame.h>
#include <adt/vector.byte.h>

#include <vm/co.h>

#define MAX_STACK_SZ 0x4096


typedef struct
{
	vector_byte_t* vec;
	int32_t top;
}eval_stack_t;

eval_stack_t* eval_stack_alloc();

uint8_t eval_stack_push_byte(eval_stack_t* eval_stack, uint8_t byte);
mc_obj_t eval_stack_push_obj(eval_stack_t* eval_stack, mc_obj_t obj);

uint8_t eval_stack_pop_byte(eval_stack_t* eval_stack);
mc_obj_t eval_stack_pop_obj(eval_stack_t* eval_stack);

uint8_t eval_stack_top_byte(eval_stack_t* eval_stack);
mc_obj_t eval_stack_top_obj(eval_stack_t* eval_stack);
 

typedef struct
{
	eval_stack_t* eval_stack;
	code_object_t* active_prog;
	mc_frame_t* active_frame;
	uint8_t* ip;
	uint8_t* eip;
	bool is_running;
}vm_t;

void vm_start();

void vm_execute_program(code_object_t* prog);

void vm_stop();

#endif