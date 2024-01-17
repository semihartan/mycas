#include <mc.h>
#include <mcstr.h>


#include <stdio.h>

void mc_print(mc_string_t format, mc_obj_t obj)
{
	enum object_type ob_type = mc_obj_get_obj_type(obj);
	switch (ob_type)
	{
		case OK_STRING:
			wprintf(format, obj);
			break;
		case OK_MATRIX:
		{
			size_t format_sz = mc_str_len(format);
			wchar_t* ws = format;
			size_t format_counter = 0;
			
			for (size_t i = 0; i < format_sz; i++)
			{
				register wchar_t wc = ws[i];
				if (wc == L'%')
				{
					i++;
					switch (ws[i])
					{
						case L'l':
							if (ws[++i] == L'f')
							{
								format_counter++;

							}
						default:
							break;
					} 
				}
				putwchar(wc);
			}
			break;
		}
	} 
}
 