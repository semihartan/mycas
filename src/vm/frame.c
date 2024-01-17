#include <vm/frame.h>

#include <gc/gc.h>

mc_frame_t* frame_alloc(mc_string_t name)
{
	mc_frame_t* self = NEWOBJP(self);
	MC_ABORT_OUT_OF_MEMORY(self);
	self->name = name;
	self->locals = nametab_alloc();
	self->args = nametab_alloc();
	return self;
}
mc_frame_t* frame_copy(mc_frame_t* _frame)
{
	mc_frame_t* copy = frame_alloc(_frame->name);
	copy->co = _frame->co;
	copy->frame_index = _frame->frame_index;
	for (size_t i = 0; i < _frame->locals->names->count; i++)
	{
		/*mc_obj_t entry = nametab_get_index(_frame->locals, i);
		nametab_set_index(copy->locals, i, entry);*/
		nametab_alloc_index(copy->locals);
	}
	for (size_t i = 0; i < _frame->args->names->count; i++)
	{
		/*mc_obj_t entry = nametab_get_index(_frame->args, i);
		nametab_set_index(copy->args, i, entry);*/
		nametab_alloc_index(copy->args);
	}
	return copy;
}
uint8_t* frame_set_return_address(mc_frame_t* _frame, uint8_t* ret_addr)
{
	return _frame->returnadr = ret_addr;
}
uint8_t* frame_get_return_address(mc_frame_t* _frame)
{
	return _frame->returnadr;
}
nametab_t* frame_locals(mc_frame_t* _frame)
{
	return _frame->locals;
}
nametab_t* frame_args(mc_frame_t* _frame)
{
	return _frame->args;
}
