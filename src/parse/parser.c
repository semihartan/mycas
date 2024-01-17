/*
* Syntax directed Top-down Recursive-Descent Parser for mclang
* 
* EBNF Grammar:
* 
* 
* 
*/
#include <parse/parser.h>
#include <parse/lstack.h>
#include <parse/ast.h>  
#include <parse/semantic.h>  
#include <vm/symtab.h>

#include <lex/lexer.h>
#include <lex/token.h>

#include <error.h>
#include <mcstr.h>
#include <util.h>


static void _error(parser_t* self, const wchar_t* message);
static void __error(anode_t* node, const wchar_t* message);
static void _expect(parser_t* self, int token);
static int _next_token(parser_t* self);

#define GET_SCOORD()						(self->act_lexer->scoord)
#define GET_SYM_SCOPE(sc)					(sc == SC_GLOBAL ? SYM_GLOBAL : SYM_LOCAL)
#define SYM_LOOKUP(name)					(self->scope == SC_GLOBAL ? symtab_lookup_global(symtab, name) : symtab_lookup_local(symtab, name))

#define LOCAL_BEGIN()						\
enum scope prev_scope;						\
do											\
{											\
	prev_scope = self->scope;				\
	self->scope = SC_LOCAL;					\
}while (false)

#define LOCAL_END()							\
self->scope = prev_scope

#define IS_ID(ast)				(AST_GET(ast->kind) == AST_ID)
#define IS_ARRAY(ast)			(AST_GET(ast->kind) == AST_ARRAY)
#define IS_NUM(ast)				(AST_GET(ast->kind) == AST_NUMBER)
#define IS_EXPRESSION(ast)		(AST_GET(ast->kind) == AST_EXPRESSION)
#define TOKEN					self->token
#define TOKEN					self->token
#define EXPECT(token)			_expect(self, token)
#define NEXT()					_next_token(self)
#define PEEK(k)					lex_peek_token(self->act_lexer, k)
#define SKIP_TOK(token)							\
do												\
{												\
	if (TOKEN == token)							\
		NEXT();									\
} while (0)							

enum expression_terminator
{
	ET_STATEMENT,
	ET_EXPRESSION,
	ET_NULL
};

static size_t _key_counter = 0;
static mc_string_t _build_key_for_string_object()
{
 	mc_string_t key = util_format(L"StringObj%u", _key_counter++);
	return key;
}
static void _check_synchronize_rvalue_with_symtab(parser_t* self, anode_t* rvalue_expr);

anode_t* parse_program(parser_t * self)
{
	/*
	* <program> = <statement-list>
	*/
	anode_t* program = ast_unary(AST_PROGRAM, NULL, GET_SCOORD()); 
	ast_add_child(program, parse_statement_list(self));
	if (TOKEN == TK_EOF)
		pop_lexer(self);
	return program;
}

//anode_t* parse_module(parser_t* self, anode_t* program)
//{
//	anode_t* module = ast_nary(AST_MODULE);
//	ast_add_child(program, parse_statement_list(self)); 
//}

anode_t* parse_array_row(parser_t* self)
{
	/*
	* <array-row> = [ <expression> { [','] <expression> } [';'] ]
	*/
	anode_t* arr_el = parse_range(self);
	if (arr_el != NULL)
	{
		anode_t* root = ast_nary(AST_ARRAY_ROW, GET_SCOORD());
		// Add the first element.
		ast_add_child(root, arr_el);
		// Check the next element. 
		arr_el = parse_range(self);
		while (TOKEN == TK_COMMA || arr_el)
		{
			SKIP_TOK(TK_COMMA);
			ast_add_child(root, arr_el);

			arr_el = parse_range(self);
		}
		 
		SKIP_TOK(TK_SEMICOLON);
		return root;
	}
	return NULL;
}

anode_t* parse_array_matrix(parser_t* self)
{
	/*
	* <array-matrix> = '[' { <array-row> } ']'
	*/
	if (TOKEN == TK_OPBRACKET)
	{
		NEXT();
		anode_t* root = ast_nary(AST_ARRAY, GET_SCOORD());
		anode_t* array_row = parse_array_row(self);
		while (array_row)
		{
			ast_add_child(root, array_row);
			array_row = parse_array_row(self);
		}
		 
		if (root->childs->count > 1)
			root->kind = AST_MATRIX;
		EXPECT(TK_CLBRACKET);
		return root;
	}
	return NULL;
} 

anode_t* parse_param_list(parser_t* self)
{
	/*
	* <param-list> = <expression> { ',' <expression> }
	*/ 
	anode_t* expr = parse_range(self);
	if (expr != NULL)
	{ 
		anode_t* param_list = ast_nary(AST_PARAM_LIST, GET_SCOORD());
		ast_add_child(param_list, expr);
		while (TOKEN == TK_COMMA)
		{
			NEXT();
			expr = parse_range(self);
			if (expr == NULL)
				_error(self, L"Expected an expression here.");
			ast_add_child(param_list, expr);
		}
		return param_list;
	}
	return NULL;
	/*anode_t* param_list = ast_nary(AST_PARAM_LIST);
	anode_t* expr = parse_range(self);
	if (expr != NULL)
		ast_add_child(param_list, expr);
	while (TOKEN == TK_COMMA)
	{
		NEXT();
		expr = parse_range(self);
		if (expr != NULL)
			ast_add_child(param_list, expr);
	}
	return param_list;*/
}

anode_t* parse_primary_expression(parser_t* self)
{
	/*
	* <primary_expression>	= 'ID' 
							| 'NUMBER' 
							| 'STRING'
							| <array-matrix>
							| '(' expression ')'
	*/
	anode_t* root = NULL;
	if (TOKEN == TK_ID)
	{ 
		root = ast_leaf1(AST_ID, (node_value_t) { .id = mc_str_duplicate(token_data.identifier) }, GET_SCOORD());
		 
		sym_t* sym = SYM_LOOKUP(root->value.id);
		if (sym)
		{
			root->frame = sym->frame;
			if (root->frame)
				root->frame_index = sym->frame->frame_index;
			root->sym_flags = sym->flags;
			root->index = sym->tab_index;
		}
		NEXT();
	}
	else if (TOKEN == TK_NUMBER)
	{
		root = ast_leaf1(AST_NUMBER, (node_value_t) { token_data.number }, GET_SCOORD());
		NEXT();
	}
	else if (TOKEN == TK_TRUE)
	{
		root = ast_leaf1(AST_TRUE, (node_value_t) { 1.0 }, GET_SCOORD());
		NEXT();
	}
	else if (TOKEN == TK_FALSE)
	{
		root = ast_leaf1(AST_FALSE, (node_value_t) { 0.0 }, GET_SCOORD());
		NEXT();
	}
	else if (TOKEN == TK_END)
	{
		root = ast_leaf0(AST_END, GET_SCOORD());
		NEXT();
	}
	else if (TOKEN == TK_COLON)
	{
		root = ast_leaf0(AST_COLON, GET_SCOORD());
		NEXT();
	}
	else if (TOKEN == TK_STRING)
	{
		root = ast_leaf1(AST_STRING, (node_value_t) { .string = mc_str_duplicate(token_data.string) }, GET_SCOORD());
		mc_string_t strobj_key = _build_key_for_string_object();
		root->strobj_key = strobj_key;
		sym_t* sym = symtab_add_sym(symtab, strobj_key, SYM_GLOBAL | SYM_STRING, root->value.string, 0, 0);
		root->index = sym->tab_index;
		nametab_set_index(globals, root->index, root->value.string);
		NEXT();
	}
	else if (TOKEN == TK_OPPAR)
	{
		NEXT();
		root = parse_expression(self);
		EXPECT(TK_CLPAR);
	}
	else if ((root = parse_array_matrix(self)) != NULL)
		return root;
	return root;
}

anode_t* parse_postfix_expression(parser_t* self)
{
	/* 
	* <postfix-expression> = <primary-expression> [ ( '(' [ <param-list> ] ')' ) | ' ]
	*/
	anode_t* primary = parse_primary_expression(self);
	if (primary)
	{
		if (IS_ID(primary))
		{
			mc_string_t id = primary->value.id;
			/*
			* BURADA KALDIM!
			*	if(symtab[id] exists)
			*		if(symtab[root].type is FUNCTION)
			*			parse_function_call(self);
			*		else if (symtab[root].type is VARIABLE)
			*			parse_array_subscript(self);
			*	else
			*		symtab.add(id)
			*/
			/*sym_t* entry = SYM_LOOKUP(self->scope, id);
			if (entry)
			{
				if (TOKEN == TK_OPPAR)
				{
					NEXT();
					anode_t* postfix = NULL;
					if (entry->flags & SYM_FUNCTION)
					{

						postfix = ast_binary(AST_FUNC_CALL, NULL, NULL);
						anode_t* param_list = parse_param_list(self);
						ast_add_child(postfix, primary);
						ast_add_child(postfix, param_list);
					}
					else if (entry->flags & SYM_VARIABLE)
					{
						postfix = ast_binary(AST_SUBSCRIPT, NULL, NULL);
						anode_t* param_list = parse_param_list(self);
						if (!param_list)
							_error(self, L"Subscript expression cannot be empty.");
						ast_add_child(postfix, primary);
						ast_add_child(postfix, param_list);
					}
					EXPECT(TK_CLPAR);
					return postfix;
				}
			}*/
			sym_t* entry = NULL;
			
			if (TOKEN == TK_OPPAR)
			{
				if ((entry = symtab_lookup_global(symtab, id)) != NULL || (entry = symtab_lookup_local(symtab, id)) != NULL)
				{
					NEXT();
					anode_t* postfix = NULL;
					if (HAS_FUNC(entry->flags))
					{
						postfix = ast_binary(AST_FUNC_CALL, NULL, NULL, GET_SCOORD());
						anode_t* param_list = parse_param_list(self);
						_check_synchronize_rvalue_with_symtab(self, param_list);
						primary->frame = entry->frame;
						if(primary->frame)
							primary->frame_index = entry->frame->frame_index;
						primary->sym_flags = entry->flags;
						primary->index = entry->tab_index;
						primary->inargc = entry->inargc;
						primary->outargc = entry->outargc;
						ast_add_child(postfix, primary);
						ast_add_child(postfix, param_list);
					}
					else if (HAS_VAR(entry->flags))
					{
						postfix = ast_binary(AST_SUBSCRIPT, NULL, NULL, GET_SCOORD());
						anode_t* param_list = parse_param_list(self);
						if (!param_list)
							_error(self, L"Subscript expression cannot be empty.");
						 
						_check_synchronize_rvalue_with_symtab(self, param_list);
						ast_add_child(postfix, primary);

						primary->frame = entry->frame;
						if (primary->frame)
							primary->frame_index = entry->frame->frame_index;
						primary->sym_flags = entry->flags;
						primary->index = entry->tab_index;

						ast_add_child(postfix, param_list);
					} 
					EXPECT(TK_CLPAR);
					return postfix;
				}
				else
					_error(self, L"Unrecognized function or variable.");

			}
			/*else
				symtab_add_sym(symtab, id, SYM_GLOBAL | SYM_VARIABLE);*/
		}
		else if (IS_ARRAY(primary))
		{
			if (TOKEN == TK_TRANS)
			{
				NEXT();
				anode_t* transpose = ast_unary(AST_TRANS | AST_FLAG_OPERATOR, primary, GET_SCOORD());
				return transpose;
			}
		}
	} 
	return primary;
}

anode_t* parse_unary_expression(parser_t* self)
{
	/*
	* <unary-expression> = { ( '-' | '+' | '~') } <postfix-expression>
	*/
	int sign = 1;
	while (TOKEN == TK_MINUS || TOKEN == TK_PLUS || TOKEN == TK_NOT)
	{
		if (TOKEN == TK_MINUS)
			sign *= -1;
		else if (TOKEN == TK_NOT)
			sign *= -1;
		NEXT();
	}
	anode_t* postfix = parse_postfix_expression(self);
	if (postfix && sign < 0)
	{
		anode_t* unary = ast_unary(AST_NEG | AST_FLAG_OPERATOR, postfix, GET_SCOORD());
		unary->sign = sign;
		return unary;
	}
	return postfix;
}

anode_t* parse_power_expression(parser_t* self)
{
	/*
	* <power-expression> = <unary-expression> { ('^' | '.^') <unary-expression> }
	*/
	anode_t* left = parse_unary_expression(self);
	while (TOKEN == TK_POW || TOKEN == TK_DOTPOW)
	{
		anode_t* root = ast_binary((TOKEN == TK_POW ? AST_POW : AST_DOTPOW) | AST_FLAG_OPERATOR, NULL, NULL, GET_SCOORD());
		NEXT();
		ast_add_child(root, left);
		ast_add_child(root, parse_unary_expression(self));
		left = root;
	}
	return left;
}

anode_t* parse_multiplicative_expression(parser_t* self)
{
	/*
	* <multiplicative_expression> = <power-expression> { ('+' | '-' | '.*' | './') <power-expression>}
	*/
	anode_t* left = parse_power_expression(self);
	while (TOKEN == TK_MUL || TOKEN == TK_DIV || TOKEN == TK_DOTMUL || TOKEN == TK_DOTDIV)
	{
		enum ast_node_kind ast_kind = 0;
		switch (TOKEN)
		{
		case TK_MUL:
			ast_kind = AST_MUL;
			break;
		case TK_DOTMUL:
			ast_kind = AST_DOTMUL;
			break;
		case TK_DIV:
			ast_kind = AST_DIV;
			break;
		case TK_DOTDIV:
			ast_kind = AST_DOTDIV;
			break;
		}
		anode_t* root = ast_binary(ast_kind | AST_FLAG_OPERATOR, NULL, NULL, GET_SCOORD());
		NEXT();
		ast_add_child(root, left);
		ast_add_child(root, parse_power_expression(self));
		left = root;
	}
	return left;
}

anode_t* parse_additive_expression(parser_t* self)
{
	/*
	* <additive_expression> = <multiplicative_expression> { ('+' | '-') <multiplicative_expression>}
	*/
	anode_t* left = parse_multiplicative_expression(self);
	while (TOKEN == TK_PLUS || TOKEN == TK_MINUS)
	{
		anode_t* root = ast_binary((TOKEN == TK_PLUS ? AST_ADD : AST_SUB) | AST_FLAG_OPERATOR, NULL, NULL, GET_SCOORD());
		NEXT();
		ast_add_child(root, left);
		ast_add_child(root, parse_multiplicative_expression(self));
		left = root;
	}
	return left;
}

anode_t* parse_relational_expression(parser_t* self)
{
	/*
	* <relational-expression> = <additive-expression> { ('<' | '<=' |'>' | '>=') <additive-expression> }
	*/
	anode_t* left = parse_additive_expression(self);
	while (TOKEN == TK_LT || TOKEN == TK_LTEQ || TOKEN == TK_GT || TOKEN == TK_GTEQ)
	{
		enum ast_node_kind ast_kind = 0;
		switch (TOKEN)
		{
			case TK_LT:
				ast_kind = AST_REL_LT;
				break;
			case TK_LTEQ:
				ast_kind = AST_REL_LTEQ;
				break;
			case TK_GT:
				ast_kind = AST_REL_GT;
				break;
			case TK_GTEQ:
				ast_kind = AST_REL_GTEQ;
				break;
		}
		anode_t* root = ast_binary(ast_kind | AST_FLAG_OPERATOR, NULL, NULL, GET_SCOORD());
		NEXT();
		ast_add_child(root, left);
		ast_add_child(root, parse_additive_expression(self));
		left = root;
	}
	return left;
}

anode_t* parse_equality_expression(parser_t* self)
{
	/*
	* <equality-expression> = <relational-expression> { ('==' | '~=') <relational-expression> }
	*/
	anode_t* left = parse_relational_expression(self);
	while (TOKEN == TK_NEQ || TOKEN == TK_EQ)
	{
		anode_t* root = ast_binary((TOKEN == TK_EQ ? AST_REL_EQ : AST_REL_NEQ) | AST_FLAG_OPERATOR, NULL, NULL, GET_SCOORD());
		NEXT();
		ast_add_child(root, left);
		ast_add_child(root, parse_relational_expression(self));
		left = root;
	}
	return left;
}

anode_t* parse_and_expression(parser_t* self)
{
	/*
	* <and-expression> = <equality-expression> { '&&' <equality-expression> }
	*/
	anode_t* left = parse_equality_expression(self);
	while (TOKEN == TK_AND)
	{
		anode_t* root = ast_binary(AST_LOG_AND | AST_FLAG_OPERATOR, NULL, NULL, GET_SCOORD());
		NEXT();
		ast_add_child(root, left);
		ast_add_child(root, parse_equality_expression(self));
		left = root;
	}
	return left;
}

anode_t* parse_or_expression(parser_t* self)
{
	/*
	* <or-expression> = <and-expression> { '||' <and-expression> }
	*/
	anode_t* left = parse_and_expression(self);
	while (TOKEN == TK_AND)
	{
		anode_t* root = ast_binary(AST_LOG_OR | AST_FLAG_OPERATOR, NULL, NULL, GET_SCOORD());
		NEXT();
		ast_add_child(root, left);
		ast_add_child(root, parse_and_expression(self));
		left = root;
	}
	return left;
}

anode_t* parse_expression(parser_t* self)
{
	anode_t* expr = parse_or_expression(self);

	return expr;
}

anode_t* parse_range(parser_t* self)
{
	/*
	* <range> = <or-expression> { ':' <or-expression> } ':' <or-expression>
	*/
	anode_t* expr = parse_or_expression(self);
	if (expr)
	{
		anode_t* range_increment = NULL;
		anode_t* range_end = NULL;
		while (TOKEN == TK_COLON)
		{
			NEXT();
			range_increment = range_end;
			range_end = parse_or_expression(self);
			if (range_increment == NULL && range_end == NULL)
				_error(self, L"Expected an expression.");
		}
		if (range_increment || range_end)
		{
			anode_t* range = ast_nary(AST_RANGE, GET_SCOORD());
			ast_add_child(range, expr);
			if (range_increment)
				ast_add_child(range, range_increment);
			if (range_end)
				ast_add_child(range, range_end);
			return ast_unary(AST_EXPRESSION, range, GET_SCOORD());
		}
		return ast_unary(AST_EXPRESSION, expr, GET_SCOORD());
	} 
	return expr;
}

static bool _is_lvalue_expr(anode_t* root)
{
	return	AST_GET(root->kind) == AST_ID			||
			AST_GET(root->kind) == AST_SUBSCRIPT	||
			AST_GET(root->kind) == AST_ARRAY;
}

static void _ad_id_to_symtab(parser_t* self, anode_t* id)
{
	if (!IS_ID(id))
		return;
	mc_string_t _id = id->value.id;
	sym_t* sym = SYM_LOOKUP(_id);
	if (!sym)
	{
		id->sym_flags = GET_SYM_SCOPE(self->scope) | SYM_VARIABLE;
		if (self->scope == SC_LOCAL)
		{
			id->frame = self->current_frame;
			id->frame_index = self->current_frame->frame_index;
		}
		symtab_add_sym_from_node(symtab, id);
	}
	else
	{
		// If the id already is in symtab, fill the required fields in this Id node as well.
		id->frame = sym->frame;
		if (id->frame)
			id->frame_index = sym->frame->frame_index;
		id->sym_flags = sym->flags;
		id->index = sym->tab_index;
	}
}

static void _add_lvalue_to_symtab(parser_t* self, anode_t* lvalue_expr)
{
	if (IS_EXPRESSION(lvalue_expr))
		lvalue_expr = ast_get_first(lvalue_expr);

	if(IS_ID(lvalue_expr))
		_ad_id_to_symtab(self, lvalue_expr);
	else if (IS_ARRAY(lvalue_expr))
	{
		anode_t* row = ast_get_first(lvalue_expr);
		for (size_t i = 0; i < row->childs->count; i++)
		{ 
			anode_t* expr = ast_get_child(row, i);
			if (IS_EXPRESSION(expr))
				expr = ast_get_first(expr);
			_ad_id_to_symtab(self, expr);
		}
	}
}

static void _check_synchronize_rvalue_id_with_symtab(parser_t* self, anode_t* id)
{
	if (!IS_ID(id))
		return;
	mc_string_t _id = id->value.id;
	sym_t* sym = SYM_LOOKUP(_id);
	if (sym)
	{
		// If the id already is in symtab, fill the required fields in this Id node as well.
		id->frame = sym->frame;
		if (id->frame)
			id->frame_index = sym->frame->frame_index;
		id->sym_flags = sym->flags;
		id->index = sym->tab_index;
	}
	else 
		__error(id, util_format(L"Unrecognized function or variable %s.", _id));
}

static void _check_synchronize_rvalue_with_symtab(parser_t* self, anode_t* rvalue_expr)
{
	if (IS_EXPRESSION(rvalue_expr))
		rvalue_expr = ast_get_first(rvalue_expr);

	if (IS_ID(rvalue_expr))
		_check_synchronize_rvalue_id_with_symtab(self, rvalue_expr);
	else if (IS_ARRAY(rvalue_expr))
	{
		anode_t* row = ast_get_first(rvalue_expr);
		for (size_t i = 0; i < row->childs->count; i++)
		{
			anode_t* expr = ast_get_child(row, i);
			if (IS_EXPRESSION(expr))
				expr = ast_get_first(expr);
			_check_synchronize_rvalue_id_with_symtab(self, expr);
		}
	}
	else if(AST_KIND(rvalue_expr) == AST_PARAM_LIST)
	{
		for (size_t i = 0; i < rvalue_expr->childs->count; i++)
		{
			anode_t* expr = ast_get_child(rvalue_expr, i);
			if(IS_EXPRESSION(expr))
				expr = ast_get_first(expr);
			_check_synchronize_rvalue_id_with_symtab(self, expr);
		}
	}
}

anode_t* parse_assignment_expression(parser_t* self)
{
	/*
	* <assginment-expression> = <range-expression> '=' <range-expression>
	*/
	bool previous_lhs_state = self->in_left_hand_side;
	self->in_left_hand_side = true;
	anode_t* expr = parse_range(self);
	self->in_left_hand_side = false;
	if (expr)
	{ 
		anode_t* lvalue = NULL; 
		if(IS_EXPRESSION(expr))
			lvalue = ast_get_first(expr);
		if (TOKEN == TK_ASSIGN)
		{ 
			if (_is_lvalue_expr(lvalue))
			{  
				// Ok we can treat this as an assginment.
				NEXT();
				anode_t* assignment = ast_binary(AST_ASSIGNMENT_EXPRESSION, NULL, NULL, GET_SCOORD());
				// Add lvalue expression. 
				ast_add_child(assignment, lvalue);

				if (IS_ARRAY(lvalue))
					sem_array_must_be_identifier(lvalue);
				else if (AST_KIND(lvalue) == AST_SUBSCRIPT)
					sem_subscript_must_be_2D(lvalue);

				// if lvalue is an id, add it to the sym table if it is not already there.
				_add_lvalue_to_symtab(self, lvalue);

				anode_t* rvalue = parse_range(self);
				if (IS_EXPRESSION(rvalue))
					rvalue = ast_get_first(rvalue);
				
				_check_synchronize_rvalue_with_symtab(self, rvalue);

				ast_add_child(assignment, rvalue);
				return assignment; 
			}
			else
				_error(self, L"Only a variable, a subscript expression and function return array can be in the left-hand side.");
		} 
		else
		{
			// We didn't find an assign symbol, treat this expression as rvalue.
			_check_synchronize_rvalue_with_symtab(self, lvalue);
		}
	}
	return expr;
}
 
anode_t* parse_selection_expression(parser_t* self)
{
	/*
	* <selection-expression> = "(" <range-expression> ")"
	*/
	bool has_open_paren = false;
	if (TOKEN == TK_OPPAR)
	{
		NEXT();
		has_open_paren = true;
	}
	anode_t* select_expr = parse_range(self); 
	if (has_open_paren)
		EXPECT(TK_CLPAR);
	return select_expr;
}

anode_t* parse_if_statement(parser_t* self)
{
	/*
	* <if-statement> = 'if' ['('] <range-expression> [')'] <statement-list> { 'elseif' <statement-list> } [ 'else' <statement-list> ] 'end'
	*/
	if (TOKEN == TK_IF)
	{
		NEXT(); 

		anode_t* if_statement = ast_nary(AST_IF_STATEMENT, GET_SCOORD());

		anode_t* if_body = ast_binary(AST_IF, NULL, NULL, GET_SCOORD());

		anode_t* select_expr = parse_selection_expression(self);

		if (!select_expr)
			_error(self, L"Expected an expression that evaluates to true or false.");

		ast_add_child(if_body, select_expr);

		anode_t* statement_list = parse_statement_list(self);
		if (statement_list->childs->count > 0)
			ast_add_child(if_body, statement_list);

		ast_add_child(if_statement, if_body);

		while (TOKEN == TK_ELSEIF)
		{
			NEXT();
			anode_t* elseif_body = ast_binary(AST_ELSEIF, NULL, NULL, GET_SCOORD());

			select_expr = parse_selection_expression(self);

			if (!select_expr)
				_error(self, L"Expected an expression that evaluates to true or false.");

			ast_add_child(elseif_body, select_expr);

			anode_t* statement_list = parse_statement_list(self);
			if (statement_list->childs->count > 0)
				ast_add_child(elseif_body, statement_list);

			ast_add_child(if_statement, elseif_body);
		}
		if (TOKEN == TK_ELSE)
		{
			NEXT();
			anode_t* else_body = ast_unary(AST_ELSE, NULL, GET_SCOORD());
			anode_t* statement_list = parse_statement_list(self);
			if (statement_list->childs->count > 0)
				ast_add_child(else_body, statement_list);

			ast_add_child(if_statement, else_body);
		}
		EXPECT(TK_END); 
		return ast_unary(AST_STATEMENT, if_statement, GET_SCOORD());
	}
	return NULL;
}

anode_t* parse_jump_statement(parser_t* self)
{
	/*
	* <jump-statement> = 'break' | 'continue' | 'return'
	*/ 
	anode_t* jump_statement = NULL;
	if (TOKEN == TK_BREAK || TOKEN == TK_CONTINUE || TOKEN == TK_RETURN)
	{ 
		if (TOKEN == TK_BREAK)
		{
			jump_statement = ast_leaf0(AST_BREAK_STATEMENT, GET_SCOORD()); 
			NEXT();
		}
		else if (TOKEN == TK_CONTINUE)
		{
			jump_statement = ast_leaf0(AST_CONTINUE_STATEMENT, GET_SCOORD());
			NEXT();
		}
		else if (TOKEN == TK_RETURN)
		{
			jump_statement = ast_leaf0(AST_RETURN, GET_SCOORD());
			NEXT();
		}
		return ast_unary(AST_STATEMENT, jump_statement, GET_SCOORD());
	}
	return jump_statement;
}

anode_t* parse_for_statement(parser_t* self)
{
	/*
	* <for-statement> = 'for'  ['('] 'ID' '=' <range-expression> [')']  <statement-list> 'end'
	*/
	if (TOKEN == TK_FOR)
	{
		NEXT();
		anode_t* for_statement = ast_nary(AST_FOR_STATEMENT, GET_SCOORD());

		bool has_open_paren = false;
		if (TOKEN == TK_OPPAR)
		{
			NEXT();
			has_open_paren = true;
		}
		anode_t* id = parse_primary_expression(self);

		if (!IS_ID(id))
			_error(self, L"Expected an identifier.");

		sym_t* sym = SYM_LOOKUP(id->value.id);
		if (!sym)
		{
			id->sym_flags = GET_SYM_SCOPE(self->scope) | SYM_VARIABLE;
			if (self->scope == SC_LOCAL)
			{
				id->frame = self->current_frame;
				id->frame_index = self->current_frame->frame_index;
			}
			symtab_add_sym_from_node(symtab, id);
		}
		ast_add_child(for_statement, id);

		EXPECT(TK_ASSIGN); 

		anode_t* range_expr = parse_range(self);

		ast_add_child(for_statement, range_expr);

		if (has_open_paren)
			EXPECT(TK_CLPAR); 

		anode_t* statement_list = parse_statement_list(self);
		if (statement_list->childs->count > 0)
			ast_add_child(for_statement, statement_list);

		EXPECT(TK_END);
		return ast_unary(AST_STATEMENT, for_statement, GET_SCOORD());
	}
	return NULL;
}

anode_t* parse_while_statement(parser_t* self)
{
	/*
	* <while-statement> = 'while'  ['('] <range-expression> [')']  <statement-list> 'end'
	*/
	if (TOKEN == TK_WHILE)
	{
		NEXT();
		anode_t* while_statement = ast_nary(AST_WHILE_STATEMENT, GET_SCOORD());
		  
		anode_t* select_expr = parse_selection_expression(self);

		if (!select_expr)
			_error(self, L"Expected an expression that evaluates to true or false.");

		ast_add_child(while_statement, select_expr);

		anode_t* statement_list = parse_statement_list(self);
		if (statement_list->childs->count > 0)
			ast_add_child(while_statement, statement_list);
		EXPECT(TK_END);
		return ast_unary(AST_STATEMENT, while_statement, GET_SCOORD());
	}
	return NULL;
}

anode_t* parse_switch_statement(parser_t* self)
{
	/*
	* <switch-statement> = 'switch' ['('] <selection-expr> [')'] { 'case' <selection-expr> <statement-list> } [ 'otherwise' <statement-list> ] 'end'
	*/
	if (TOKEN == TK_SWITCH)
	{
		NEXT();
		anode_t* switch_statement = ast_nary(AST_SWITCH_STATEMENT, GET_SCOORD());
		
		anode_t* select_expr = parse_selection_expression(self);

		if (!select_expr)
			_error(self, L"Expected an expression that evaluates to true or false.");

		ast_add_child(switch_statement, select_expr);

		/*
		* This is a special case for switch statements: A newline character can be found between
		* switch and case keywords and, since no parse_statement_list call is done, these newline
		* character cannot be skipped. Skip it manually here.
		*/
		while (TOKEN == TK_NEWLINE)
			NEXT();

		while (TOKEN == TK_CASE)
		{
			NEXT(); 

			anode_t* case_body = ast_binary(AST_CASE, NULL, NULL, GET_SCOORD());

			select_expr = parse_selection_expression(self);

			if (!select_expr)
				_error(self, L"Expected a case expression.");

			ast_add_child(case_body, select_expr);

			anode_t* statement_list = parse_statement_list(self);
			if (statement_list->childs->count > 0)
				ast_add_child(case_body, statement_list);

			ast_add_child(switch_statement, case_body);
		}
		if (TOKEN == TK_OTHERWISE)
		{
			NEXT();
			anode_t* otherwise_body = ast_unary(AST_OTHERWISE, NULL, GET_SCOORD());
			anode_t* statement_list = parse_statement_list(self);
			if (statement_list->childs->count > 0)
				ast_add_child(otherwise_body, statement_list);

			ast_add_child(switch_statement, otherwise_body);
		}
		EXPECT(TK_END);
		return ast_unary(AST_STATEMENT, switch_statement, GET_SCOORD());
	}
	return NULL;
}

anode_t* parse_function_declaration(parser_t* self)
{
	/*
	* <function-declaration> = 'function' [ <primary-expression> '=' ] 'ID' '(' <param-list> ')' <statement-list> 'end'
	*/
	if (TOKEN == TK_FUNC)
	{
		NEXT();
		LOCAL_BEGIN();
		anode_t* function = ast_nary(AST_FUNCTION_DECLARATION, GET_SCOORD());

		anode_t* primary = parse_primary_expression(self);
		 
		if (!primary || (!IS_ID(primary) && !IS_ARRAY(primary)))
			_error(self, L"Expected a return array or an identifier.");

		anode_t* function_name = primary;
		anode_t* return_expr = NULL; 

		if (TOKEN == TK_ASSIGN)
		{
			NEXT(); 
			primary = parse_primary_expression(self);
			if (!IS_ID(primary))
				_error(self, L"Expected a function name."); 

			// We found an ID after '=', this is function name. So, accept the previous expr
			// as return expression.
			return_expr = function_name;
			function_name = primary; 

			ast_add_child(function, return_expr);
			ast_add_child(function, function_name);
		}
		else
		{
			// Expected a function name after '='.
			if(!IS_ID(primary))
				_error(self, L"Expected an identifier.");
		}

		// If there is already a function that has the same name as this, issue an error.
		if (SYM_LOOKUP(function_name->value.id))
			_error(self, util_format(L"There is already function declaration named %s.", function_name->value.id));

		// If we didn't found a return expr, then add the function name.
		if(!return_expr)
			ast_add_child(function, function_name);
		 
		sym_t* func_sym;
		sym_t* sym;
		// Add function name to the symbol table.
		function_name->sym_flags = GET_SYM_SCOPE(prev_scope) | SYM_FUNCTION;
		func_sym = symtab_add_sym_from_node(symtab, function_name);
		self->current_frame = function_name->frame;
		EXPECT(TK_OPPAR);

		anode_t* param_list = parse_param_list(self);
		ast_add_child(function, param_list);
		func_sym->inargc = function_name->inargc = param_list->childs->count;
		for (size_t i = 0; i < param_list->childs->count; i++)
		{
			anode_t* param_expr = ast_get_child(param_list, i);
			 
			if (IS_EXPRESSION(param_expr))
			{
				anode_t* child_expr = ast_get_first(param_expr);
				if (!IS_ID(child_expr))
					_error(self, L"Elements of parameter list must be identifier.");
				child_expr->sym_flags = SYM_LOCAL | SYM_VARIABLE;
				child_expr->frame = self->current_frame;
				child_expr->frame_index = self->current_frame->frame_index;
				sym = symtab_add_sym_from_node(symtab, child_expr);

			} 
		}

		EXPECT(TK_CLPAR);

		// Add the names in the function return expression to the symbol table, if we have one.
		if (return_expr)
		{
			// If return expression is just a single identifier.
			if (IS_ID(return_expr))
			{
				return_expr->sym_flags = SYM_LOCAL | SYM_VARIABLE;
				return_expr->frame = self->current_frame;
				return_expr->frame_index = self->current_frame->frame_index;
				sym = symtab_add_sym_from_node(symtab, return_expr);
				func_sym->outargc = function_name->outargc = 1;
			}
			else if (IS_ARRAY(return_expr))
			{
				
				for (size_t i = 0; i < return_expr->childs->count; i++)
				{
					anode_t* row = ast_get_child(return_expr, i); 
					func_sym->outargc = function_name->outargc = row->childs->count;
					for (size_t j = 0; j < row->childs->count; j++)
					{
						anode_t* row_element = ast_get_child(row, j);
						if (IS_EXPRESSION(row_element))
						{
							anode_t* child_expr = ast_get_first(row_element);
							if (!IS_ID(child_expr))
								_error(self, L"Elements of return array must be identifier.");
							child_expr->sym_flags = SYM_LOCAL | SYM_VARIABLE;
							child_expr->frame = self->current_frame;
							child_expr->frame_index = self->current_frame->frame_index;
							sym = symtab_add_sym_from_node(symtab, child_expr);

						}
					}
				}
				
			}
		}
		
		anode_t* statement_list = parse_statement_list(self);
		if (statement_list->childs->count > 0)
			ast_add_child(function, statement_list);
		
		EXPECT(TK_END);
		LOCAL_END();
		return ast_unary(AST_STATEMENT, function, GET_SCOORD());
	}
	return NULL;
}

anode_t* parse_load_statement(parser_t* self)
{
	if (TOKEN == TK_LOAD)
	{
		NEXT();
		if (TOKEN != TK_STRING)
			_error(self, L"Expected file to load.");
		mc_string_t file_path = token_data.string;
		push_lexer(self, lex_from_file(file_path));
		NEXT();
		// Start from the beginning for the new module.
		parse_program(self);
	}
}

enum expression_terminator parse_expression_terminator(parser_t* self)
{
	/*
	* <expression-terminator> = (';' | ',' | '\n' | 'EOF')
	*/
	enum expression_terminator terminator = ET_NULL;
	if (TOKEN == TK_SEMICOLON)
	{
		NEXT();
		terminator = ET_STATEMENT;
	}
	else if (TOKEN == TK_NEWLINE || TOKEN == TK_COMMA || TOKEN == TK_EOF)
	{
		NEXT();
		terminator = ET_EXPRESSION;
	}
	return terminator;
}

anode_t* parse_statement_expression(parser_t* self)
{ 
	/*
	* <statement-expression> = 
	*						 ( <assignment-expression>
	*						 | <if-statement>
	*						 | <switch-statement>
	*						 | <while-statement>
	*						 | <for-statement>
	*						 | <function-declaration>
	*						 | <jump-declaration> ) [ <expression-terminator> ]
	*/
	typedef anode_t* (*parse_func_t)(parser_t* self);

	parse_func_t statement_parsers[] = { 
		parse_assignment_expression, 
		parse_if_statement, 
		parse_switch_statement, 
		parse_while_statement,
		parse_for_statement, 
		parse_jump_statement,  
		parse_function_declaration, };

	anode_t* statement_expr = parse_assignment_expression(self);
	if (statement_expr)
	{
		if (AST_GET(statement_expr->kind) == AST_EXPRESSION || 
			AST_GET(statement_expr->kind) == AST_ASSIGNMENT_EXPRESSION)
		{
			enum expression_terminator terminator = parse_expression_terminator(self);
			
			if (terminator == ET_STATEMENT)
				return ast_unary(AST_STATEMENT, statement_expr, GET_SCOORD());
			else if (terminator == ET_EXPRESSION)
				return ast_unary(AST_EXPRESSION_STATEMENT, statement_expr, GET_SCOORD());
		}
	}
	for (size_t i = 1; i < sizeof(statement_parsers) / sizeof(statement_parsers[0]); i++)
	{
		statement_expr = statement_parsers[i](self);  
		if (statement_expr)
			return statement_expr;
	} 
	if (TOKEN == TK_NEWLINE)
	{
		// If we found nothing but a newline then treat this as NULL_STATEMENT.
		NEXT();
		return ast_leaf0(AST_NULL_STATEMENT, GET_SCOORD());
	}
	return NULL;
}

anode_t* parse_statement_list(parser_t* self)
{
	/*
	*	<statement-list> = { <statement-expression> }
	*/
	anode_t* statement_list = ast_nary(AST_STATEMENT_LIST, GET_SCOORD());

	while (true)
	{	
		anode_t* statement_expr = parse_statement_expression(self);
		if (!statement_expr)
			break;
		if (statement_expr->kind != AST_NULL_STATEMENT)
			ast_add_child(statement_list, statement_expr);
		if (TOKEN == TK_EOF)
			break;
	} 

	
	return statement_list;
} 

static void _expect(parser_t* self, int token)
{
	if (self->token != token) 
		_error(self, util_format(L"Expected %s but found %s.", tok_token_to_str(token), tok_token_to_str(self->token)));  
	_next_token(self);
}

static int _next_token(parser_t* self)
{
	self->token = lex_next_token(self->act_lexer);
	return self->token; 
}

static void _error(parser_t* self, const wchar_t* message)
{
	fflush(stderr); 
	fwprintf(stderr, L"%s(%d, %d): Error: %s\n", self->act_lexer->fd->file_path, self->act_lexer->line_no, self->act_lexer->column_no,  message);
	exit(MC_ERROR_FAILURE);
}
static void __error(anode_t* node, const wchar_t* message)
{
	fflush(stderr);
	fwprintf(stderr, L"%s(%d, %d): Error: %s\n", node->scoord.file, node->scoord.line_no, node->scoord.column_no, message);
	exit(MC_ERROR_FAILURE);
}