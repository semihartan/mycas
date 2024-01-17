#include <private/error.h>

#include <stdio.h>
#include <stdlib.h> 


inline void warning_if_null(void* _ptr, bool _treat_error)
{
	if (!_ptr)
	{
		fprintf(stderr, "Failed to allocate memory.\n"); 
		if(_treat_error)
			exit(EXIT_FAILURE);
	}
}

inline void throw_if_null(void* _ptr)
{
	warning_if_null(_ptr, true);
}