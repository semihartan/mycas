#include <util.h>

#include <common.h> 
#include <mcstr.h> 
#include <error.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
 
#include <gc/gc.h>

#define FORMAT_BUFFER_LEN 0x1000

static wchar_t _format_buffer[FORMAT_BUFFER_LEN];

mc_string_t util_format(const wchar_t* format, ...)
{
	va_list ap;
	va_start(ap, format);
	mc_string_t formatted_output = util_vformat(format, ap);
	va_end(ap); 
	return formatted_output;
}

mc_string_t util_vformat(const wchar_t* format, va_list ap)
{
	int needed_char = vswprintf(_format_buffer, FORMAT_BUFFER_LEN, format, ap);
	 
	const wchar_t* formatted_output = mc_str_empty(needed_char);

	MC_ERROR_RETVAL(formatted_output, MC_MESSAGE_NOMEM, NULL);

	return wmemcpy(formatted_output, _format_buffer, needed_char);
}