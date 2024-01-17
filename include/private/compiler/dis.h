#pragma once

#ifndef DISSASSEMBLER_H
#define DISSASSEMBLER_H

#include <vm/co.h>
#include <mcstr.h>

mc_string_t co_disassemble(code_object_t* co); 

#endif