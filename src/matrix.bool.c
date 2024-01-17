#include <matrix.bool.h>
#include <mcstr.h>
#include <error.h>

#include <string.h>

void mc_matrix_copy_logical_scalar(mc_bool_t* dst, size_t dsti, size_t dstj, mc_bool_t* src, size_t srci, size_t srcj, size_t dm, size_t dn, size_t n1, size_t n2)
{
	for (size_t i = 0; i < dm; i++)
	{
		for (size_t j = 0; j < dn; j++)
		{
			dst[(dsti + i) * n1 + (dstj + j)] = src[(srci + i) * n2 + (srcj + j)];
		}
	}
}
void mc_array_copy_logical_scalar(mc_bool_t* dst, size_t dstj, mc_bool_t* src, size_t srcj, size_t length)
{
	for (size_t j = 0; j < length; j++)
	{ 
		dst[dstj + j] = src[srcj + j];
	}
}

void mc_matrix_all_logical_scalar(mc_bool_t* dest, mc_bool_t* src, size_t length)
{
	dest[0] = true;
	for (size_t i = 0; i < length; i++)
	{
		if (!src[i])
		{
			dest[0] = false;
			return;
		}
	}
}
void mc_matrix_transpose_logical_scalar(mc_bool_t* dest, mc_bool_t* src, size_t m, size_t n)
{
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			dest[i * n + j] = src[j * m + i];
		}
	}
}
void mc_matrix_and_logical_scalar(mc_bool_t* dest, mc_bool_t* left, mc_bool_t* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		dest[i] = left[i] & right[i];
	}
}
void mc_matrix_or_logical_scalar(mc_bool_t* dest, mc_bool_t* left, mc_bool_t* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		dest[i] = left[i] | right[i];
	}
}
void mc_matrix_neg_logical_scalar(mc_bool_t* dest, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		dest[i] = ~dest[i];
	}
}
void mc_matrix_double_to_logical_scalar(mc_bool_t* dst, double* src, size_t length) 
{
	for (size_t i = 0; i < length; i++)
	{
		dst[i] = src[i];
	}
}
void mc_matrix_str_to_logical_scalar(mc_bool_t* dst, mc_string_t* src, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		if (wcscmp(L"true", src[i]) == 0)
			dst[i] = true;
		else if (wcscmp(L"false", src[i]) == 0)
			dst[i] = false;
		else
			MC_ERROR_ABORT(0, L"Only 'true' and 'false' can be converted to logical value.", MC_ERROR_FAILURE);
	}
}