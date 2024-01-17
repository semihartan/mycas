#include <parse/parser.h>
#include <parse/lstack.h>

#include <mcdef.h>
#include <gc/gc.h>

static int comparer_lexer(lexer_t* left, lexer_t* right)
{
	return left == right;
}

static parser_t* parser_alloc()
{
	parser_t* self = NEWOBJP(self);
	MC_ERROR_OUT_OF_MEMORY(self);
	self->lex_stack = vector__alloc(comparer_lexer, false);
	self->scope = SC_GLOBAL;
	return self;
}

parser_t* parser_open_from_file(wchar_t* file_path)
{
	parser_t* self = parser_alloc();
	self->act_lexer = lex_from_file(file_path);
	push_lexer(self, self->act_lexer);
	return self;
}
parser_t* parser_open_from_fd(file_t* fd)
{
	parser_t* self = parser_alloc(); 
	push_lexer(self, lex_from_fd(fd));
	return self;
}
