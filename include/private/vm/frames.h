#pragma  once

#ifndef MC_STACK_FRAMES_H
#define MC_STACK_FRAMES_H

#include <adt/vector.void.h>

#include <vm/frame.h>

extern vector__t* frames_list;
extern vector__t* stack_frames;

mc_frame_t* vm_alloc_frame(mc_string_t name);

MC_INLINE mc_frame_t* vm_push_stack_frame(mc_frame_t* frame);
MC_INLINE mc_frame_t* vm_top_stack_frame();
mc_frame_t* vm_pop_stack_frame();

#endif