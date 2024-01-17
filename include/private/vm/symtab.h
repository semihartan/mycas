/**
* 
* @author Semih Artan
*/
#pragma once 

#ifndef SYMTAB_H
#define SYMTAB_H

#include <vm/frame.h>
#include <vm/nametab.h>
#include <vm/sym.h>
#include <parse/ast.h>

#include <mcstr.h>
#include <adt/ht.h>
#include <adt/vector.void.h> 




typedef struct
{
	htab_t* intern_ht;
}symtab_t;


extern symtab_t* symtab;
extern nametab_t* globals;

symtab_t* symtab_alloc();

/**
 * @brief If there is no symbol named 'name', adds it to the symbol table and returns true, otherwise false.
 * @param symtab The symbol table into which 'name' will be added.
 * @param name Te name of symbol.
 * @param flags The flags that specifies the kind of the symbol.
 * @return True if it adds the symbol, false otherwise.
*/
sym_t* symtab_add_sym(symtab_t* symtab, mc_string_t name, enum sym_flag flags, mc_obj_t obj, int _inargc, int _outargc);

sym_t* symtab_add_sym_from_node(symtab_t* symtab, anode_t* node);

sym_t* symtab_lookup_global(symtab_t* symtab, mc_string_t name);

sym_t* symtab_lookup_local(symtab_t* symtab, mc_string_t name);

void symtab_dump(symtab_t* symtab);

#endif