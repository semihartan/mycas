#include <io/strnorm.h>

char* strnorm(char* _buffer, size_t* _num_of_nl)
{
	char* src = _buffer;
	char* dest = _buffer;
	size_t num_of_nl = 0;
	while (*src)
	{
		if (*src == '\r')
		{
			*dest = '\n';
			if (*(src + 1) == '\n')
				src++;
			++num_of_nl;
		}

		*dest++ = *src++;
	}
	if (_num_of_nl)
		*_num_of_nl = num_of_nl;
	return *dest = '\0', dest;
}
wchar_t* wcsnorm(wchar_t* _buffer, size_t* _num_of_nl)
{
	wchar_t* src = _buffer;
	wchar_t* dest = _buffer;
	size_t num_of_nl = 0;
	while (*src)
	{
		if (*src == L'\r')
		{
			*dest = L'\n';
			if (*(src + 1) == L'\n')
				src++;
		}
		*dest++ = *src++;
	}
	if (_num_of_nl)
		*_num_of_nl = num_of_nl;
	return *dest = L'\0', dest;
}