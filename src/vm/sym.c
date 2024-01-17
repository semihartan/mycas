#include <vm/sym.h>

#include <adt/sb.h>

static mc_string_t _flag_to_str(enum sym_flag flag)
{
	mc_string_t str = NULL; 
	if (HAS_VAR(flag))
		str = Str("(Variable");
	else if (HAS_FUNC(flag))
		str = Str("(Function");
	else if (HAS_STRING(flag))
		str = Str("(String");
	if (HAS_GLOBAL(flag))
		str = mc_str_append(str, Str(" | Global)"));
	else if (HAS_LOCAL(flag))
		str = mc_str_append(str, Str(" | Local)"));
	return str;
}

mc_string_t sym_to_str(sym_t* sym)
{
	sb_t* sb = sb_alloc();
	
	sb_appendf(sb, Str("Sym.Name: %10s, Sym.Flag: %20s, Sym.Index: %.3u, "), sym->name, _flag_to_str(sym->flags), sym->tab_index);
	if (HAS_STRING(sym->flags))
		sb_appendf(sb, Str("Sym.String: %s"), sym->obj);
	else
		sb_appendf(sb, Str("Sym.FrameObj: %p"), sym->frame);
	if (HAS_FUNC(sym->flags))
		sb_appendf(sb, Str("Sym.InArgc: %d, Sym.OutArgc: %d"), sym->inargc, sym->outargc);
	return sb_to_str(sb);
}