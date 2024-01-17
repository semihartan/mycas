#ifndef FINPUT_H
#define FINPUT_H
  
#include <stddef.h>
#include <stdint.h>

#include <libiconv/iconv.h> 

#include <io/io.h>


// The opaque type to encapsulate important file data fields.
/* This is a just a wrapper around the standard file desc. */
typedef struct _file
{
	FILE* internal_fd;
	const wchar_t* file_path;
	const wchar_t* file_base_name;
	const char* encoding;
	size_t file_ptr;
	size_t bytes_read;
	iconv_t iconv_ctx;
	struct file_state
	{
		size_t file_ptr;
		size_t bytes_read;
	}state;
	struct block_buffer
	{
		size_t buf_length;
		char buffer[MAXIMUM_BLOCK_SZ + 1];
	}io_buffer;
}file_t;
 
/*
* @bried Opens the _file_path in read only mode.
* 
*/
file_t* io_open_file(const wchar_t* _file_path);

/*
* @brief Closes the file pointed to by _file_descriptor.
*/
int io_close_file(const file_t** _file_descriptor);
 
/**
* 
*	@brief Reads one disk sector from the file, converts it into UTF-16. 
*	
*	@param _fd The file descriptor from which to read. 
*	@param _block The buffer to which to write the block read. 
* 
*	@return
*		* if there is no character to read, it returns EOF.
*	@return
*		* if it successfully reads a line, it returns 0, otherwise non-zero.
**/
int io_read_block(file_t* _fd, wchar_t _block[MAXIMUM_BLOCK_SZ + 1], size_t* _num_of_chars);

/*
* @brief Saves the stateful data related to the file object such as bytes read and current file pointer.
* 
* @return
*	No return value.
*/
void io_save_file_state(file_t* _file_descriptor);

/*
* @brief Restore the stateful data saved beforeward.
*
* @return
*	No return value.
*/
void io_restore_file_state(file_t* _file_descriptor);

/*
* @brief Gets the file base name.
*/ 
const wchar_t* io_get_file_base_name(file_t* _fd);

#endif // !FINPUT_H
