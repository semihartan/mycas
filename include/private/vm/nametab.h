#pragma once

#ifndef GLOBAL_TABLE_H
#define GLOBAL_TABLE_H

#include <mc.h>
#include <adt/vector.void.h>

typedef struct
{
	vector__t* names;
	size_t empty_index;
}nametab_t;

nametab_t* nametab_alloc();

size_t nametab_alloc_index(nametab_t* _nametab);

mc_obj_t nametab_get_index(nametab_t* _nametab, size_t _index);
mc_obj_t nametab_set_index(nametab_t* _nametab, size_t _index, mc_obj_t _obj);


#endif