#include "arithmetic.h"
#include <stdio.h>

/* Implementation of Fixed-Point Real Arithmetic operations as seen in
   Appendix B.6 of the PintOS specification.

   These real arithmetic operations are used in the advanced scheduler for 
   calculations like recent_cpu and load_avg */

/* Convert an integer to a fixed point number */
fixed_point_t int_to_fp (int32_t i)
{
    return i * F;
}

/* Convert an fixed point number to an integer, rounding down to 0 always */
int32_t fp_to_int_rt0 (fixed_point_t x)
{
    return x / F;
}

/* Convert fixed point to integer, rounding to nearest */
fixed_point_t fp_to_int_rtn (fixed_point_t x)
{
    if (x > 0)
        return (x + F / 2) / F;
    else
        return (x - F / 2) / F;
}

/* Sum two fixed point numbers */
fixed_point_t sum_two_fps (fixed_point_t x, fixed_point_t y)
{
    return x + y;
}

/* Sum an integer I with a fixed point number FP */
fixed_point_t sum_int_fp (int32_t i, fixed_point_t fp)
{
    return fp + i * F;
}

/* Subtract fixed point Y from fixed point X */
fixed_point_t diff_two_fps (fixed_point_t x, fixed_point_t y)
{
    return x - y;
}

/* Subtract integer I from fixed point FP */
fixed_point_t diff_int_fp (int32_t i, fixed_point_t fp)
{
    return fp - i * F;
}

/* Multiply two fixed point numbers */
fixed_point_t mul_two_fps (fixed_point_t x, fixed_point_t y)
{
    return ((int64_t) x) * y / F;
}

/* Multiply an integer I by a fixed point number FP */
fixed_point_t mul_int_fp (int32_t i, fixed_point_t fp)
{
    return i * fp;
}

/* Divide fp number X by fp number Y */
fixed_point_t div_two_fps (fixed_point_t x, fixed_point_t y)
{
    return ((int64_t) x) * F / y;
}

/* Divide a floating point number FP by an integer I */
fixed_point_t div_int_fp (int32_t i, fixed_point_t fp)
{
    return fp / i;
}
