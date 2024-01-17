#include <io/buffer.h>
#include <io/file.h>
#include <util.h>
#include <error.h>

#include <stdio.h>
 

int fill_buffer(file_t* _fd)
{
	int bytes_read = _fd->bytes_read = _fd->io_buffer.buf_length = fread(_fd->io_buffer.buffer, sizeof(char), MAXIMUM_BLOCK_SZ, _fd->internal_fd);

	if (bytes_read == 0 && feof(_fd->internal_fd))
		return EOF;

	MC_ERROR_RETVAL(bytes_read != 0 && ferror(_fd->internal_fd) == 0,
		(wchar_t*)util_format(L"Failed to read the file %s.", io_get_file_base_name(_fd)), -1);
	_fd->file_ptr = ftell(_fd->internal_fd);

	// Null-terminate the buffer for the strnorm function to work as expected.
	_fd->io_buffer.buffer[bytes_read] = '\0';
	return bytes_read;

}