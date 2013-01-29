#ifndef FIXED_POINT_ARITMATIC_H
#define FIXED_POINT_ARITMATIC_H

#include <stdint.h>

/* These operations are implemented using a p.q fixed point format. I have 
   chosen p = 17, q = 14 as in the specification. F is just 1 << Q */

#define Q 14
#define F (1 << Q)

typedef int32_t fixed_point_t;

/* Conversion methods */
fixed_point_t int_to_fp (int32_t i);
int32_t fp_to_int_rt0 (fixed_point_t x);
int32_t fp_to_int_rtn (fixed_point_t x);

/* Arithmetic operations */
fixed_point_t sum_two_fps (fixed_point_t x, fixed_point_t y);
fixed_point_t sum_int_fp (int32_t i, fixed_point_t fp);

fixed_point_t diff_two_fps (fixed_point_t x, fixed_point_t y);
fixed_point_t diff_int_fp (int32_t i, fixed_point_t fp);

fixed_point_t mul_two_fps (fixed_point_t x, fixed_point_t y);
fixed_point_t mul_int_fp (int32_t i, fixed_point_t fp);

fixed_point_t div_two_fps (fixed_point_t x, fixed_point_t y);
fixed_point_t div_int_fp (int32_t i, fixed_point_t fp);

#endif