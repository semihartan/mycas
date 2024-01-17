#pragma once

#ifndef MATRIX_DOUBLE_H
#define MATRIX_DOUBLE_H

#include <mc.h>
#include <error.h>

#include <gc/gc.h>

void mc_matrix_copy_double_scalar(double* dst, size_t dsti, size_t dstj, double* src, size_t srci, size_t srcj, size_t dm, size_t dn, size_t n1, size_t n2);
void mc_array_copy_double_scalar(double* dst, size_t dstj, double* src, size_t srcj, size_t length);

void mc_matrix_all_double_scalar(mc_bool_t* dest, double* src, size_t length);
void mc_matrix_transpose_double_scalar(double* dest, double* src, size_t m, size_t n);
void mc_matrix_neg_double_scalar(double* dest, double* src, size_t length);
void mc_matrix_add_double_scalar(double* dest, double* left, double* right, size_t length);
void mc_matrix_sub_double_scalar(double* dest, double* left, double* right, size_t length);
void mc_matrix_mul_double_scalar(double* dest, double* left, double* right, size_t m, size_t n, size_t k);
void mc_matrix_dotmul_double_scalar(double* dest, double* left, double* right, size_t length);
void mc_matrix_dotdiv_double_scalar(double* dest, double* left, double* right, size_t length);
void mc_matrix_dotpow_double_scalar(double* dest, double* left, double* right, size_t length);

void mc_matrix_lt_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length);
void mc_matrix_lte_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length);
void mc_matrix_gt_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length);
void mc_matrix_gte_double_scalar(mc_bool_t* dest, double* left, double* right, size_t length);
void mc_matrix_logical_to_double_scalar(double* dst, mc_bool_t* src, size_t length);
void mc_matrix_str_to_double_scalar(double* dst, mc_string_t* src, size_t length);

//void mc_matrix_gen_array_scalar(double* dest, size_t length, double start, double increment);

#endif