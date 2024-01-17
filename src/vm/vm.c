#include <vm/vm.h>

#include <vm/ins.h>
#include <vm/symtab.h>
#include <vm/frames.h>

#include <mc.h> 


#include <error.h>
#include <gc/gc.h>



static vm_t* _vm_instance = NULL;

extern symtab_t* symtab = NULL;
extern nametab_t* globals = NULL;
extern vector__t* frames_list = NULL;
extern vector__t* stack_frames = NULL;

static instr_t _vm_decode_instruction(uint8_t** ip);
static instr_t _vm_set_code_object(code_object_t* co);
static void _vm_start_execution_cycle();

static vm_t* _vm_alloc()
{
	vm_t* instance = NEWOBJP(_vm_instance);
	MC_ABORT_OUT_OF_MEMORY(instance);
	instance->eval_stack = eval_stack_alloc();
	return instance;
}

eval_stack_t* eval_stack_alloc()
{
	eval_stack_t* stack = NEWOBJP(stack);
	stack->vec = vector_byte_alloc(compare_int8_t, MAX_STACK_SZ);
	stack->top = -1;
	return stack;
}

MC_SINLINE size_t eval_stack_size(eval_stack_t* estack)
{
	return estack->top + 1;
}

uint8_t eval_stack_push_byte(eval_stack_t* eval_stack, uint8_t byte)
{
	return ++eval_stack->top, vector_byte_push_back(eval_stack->vec, byte);
}

mc_obj_t eval_stack_push_obj(eval_stack_t* eval_stack, mc_obj_t obj)
{
	eval_stack_push_byte(eval_stack, (obj >> 0) & 0x00000000000000FFllu);
	eval_stack_push_byte(eval_stack, (obj >> 8) & 0x00000000000000FFllu);
	eval_stack_push_byte(eval_stack, (obj >> 16) & 0x00000000000000FFllu);
	eval_stack_push_byte(eval_stack, (obj >> 24) & 0x00000000000000FFllu);
	eval_stack_push_byte(eval_stack, (obj >> 32) & 0x00000000000000FFllu);
	eval_stack_push_byte(eval_stack, (obj >> 40) & 0x00000000000000FFllu);
	eval_stack_push_byte(eval_stack, (obj >> 48) & 0x00000000000000FFllu);
	eval_stack_push_byte(eval_stack, (obj >> 56) & 0x00000000000000FFllu);
	return obj;
}

uint8_t eval_stack_pop_byte(eval_stack_t* eval_stack)
{
	if (eval_stack_size(eval_stack) == 0)
		MC_ABORT(L"Stack is empty.");
	return --eval_stack->top, vector_byte_pop_back(eval_stack->vec);
}

mc_obj_t eval_stack_pop_obj(eval_stack_t* eval_stack)
{
	if (eval_stack_size(eval_stack) < MC_OBJECT_SZ)
		return NULL;
	mc_obj_t obj = ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 56);
	obj |= ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 48);
	obj |= ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 40);
	obj |= ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 32);
	obj |= ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 24);
	obj |= ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 16);
	obj |= ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 8);
	obj |= ((mc_obj_t)eval_stack_pop_byte(eval_stack) << 0);
	return obj;
}

uint8_t eval_stack_top_byte(eval_stack_t* eval_stack)
{
	return vector_byte_get(eval_stack->vec, eval_stack->top);
}

mc_obj_t eval_stack_top_obj(eval_stack_t* eval_stack)
{
	if (eval_stack_size(eval_stack) < MC_OBJECT_SZ)
		return NULL;
	mc_obj_t obj = *(mc_obj_t*)(eval_stack->vec->array->inner + eval_stack->vec->count - MC_OBJECT_SZ);
	return obj;
}

void vm_start()
{
	if (!_vm_instance)
	{
		_vm_instance = _vm_alloc();
	}
	else if (_vm_instance && _vm_instance->is_running)
	{
		wprintf("WARNING: A VM instance is already running.\n");
		return;
	}
	eval_stack_push_obj(_vm_instance->eval_stack, 0x123456);
	eval_stack_push_obj(_vm_instance->eval_stack, 0x123456);
	mc_obj_t obj1 = eval_stack_pop_obj(_vm_instance->eval_stack);
	mc_obj_t obj2 = eval_stack_pop_obj(_vm_instance->eval_stack);


	globals = nametab_alloc();
	frames_list = vector__alloc(NULL);
	stack_frames = vector__alloc(NULL);
	symtab = symtab_alloc();
	
	sym_t* sym = symtab_add_sym(symtab, Str("disp"), SYM_GLOBAL | SYM_FUNCTION, NULL, 1, 0);
	symtab_add_sym(symtab, Str("print"),			SYM_GLOBAL | SYM_FUNCTION, NULL, 1, 0);
	symtab_add_sym(symtab, Str("vercat"),			SYM_GLOBAL | SYM_FUNCTION, NULL, 2, 0);
	symtab_add_sym(symtab, Str("hozcat"),			SYM_GLOBAL | SYM_FUNCTION, NULL, 2, 0);
}

void vm_execute_program(code_object_t* prog)
{
	if (!_vm_instance)
		return;
	size_t program_sz = co_get_length(prog);
	if (program_sz == 0)
		return;
	_vm_set_code_object(prog);
	_vm_instance->eip = _vm_instance->ip + program_sz;
	_vm_start_execution_cycle();
}

void vm_stop();

mc_frame_t* vm_alloc_frame(mc_string_t name)
{
	mc_frame_t* frame = frame_alloc(name);
	frame->frame_index = frames_list->count;
	return vector__push_back(frames_list, frame); 
}

MC_INLINE mc_frame_t* vm_push_stack_frame(mc_frame_t* frame)
{ 
	return _vm_instance->active_frame = vector__push_back(stack_frames, frame);
}

mc_frame_t* vm_pop_stack_frame()
{
	mc_frame_t* popped_frame = vector__pop_back(stack_frames);
	_vm_instance->active_frame = vm_top_stack_frame();
	return popped_frame;
}
mc_frame_t* vm_top_stack_frame()
{
	return vector__get(stack_frames, stack_frames->count - 1);
}

static instr_t _vm_set_code_object(code_object_t* co)
{
	_vm_instance->ip = co->code->array->inner; 
}

static instr_t _vm_decode_instruction(uint8_t** ip)
{
	instr_t ins;
	uint8_t opcode = **ip;
	switch (opcode)
	{
#define xx(a, b)										\
			case a:										\
				ins.op_byte = opcode;					\
				++(*ip);								\
				break;

#define yy(a, b, c, d, e, f)							\
			case a:										\
			{											\
				ins.op_byte = opcode;					\
				++(*ip);								\
				ins.operand = *ip;						\
				*ip += sizeof(b);						\
				break;									\
			}
		default:
			MC_ABORT(L"Unrecognized bytecode.");
			break;
#include <vm/opcodes.h>

#undef yy
#undef xx
	}
	return ins;
}

static void _vm_start_execution_cycle()
{
	uint8_t* eip = _vm_instance->eip;
	
	#define IP		_vm_instance->ip

	typedef eval_stack_t estack_t;
	
	estack_t* estack = _vm_instance->eval_stack;

	while (true)
	{
		instr_t ins = _vm_decode_instruction(&IP);
		switch (ins.op_byte)
		{
			case OP_LDGLOB:
			{
				int32_t global_index = *(int32_t*)ins.operand;
				mc_obj_t obj = nametab_get_index(globals, global_index);
				eval_stack_push_obj(estack, obj);
				break;
			}
			case OP_LDLOC:
			{
				int32_t local_index = *(int32_t*)ins.operand;
				nametab_t* locals = _vm_instance->active_frame->locals;
				mc_obj_t obj = nametab_get_index(locals, local_index);
				eval_stack_push_obj(estack, obj);
				break;
			}
			case OP_LDFRAME:
			{
				int32_t frame_index = *(int32_t*)ins.operand;
				mc_frame_t* empty_frame = vector__get(frames_list, frame_index);
				eval_stack_push_obj(estack, empty_frame);
				break;
			}
			case OP_LDSTR:
				/*int32_t local_index = *(int32_t*)ins.operand;
				nametab_t* locals = _vm_instance->active_frame->locals;
				mc_obj_t obj = nametab_get_index(locals, local_index);
				eval_stack_push_obj(estack, obj);*/
				break;
			case OP_STGLOB:
			{
				int32_t global_index = *(int32_t*)ins.operand;
				mc_obj_t obj = eval_stack_pop_obj(estack);
				nametab_set_index(globals, global_index, obj);
				break;
			}
			case OP_STLOC:
			{
				int32_t local_index = *(int32_t*)ins.operand;
				nametab_t* locals = _vm_instance->active_frame->locals;
				mc_obj_t obj = eval_stack_pop_obj(estack);
				nametab_set_index(locals, local_index, obj);
				break;
			}
			case OP_PUSHSD:
			{
				double number = *(double*)ins.operand;
				mc_obj_t obj = mc_scalar_double_alloc(number);
				eval_stack_push_obj(estack, obj);
				break;
			}
			case OP_PUSHSL:
			{
				mc_bool_t logical = *(mc_bool_t*)ins.operand;
				mc_obj_t obj = mc_scalar_logical_alloc(logical);
				eval_stack_push_obj(estack, obj);
				break;
			}
			case OP_POP:  
			{
				eval_stack_pop_obj(estack);
				break;
			}
			case OP_DISP:
			{
				mc_obj_t obj = eval_stack_pop_obj(estack);
				mc_disp(obj);
				if(obj)
					eval_stack_push_obj(estack, NULL);
				break;
			}
			case OP_CALL:
			{
				mc_frame_t* empty_frame = eval_stack_pop_obj(estack);
				mc_frame_t* stack_frame = frame_copy(empty_frame);
				vm_push_stack_frame(stack_frame);

				int32_t inargc = *(int32_t*)ins.operand;
				for (size_t i = 0; i < inargc; i++)
				{
					mc_obj_t obj = eval_stack_pop_obj(estack);
					nametab_t* locals = stack_frame->locals;
					nametab_set_index(locals, i, obj);
				}
				stack_frame->returnadr = IP;
				_vm_set_code_object(stack_frame->co);
				break;
			}
			case OP_RET:
			{
				mc_frame_t* frame = vm_pop_stack_frame();
				_vm_instance->ip = frame->returnadr;
				_vm_instance->active_frame = vm_pop_stack_frame();
				break;
			}
			case OP_SUB2D:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = eval_stack_pop_obj(estack);
				mc_obj_t obj4 = mc_matrix_build_2dsubscript(obj1, obj2, obj3);
				eval_stack_push_obj(estack, obj4);
				break;
			}
			case OP_RANGE:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = eval_stack_pop_obj(estack);
				mc_obj_t obj4 = mc_array_gen_m(obj1, obj2, obj3);
				eval_stack_push_obj(estack, obj4);
				break;
			}
			case OP_SUBS:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = eval_stack_pop_obj(estack);
				mc_obj_t obj4 = mc_matrix_subscript_set(obj1, obj2, obj3);
				eval_stack_push_obj(estack, obj4);
				break;
			}
			case OP_SUBG:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack); 
				mc_obj_t obj3 = mc_matrix_subscript_get(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_VCAT:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_vercat(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_HCAT:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_horzcat(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_ADD:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_add(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_SUB:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_sub(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_MUL:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_mul(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_DIV:
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_div(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			case OP_POW:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_pow(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_DOTMUL:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_dotmul(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_DOTDIV:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_dotdiv(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_DOTPOW:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_dotpow(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_CMP:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = NULL;
				int32_t cmp_op = *(int32_t*)ins.operand;
				switch (cmp_op)
				{
				case 0:
					obj3 = mc_matrix_equals(obj1, obj2);
					break;
				case 1:
					obj3 = mc_matrix_nequals(obj1, obj2);
					break;
				case 2:
					obj3 = mc_matrix_lt(obj1, obj2);
					break;
				case 3:
					obj3 = mc_matrix_lte(obj1, obj2);
					break;
				case 4:
					obj3 = mc_matrix_gt(obj1, obj2);
					break;
				case 5:
					obj3 = mc_matrix_gte(obj1, obj2);
					break;
				} 
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_AND:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_land(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_OR:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = eval_stack_pop_obj(estack);
				mc_obj_t obj3 = mc_matrix_lor(obj1, obj2);
				eval_stack_push_obj(estack, obj3);
				break;
			}
			case OP_NEG:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = mc_matrix_neg(obj1, obj2);
				eval_stack_push_obj(estack, obj2);
				break;
			}
			case OP_TRANS:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_obj_t obj2 = mc_matrix_transp(obj1, obj2);
				eval_stack_push_obj(estack, obj2);
				break;
			}
			case OP_GITER:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				mc_matrix_iter_t* riter = mc_matrix_riter(obj1);
				eval_stack_push_obj(estack, riter);
				break;
			}
			case OP_FOR:
			{
				mc_matrix_iter_t* riter = eval_stack_top_obj(estack);
				mc_bool_t ret = mc_matrix_riter_next(riter); 
				if (!ret)
				{
					eval_stack_pop_obj(estack);
					int32_t disp = *(int32_t*)ins.operand;
					IP = IP + disp;
				}
				eval_stack_push_obj(estack, riter->iter);
				break;
			}
			case OP_JMP:
			{
				int32_t disp = *(int32_t*)ins.operand;
				IP = IP + disp;
				if (disp < 0)
					IP -= 5;
				break;
			}
			case OP_JMPIF:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				if (mc_matrix_is_false(obj1))
				{
					//eval_stack_pop_obj(estack);
					int32_t disp = *(int32_t*)ins.operand;
					IP = IP + disp;
				}
				break;
			}
			case OP_JMPIT:
			{
				mc_obj_t obj1 = eval_stack_pop_obj(estack);
				if (mc_matrix_is_true(obj1))
				{
					//eval_stack_pop_obj(estack);
					int32_t disp = *(int32_t*)ins.operand;
					IP = IP + disp;
				}
				break;
			}
			case OP_NOP:
				// No operation :) 
				break;
			case OP_HALT:
				goto halt;

			default:
				break;
		}
	}
halt:
	return;
}