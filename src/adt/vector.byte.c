#include <adt/vector.byte.h>

array_byte_t* array_byte_alloc(size_t length) 
{
    array_byte_t* self = GC_malloc(sizeof(*self)); 
    do {
        if (!(self != ((void*)0))) {
            fflush((__acrt_iob_func(2))); fwprintf((__acrt_iob_func(2)), L"%s\n", L"Out of memory."); return ((void*)0);
        }
    } 
    while (0); 
    self->length = length; 
    self->inner = (uint8_t*)GC_malloc(sizeof(*self->inner) * self->length); 
    do {
        if (!(self->inner != ((void*)0))) {
            fflush((__acrt_iob_func(2))); fwprintf((__acrt_iob_func(2)), L"%s\n", L"Out of memory."); return ((void*)0);
        }
    } while (0); 
    memset(self->inner, 0, self->length * sizeof(uint8_t)); 
    return self;
} 
void array_byte_free(array_byte_t* self) 
{
    if (self) 
    {
        if (self->inner) GC_free(self->inner); 
        GC_free(self);
    }
} 
uint8_t array_byte_get(array_byte_t* self, size_t index) {
    if (index < 0 || index >= self->length) 
        return ((void*)0); 
    return self->inner[index];
} 
uint8_t array_byte_set(array_byte_t* self, size_t index, uint8_t value) {
    if (index < 0 || index >= self->length) 
        return -1; 
    return self->inner[index] = value;
} 
void array_byte_clear_from_size(array_byte_t* self, size_t from, size_t size) {
    for (size_t i = from; i < size; i++) {
        if (self->inner[i]) GC_free(self->inner[i]); 
        self->inner[i] = 0;
    }
} 
void array_byte_clear_from(array_byte_t* self, size_t from) {
    array_byte_clear_from_size(self, from, self->length - from);
} 
void array_byte_clear(array_byte_t* self) {
    array_byte_clear_from(self, 0);
} 
void array_byte_copy(array_byte_t* destination, size_t destinationIndex, array_byte_t* source, size_t sourceIndex, size_t size) {
    if ((destination->length - destinationIndex) < size || (source->length - sourceIndex) < size) 
        return; 
    memcpy(destination->inner + destinationIndex, source->inner + sourceIndex, size * sizeof(uint8_t));
}

extern size_t byte_capacity = 8; 

vector_byte_t* vector_byte_alloc(compare_byte_t _comparer, ...) {
    vector_byte_t* self = GC_malloc(sizeof(*self)); do {
        if (!(self != ((void*)0))) {
            fflush((__acrt_iob_func(2))); fwprintf((__acrt_iob_func(2)), L"%s\n", L"Out of memory."); 
            return ((void*)0);
        }
    } while (0); 
    self->count = 0;
    self->comparer = _comparer; 
    self->capacity = byte_capacity; 
    self->array = array_byte_alloc(byte_capacity != 8 ? byte_capacity : 8); byte_capacity = 8; 
    return self;
} 
void vector_byte_free(vector_byte_t* self) {
    if (self) {
        if (self->array) array_byte_free(self->array); GC_free(self);
    }
} 
void vector_byte_clear(vector_byte_t* self) {
    array_byte_clear(self->array); self->count = 0;
} 
uint8_t vector_byte_get(vector_byte_t* self, size_t index) {
    if (index < 0 || index >= self->count) 
        return ((void*)0); 
    return array_byte_get(self->array, index);
} 
uint8_t vector_byte_set(vector_byte_t* self, size_t index, uint8_t item) {
    if (index < 0 || index >= self->count) 
        return ((void*)0); 
    return array_byte_set(self->array, index, item);
} 
size_t vector_byte_index_of(vector_byte_t* self, uint8_t item) {
    for (size_t i = 0; i < self->count; i++) {
        if (self->comparer(vector_byte_get(self, i), item) == 0) 
            return i;
    } return -1;
} 
uint8_t vector_byte_insert(vector_byte_t* self, size_t index, uint8_t item) {
    size_t capacity = self->capacity; 
    size_t count = self->count; 
    if (self->count >= capacity) {
        array_byte_t* dest = array_byte_alloc(capacity * 2); 
        array_byte_copy(dest, 0, self->array, 0, count); 
        array_byte_free(self->array);
        self->array = dest;
        self->capacity = capacity * 2;
    } 
    if (index < count) {
        array_byte_copy(self->array, index + 1, self->array, index, count - index);
    } 
    array_byte_set(self->array, index, item); 
    self->count++; 
    return item;
} 
uint8_t vector_byte_push_back(vector_byte_t* self, uint8_t item) {
    return vector_byte_insert(self, self->count, item);
} 
uint8_t vector_byte_push_front(vector_byte_t* self, uint8_t item) {
    return vector_byte_insert(self, 0, item);
} 
uint8_t vector_byte_delete(vector_byte_t* self, size_t index) {
    if (index <= 0 || index >= self->count) 
        return ((void*)0); 
    uint8_t deleted = vector_byte_get(self, index); 
    vector_byte_set(self, index, ((void*)0)); 
    if (index < self->count-- - 1) 
        array_byte_copy(self->array, index, self->array, index + 1, self->count - index); 
    return deleted;
} 
uint8_t vector_byte_remove(vector_byte_t* self, uint8_t item) {
    size_t index = vector_byte_index_of(self, item); return vector_byte_delete(self, index);
} 
uint8_t vector_byte_pop_back(vector_byte_t* self) {
    return vector_byte_delete(self, self->count - 1);
} 
uint8_t vector_byte_pop_front(vector_byte_t* self) {
    return vector_byte_delete(self, 0);
} 
_Bool vector_byte_contains(vector_byte_t* self, uint8_t item) {
    return vector_byte_index_of(self, item) != -1;
} 
uint8_t vector_byte_insert_after(vector_byte_t* self, uint8_t after, uint8_t item) {
    size_t index_after = vector_byte_index_of(self, after) + 1; return vector_byte_insert(self, index_after, item);
} 
uint8_t vector_byte_insert_before(vector_byte_t* self, uint8_t before, uint8_t item) {
    size_t index_before = vector_byte_index_of(self, before) - 1; vector_byte_insert(self, index_before, item);
}