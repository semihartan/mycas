#include <adt/ht.h>
#include <error.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <gc/gc.h>

// Hash table entry (slot may be filled or empty).
struct ht_entry {
    mc_string_t key;  // key is NULL if this slot is empty
    void* value;
};

// Hash table structure: create with ht_create, free with ht_destroy.

#define INITIAL_CAPACITY 128  // must not be zero

htab_t* ht_alloc(void) {
    // Allocate space for hash table struct.
    htab_t* self = GC_MALLOC(sizeof(htab_t));
    MC_ERROR_OUT_OF_MEMORY(self);
    self->length = 0;
    self->capacity = INITIAL_CAPACITY;

    // Allocate (zero'd) space for entry buckets.
    self->entries = GC_MALLOC(sizeof(ht_entry_t) * self->capacity);
    if (self->entries == NULL) {
        GC_FREE(self); // error, free table before we return!
        MC_ERROR_OUT_OF_MEMORY(self);
    }
    return self;
}

void ht_free(htab_t* self) {
    // First free allocated keys.
    for (size_t i = 0; i < self->capacity; i++) {
        GC_FREE((void*)self->entries[i].key);
    }

    // Then free entries array and table itself.
    GC_FREE(self->entries);
    GC_FREE(self);
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_key(mc_string_t key) {
    uint64_t hash = FNV_OFFSET;
    for (const wchar_t* p = key; *p; p++) {
        hash ^= (uint64_t)(wchar_t)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

void* ht_get(htab_t* self, mc_string_t key) {
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(self->capacity - 1));

    // Loop till we find an empty entry.
    while (self->entries[index].key != NULL) {
        if (wcscmp(key, self->entries[index].key) == 0) {
            // Found key, return value.
            return self->entries[index].value;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= self->capacity) {
            // At end of entries array, wrap around.
            index = 0;
        }
    }
    return NULL;
}

// Internal function to set an entry (without expanding table).
static void* ht_set_entry(ht_entry_t* entries, size_t capacity,
    mc_string_t key, void* value, size_t* plength) {
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    // Loop till we find an empty entry.
    while (entries[index].key != NULL) {
        if (wcscmp(key, entries[index].key) == 0) {
            // Found key (it already exists), update value.
            entries[index].value = value;
            return entries[index].key;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= capacity) {
            // At end of entries array, wrap around.
            index = 0;
        }
    }

    // Didn't find key, allocate+copy if needed, then insert it.
    if (plength != NULL) {
        key = mc_str_duplicate(key);
        if (key == NULL) {
            return NULL;
        }
        (*plength)++;
    }
    entries[index].key = key;
    entries[index].value = value;
    return value;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
static bool ht_expand(htab_t* table) {
    // Allocate new entries array.
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;  // overflow (capacity would be too big)
    }
    ht_entry_t* new_entries = GC_MALLOC(sizeof(ht_entry_t) * new_capacity);
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->capacity; i++) {
        ht_entry_t entry = table->entries[i];
        if (entry.key != NULL) {
            ht_set_entry(new_entries, new_capacity, entry.key,
                entry.value, NULL, NULL);
        }
    }

    // Free old entries array and update this table's details.
    GC_FREE(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

void* ht_set(htab_t* table, mc_string_t key, void* value) {
    assert(value != NULL);
    if (value == NULL) {
        return NULL;
    }

    // If length will exceed half of current capacity, expand it.
    if (table->length >= table->capacity / 2) {
        if (!ht_expand(table)) {
            return NULL;
        }
    }

    // Set entry and update length.
    return ht_set_entry(table->entries, table->capacity, key, value,
        &table->length);
}

size_t ht_length(htab_t* self) {
    return self->length;
}

ht_iter_t ht_iterator(htab_t* self) {
    ht_iter_t it;
    it._table = self;
    it._index = 0;
    return it;
}

bool ht_next(ht_iter_t* it) {
    // Loop till we've hit end of entries array.
    htab_t* table = it->_table;
    while (it->_index < table->capacity) {
        size_t i = it->_index;
        it->_index++;
        if (table->entries[i].key != NULL) {
            // Found next non-empty item, update iterator key and value.
            ht_entry_t entry = table->entries[i];
            it->key = entry.key;
            it->value = entry.value;
            return true;
        }
    }
    return false;
}