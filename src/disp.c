#include <mc.h>
#include <mcstr.h>
 

#include <stdio.h>


void mc_disp(mc_obj_t obj)
{
	if (!obj)
		return;
	mc_string_t str = mc_obj_str(obj);
	(void)_putws(str);
}