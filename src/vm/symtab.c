#include <vm/symtab.h>
#include <vm/frames.h>


#include <mcdef.h>
#include <mcstr.h>
#include <error.h>

#include <gc/gc.h>


static sym_t* _sym_alloc(mc_string_t name, enum sym_flag flags, size_t index, mc_frame_t* frame, mc_obj_t obj, int inargc, int outargc)
{
	sym_t* self = NEWOBJP(self);
	MC_ERROR_OUT_OF_MEMORY(self);
	self->name = name;
	self->flags = flags;
	if (frame)
		self->frame = frame;
	if (obj)
		self->obj = obj;
	self->tab_index = index;
	self->inargc = inargc;
	self->outargc = outargc;
	return self;
}

symtab_t* symtab_alloc()
{
	symtab_t* self = NEWOBJP(self);
	MC_ERROR_OUT_OF_MEMORY(self); 
	self->intern_ht = ht_alloc();
	return self;
}
sym_t* symtab_add_sym(symtab_t* symtab, mc_string_t name, enum sym_flag flags, mc_obj_t obj, int _inargc, int _outargc)
{
	size_t index = -1;
	mc_frame_t* frame = NULL;  
	int inargc = 0;
	int outargc = 0;
	if (HAS_GLOBAL(flags))
	{
		index = nametab_alloc_index(globals);
	}
	if (HAS_FUNC(flags))
	{
		frame = vm_alloc_frame(name); 
		inargc = _inargc;
		outargc = _outargc;
	}
	if (ht_get(symtab->intern_ht, name))
		return NULL;
	return ht_set(symtab->intern_ht, name, _sym_alloc(name, flags, index, frame, obj, inargc, outargc));
}
sym_t* symtab_add_sym_from_node(symtab_t* symtab, anode_t* node)
{
	size_t index = -1;
	mc_frame_t* frame = NULL;
	mc_string_t name = node->value.id;
	int inargc = 0;
	int outargc = 0;
	if (HAS_LOCAL(node->sym_flags))
	{
		name = mc_str_prepend(name, Str("_"));
		frame = node->frame;
		index = node->index = nametab_alloc_index(node->frame->locals);
	} 
	if (HAS_GLOBAL(node->sym_flags))
	{
		index = node->index = nametab_alloc_index(globals);
	}
	if (HAS_FUNC(node->sym_flags))
	{
		frame = vm_alloc_frame(name);
		node->frame = frame;
		node->frame_index = frame->frame_index;
		inargc = node->inargc;
		outargc = node->outargc;
	}
	if (ht_get(symtab->intern_ht, name))
		return NULL;

	return ht_set(symtab->intern_ht, name, _sym_alloc(name, node->sym_flags, index, frame, NULL, inargc, outargc));
}
 
sym_t* symtab_lookup(symtab_t* symtab, mc_string_t name, enum sym_flag flags)
{
	if(HAS_LOCAL(flags))
		name = mc_str_prepend(name, Str("_"));
	return ht_get(symtab->intern_ht, name);
}

sym_t* symtab_lookup_global(symtab_t* symtab, mc_string_t name)
{
	return symtab_lookup(symtab, name, SYM_GLOBAL);
}

sym_t* symtab_lookup_local(symtab_t* symtab, mc_string_t name)
{
	return symtab_lookup(symtab, name, SYM_LOCAL);
}
void symtab_dump(symtab_t* symtab)
{
	_putws(L"Symbol Table Dump");
	size_t index = 1; 
	ht_iter_t iter = ht_iterator(symtab->intern_ht);
	while (ht_next(&iter))
	{
		mc_string_t key = iter.key;
		sym_t* sym = iter.value;
		wprintf(L"[%.3u. Key: %16s, %16s]\n", index++, key, sym_to_str(sym));
	}
}