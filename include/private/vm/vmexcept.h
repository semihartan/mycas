#pragma once

#ifndef VM_EXCEPTION_H
#define VM_EXCEPTION_H

#include <mcstr.h>
 

void vm_raise_exception(mc_string_t str);

#endif