#pragma once

#ifndef MATRIX_BOOL_H
#define MATRIX_BOOL_H

#include <mc.h>

#include <gc/gc.h>

void mc_matrix_copy_logical_scalar(mc_bool_t* dst, size_t dsti, size_t dstj, mc_bool_t* src, size_t srci, size_t srcj, size_t dm, size_t dn, size_t n1, size_t n2);
void mc_array_copy_logical_scalar(mc_bool_t* dst, size_t dstj, mc_bool_t* src, size_t srcj, size_t length);
void mc_matrix_all_logical_scalar(mc_bool_t* dest, mc_bool_t* src, size_t len);
void mc_matrix_transpose_logical_scalar(mc_bool_t* dest, mc_bool_t* src, size_t m, size_t n);
void mc_matrix_and_logical_scalar(mc_bool_t* dst, mc_bool_t* left, mc_bool_t* right, size_t length);
void mc_matrix_or_logical_scalar(mc_bool_t* dst, mc_bool_t* left, mc_bool_t* right, size_t length);
void mc_matrix_neg_logical_scalar(mc_bool_t* dst, size_t length);
void mc_matrix_double_to_logical_scalar(mc_bool_t* dst, double* src, size_t length);
void mc_matrix_str_to_logical_scalar(mc_bool_t* dst, mc_string_t* src, size_t length);

#endif