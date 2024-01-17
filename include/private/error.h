#pragma once

#ifndef ERROR_H
#define ERROR_H



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define MC_ERROR_SUCCESS 0
#define MC_ERROR_FAILURE 1

#define MC_MESSAGE_NOMEM L"Out of memory."  
 

/*
* Some useful acros to output error messages to user when the condition specified with cond becomes false.
*/

#define MCAS_WARNING_FAILURE(cond, message) \
do\
{\
	if (!(cond))\
	{\
		fprintf(stderr, message);\
	}\
}while(0)

#define MCAS_ASSERT(cond, message) \
do\
{\
	if (!(cond))\
	{\
		_wassert(message, _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0);\
	}\
}while(0)

#define MC_ERROR_RETVAL(cond, message, ret) \
do {\
	if(!(cond))\
	{\
		fflush(stderr);\
		fwprintf(stderr, L"%s\n", message);\
		return ret;\
	}\
} while (0)

#define MC_ERROR_NORETVAL(cond, message) \
do {\
	if(!(cond))\
	{\
		fflush(stderr);\
		fwprintf(stderr, L"%s\n", message);\
		return;\
	}\
} while (0)

#define MC_ERROR_ABORT(cond, message, error_no) \
do {\
	if(!(cond))\
	{\
		fflush(stderr);\
		fwprintf(stderr, L"%s\n", message);\
		exit(error_no);\
	}\
} while (0)


#ifdef defined(MC_RUNTIME)
	#define MC_ABORT(message)					vm_raise_exception(message)
#else
	#define MC_ABORT(message)					MC_ERROR_ABORT(0, message, MC_ERROR_FAILURE)
#endif // defined(MC_RUNTIME)




#define MC_ERROR_OUT_OF_MEMORY(ptr) MC_ERROR_RETVAL(ptr != NULL, MC_MESSAGE_NOMEM, NULL)
#define MC_ABORT_OUT_OF_MEMORY(ptr) MC_ERROR_ABORT(ptr != NULL, MC_MESSAGE_NOMEM, MC_ERROR_FAILURE)

#define WARNINGIFN(ptr) warning_if_null(ptr, false)

inline void warning_if_null(void* _ptr, bool _treat_error);
/*
* Utility functions to throw an error if the _ptr is NULL.
*/
inline void throw_if_null(void* _ptr);

#endif