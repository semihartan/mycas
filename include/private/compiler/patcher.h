#pragma once

#ifndef PATCHER_OBJECT_H
#define PATCHER_OBJECT_H

#include <adt/vector.int.h>

typedef struct
{
	// Selection Constructor Patch Offsets Stack
	vector_int_t* sco_stack;
	// Block Sizes Stack
	vector_int_t* bs_stack;
	// Selection Terminator Patch Offsets Stack
	vector_int_t* sto_stack;
	// Accumulated Block Sizes Stack.
	vector_int_t* abs_stack;
}jump_patcher_t;



typedef code_object_t;


jump_patcher_t* jump_patcher_alloc();

void jump_patcher_push_jmpif(jump_patcher_t* jp, int jmpif_offset, int block_sz);

void jump_patcher_push_jmp(jump_patcher_t* jp, int jmp_offset, int block_sz);

void jump_patcher_patch(jump_patcher_t* jp, code_object_t* co);

int jump_patcher_push_sco(jump_patcher_t* jp, int offset);
int jump_patcher_pop_sco(jump_patcher_t* jp);
int jump_patcher_top_sco(jump_patcher_t* jp);

int jump_patcher_push_sto(jump_patcher_t* jp, int offset);
int jump_patcher_pop_sto(jump_patcher_t* jp);
int jump_patcher_top_sto(jump_patcher_t* jp);

int jump_patcher_push_bsz(jump_patcher_t* jp, int sz);
int jump_patcher_pop_bsz(jump_patcher_t* jp);
int jump_patcher_top_bsz(jump_patcher_t* jp);

int jump_patcher_push_absz(jump_patcher_t* jp, int sz);
int jump_patcher_pop_absz(jump_patcher_t* jp);
int jump_patcher_top_absz(jump_patcher_t* jp);


typedef struct
{
	size_t loop_start;
	size_t loop_end;
	vector_int_t* bo_stack;
	vector_int_t* co_stack;
}loop_frame_t;

loop_frame_t* loop_frame_alloc();

void loop_frame_push_bo(loop_frame_t* lf, int bo);
void loop_frame_push_co(loop_frame_t* lf, int bo); 
int loop_frame_pop_bo(loop_frame_t* lf);
int loop_frame_pop_co(loop_frame_t* lf);
void loop_frame_patch(loop_frame_t* lf, code_object_t* co);

#endif