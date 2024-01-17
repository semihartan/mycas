#pragma once

#ifndef COMPILER_H
#define COMPILER_H

#include <vm/co.h>
#include <parse/ast.h>




code_object_t* comp_program(anode_t* root); 

code_object_t* comp_function(anode_t* root);

#endif