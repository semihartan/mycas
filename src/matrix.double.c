#include <matrix.double.h>

#include <mcmath.h>


void mc_matrix_copy_double_scalar(double* dst, size_t dsti, size_t dstj, double* src, size_t srci, size_t srcj, size_t dm, size_t dn, size_t n1, size_t n2)
{
	for (size_t i = 0; i < dm; i++)
	{
		for (size_t j = 0; j < dn; j++)
		{
			dst[(dsti + i) * n1 + (dstj + j)] = src[(srci + i) * n2 + (srcj + j)];
		}
	}
}
void mc_array_copy_double_scalar(double* dst, size_t dstj, double* src, size_t srcj, size_t length)
{
	for (size_t j = 0; j < length; j++)
	{
		dst[dstj + j] = src[srcj + j];
	}
}
void mc_matrix_all_double_scalar(mc_bool_t* dest, double* src, size_t length)
{
	dest[0] = true;
	for (size_t i = 0; i < length; i++)
	{
		if (src[i] == 0.0)
		{
			dest[0] = false;
			return;
		}
	}
}
void mc_matrix_transpose_double_scalar(double* dest, double* src, size_t m, size_t n)
{
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			dest[i * n + j] = src[j * m + i];
		}
	}
}
void mc_matrix_neg_double_scalar(double* dest, double* src, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = -src[i];
}
void mc_matrix_add_double_scalar(double* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] + right[i];
}
void mc_matrix_sub_double_scalar(double* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] - right[i];
}
void mc_matrix_mul_double_scalar(double* dest, double* left, double* right, size_t m, size_t n, size_t k)
{
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < k; j++)
		{
			double c = 0.0;
			for (size_t l = 0; l < n; l++)
			{
				c += left[i * n + l] + right[l * k + j];
			}
			dest[i * k + j] = c;
		}
	}
}
void mc_matrix_dotmul_double_scalar(double* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] * right[i];
}
void mc_matrix_dotdiv_double_scalar(double* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] / right[i];
}
void mc_matrix_dotpow_double_scalar(double* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = pow(left[i], right[i]);
}

void mc_matrix_lt_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] < right[i];
}
void mc_matrix_lte_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] <= right[i];
}
void mc_matrix_gt_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] > right[i];
}
void mc_matrix_gte_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length)
{
	for (size_t i = 0; i < length; i++)
		dest[i] = left[i] >= right[i];
}
void mc_matrix_logical_to_double_scalar(double* dst, mc_bool_t* src, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		dst[i] = src[i];
	}
}
void mc_matrix_str_to_double_scalar(double* dst, mc_string_t* src, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		dst[i] = wcstod(src[i], NULL);
	}
}