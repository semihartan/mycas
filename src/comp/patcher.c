#include <compiler/patcher.h>

#include <mcdef.h>
#include <error.h>

#include <gc/gc.h>

jump_patcher_t* jump_patcher_alloc()
{
	jump_patcher_t* self = NEWOBJP(self);
	MC_ABORT_OUT_OF_MEMORY(self);
	self->sco_stack = vector_int_alloc(compare_int32_t);
	self->sto_stack = vector_int_alloc(compare_int32_t);
	self->abs_stack = vector_int_alloc(compare_int32_t);
	self->bs_stack = vector_int_alloc(compare_int32_t);
	return self;
}

void jump_patcher_push_jmpif(jump_patcher_t* jp, int jmpif_offset, int block_sz)
{
	
	jump_patcher_push_sco(jp, jmpif_offset);
	jump_patcher_push_bsz(jp, block_sz);
}

void jump_patcher_push_jmp(jump_patcher_t* jp, int jmp_offset, int block_sz)
{
	if (jp->bs_stack->count > 0)
	{
		int blksz = jump_patcher_top_bsz(jp);
		blksz += 5;
		vector_int_set(jp->bs_stack, jp->bs_stack->count - 1, blksz);
	}
	for (size_t i = 0; i < jp->abs_stack->count; i++)
	{
		int ablksz = vector_int_get(jp->abs_stack, i);
		ablksz += block_sz;
		vector_int_set(jp->abs_stack, i, ablksz);
	} 
	jump_patcher_push_sto(jp, jmp_offset);
	jump_patcher_push_absz(jp, block_sz);
}

typedef code_object_t;

void jump_patcher_patch(jump_patcher_t* jp, code_object_t* co);

int jump_patcher_push_sco(jump_patcher_t* jp, int offset)
{
	return vector_int_push_back(jp->sco_stack, offset);
}
int jump_patcher_pop_sco(jump_patcher_t* jp)
{
	return vector_int_pop_back(jp->sco_stack);
}
int jump_patcher_top_sco(jump_patcher_t* jp)
{
	return vector_int_get(jp->sco_stack, jp->sco_stack->count - 1);
}

int jump_patcher_push_sto(jump_patcher_t* jp, int offset)
{
	return vector_int_push_back(jp->sto_stack, offset);
}
int jump_patcher_pop_sto(jump_patcher_t* jp)
{
	return vector_int_pop_back(jp->sto_stack);
}
int jump_patcher_top_sto(jump_patcher_t* jp)
{
	return vector_int_get(jp->sto_stack, jp->sto_stack->count - 1);
}

int jump_patcher_push_bsz(jump_patcher_t* jp, int offset)
{
	return vector_int_push_back(jp->bs_stack, offset);
}
int jump_patcher_pop_bsz(jump_patcher_t* jp)
{
	return vector_int_pop_back(jp->bs_stack);
}
int jump_patcher_top_bsz(jump_patcher_t* jp)
{
	return vector_int_get(jp->bs_stack, jp->bs_stack->count - 1);
}

int jump_patcher_push_absz(jump_patcher_t* jp, int offset)
{
	return vector_int_push_back(jp->abs_stack, offset);
}
int jump_patcher_pop_absz(jump_patcher_t* jp)
{
	return vector_int_pop_back(jp->abs_stack);
}
int jump_patcher_top_absz(jump_patcher_t* jp)
{
	return vector_int_get(jp->abs_stack, jp->abs_stack->count - 1);
}

loop_frame_t* loop_frame_alloc()
{
	loop_frame_t* self = NEWOBJP(self);
	MC_ABORT_OUT_OF_MEMORY(self);
	self->bo_stack = vector_int_alloc(compare_int32_t);
	self->co_stack = vector_int_alloc(compare_int32_t); 
	return self;
}

void loop_frame_push_bo(loop_frame_t* lf, int bo)
{
	vector_int_push_back(lf->bo_stack, bo);
}
void loop_frame_push_co(loop_frame_t* lf, int co)
{
	vector_int_push_back(lf->co_stack, co);
}
int loop_frame_pop_bo(loop_frame_t* lf) 
{
	return vector_int_pop_back(lf->bo_stack);
}
int loop_frame_pop_co(loop_frame_t* lf)
{
	return vector_int_pop_back(lf->co_stack);
}