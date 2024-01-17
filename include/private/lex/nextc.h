#pragma once

#ifndef NEXTC_H
#define NEXTC_H

#include <io/io.h>

#include <wchar.h>


extern wchar_t _buffer0[MAXIMUM_BLOCK_SZ + 1];
extern wchar_t _buffer1[MAXIMUM_BLOCK_SZ + 1];

wchar_t next_char();

#endif