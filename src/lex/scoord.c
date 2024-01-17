#include <lex/scoord.h>

scoord_t scoord(wchar_t* file, int line_no, int column_no)
{
	return (scoord_t) { file, line_no, column_no };
}