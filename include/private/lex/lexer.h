#pragma once

#ifndef LEXER_H
#define LEXER_H

#include <common.h>

#include <io/file.h>
#include <lex/scoord.h>

#define MAXIMUM_TOKEN_SZ 0x200

// Defines state of current lexing process.
typedef struct
{
	const file_t* fd;
	wchar_t* cp;
	wchar_t* ep;
	size_t line_no;
	size_t column_no;
	scoord_t scoord;
	wchar_t io_block[MAXIMUM_BLOCK_SZ + 1]; 
	struct state
	{
		wchar_t* cp;
		size_t line_no;
		size_t column_no;
		bool restore_block;
		wchar_t io_block[MAXIMUM_BLOCK_SZ + 1];
	}state;
}lexer_t;
 
/**
 * @brief Creates a lexer object for a file path.
 * @param _file_path The file path for which a lexer will be created.
 * @return The lexer object that has been created, NULL if an error has ocurred. 
*/
lexer_t* lex_from_file(const wchar_t* _file_path);

/**
 * @brief Creates a lexer object for an internal file object.
 * @param _fd The file object from which a lexer will be created.
 * @return The lexer object that has been created, NULL if an error has ocurred.
*/
lexer_t* lex_from_fd(const file_t* _fd);

/**
 * @brief Creates a lexer object for a C wide string.
 * @param _fd The file object from which a lexer will be created.
 * @return The lexer object that has been created, NULL if an error has ocurred.
*/
lexer_t* lex_from_cstr(const wchar_t* _str);

/**
 * @brief Tokenizes the input and returns the next lexeme.
 * @param _lexer The lexer object.
 * @return Token that has benn processed.
*/
int lex_next_token(lexer_t* _lexer);

/**
 * @brief The function to be used to lookahead k token without consuming it.
 * @param k The lookahead count.
 * @return Token that has been lookahead. 
*/
int lex_peek_token(lexer_t* _lexer, int k);

#endif