#pragma once

#ifndef SOURCE_COORDINATE_H
#define SOURCE_COORDINATE_H

#include <wchar.h>

typedef struct
{
	wchar_t* file;
	int line_no;
	int column_no;
}scoord_t;

scoord_t scoord(wchar_t* file, int, int);

#endif