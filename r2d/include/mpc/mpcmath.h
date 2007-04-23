/*
 * Musepack audio compression
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <mpc/mpc_types.h>

#ifndef M_PI
# define M_PI            3.1415926535897932384626433832795029     // 4*atan(1)
# define M_PIl           3.1415926535897932384626433832795029L
# define M_LN2           0.6931471805599453094172321214581766     // ln(2)
# define M_LN2l          0.6931471805599453094172321214581766L
# define M_LN10          2.3025850929940456840179914546843642     // ln 10 */
# define M_LN10l         2.3025850929940456840179914546843642L
#endif

// fast but maybe more inaccurate, use if you need speed
#if defined(__GNUC__) && !defined(__APPLE__)
#  define SIN(x)      sinf ((float)(x))
#  define COS(x)      cosf ((float)(x))
#  define ATAN2(x,y)  atan2f ((float)(x), (float)(y))
#  define SQRT(x)     sqrtf ((float)(x))
#  define LOG(x)      logf ((float)(x))
#  define LOG10(x)    log10f ((float)(x))
#  define POW(x,y)    expf (logf(x) * (y))
#  define POW10(x)    expf (M_LN10 * (x))
#  define FLOOR(x)    floorf ((float)(x))
#  define IFLOOR(x)   (int) floorf ((float)(x))
#  define FABS(x)     fabsf ((float)(x))
#else
# define SIN(x)      (float) sin (x)
# define COS(x)      (float) cos (x)
# define ATAN2(x,y)  (float) atan2 (x, y)
# define SQRT(x)     (float) sqrt (x)
# define LOG(x)      (float) log (x)
# define LOG10(x)    (float) log10 (x)
# define POW(x,y)    (float) pow (x,y)
# define POW10(x)    (float) pow (10., (x))
# define FLOOR(x)    (float) floor (x)
# define IFLOOR(x)   (int)   floor (x)
# define FABS(x)     (float) fabs (x)
#endif

#define SQRTF(x)     SQRT (x)
#define COSF(x)      COS (x)
#define ATAN2F(x,y)  ATAN2 (x,y)
#define IFLOORF(x)   IFLOOR (x)

typedef union mpc_floatint
{
	float   f;
	mpc_int32_t n;
} mpc_floatint;

static mpc_inline mpc_int32_t mpc_lrintf(float fVal)
{
	mpc_floatint tmp;
	tmp.f = fVal  + 0x00FF8000;
	return tmp.n - 0x4B7F8000;
}

static mpc_inline float mpc_nearbyintf(float fVal)
{
	return (float) mpc_lrintf(fVal);
}

