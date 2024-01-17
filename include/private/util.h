#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <mcstr.h>
#include <wchar.h>

mc_string_t util_format(const wchar_t* format, ...);
mc_string_t util_vformat(const wchar_t* format, va_list ap); 

#endif