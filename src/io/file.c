#include <io/file.h>

#include <io/buffer.h>
#include <io/norm.h>
#include <io/enc.h>

#include <error.h>
#include <util.h>
#include <mcstr.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h> 

#include <gc/gc.h>

#include <cwalk/cwalk.h> 



static long _file_seek(file_t* _fd, long seek, int origin);

file_t* io_open_file(const wchar_t* _file_path)
{
	// Allocate emmory for the file object. Remember the memory allocated as zero-initialized by GC_MALLOC.
	// Therefore, only initialize the non-zero fields at the beginning.
	file_t* fd = GC_MALLOC(sizeof * fd);
	 
	MC_ERROR_RETVAL(_wfopen_s(&fd->internal_fd, _file_path, L"rb") == 0, util_format(L"Failed to open file %s.", _file_path), NULL);

	// Detect the file encoding.
	detect_file_encoding(fd);
	// Open a iconv context for charset conversion.
	fd->iconv_ctx = iconv_open("UTF-16LE", fd->encoding);
	MC_ERROR_RETVAL(fd->iconv_ctx != -1, L"libiconv_open failed.", NULL);
	// Store file path and base name.
	fd->file_path = _file_path;
	cwk_path_get_basename(_file_path, &fd->file_base_name, NULL);
	return fd;
}

int io_close_file(const file_t** _fdp)
{
	file_t* fd = *_fdp;
	MC_ERROR_RETVAL(fclose(fd->internal_fd) == 0, L"Failed to close te file.", EOF);
	MC_ERROR_RETVAL(iconv_close(fd->iconv_ctx) == 0, L"Failed to free libiconv resources.", EOF);
	GC_FREE(fd);
	return 0;
}

int io_read_block(file_t* _fd, wchar_t _block[MAXIMUM_BLOCK_SZ + 1], size_t* _num_of_chars)
{
	int result = MC_ERROR_SUCCESS;
	if (_fd->io_buffer.buf_length <= 0)
		result = fill_buffer(_fd);
	if (result == EOF)
		return EOF;
	 
	normalize_new_lines(_fd);

	size_t inbytesleft = _fd->io_buffer.buf_length;
	size_t outbytesleft = sizeof(*_block) * MAXIMUM_BLOCK_SZ;
	char* in_buf = _fd->io_buffer.buffer;
	char* out_buf = _block;
	size_t nb_of_convs = iconv(_fd->iconv_ctx, &in_buf, &inbytesleft, &out_buf, &outbytesleft);
	int err = errno;
	if (nb_of_convs == (size_t)-1)
	{
		MC_ERROR_NORETVAL(errno != EILSEQ, L"An invalid multibyte sequence is encountered in the input.");
		MC_ERROR_NORETVAL(errno != E2BIG, L"The output buffer has no more room for the next converted character.");
		if (errno == EINVAL || inbytesleft > 0)
		{
			_file_seek(_fd, -(long)inbytesleft, SEEK_CUR);
		}
	}
	size_t written_bytes = sizeof(wchar_t) * MAXIMUM_BLOCK_SZ - outbytesleft;
	size_t wchars_written = written_bytes / 2;
	if (_num_of_chars)
	{
		*_num_of_chars = wchars_written;
	}
	_block[wchars_written] = L'\0';

	_fd->io_buffer.buf_length = 0; 
	return MC_ERROR_SUCCESS;
}

void io_save_file_state(file_t* _fd)
{
	_fd->state.bytes_read = _fd->bytes_read;
	_fd->state.file_ptr = _fd->file_ptr;
}

void io_restore_file_state(file_t* _fd)
{
	_fd->bytes_read = _fd->state.bytes_read; 
	MC_ERROR_NORETVAL(_file_seek(_fd, _fd->state.file_ptr, SEEK_SET) == 0, L"Failed to restore file.");
}

const wchar_t* io_get_file_base_name(file_t* _fd)
{
	assert(_fd != NULL);
	if (_fd)
		return _fd->file_base_name; 
}
static long _file_seek(file_t* _fd, long offset, int origin)
{
	long result = fseek(_fd->internal_fd, offset, origin);
	if (origin == SEEK_CUR)
		_fd->file_ptr += offset;
	else if(origin == SEEK_SET)
		_fd->file_ptr = offset;
	return result;
}