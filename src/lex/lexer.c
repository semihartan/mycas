#include <lex/lexer.h> 
#include <lex/token.h>

#include <error.h>
#include <util.h> 

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <wctype.h>

#include <gc/gc.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
  
extern token_data_t token_data = { 0 };
 

#define MAXIMUM_TOK_BUF_SZ 0x10000

static wchar_t _token_buf[MAXIMUM_TOK_BUF_SZ + 1];  
static wchar_t* _tp;
 
static int _next_block(lexer_t* self);
static wchar_t _next_char(lexer_t* self);
static void _save_state(lexer_t* self, size_t max_advance);
static void _restore_state(lexer_t* self);
static void _lex_error(lexer_t* self, const wchar_t* _message);
static void _lex_update_scoord(lexer_t* self);
static bool _lex_match_keyword(lexer_t* self, wchar_t* _keyword, size_t _kw_len);
static bool _lex_match_number(lexer_t* self);
static bool _match_string(lexer_t* self);


static struct keyword_pair
{
	const wchar_t* keyword;
	size_t kw_len;
	int token;
}
_keywords[] = {
	{ L"break", 5, TK_BREAK},
	{ L"case", 4, TK_CASE},
	{ L"continue", 8, TK_CONTINUE},
	{ L"elseif", 6, TK_ELSEIF},
	{ L"else", 4, TK_ELSE},  
	{ L"end", 3, TK_END},  
	{ L"false", 5, TK_FALSE},  
	{ L"for", 3, TK_FOR},  
	{ L"function", 8, TK_FUNC},  
	{ L"if", 2, TK_IF},  
	{ L"load", 2, TK_LOAD},  
	{ L"otherwise", 9, TK_OTHERWISE},  
	{ L"return", 6, TK_RETURN},  
	{ L"switch", 6, TK_SWITCH},  
	{ L"while", 5, TK_WHILE},  
	{ L"true", 4, TK_TRUE},  
};

lexer_t* lex_from_file(const wchar_t* _file_path)
{
	file_t* fd = io_open_file(_file_path);

	MC_ERROR_RETVAL(fd != NULL, L"", NULL);

	return lex_from_fd(fd);
}

lexer_t* lex_from_fd(const file_t* _fd)
{
	lexer_t* self = GC_MALLOC(sizeof(*self));

	MC_ERROR_RETVAL(self != NULL, MC_MESSAGE_NOMEM, NULL);

	self->fd = _fd;
	self->line_no = 1;
	self->column_no = 1;
	_tp = _token_buf;
	_next_block(self);
	//_next_char(self);
	return self;
}


lexer_t* lex_from_cstr(const wchar_t* _str)
{
	//HANDLE hFile = CreateFil 
}

#define CC self->cp[0]
#define CP self->cp 
#define NC() _next_char(self)

int lex_next_token(lexer_t* self)
{
	// Reset token buffer.
	_tp = _token_buf;
	// If the next character is a NULL, this means that all the input is consumed.
	if (CC == L'\0')
	{ 
		return TK_EOF;
	}

	// Skip the white spaces.
	while (iswblank(CC))
		NC();

	// Recognize ellipsis characters.
	_save_state(self, 4);
	if (NC() == L'.' && NC() == L'.')
	{
		if (!(NC() == L'.' && NC() == L'\n'))
			_lex_error(self, L"Bad line continuation ellipsis."); 
		while (iswblank(CC))
			NC();
	}
	else 
		_restore_state(self);

	
	// Ignore comments.
	_save_state(self, 2);
	if (NC() == L'/') 
	{
		bool comment_closed = false;
		if (CC == L'/')
		{
			NC();
			while (CC != L'\n')
				NC();
		}
		else if (CC == L'*')
		{
			NC();
			while ((CC != L'\0'))
			{

				if (CC == L'*')
				{
					NC();
					if (NC() == L'/')
					{
						comment_closed = true;
						break;
					}
				}
				NC();
			}
			if (!comment_closed && CC == L'\0')
				_lex_error(self, L"Comment unclosed at end of file.");
			else if(CC == L'\0')
				return TK_EOF;
		}
		else
			_restore_state(self);
	}
	else
		_restore_state(self);

	// We use longest matching rule to recognize tokens. That means that if the lexer recognizes 
	// two tokens that start with the same char sequence, it will choose the longest one as the 
	// next token.
	// To implement this rule, we sort the recognizing branches by the length of the tokens they 
	// will recognize.  

	// First we scan simple tokens such as operators, punctuations, etc..
	_lex_update_scoord(self);
	switch (CC)
	{
	case L'=':
		NC();
		if (CC == L'=')
			return NC(), TK_EQ;
		return TK_ASSIGN;
	case L'~':
		NC();
		if (CC == L'=')
			return NC(), TK_NEQ;
		return TK_NOT;
	case L'>':
		NC();
		if (CC == L'=')
			return NC(), TK_GTEQ;
		return TK_GT;
	case L'<':
		NC();
		if (CC == L'=')
			return NC(), TK_LTEQ;
		return TK_LT;
	case L']':
		NC(); 
		return TK_CLBRACKET;
	case  L'\'':
		return NC(), TK_TRANS;
#define xx(a, b, c) 
#define zz(a, b, c, d) 

#define yy(a, b, c) \
case c:\
	return NC(), a;

#include <lex/tokens.h>

#undef zz
#undef yy
#undef xx

	case L'.':
		NC();
		if (CC == L'*')
			return NC(), TK_DOTMUL;
		else if (CC == L'/')
			return NC(), TK_DOTDIV;
		else if (CC == L'^')
			return NC(), TK_DOTPOW;
		return TK_DOT;  
	default: 
		break;
	}
	
	for (size_t i = 0; i < sizeof(_keywords) / sizeof(_keywords[0]); i++)
	{
		wchar_t* keyword = _keywords[i].keyword;
		if (_lex_match_keyword(self,  keyword, _keywords[i].kw_len))
			return _keywords[i].token;
	} 
	if (iswalpha(CC) || CC == L'_')
	{
		_lex_update_scoord(self);
		*_tp++ = NC();
		while (iswalnum(CC) || CC == L'_')
		{
			*_tp++ = NC();
		}
		*_tp = L'\0';
		token_data.identifier = mc_str_init(_token_buf);
		_tp = _token_buf;
		return TK_ID;
	} 
	else if (_lex_match_number(self))
	{ 
		token_data.number = wcstod(_token_buf, NULL);
		return TK_NUMBER;
	}
	else if (_match_string(self))
	{
		token_data.string = mc_str_init(_token_buf);
		return TK_STRING;
	}
	_lex_error(self, L"Failed to recognize the character.");
}
 
int lex_peek_token(lexer_t* self, int k)
{ 
	int token_kind = 0;
	_save_state(self, SIZE_MAX);
	for (size_t i = 0; i < k; i++)
	{
		token_kind = lex_next_token(self);
	}
	_restore_state(self); 
	return token_kind;
}

static void _lex_error(lexer_t* _lexer, const wchar_t *_message)
{
	const wchar_t* message = util_format(L"%s(%d, %d): %s", _lexer->fd->file_base_name, _lexer->line_no, _lexer->column_no, _message);
	MC_ERROR_ABORT(0, message, MC_ERROR_FAILURE);
}

static int _next_block(lexer_t* _lexer)
{
	size_t chs_read = 0;
	int result = io_read_block(_lexer->fd, _lexer->io_block, &chs_read);
	if (result == EOF)
		return EOF;
	_lexer->cp = _lexer->io_block;
	_lexer->ep = _lexer->cp + chs_read;
	return MC_ERROR_SUCCESS;
}

static wchar_t _next_char(lexer_t* self)
{
	if (CC == L'\0')
	{
		if (_next_block(self) == EOF)
			return L'\0';
	}
	else if (CC == L'\n')
	{
		++self->line_no;
		self->column_no = 0;
	}
	return ++self->column_no, *(self->cp)++;
}

static void _save_state(lexer_t* self, size_t max_advance)
{
	io_save_file_state(self->fd);
	self->state.line_no = self->line_no;
	self->state.column_no = self->column_no;
	self->state.cp = self->cp;
	if (max_advance > (self->cp - self->ep))
	{
		wcscpy(self->state.io_block, self->io_block);
		self->state.restore_block = true;
	}
}

static void _restore_state(lexer_t* self)
{
	io_restore_file_state(self->fd);
	self->line_no = self->state.line_no;
	self->column_no = self->state.column_no;
	self->cp = self->state.cp;
	if(self->state.restore_block)
		wcscpy(self->io_block, self->state.io_block);
}

static bool _lex_match_number(lexer_t* self)
{
	_lex_update_scoord(self);
	if (!iswdigit(CC))
		return false;
	enum state
	{
		START,
		DIGIT0,
		DOT,
		DIGIT1,
		E,
		DIGIT2,
	}; 
	enum state state = START;
	enum state final_set[] = { DIGIT0, DIGIT1, DIGIT2 };

	for (register wchar_t c = _next_char(self); c; c = _next_char(self))
	{
		switch (state)
		{
		case START:
			if (iswdigit(c))
				state = DIGIT0;
			else
				goto accept_test;
			break;
		case DIGIT0:
			if (iswdigit(c))
				state = DIGIT0;
			else if (c == L'.')
				state = DOT;
			else
				goto accept_test;
			break;
		case DOT:
			if (iswdigit(c))
				state = DIGIT1;
			else
				_lex_error(self, L"The character sequence doesn't form a valid number literal.");
			break;
		case DIGIT1:
			if (iswdigit(c))
				state = DIGIT1;
			else if (c == L'e')
				state = E;
			else
				_lex_error(self, L"The character sequence doesn't form a valid number literal.");
			break;
		case E:
			if (iswdigit(c))
				state = DIGIT2;
			else
				_lex_error(self, L"The character sequence doesn't form a valid number literal.");
			break;
		case DIGIT2:
			if (iswdigit(c))
				state = DIGIT2;   
			break;
		}
		*_tp++ = c;
		if (!iswdigit(CC))
			goto accept_test;
	} 
accept_test:
	for (size_t i = 0; i < sizeof(final_set)/sizeof(final_set[0]); i++)
	{
		if (state == final_set[i]) 
		{
			*_tp = L'\0';
			return true;
		}
	}
	return false;
}

static bool _lex_match_keyword(lexer_t* self, wchar_t* _keyword, size_t _kw_len)
{
	_lex_update_scoord(self);
	_save_state(self, 9);
	size_t i = 0; 
	for (wchar_t c = _next_char(self); *_keyword && c && c == *_keyword; ++_keyword, c = _next_char(self), ++i);

	if (i != _kw_len)
	{
		_restore_state(self);
		return false;
	}
	return true;
}

static bool _match_string(lexer_t* self)
{
	_lex_update_scoord(self);
	if (CC != L'\'' && CC != L'\"')
		return false;
	wchar_t quote = _next_char(self); // Save the quote and skip it.

	register wchar_t c = _next_char(self);
	for (; c && c != L'\n' && c != quote; c = _next_char(self))
	{  
		if (c == L'\\')
		{
			c = _next_char(self);
			switch (c)
			{
				case L'\\':
					*_tp++ = L'\\';
					break;
				case L'n':
					*_tp++ = L'\n';
					break;
				case L't':
					*_tp++ = L'\t';
					break;
				case L'r':
					*_tp++ = L'\r';
					break;
				case L'\'':
					*_tp++ = L'\'';
					break;
				case L'"':
					*_tp++ = L'"';
					break;
				default:
					_lex_error(self, L"Unknown escape character.");
					break;
			} 
			c = CC;
			continue;
		}
		*_tp++ = c; 
	}
	if(c != quote)
		_lex_error(self, L"Unfinished string literal.");
	*_tp = L'\0';
	return true;
}

static void _lex_update_scoord(lexer_t* self)
{
	self->scoord = scoord(self->fd->file_path, self->line_no, self->column_no);
}