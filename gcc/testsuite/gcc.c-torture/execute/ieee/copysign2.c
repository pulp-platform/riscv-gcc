#include <string.h>
#include <stdlib.h>
#include <float.h>

#define fpsizeoff	sizeof(float)
#define fpsizeof	sizeof(double)
#define fpsizeofl	sizeof(long double)

/* Work around the fact that with the Intel double-extended precision,
   we've got a 10 byte type stuffed into some amount of padding.  And
   the fact that -ffloat-store is going to stuff this value temporarily
   into some bit of stack frame that we've no control over and can't zero.  */
#if LDBL_MANT_DIG == 64
# if defined(__i386__) || defined(__x86_64__) || defined (__ia64__)
#  undef fpsizeofl
#  define fpsizeofl	10
# endif
#endif



#define TEST(TYPE, EXT)						\
static TYPE Y##EXT[] = {					\
  2.0, -2.0, -2.0, -2.0, -2.0, 2.0, -0.0, __builtin_inf##EXT ()	\
};								\
static const TYPE Z##EXT[] = {					\
  1.0, -1.0, -1.0, -0.0, -0.0, 0.0, -__builtin_inf##EXT (),	\
  __builtin_nan##EXT ("")					\
};								\
								\
void test##EXT (void)						\
{								\
  TYPE r[8];							\
  int i;							\
  r[0] = __builtin_copysign##EXT (1.0, Y##EXT[0]);		\
  r[1] = __builtin_copysign##EXT (1.0, Y##EXT[1]);		\
  r[2] = __builtin_copysign##EXT (-1.0, Y##EXT[2]);		\
  r[3] = __builtin_copysign##EXT (0.0, Y##EXT[3]);		\
  r[4] = __builtin_copysign##EXT (-0.0, Y##EXT[4]);		\
  r[5] = __builtin_copysign##EXT (-0.0, Y##EXT[5]);		\
  r[6] = __builtin_copysign##EXT (__builtin_inf##EXT (), Y##EXT[6]); \
  r[7] = __builtin_copysign##EXT (-__builtin_nan##EXT (""), Y##EXT[7]); \
  for (i = 0; i < 8; ++i)					\
    if (memcmp (r+i, Z##EXT+i, fpsizeof##EXT) != 0)		\
      abort ();							\
}

TEST(float, f)
TEST(double, )
TEST(long double, l)

int main()
{
  testf();
  test();
  testl();
  return 0;
}
