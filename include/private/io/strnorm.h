#pragma once

#ifndef STRNORM_H
#define STRNORM_H

#include <wchar.h>

/*
* @brief 
*	Normalizes the newline characters in a buffer by converting Windoes-style and Maac-style
* newline charactrs into Unix-style newline characters. The conversion is done in-place. Therefore,
* it doesn't require a second buffer.
* 
* @return
*		It returns a pointer past to the last character in the normalized buffer, which makes calculating
* string length and string-related porcess convenient..
*/
char* strnorm(char* _buffer, size_t* _num_of_nl);

/*
* @brief
*	The unicode version. It works the same as the ANSI version, strnorm except working a buffer of wide chars.
*
* @return
*		It returns a pointer past to the last character in the normalized buffer, which makes calculating
* string length and string-related porcess convenient..
*/
wchar_t* wcsnorm(wchar_t* _buffer, size_t* _num_of_nl);

#endif