#pragma once

#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <mcstr.h>

#define CHUNKCAPACITY 2048

struct chunk;

typedef struct
{
	struct chunk* first_chunk;
	struct chunk* last_chunk;
	int length;
}string_builder_t;

typedef string_builder_t sb_t;

string_builder_t* sb_alloc();

void sb_append(string_builder_t* self, mc_string_t text);
void sb_appendl(string_builder_t* self, mc_string_t text);
void sb_appendf(string_builder_t* self, mc_string_t format, ...);
void sb_clear(string_builder_t* self);

mc_string_t sb_to_str(string_builder_t* self);

#endif



