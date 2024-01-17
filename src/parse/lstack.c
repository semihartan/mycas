#include <parse/lstack.h>


lexer_t* push_lexer(parser_t* self, lexer_t* lexer)
{
	self->act_lexer = lexer;
	self->token = lex_next_token(self->act_lexer);
	return vector__push_back(self->lex_stack, lexer);
}

lexer_t* pop_lexer(parser_t* self)
{
	self->act_lexer = vector__get(self->lex_stack, self->lex_stack->count - 2);
	return vector__pop_back(self->lex_stack);
}