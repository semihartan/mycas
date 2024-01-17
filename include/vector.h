#pragma once

#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector
{
	size_t length;
	double* data;
}vector_t;

vector_t* mcas_vector_alloc(size_t _length);
size_t mcas_vector_length(vector_t *_vec);

#endif