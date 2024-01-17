#include <parse/semantic.h>
#include <lex/scoord.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static struct analyzer_state
{
	bool in_if_statement			: 1;
	bool in_switch_statement		: 1;
	bool in_for_statement			: 1;
	bool in_while_statement			: 1;
	bool in_function_declaration	: 1;
}state = { false, false };

static void _error(anode_t* node, const wchar_t* message)
{
	fflush(stderr);
	fwprintf(stderr, L"%s(%d, %d): Error: %s\n", node->scoord.file, node->scoord.line_no, node->scoord.column_no, message);
	exit(MC_ERROR_FAILURE);
}
 
static void is_jump_statement_in_if_statement(anode_t* if_statement);
static void is_jump_statement_in_switch_statement(anode_t* switch_statement);
static void is_function_return_array_valid(anode_t* function_decl);

void semantic_analyze(anode_t* root)
{
	if (!root)
		return;
	if (AST_KIND(root) == AST_ASSIGNMENT_EXPRESSION)
	{
		anode_t* lvalue = ast_get_first(root);
		if (AST_KIND(lvalue) == AST_ARRAY)
		{
			sem_array_must_be_identifier(lvalue);
			return;
		}
	}
	else if (AST_KIND(root) == AST_STATEMENT)
	{
		anode_t* statement = ast_get_first(root);
		switch (AST_KIND(statement))
		{
			case AST_BREAK_STATEMENT:
			case AST_CONTINUE_STATEMENT:
			{
				if((state.in_if_statement || state.in_switch_statement) && (!state.in_while_statement && !state.in_for_statement))
					_error(statement, L"Break and continue statements cannot be in if statement.");
				return;
			}
			case AST_IF_STATEMENT:
			{
				bool previous_state = state.in_if_statement;
				state.in_if_statement = true;
				is_jump_statement_in_if_statement(statement);
				state.in_if_statement = previous_state;
				return;
			}
			case AST_SWITCH_STATEMENT:
			{
				bool previous_state = state.in_switch_statement;
				state.in_switch_statement = true;
				is_jump_statement_in_switch_statement(statement);
				state.in_switch_statement = previous_state;
				return;
			}
			case AST_WHILE_STATEMENT:
			{
				bool previous_state = state.in_while_statement;
				state.in_while_statement = true;
				semantic_analyze(statement);
				state.in_while_statement = previous_state;
				return;
			}
			case AST_FOR_STATEMENT:
			{
				bool previous_state = state.in_for_statement;
				state.in_for_statement = true;
				semantic_analyze(statement);
				state.in_for_statement = previous_state;
				return;
			}
			case AST_FUNCTION_DECLARATION:
			{
				bool previous_state = state.in_function_declaration;
				state.in_function_declaration = true;
				semantic_analyze(statement);
				state.in_function_declaration = previous_state;
				return;
			}
		}
	}
	for (size_t i = 0; i < root->childs->count; i++)
	{
		semantic_analyze(ast_get_child(root, i));
	}
}

void sem_array_must_be_identifier(anode_t* array)
{
	anode_t* array_row = ast_get_first(array);
	for (size_t i = 0; i < array_row->childs->count; i++)
	{ 
		anode_t* expr = ast_get_child(array_row, i);
		expr = ast_get_first(expr);
		if (AST_KIND(expr) != AST_ID)
			_error(expr, L"Assigning the function output to this expression is not supported.");
	}
}

void sem_subscript_must_be_2D(anode_t* subscript)
{
	anode_t* name = ast_get_first(subscript);
	anode_t* param_list = ast_get_second(subscript);
	 if(param_list->childs->count > 2)
			_error(ast_get_third(param_list), L"Only 2D matrix dimension is supported");
}

static void is_jump_statement_in_if_statement(anode_t* if_statement)
{
	anode_t* block = NULL;
	anode_t* if_node = ast_get_first(if_statement);
	block = ast_get_second(if_node);
	semantic_analyze(block);

	size_t i = 1;
	anode_t* other = ast_get_child(if_statement, i);
	while (i < if_statement->childs->count)
	{
		switch (AST_KIND(other))
		{
			case AST_ELSEIF:
			{
				block = ast_get_second(other);
				semantic_analyze(block);
				break;
			}
			case AST_ELSE:
			{
				block = ast_get_first(other);
				semantic_analyze(block);
				break;
			} 
		}
		i++;
	}
}

static void is_jump_statement_in_switch_statement(anode_t* switch_statement)
{
	anode_t* block = NULL; 

	size_t i = 1;
	anode_t* case_otherwise = ast_get_child(switch_statement, i);
	while (i < switch_statement->childs->count)
	{
		switch (AST_KIND(case_otherwise))
		{
			case AST_CASE:
			{
				block = ast_get_second(case_otherwise);
				semantic_analyze(block);
				break;
			}
			case AST_OTHERWISE:
			{
				block = ast_get_first(case_otherwise);
				semantic_analyze(block);
				break;
			}
		}
		i++;
	}
}

static void is_function_return_array_valid(anode_t* function_decl)
{
	anode_t* function_name = ast_get_first(function_decl);
	anode_t* return_expr = ast_get_first(function_decl);
	if (function_decl->childs->count == 3)
		return_expr = NULL;
	else
		function_name = ast_get_second(function_decl);
	
	if (AST_KIND(return_expr) == AST_ARRAY)
	{
		if (return_expr->childs->count >= 2)
			_error(return_expr, L"Matrix is not supported in function return expression.");
		anode_t* array_row = ast_get_first(return_expr);
		for (size_t i = 0; i < array_row->childs->count; i++)
		{
			anode_t* expr = ast_get_child(array_row, i);
			expr = ast_get_first(expr);
			if (AST_KIND(expr) != AST_ID)
				_error(expr, L"Function return array must consists of only identifiers.");
		}
	}
}