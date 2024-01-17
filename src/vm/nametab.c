#include <vm/nametab.h>

#include <mcdef.h>

#include <gc/gc.h>


int32_t mc_obj_comparer(mc_obj_t obj1, mc_obj_t obj2)
{
	if (obj1 == obj2)
		return 0;
	else
		return -1;
}

nametab_t* nametab_alloc()
{
	nametab_t* self = NEWOBJP(self);
	MC_ABORT_OUT_OF_MEMORY(self);
	self->names = vector__alloc(mc_obj_comparer);
	self->empty_index = 0;
	return self;
}

size_t nametab_alloc_index(nametab_t* _nametab)
{
	vector__push_back(_nametab->names, NULL);
	return _nametab->empty_index++;
}

mc_obj_t nametab_get_index(nametab_t* _nametab, size_t _index)
{
	return vector__get(_nametab->names, _index);
}
mc_obj_t nametab_set_index(nametab_t* _nametab, size_t _index, mc_obj_t _obj)
{
	return vector__set(_nametab->names, _index, _obj);
}

