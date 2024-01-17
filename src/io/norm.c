#include <io/norm.h>
#include <io/strnorm.h>


void normalize_new_lines(file_t* _fd)
{
	size_t nconv = 0;
	char* end = strnorm(_fd->io_buffer.buffer, &nconv);
	_fd->io_buffer.buf_length = end - _fd->io_buffer.buffer;
}