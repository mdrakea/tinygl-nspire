#ifndef TINYGL_FIXED_H
#define TINYGL_FIXED_H

#include <stdint.h>

typedef int32_t GLfixed;

#define TGL_FIX_BITS 16
#define TGL_FIX_ONE ((GLfixed)1 << TGL_FIX_BITS)
#define TGL_FIX_HALF (TGL_FIX_ONE >> 1)
#define TGL_FIX_ZERO ((GLfixed)0)

#define TGL_FIX_PI ((GLfixed)205887)
#define TGL_FIX_2PI ((GLfixed)411775)

#define TGL_I(x) ((GLfixed)((int64_t)(x) * TGL_FIX_ONE))
#define TGL_FRAC(num, den) ((GLfixed)(((int64_t)(num) * TGL_FIX_ONE) / (den)))

static inline GLfixed tgl_fix_abs(GLfixed a)
{
  return a < 0 ? -a : a;
}

static inline GLfixed tgl_fix_mul(GLfixed a, GLfixed b)
{
  return (GLfixed)(((int64_t)a * (int64_t)b) >> TGL_FIX_BITS);
}

static inline GLfixed tgl_fix_div(GLfixed a, GLfixed b)
{
  if (b == 0) return 0;
  return (GLfixed)(((int64_t)a * TGL_FIX_ONE) / b);
}

static inline GLfixed tgl_fix_div_int(GLfixed a, int b)
{
  if (b == 0) return 0;
  return a / b;
}

static inline int tgl_fix_to_int(GLfixed a)
{
  if (a < 0) return -(int)((-a) >> TGL_FIX_BITS);
  return (int)(a >> TGL_FIX_BITS);
}

static inline int tgl_fix_mul_int_to_int(GLfixed a, int b)
{
  int64_t v = a * (int64_t)b;
  if (v < 0) return -(int)((-v) >> TGL_FIX_BITS);
  return (int)(v >> TGL_FIX_BITS);
}

static inline int tgl_fix_to_range(GLfixed a, int min, int max)
{
  return min + tgl_fix_mul_int_to_int(a, max - min);
}

static inline GLfixed tgl_fix_clamp(GLfixed a, GLfixed min, GLfixed max)
{
  if (a < min) return min;
  if (a > max) return max;
  return a;
}

static inline uint64_t tgl_isqrt_u64(uint64_t n)
{
  uint64_t root = 0;
  uint64_t bit = (uint64_t)1 << 62;

  while (bit > n) bit >>= 2;
  while (bit != 0) {
    if (n >= root + bit) {
      n -= root + bit;
      root = (root >> 1) + bit;
    } else {
      root >>= 1;
    }
    bit >>= 2;
  }

  return root;
}

static inline GLfixed tgl_fix_sqrt(GLfixed a)
{
  if (a <= 0) return 0;
  return (GLfixed)tgl_isqrt_u64((uint64_t)a << TGL_FIX_BITS);
}

static inline GLfixed tgl_fix_powi(GLfixed base, int exp)
{
  GLfixed result = TGL_FIX_ONE;

  if (exp <= 0) return result;
  while (exp != 0) {
    if (exp & 1) result = tgl_fix_mul(result, base);
    exp >>= 1;
    if (exp != 0) base = tgl_fix_mul(base, base);
  }

  return result;
}

#endif /* TINYGL_FIXED_H */
