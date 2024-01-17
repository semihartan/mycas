#pragma once

#ifndef PARSER_H
#define PARSER_H

#include <vm/frame.h>
#include <parse/ast.h>
#include <lex/scoord.h>
#include <lex/lexer.h>
#include <io/file.h>

#include <adt/vector.void.h>
 
enum scope
{
	SC_LOCAL,
	SC_GLOBAL,
};


typedef struct
{
	lexer_t* act_lexer;
	vector__t* lex_stack;
	enum token_kind token;
	enum scope scope;
	mc_frame_t* current_frame; 
	uint8_t in_left_hand_side : 1;
}parser_t;

parser_t* parser_open_from_file(wchar_t* file_path);
parser_t* parser_open_from_fd(file_t* fd);

anode_t* parse_program(parser_t* self);
anode_t* parse_module(parser_t* self, anode_t* program);

anode_t* parse_array_row(parser_t* self); 
anode_t* parse_array_matrix(parser_t* self); 
anode_t* parse_param_list(parser_t* self);
anode_t* parse_primary_expression(parser_t* self);
anode_t* parse_postfix_expression(parser_t* self);
anode_t* parse_unary_expression(parser_t* self);
anode_t* parse_power_expression(parser_t* self);
anode_t* parse_multiplicative_expression(parser_t* self);
anode_t* parse_additive_expression(parser_t* self);
anode_t* parse_relational_expression(parser_t* self);
anode_t* parse_equality_expression(parser_t* self);
anode_t* parse_and_expression(parser_t* self);
anode_t* parse_or_expression(parser_t* self); 

anode_t* parse_expression(parser_t* self);
anode_t* parse_range(parser_t* self);
anode_t* parse_assignment_expression(parser_t* self);

anode_t* parse_if_statement(parser_t* self);
anode_t* parse_jump_statement(parser_t* self);
anode_t* parse_for_statement(parser_t* self);
anode_t* parse_while_statement(parser_t* self);
anode_t* parse_switch_statement(parser_t* self);
anode_t* parse_function_declaration(parser_t* self);

anode_t* parse_statement_expression(parser_t* self);


anode_t* parse_load_statement(parser_t* self);
anode_t* parse_statement(parser_t* self);

anode_t* parse_statement_list(parser_t* self);


#endif