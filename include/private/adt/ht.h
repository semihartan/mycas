#pragma once
#ifndef _HT_H
#define _HT_H

#include <stdbool.h>
#include <stddef.h>
#include <mcstr.h>

 
typedef struct htab htab_t;
typedef struct ht_entry ht_entry_t;

struct htab {
    ht_entry_t* entries;  // hash slots
    size_t capacity;    // size of _entries array
    size_t length;      // number of items in hash table
};

/**
 * @brief Create hash table and return pointer to it, or NULL if out of memory. 
 * @return The hash table that has been created.
*/
htab_t* ht_alloc(void);

// Free memory allocated for hash table, including allocated keys.
void ht_free(htab_t* table);


// Get item with given key (NUL-terminated) from hash table. Return
// value (which was set with ht_set), or NULL if key not found.
void* ht_get(htab_t* table, mc_string_t key);

// Set item with given key (NUL-terminated) to value (which must not
// be NULL). If not already present in table, key is copied to newly
// allocated memory (keys are freed automatically when ht_destroy is
// called). Return address of copied key, or NULL if out of memory.
void* ht_set(htab_t* table, mc_string_t key, void* value);

// Return number of items in hash table.
size_t ht_length(htab_t* table);

// Hash table iterator: create with ht_iterator, iterate with ht_next.
typedef struct {
    mc_string_t key;  // current key
    void* value;      // current value

    // Don't use these fields directly.
    htab_t* _table;       // reference to hash table being iterated
    size_t _index;    // current index into ht._entries
} ht_iter_t;

// Return new hash table iterator (for use with ht_next).
ht_iter_t ht_iterator(htab_t* table);

// Move iterator to next item in hash table, update iterator's key
// and value to current item, and return true. If there are no more
// items, return false. Don't call ht_set during iteration.
bool ht_next(ht_iter_t* it);

#endif // _HT_H