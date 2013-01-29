#ifndef FIXED_POINT_ARITMATIC_H
#define FIXED_POINT_ARITMATIC_H

#include <stdint.h>

/* These operations are implemented using a p.q fixed point format. I have 
   chosen p = 17, q = 14 as in the specification. F is just 1 << Q */

#define P 17
#define Q 14
#define F 16384

/* Conversion methods */
int32_t int_to_fp (int32_t i);
int32_t fp_to_int_rt0 (int32_t x);
int32_t fp_to_int_rtn (int32_t x);

/* Arithmetic operations */
int32_t sum_two_fps (int32_t x, int32_t y);
int32_t sum_int_fp (int32_t i, int32_t fp);

int32_t diff_two_fps (int32_t x, int32_t y);
int32_t diff_int_fp (int32_t i, int32_t fp);

int32_t mul_two_fps (int32_t x, int32_t y);
int32_t mul_int_fp (int32_t i, int32_t fp);

int32_t div_two_fps (int32_t x, int32_t y);
int32_t div_int_fp (int32_t i, int32_t fp);

#endif