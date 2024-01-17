#pragma once

#ifndef TOKEN_H
#define TOKEN_H

#include <common.h>
#include <mcstr.h>

/*
* Supplementary data for the specific tokens, such as ID, NUMBER, STRING
*/
typedef union
{
	mc_string_t identifier;
	mc_string_t string;
	double number;
}token_data_t;

extern token_data_t token_data;

enum token_kind
{ 

	#define xx(a, b, c)  a,
	#define yy(a, b, c)  a,
	#define zz(a, b, c, d)  a,
	
	#include <lex/tokens.h>

	#undef zz
	#undef yy
	#undef xx
};

const mc_string_t tok_token_to_str(int token_kind);

#endif