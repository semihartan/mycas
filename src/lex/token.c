#include <lex/token.h>
#include <util.h>

const mc_string_t tok_token_to_str(int _token_kind)
{
#define xx(a, b, c) \
case a:\
	return b;
#define yy(a, b, c) \
case a:\
	return b;

#define zz(a, b, c, d) \
case a:\
	return util_format(L"%s: " d, b, token_data.c);

	switch (_token_kind)
	{ 

#include <lex/tokens.h>
	default:
		return L"Unknown Token.";
	}

#undef xx
#undef yy

}