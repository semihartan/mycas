#pragma once

#ifndef AST_H
#define AST_H

#include <adt/vector.void.h>
#include <mcstr.h>

#include <vm/sym.h>
#include <vm/frame.h>
#include <lex/scoord.h>

#define AST_KIND_MASK		0x000000FF
#define AST_TYPE_MASK		0xFF000000

// Begin: Deprecated  
#define AST_GET_TYPE(x)		((x) & AST_TYPE_MASK)
#define AST_GET(x)			((x) & AST_KIND_MASK)
// End: Depreated

#define AST_KIND(x)			((x->kind) & AST_KIND_MASK)
#define AST_TYPE(x)			((x->kind) & AST_TYPE_MASK)

#define HAS_NULLARY(x)		((AST_TYPE(x) & AST_FLAG_NULLARY	)	!= 0)
#define HAS_UNARY(x)		((AST_TYPE(x) & AST_FLAG_UNARY		)	!= 0)
#define HAS_BINARY(x)		((AST_TYPE(x) & AST_FLAG_BINARY		)	!= 0)
#define HAS_NARY(x)			((AST_TYPE(x) & AST_FLAG_NARY		)	!= 0)
#define HAS_OPERATOR(x)		((AST_TYPE(x) & AST_FLAG_OPERATOR	)	!= 0)

#define HAS_UNARY_OP(x)		(HAS_UNARY(x)	&& HAS_OPERATOR(x))
#define HAS_BINARY_OP(x)	(HAS_BINARY(x)	&& HAS_OPERATOR(x)) 

#define xx(a, b)			a,
#define yy(a, b, c, d)		a,
#define zz(a, b, c)			a,
#define tt(a, b, c, d)		a,
#define uu(a, b, c)			a,

enum ast_node_kind
{
	#include <parse/anodes.h>


	AST_FLAG_NULLARY	= 0x00000000,
	AST_FLAG_UNARY		= 0x01000000,
	AST_FLAG_BINARY		= 0x02000000,
	AST_FLAG_NARY		= 0x04000000,
	AST_FLAG_OPERATOR	= 0x08000000,
}; 

#undef uu
#undef tt
#undef zz
#undef yy
#undef xx



typedef union
{
	double num;
	mc_string_t id;
	mc_string_t string;
}node_value_t;

typedef struct
{
	enum ast_node_kind kind;
	vector__t* childs;
	node_value_t value;
	int sign;
	size_t index;
	size_t frame_index;
	mc_string_t strobj_key;
	mc_frame_t* frame;
	enum sym_flag sym_flags;
	scoord_t scoord;
	int inargc;
	int outargc;
}ast_node_t;

typedef ast_node_t anode_t;

ast_node_t* ast_leaf0(enum ast_node_kind kind, scoord_t scoord);
ast_node_t* ast_leaf1(enum ast_node_kind kind, node_value_t value, scoord_t scoord);
ast_node_t* ast_unary(enum ast_node_kind kind, ast_node_t* child, scoord_t scoord);
ast_node_t* ast_binary(enum ast_node_kind kind, ast_node_t* left, ast_node_t* right, scoord_t scoord);
MC_INLINE ast_node_t* ast_nary(enum ast_node_kind kind, scoord_t scoord);
ast_node_t* ast_add_child(ast_node_t* root, ast_node_t* child);
ast_node_t* ast_get_child(ast_node_t* root, size_t index);
ast_node_t* ast_get_first(ast_node_t* root);
ast_node_t* ast_get_second(ast_node_t* root);
ast_node_t* ast_get_third(ast_node_t* root);
ast_node_t* ast_get_last(ast_node_t* root);
mc_string_t ast_node_to_str(ast_node_t* root);
void ast_print_tree(ast_node_t* root);

#endif