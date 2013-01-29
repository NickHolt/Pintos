#include "arithmetic.h"

/* Implementation of Fixed-Point Real Arithmetic operations as seen in
   Appendix B.6 of the PintOS specification.

   These real arithmetic operations are used in the advanced scheduler for 
   calculations like recent_cpu and load_avg */

/* Convert an integer to a fixed point number */
int32_t int_to_fp (int32_t i)
{
    return i * F;
}

/* Convert an fixed point number to an integer, rounding down to 0 always */
int32_t fp_to_int_rt0 (int32_t x)
{
    return x / F;
}

/* Convert fixed point to integer, rounding to nearest */
int32_t fp_to_int_rtn (int32_t x)
{
    if (x > 0)
        return (x + F / 2) / F;
    else
        return (x - F / 2) / F;
}

/* Sum two fixed point numbers */
int32_t sum_two_fps (int32_t x, int32_t y)
{
    return x + y;
}

/* Sum an integer I with a fixed point number FP */
int32_t sum_int_fp (int32_t i, int32_t fp)
{
    return fp + i * F;
}

/* Subtract fixed point Y from fixed point X */
int32_t diff_two_fps (int32_t x, int32_t y)
{
    return x - y;
}

/* Subtract integer I from fixed point FP */
int32_t diff_int_fp (int32_t i, int32_t fp)
{
    return fp - i * F;
}

/* Multiply two fixed point numbers */
int32_t mul_two_fps (int32_t x, int32_t y)
{
    return ((int64_t) x) * y / F;
}

/* Multiply an integer I by a fixed point number FP */
int32_t mul_int_fp (int32_t i, int32_t fp)
{
    return i * fp;
}

/* Divide fp number X by fp number Y */
int32_t div_two_fps (int32_t x, int32_t y)
{
    return ((int64_t) x) * F / y;
}

/* Divide a floating point number FP by an integer I */
int32_t div_int_fp (int32_t i, int32_t fp)
{
    return fp / i;
}
