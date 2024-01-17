
zz(TK_ID, L"Identifier", identifier, L"%s")
zz(TK_NUMBER, L"Number", number, L"%lf")
zz(TK_STRING, L"String", string, L"%s")
xx(TK_TRUE, L"true", 0)
xx(TK_FALSE, L"false", 0)

/******** Begin Operators ********/

xx(TK_ASSIGN, L"=", L'=')

/******** Begin Binary Operators ********/
yy(TK_PLUS, L"+", L'+')
yy(TK_MINUS, L"-", L'-')
yy(TK_MUL, L"*", L'*')
yy(TK_DIV, L"/", L'/')
yy(TK_POW, L"^", L'^')

xx(TK_DOT, L"Dot", 0)
xx(TK_DOTMUL, L".*", 0)
xx(TK_DOTDIV, L"./", 0)
xx(TK_DOTPOW, L".^", 0)

/******** End Binary Operators ********/
 

xx(TK_NOT,			L"~", L'~') 

yy(TK_OPPAR,		L"(", L'(')
yy(TK_CLPAR,		L")", L')')

yy(TK_OPBRACE,		L"{", L'{')
yy(TK_CLBRACE,		L"}", L'}')

yy(TK_OPBRACKET,	L"[", L'[')
xx(TK_CLBRACKET,	L"]", L']')

xx(TK_TRANS,		L"'", L'\'')

yy(TK_COMMA,		L",", L',')
yy(TK_COLON,		L":", L':')
yy(TK_SEMICOLON,	L";", L';')

/******** Begin Logical Operators ********/
xx(TK_AND,			L"And", 0)
xx(TK_OR,			L"Or", 0)
/******** End Logical Operators ********/

/******** Begin Relational Operators ********/
xx(TK_GT,			L">", 0)
xx(TK_LT,			L"<", 0)
xx(TK_EQ,			L"==", 0)
xx(TK_NEQ,			L"~=", 0)
xx(TK_GTEQ,			L">=", 0)
xx(TK_LTEQ,			L"<=", 0)
/******** End Relational Operators ********/

/******** End Operators ********/

/******** Begin Keywords ********/

xx(TK_IF,			L"if", 0)
xx(TK_ELSEIF,		L"elseif", 0)
xx(TK_ELSE,			L"else", 0)
xx(TK_SWITCH,		L"switch", 0)
xx(TK_CASE,			L"case", 0)
xx(TK_OTHERWISE,	L"otherwise", 0)
xx(TK_WHILE,		L"while", 0)
xx(TK_FOR,			L"for", 0)
xx(TK_BREAK,		L"break", 0)
xx(TK_CONTINUE,		L"continue", 0)
xx(TK_LOAD,			L"load", 0)
xx(TK_FUNC,			L"function", 0)
xx(TK_RETURN,		L"return", 0)
xx(TK_END,			L"end", 0)

/******** End Keywords ********/
yy(TK_NEWLINE,		L"New Line", L'\n') 
xx(TK_EOF,			L"End Of File", 0)
