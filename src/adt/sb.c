#include <adt/sb.h>

#include <mcdef.h>
#include <error.h>
#include <util.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <memory.h>
#include <wchar.h>

#include <gc/gc.h>
 
struct chunk
{
	struct chunk* next;
	mc_string_t buffer;
	int count;
};

static void chunk_clear(struct chunk* chunk);

static struct chunk* alloc_chunk()
{
	struct chunk* chunk = NEWOBJP(chunk);
	MC_ERROR_OUT_OF_MEMORY(chunk); 
	chunk->buffer = mc_str_empty(CHUNKCAPACITY);
	chunk_clear(chunk);
	return chunk;
}
static void chunk_clear(struct chunk* chunk)
{
	if (!chunk)
		return;
	chunk->next = NULL;
	mc_str_erase(chunk->buffer);
	chunk->count = 0;
}
string_builder_t* sb_alloc()
{
	string_builder_t* self = NEWOBJP(self);
	MC_ERROR_OUT_OF_MEMORY(self);
	self->first_chunk = alloc_chunk();
	self->last_chunk = self->first_chunk;
	self->length = 0;
	return self;
}
 
void sb_append(string_builder_t* self, mc_string_t text)
{ 
	struct chunk* chunk = self->last_chunk;

	int str_len = mc_str_len(text); /* + sizeof(NEWLINE)*/;
	self->length += str_len;
	
	int available_size = CHUNKCAPACITY - chunk->count;
	if (str_len > available_size)
	{
		int to_written_sz = str_len - available_size;
		int written_sz = 0;
		int nb_of_req_chunks = to_written_sz / CHUNKCAPACITY;
		int remaining_sz = to_written_sz % CHUNKCAPACITY;

		wmemcpy((wchar_t*)chunk->buffer + chunk->count, (wchar_t*)text + written_sz, available_size);
		chunk->count += available_size;
		written_sz += available_size;
		while (nb_of_req_chunks > 0)
		{
			chunk->next = alloc_chunk();
			chunk = chunk->next;
			self->last_chunk = chunk;
			wmemcpy((wchar_t*)chunk->buffer, (wchar_t*)text + written_sz, CHUNKCAPACITY);
			chunk->count += CHUNKCAPACITY;
			written_sz += CHUNKCAPACITY;
			nb_of_req_chunks--;
		}
		if (remaining_sz > 0)
		{
			chunk->next = alloc_chunk();
			chunk = chunk->next;
			wmemcpy((wchar_t*)chunk->buffer, (wchar_t*)text + written_sz, remaining_sz);
			chunk->count += remaining_sz;
		}
	}
	else
	{
		wmemcpy((wchar_t*)chunk->buffer + chunk->count, (wchar_t*)text, str_len);
		chunk->count += str_len;
		//chunk->next = AllocChunk();
	}
}

void sb_appendl(string_builder_t* self, mc_string_t text)
{
	sb_append(self, text);
	sb_append(self, Str("\r\n"));
}

void sb_appendf(string_builder_t* self, mc_string_t format, ...)
{
	va_list ap;
	va_start(ap, format);
	mc_string_t output = util_vformat(format, ap); 
	va_end(ap);
	sb_append(self, output); 
}

void sb_clear(string_builder_t* self)
{
	struct chunk* chunk = self->first_chunk;
	while (chunk != NULL)
	{
		struct chunk* next = chunk->next;
		if (chunk == self->first_chunk)
		{
			chunk_clear(chunk);
			goto next_chunk;
		}
		if (chunk->buffer)
		{
			GC_FREE(chunk->buffer);
		} 
		GC_FREE(chunk);	
	next_chunk:
		chunk = next;
	}
	self->length = 0;
}

mc_string_t sb_to_str(string_builder_t* self)
{
	size_t written_sz = 0;
	mc_string_t string = mc_str_empty(self->length);
	wchar_t* str = string;
	struct chunk* chunk = self->first_chunk;
	while (chunk)
	{
		wmemcpy(str + written_sz, (wchar_t*)chunk->buffer, chunk->count);
		written_sz += chunk->count;
		chunk = chunk->next;
	}
	str[self->length] = L'\0'; 
	return string;
}