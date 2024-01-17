#include <compiler/comp.h>
#include <compiler/patcher.h>
#include <adt/vector.byte.h>
#include <adt/vector.int.h>
#include <vm/symtab.h>

vector__t* patcher_stack = NULL;
vector__t* loop_frame_stack = NULL;

static struct compiler_state
{
	bool in_statement : 1;
	bool in_func : 1;
	bool in_lhs : 1;
}
state = { false, false };

static jump_patcher_t* _comp_push_patcher(jump_patcher_t* jp)
{
	return vector__push_back(patcher_stack, jp);
} 
static jump_patcher_t* _comp_pop_patcher()
{
	return vector__pop_back(patcher_stack);
} 
static loop_frame_t* _comp_push_loop_frame(loop_frame_t* jp)
{
	return vector__push_back(loop_frame_stack, jp);
}
static loop_frame_t* _comp_pop_loop_frame()
{
	return vector__pop_back(loop_frame_stack);
}
static loop_frame_t* _comp_top_loop_frame()
{
	return vector__get(loop_frame_stack, loop_frame_stack->count - 1);
}

static void _comp_rvalue_id(anode_t* root, code_object_t* co);
static void _comp_lvalue_id(anode_t* root, code_object_t* co);
static void _comp_num(anode_t* root, code_object_t* co);
static void _comp_string(anode_t* root, code_object_t* co);
static void _comp_true_false(anode_t* root, code_object_t* co);
static void _comp_param_list(anode_t* root, code_object_t* co);
static void _comp_array(anode_t* root, code_object_t* co);
static void _comp_matrix(anode_t* root, code_object_t* co);
static void _comp_range(anode_t* root, code_object_t* co);
static void _comp_lvalue_subscript(anode_t* root, code_object_t* co);
static void _comp_rvalue_subscript(anode_t* root, code_object_t* co);
static void _comp_transpose(anode_t* root, code_object_t* co); 
static void _comp_operator(anode_t* root, code_object_t* co);
static void _comp_expression(anode_t* root, code_object_t* co);
static bool _comp_assignment(anode_t* root, code_object_t* co);
static void _comp_if_block(anode_t* root, code_object_t* co, jump_patcher_t* jp);
static void _comp_elseif_block(anode_t* root, code_object_t* co, jump_patcher_t* jp);
static void _comp_else_block(anode_t* root, code_object_t* co, jump_patcher_t* jp);
static void _comp_if_statement(anode_t* root, code_object_t* co);
static void _comp_switch_statement(anode_t* root, code_object_t* co);
static void _comp_continue_statement(anode_t* root, code_object_t* co);
static void _comp_break_statement(anode_t* root, code_object_t* co);
static void _comp_for_statement(anode_t* root, code_object_t* co);
static void _comp_while_statement(anode_t* root, code_object_t* co);
static void _comp_function_declaration(anode_t* root);
static void _comp_statement(anode_t* root, code_object_t* co);
static void _comp_statement_list(anode_t* root, code_object_t* co, size_t* emitted_code_sz);
 
static void _comp_rvalue_id(anode_t* root, code_object_t* co)
{
	int index = root->index; 
	if (HAS_GLOBAL(root->sym_flags))
		co_emit_ldglob(co, index);
	else if (HAS_LOCAL(root->sym_flags))
		co_emit_ldloc(co, index); 
}

static void _comp_lvalue_id(anode_t* root, code_object_t* co)
{
	int index = root->index;
	if (HAS_GLOBAL(root->sym_flags))
	{
		co_emit_stglob(co, index); 
	}
	else if (HAS_LOCAL(root->sym_flags))
	{
		co_emit_stloc(co, index); 
	}
	if (state.in_lhs)
		_comp_rvalue_id(root, co);
}

static void _comp_num(anode_t* root, code_object_t* co)
{
	co_emit_pushsd(co, root->value.num); 
}

static void _comp_string(anode_t* root, code_object_t* co)
{
	co_emit_ldglob(co, root->index); 
}

static void _comp_true_false(anode_t* root, code_object_t* co)
{
	if (AST_KIND(root) == AST_TRUE)
		co_emit_pushsl(co, 1); 
	else
		co_emit_pushsl(co, 0);
}

static void _comp_array_row(anode_t* root, code_object_t* co)
{
	if (root->childs->count == 0)
		return;
	anode_t* expr = ast_get_child(root, root->childs->count - 1);
	_comp_expression(expr, co);
	for (int i = root->childs->count - 2; i >= 0; --i)
	{
		anode_t* expr = ast_get_child(root, i);
		_comp_expression(expr, co);
		co_emit_hcat(co);
 	}
}

static void _comp_array(anode_t* root, code_object_t* co)
{
	anode_t* row = ast_get_first(root);
	_comp_array_row(row, co); 
}

static void _comp_matrix(anode_t* root, code_object_t* co)
{
	if (root->childs->count == 0)
		return;
	anode_t* row = ast_get_child(root, root->childs->count - 1);
	_comp_array_row(row, co);
	for (int i = root->childs->count - 2; i >= 0; --i)
	{
		row = ast_get_child(root, i);
		_comp_array_row(row, co);
		co_emit_vcat(co);
	}
}

static void _comp_range(anode_t* root, code_object_t* co)
{
	anode_t* start = ast_get_first(root);
	anode_t* increment = ast_get_second(root);
	anode_t* end = ast_get_third(root);
	_comp_expression(end, co);
	_comp_expression(increment, co);
	_comp_expression(start, co);
	co_emit_range(co);
}

static void _comp_param_list(anode_t* root, code_object_t* co)
{
	for (int i = root->childs->count - 1; i >= 0; --i)
	{
		anode_t* param = ast_get_child(root, i);
		_comp_expression(param, co);
	}
}
	   
static void _comp_lvalue_subscript(anode_t* root, code_object_t* co) 
{
	anode_t* name = ast_get_first(root);
	anode_t* param_list = ast_get_second(root);
	_comp_param_list(param_list, co);
	_comp_lvalue_id(name, co);
	co_emit_subs(co); /*
	if (state.in_statement)
		_comp_rvalue_id(name, co);*/
}
	   
static void _comp_rvalue_subscript(anode_t* root, code_object_t* co)
{
	anode_t* name = ast_get_first(root);
	anode_t* param_list = ast_get_second(root);
	_comp_param_list(param_list, co);
	_comp_rvalue_id(name, co);
	 co_emit_subg(co); 
}

static void _comp_transpose(anode_t* root, code_object_t* co)
{
	anode_t* child = ast_get_first(root);
	_comp_expression(child, co);
	co_emit_trans(co);
}

static void _comp_operator(anode_t* root, code_object_t* co)
{ 
	_comp_expression(ast_get_second(root), co);
	// If this operator is binary operator then compile the second operator.
	if(HAS_BINARY(root))
		_comp_expression(ast_get_first(root), co);

#define _(opcode) co_emit_##opcode(co)
#define __(opcode, arg) co_emit_##opcode(co, arg)

#define xx(a, b)

#define yy(a, b, c, d)

#define zz(a, b, c)					\
case a:								\
	_(c);							\
	break;						

#define tt(a, b, c, d)				\
case a:								\
	__(c, d);						\
	break;		 

	switch (AST_KIND(root))
	{
		#include <parse/anodes.h>
	default:
		break;
	}
	 
#undef tt
#undef zz
#undef yy
#undef xx
}

static void _comp_function_call(anode_t* root, code_object_t* co)
{
	anode_t* function_name = ast_get_first(root);
	mc_string_t fname = function_name->value.id;
	anode_t* param_list = ast_get_second(root); 
	_comp_param_list(param_list, co);
	
	// If not built-in, 
	if (wcscmp(fname, L"disp") == 0)
		co_emit_disp(co);
	else if (wcscmp(fname, L"print") == 0)
		co_emit_disp(co);
	else if (wcscmp(fname, L"vercat") == 0)
		co_emit_vcat(co);
	else if (wcscmp(fname, L"horzcat") == 0)
		co_emit_hcat(co);
	else
	{ 
		co_emit_ldframe(co, function_name->frame_index);
		co_emit_call(co, param_list->childs->count);
	}
}

static void _comp_expression(anode_t* root, code_object_t* co)
{
	if (AST_KIND(root) == AST_EXPRESSION)
		root = ast_get_first(root);
	switch (AST_KIND(root))
	{
		case AST_ID:
			_comp_rvalue_id(root, co);  
			break;
		case AST_NUMBER:
			_comp_num(root, co);
			break;
		case AST_TRUE:
		case AST_FALSE:
			_comp_true_false(root, co);
			break;
		case AST_STRING:
			_comp_string(root, co);
			break;
		case AST_ARRAY:
			_comp_array(root, co);
			break;
		case AST_MATRIX:
			_comp_matrix(root, co);
			break;
		case AST_RANGE:
			_comp_range(root, co);
			break;
		case AST_FUNC_CALL:
			_comp_function_call(root, co);
			break;
		case AST_SUBSCRIPT:
			_comp_rvalue_subscript(root, co);
			break;
	}
	if (HAS_OPERATOR(root))
	{
		_comp_operator(root, co);
	}
}

static bool _comp_assignment(anode_t* root, code_object_t* co)
{
	anode_t* lvalue = ast_get_first(root);
	anode_t* rvalue = ast_get_second(root);
 	/*bool previous_lhs_state = state.in_lhs;
	state.in_lhs = false;*/
	_comp_expression(rvalue, co);
	//state.in_lhs = true;
	switch (AST_KIND(lvalue))
	{
		case AST_ID:
		{
			_comp_lvalue_id(lvalue, co);
			_comp_rvalue_id(lvalue, co);
			return true;
		}
		case AST_SUBSCRIPT:
		{
			_comp_lvalue_subscript(lvalue, co);
			return true;
		}
		case AST_ARRAY:
		{

			if (AST_KIND(rvalue) == AST_FUNC_CALL)
			{
				anode_t* func_name = ast_get_first(rvalue);
				func_name->outargc;
				anode_t* row = ast_get_first(lvalue);
				size_t i = 0;
				for (; i < row->childs->count; i++)
				{
					anode_t* expr = ast_get_child(row, i);
					if (AST_KIND(expr) == AST_EXPRESSION)
						expr = ast_get_first(expr);
					_comp_lvalue_id(expr, co);
				} 
				for (; i < func_name->outargc; i++)
				{
					co_emit_pop(co);
				}
			}
			return false;
		}
	}
	return true;
	//state.in_lhs = previous_lhs_state;
} 

static void _comp_if_block(anode_t* root, code_object_t* co, jump_patcher_t* jp)
{
	anode_t* selection_expr = ast_get_first(root);
	anode_t* if_body = ast_get_second(root);
	_comp_expression(selection_expr, co);
	co_emit_jmpif(co, 0);
	size_t offset = co->code->count - 4;
	size_t block_sz = 0;
	_comp_statement_list(if_body, co, &block_sz); 

	jump_patcher_push_jmpif(jp, offset, block_sz);
}

static void _comp_elseif_block(anode_t* root, code_object_t* co, jump_patcher_t* jp)
{
	anode_t* selection_expr = ast_get_first(root);
	anode_t* body = ast_get_second(root);

	co_emit_jmp(co, 0);
	size_t jmp_offset = co->code->count - 4;

	
	size_t selection_code_sz = CO_GET_LENGTH(co); 
	_comp_expression(selection_expr, co); 

	co_emit_jmpif(co, 0);
	size_t jmpif_offset = CO_GET_LENGTH(co) - 4; 

	selection_code_sz = CO_GET_LENGTH(co) - selection_code_sz;

	size_t block_sz = 0;
	_comp_statement_list(body, co, &block_sz);
	jump_patcher_push_jmp(jp, jmp_offset, selection_code_sz + block_sz + 5);
	jump_patcher_push_jmpif(jp, jmpif_offset, block_sz);
}

static void _comp_else_block(anode_t* root, code_object_t* co, jump_patcher_t* jp)
{
	anode_t* else_body = ast_get_first(root); 
	
	size_t jmp_offset = co->code->count;
	co_emit_jmp(co, 0);
	jmp_offset++;
	
	size_t block_sz = 0;
	_comp_statement_list(else_body, co, &block_sz);
	jump_patcher_push_jmp(jp, jmp_offset, block_sz);
}

void jump_patcher_patch(jump_patcher_t* jp, code_object_t* co)
{
	while (jp->sco_stack->count > 0)
	{
		int offset = jump_patcher_pop_sco(jp);
		int relative_addr = jump_patcher_pop_bsz(jp);
		co_insert_i32(co, relative_addr, offset);
	}
	while (jp->sto_stack->count > 0)
	{
		int offset = jump_patcher_pop_sto(jp);
		int relative_addr = jump_patcher_pop_absz(jp);
		co_insert_i32(co, relative_addr, offset);
	}
}

static void _comp_if_statement(anode_t* root, code_object_t* co)
{
	anode_t* if_block = ast_get_first(root); 

	jump_patcher_t* patcher = jump_patcher_alloc();
	_comp_push_patcher(patcher);

	_comp_if_block(if_block, co, patcher);

	anode_t* other_block = NULL;
	// if there is an elseif, else chain, compile them.
	size_t i = 1;
	while (i < root->childs->count)
	{
		anode_t* child = ast_get_child(root, i++);
		if (AST_KIND(child) == AST_ELSEIF)
			_comp_elseif_block(child, co, patcher);
		else if(AST_KIND(child) == AST_ELSE)
			_comp_else_block(child, co, patcher);
	} 
	patcher = _comp_pop_patcher();
	jump_patcher_patch(patcher, co);
}

static void _comp_case_block(anode_t* root, anode_t* selection_expr, bool is_first, jump_patcher_t* jp, code_object_t* co)
{
	anode_t* case_expr = ast_get_first(root);
	anode_t* case_body = ast_get_second(root);
	size_t jmp_offset;

	if (!is_first)
	{
		co_emit_jmp(co, 0);
		jmp_offset = CO_GET_LENGTH(co) - 4;
	}
	size_t total_sz = CO_GET_LENGTH(co);
	_comp_expression(selection_expr, co);
	_comp_expression(case_expr, co);
	co_emit_cmp(co, 0);
	co_emit_jmpif(co, 0);
	size_t jmpif_offset = CO_GET_LENGTH(co) - 4;
	size_t blksz = 0;
	_comp_statement_list(case_body, co, &blksz);

	total_sz = CO_GET_LENGTH(co) - total_sz;
	if(!is_first)
		jump_patcher_push_jmp(jp, jmp_offset, total_sz + 5);
	jump_patcher_push_jmpif(jp, jmpif_offset, blksz); 
	
}

static void _comp_otherwise_block(anode_t* root, bool is_first, jump_patcher_t* jp, code_object_t* co)
{ 
	anode_t* otherwise_body = ast_get_first(root);
	size_t jmp_offset;

	if (!is_first)
	{
		co_emit_jmp(co, 0);
		jmp_offset = CO_GET_LENGTH(co) - 4;
	}
	size_t total_sz = CO_GET_LENGTH(co);   
	size_t blksz = 0;
	_comp_statement_list(otherwise_body, co, &blksz);

	total_sz = CO_GET_LENGTH(co) - total_sz;
	 
	if (!is_first)
		jump_patcher_push_jmp(jp, jmp_offset, total_sz + 5);
}

static void _comp_switch_statement(anode_t* root, code_object_t* co)
{
	anode_t* selection_expr = ast_get_first(root);
	
	jump_patcher_t* patcher = jump_patcher_alloc(); 

	_comp_push_patcher(patcher);

	anode_t* case_otherwise = ast_get_second(root);

	// if switch statement consists of only otherwise statement.
	if (AST_KIND(case_otherwise) == AST_OTHERWISE)
	{
		_comp_otherwise_block(case_otherwise, true, patcher, co);
	}
	else
	{
		_comp_case_block(case_otherwise, selection_expr, true, patcher, co);

		size_t i = 1;	// Switch Selection + Previous case 
		while (i < root->childs->count)
		{
			case_otherwise = ast_get_child(root, i++);
			
			if(AST_KIND(case_otherwise) == AST_CASE)
				_comp_case_block(case_otherwise, selection_expr, false, patcher, co);
			else if(AST_KIND(case_otherwise) == AST_OTHERWISE)
				_comp_otherwise_block(case_otherwise, false, patcher, co);
		}
	}
	patcher = _comp_pop_patcher();
	jump_patcher_patch(patcher, co);
}

void loop_frame_patch(loop_frame_t* lf, code_object_t* co)
{
	while (lf->bo_stack->count > 0)
	{
		int offset = loop_frame_pop_bo(lf);
		int relative_addr = lf->loop_end - (offset + 4);
		co_insert_i32(co, relative_addr, offset);
	}
	while (lf->co_stack->count > 0)
	{
		int offset = loop_frame_pop_co(lf);
		int relative_addr = lf->loop_start - (offset - 1);
		co_insert_i32(co, relative_addr, offset);
	}
}

static void _comp_break_statement(anode_t* root, code_object_t* co)
{
	loop_frame_t* lp = _comp_top_loop_frame();
	size_t jmp_offset = CO_GET_LENGTH(co);
	co_emit_jmp(co, 0);
	jmp_offset++;
	loop_frame_push_bo(lp, jmp_offset);
}

static void _comp_continue_statement(anode_t* root, code_object_t* co)
{
	loop_frame_t* lp = _comp_top_loop_frame();
	size_t jmp_offset = CO_GET_LENGTH(co);
	co_emit_jmp(co, 0);
	jmp_offset++;
	loop_frame_push_co(lp, jmp_offset);
}

static void _comp_while_statement(anode_t* root, code_object_t* co)
{
	loop_frame_t* lf = loop_frame_alloc();
	_comp_push_loop_frame(lf);
	lf->loop_start = CO_GET_LENGTH(co);

	anode_t* selection_expr = ast_get_first(root);
	anode_t* body = ast_get_second(root);

	size_t selection_plus_body = CO_GET_LENGTH(co);

	_comp_expression(selection_expr, co);

	co_emit_jmpif(co, 0);
	size_t jmpif_offset = CO_GET_LENGTH(co) - 4;

	size_t body_plus_jmp = 0;
	_comp_statement_list(body, co, &body_plus_jmp);

	selection_plus_body = CO_GET_LENGTH(co) - selection_plus_body;
	co_emit_jmp(co, -selection_plus_body);

	body_plus_jmp += 5;
	co_insert_i32(co, body_plus_jmp, jmpif_offset);
	
	lf->loop_end = CO_GET_LENGTH(co);
	loop_frame_patch(lf, co);
	_comp_pop_loop_frame();
}

static void _comp_for_statement(anode_t* root, code_object_t* co)
{
	loop_frame_t* lf = loop_frame_alloc();
	_comp_push_loop_frame(lf);
	lf->loop_start = CO_GET_LENGTH(co);

	anode_t* iter_var = ast_get_first(root);
	anode_t* iterable = ast_get_second(root);
	anode_t* body = ast_get_third(root);
	
	_comp_expression(iterable, co);
	co_emit_giter(co); 
	size_t for_plus_body = CO_GET_LENGTH(co);

	co_emit_for(co, 0);
	size_t for_offset = CO_GET_LENGTH(co) - 4;

	size_t body_plus_jmp = CO_GET_LENGTH(co);
	_comp_lvalue_id(iter_var, co);
	body_plus_jmp = CO_GET_LENGTH(co) - body_plus_jmp;
	
	_comp_statement_list(body, co, &body_plus_jmp);
	
	for_plus_body = CO_GET_LENGTH(co) - for_plus_body;
	co_emit_jmp(co, -for_plus_body);
	
	body_plus_jmp += 5;

	co_insert_i32(co, body_plus_jmp, for_offset);
	lf->loop_end = CO_GET_LENGTH(co);
	loop_frame_patch(lf, co);
	_comp_pop_loop_frame();
}

static void _comp_function_declaration(anode_t* root)
{
	code_object_t* co = co_alloc();
	anode_t* function_name = ast_get_first(root);
	anode_t* return_expr = NULL;
	if(root->childs->count > 3)
	{
		return_expr = ast_get_first(root);
		function_name = ast_get_second(root);
	}

	anode_t* body = ast_get_last(root);

	_comp_statement_list(body, co, NULL);
	if (return_expr)
	{
		if (AST_KIND(return_expr) == AST_ID)
			_comp_rvalue_id(return_expr, co);
		else
		{ 
			anode_t* row = ast_get_first(return_expr);
			for (int i = row->childs->count - 1; i >= 0 ; --i)
			{
				anode_t* expr = ast_get_child(row, i);
				if(AST_KIND(expr) == AST_EXPRESSION)
					expr = ast_get_first(expr);
				_comp_rvalue_id(expr, co);
			}
		}
	}
	else
		co_emit_pushr(co, NULL);
	co_emit_ret(co);
	mc_string_t asm_listing = co_disassemble(co);
	wprintf(L"\n%s\n", asm_listing);
	function_name->frame->co = co;
}

static void _comp_statement(anode_t* root, code_object_t* co)
{
	bool previous_state = state.in_statement;
	state.in_statement = true;
	if (AST_GET(root->kind) == AST_STATEMENT)
	{
		
		anode_t* child = ast_get_first(root);
		child = ast_get_first(root);
		switch (AST_GET(child->kind))
		{
			case AST_EXPRESSION:
				_comp_expression(child, co);
				co_emit_pop(co);
				break;
			case AST_ASSIGNMENT_EXPRESSION:
				if(_comp_assignment(child, co))
					co_emit_pop(co);
				break;
			case AST_IF_STATEMENT:
				state.in_statement = false;
				_comp_if_statement(child, co);
				break;
			case AST_SWITCH_STATEMENT:
				state.in_statement = false;
				_comp_switch_statement(child, co);
				break;
			case AST_FOR_STATEMENT:
				state.in_statement = false;
				_comp_for_statement(child, co);
				break;
			case AST_WHILE_STATEMENT:
				state.in_statement = false;
				_comp_while_statement(child, co);
				break;
			case AST_BREAK_STATEMENT: 
				_comp_break_statement(child, co);
				break;
			case AST_CONTINUE_STATEMENT:
				_comp_continue_statement(child, co);
				break;
			case AST_RETURN:
				if(state.in_func)
					co_emit_ret(co);
				else
					co_emit_halt(co);
				break;
			case AST_FUNCTION_DECLARATION:
				state.in_statement = false;
				state.in_func = true;
				_comp_function_declaration(child);
				state.in_func = false;
				break;
		}
	}
	else if (AST_GET(root->kind) == AST_EXPRESSION_STATEMENT)
	{
		anode_t* expr = ast_get_first(root); 
		expr = ast_get_first(root);
		if (AST_KIND(expr) == AST_EXPRESSION)
			_comp_expression(expr, co); 
		else if (AST_KIND(expr) == AST_ASSIGNMENT_EXPRESSION)
			_comp_assignment(expr, co); 
		co_emit_disp(co);
	} 
	state.in_statement = previous_state;
}

static void _comp_statement_list(anode_t* root, code_object_t* co, size_t* emitted_code_sz)
{ 
	size_t previous_counter = CO_GET_LENGTH(co);
	for (size_t i = 0; i < root->childs->count; i++)
	{
		anode_t* child = ast_get_child(root, i);
		_comp_statement(child, co);
	} 
	if(emitted_code_sz)
		*emitted_code_sz += CO_GET_LENGTH(co) - previous_counter;
}

code_object_t* comp_program(anode_t* root)
{
	patcher_stack = vector__alloc(NULL);
	loop_frame_stack = vector__alloc(NULL);
	anode_t* statement_list = ast_get_first(root);
	code_object_t* co = co_alloc(); 
	_comp_statement_list(statement_list, co, NULL); 
	co_emit_halt(co);
	return co;
}


