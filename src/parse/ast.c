#include <parse/ast.h> 

#include <mcdef.h>
#include <error.h>
#include <util.h>

#include <stdio.h>

#include <gc/gc.h>

static int comparer_ast(ast_node_t* left, ast_node_t* right)
{
	return 0;
}

static ast_node_t* ast_node_alloc(enum ast_node_kind kind, size_t capacity, scoord_t scoord)
{
	ast_node_t* node = NEWOBJP(node);
	MC_ERROR_OUT_OF_MEMORY(node);
	node->kind = kind;
	node->childs = vector__alloc(comparer_ast, _capacity = capacity);
	node->sign = 1;
	node->scoord = scoord;
	return node;
}
ast_node_t* ast_leaf0(enum ast_node_kind kind, scoord_t scoord)
{
	ast_node_t* node = ast_node_alloc(kind, 0, scoord);
	node->kind |= AST_FLAG_NULLARY; 
	return node;
}
ast_node_t* ast_leaf1(enum ast_node_kind kind, node_value_t value, scoord_t scoord)
{
	ast_node_t* node = ast_node_alloc(kind, 0, scoord);
	node->kind |= AST_FLAG_NULLARY;
	node->value = value;
	return node;
}

ast_node_t* ast_unary(enum ast_node_kind kind, ast_node_t* child, scoord_t scoord)
{
	ast_node_t* node = ast_node_alloc(kind, 1, scoord);
	node->kind |= AST_FLAG_UNARY;
	if(child)
		ast_add_child(node, child);
	return node;
}
ast_node_t* ast_binary(enum ast_node_kind kind, ast_node_t* left, ast_node_t* right, scoord_t scoord)
{
	ast_node_t* node = ast_node_alloc(kind, 2, scoord);
	node->kind |= AST_FLAG_BINARY;
	if (left)
		ast_add_child(node, left);
	if (right)
		ast_add_child(node, right);
	return node;
} 
 
MC_INLINE ast_node_t* ast_nary(enum ast_node_kind kind, scoord_t scoord)
{
	anode_t* node = ast_node_alloc(kind, DEFAULT_CAPACITY_SZ, scoord);
	node->kind |= AST_FLAG_NARY;
	return node;
}

ast_node_t* ast_add_child(ast_node_t* root, ast_node_t* child)
{
	if (child == NULL)
		return NULL;
	return vector__push_back(root->childs, child);
}

ast_node_t* ast_get_child(ast_node_t* root, size_t index)
{ 
	return vector__get(root->childs, index);
}

ast_node_t* ast_get_first(ast_node_t* root)
{
	return vector__get(root->childs, 0);
}
ast_node_t* ast_get_second(ast_node_t* root)
{
	return vector__get(root->childs, 1);
}
ast_node_t* ast_get_third(ast_node_t* root)
{
	return vector__get(root->childs, 2);
}
ast_node_t* ast_get_last(ast_node_t* root)
{
	return vector__get(root->childs, root->childs->count - 1);
}
static void _print_tree(ast_node_t* root, int level)
{
	for (int i = 0; i < level * 2; i++)
		putwchar(L' ');

	wprintf(L"%s>\n", ast_node_to_str(root));

	for (size_t i = 0; i < root->childs->count; i++)
	{
		_print_tree(ast_get_child(root, i), level + 1);
	}
}
void ast_print_tree(ast_node_t* root)
{
	_print_tree(root, 0);
}
mc_string_t ast_node_to_str(ast_node_t* root)
{
#define xx(a, b)												\
case a:															\
	return util_format(L"(%s)", L###a);

#define yy(a, b, c, d)											\
case a:															\
	return util_format(L"(%s, " d L")", L###a, root->value.c);

#define zz(a, b, c)												\
case a:															\
	return util_format(L"(%s)", L###a);

#define tt(a, b, c, d)											\
case a:															\
	return util_format(L"(%s)", L###a);

#define uu(a, b, c)												\
case a:															\
	return util_format(L"(%s)", L###a);

	switch (AST_GET(root->kind))
	{ 

		#include <parse/anodes.h>	

		default:
			return mc_str_init(L"Unknown Node.");
	} 

#undef uu
#undef tt
#undef zz
#undef yy
#undef xx
}