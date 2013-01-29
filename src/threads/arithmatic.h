#ifndef FIXED_POINT_ARITMATIC_H
#define FIXED_POINT_ARITMATIC_H

/* This operations are implemented using a p.q fixed point format. I have 
   chosen p = 17, q = 14 as in the specification. F is just 1 << Q */

#define P 17
#define Q 14
#define F 16384

/* Conversion methods */
int64_t int_to_fp (int64_t fp);
int64_t fp_to_int_rt0 (int64_t x);
int64_t fp_to_int_rtn (int64_t x);

/* Arithmetic operations */
int64_t sum_two_fps (int64_t x, int64_t y);
int64_t sum_int_fp (int64_t i, int64_t fp);

int64_t diff_two_fps (int64_t x, int64_t y);
int64_t diff_int_fp (int64_t i, int64_t fp);

int64_t mul_two_fps (int64_t x, int64_t y);
int64_t mul_int_fp (int64_t i, int64_t fp);

int64_t div_two_fps (int64_t x, int64_t y);
int64_t div_int_fp (int64_t i, int64_t fp);

#endif