#include <io/buffer.h>
#include <io/file.h>
#include <error.h>

#include <string.h>

#include <uchardet/uchardet.h>

const char* detect_file_encoding(file_t* _fd)
{
	FILE* internal_fd = _fd->internal_fd;
	const char* encoding;
	int bytes_read = fill_buffer(_fd);

	uchardet_t ud = uchardet_new();

	int ret_val = uchardet_handle_data(ud, _fd->io_buffer.buffer, bytes_read);
	MC_ERROR_RETVAL(ret_val == 0, L"On handle processing, occured out of memory.", NULL);
	uchardet_data_end(ud);
	size_t n = uchardet_get_n_candidates(ud);
	encoding = strdup(uchardet_get_charset(ud));
	_fd->encoding = encoding;
	uchardet_delete(ud);
	/*Detect* detect_ctx = detect_init();

	DetectObj* detect_obj;

	MCAS_ERROR_RETVAL((detect_obj = detect_obj_init()) != NULL, L"libchardet failed to allocate memory", NULL);

	int ret_val = detect_r(_fd->io_buffer.buffer, bytes_read, &detect_obj);

	MCAS_ERROR_RETVAL(ret_val != CHARDET_OUT_OF_MEMORY, L"On handle processing, occured out of memory.", NULL);
	MCAS_ERROR_RETVAL(ret_val != CHARDET_NULL_OBJECT, L"2st argument of chardet() is must memory allocation with detect_obj_init API.", NULL);

	encoding = strdup(detect_obj->encoding);
	detect_obj_free(&detect_obj);
	detect_destroy(&detect_ctx);*/

	
	return encoding;
}
