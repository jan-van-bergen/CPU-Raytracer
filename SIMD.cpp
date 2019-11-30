#include "SIMD.h"

#define USE_SSE2
#include <mathfun/sse_mathfun.h>

#include <mathfun/avx_mathfun.h>

SIMD_float4 SIMD_float4::sin(SIMD_float4 floats) { return SIMD_float4(sin_ps(floats.data)); }
SIMD_float4 SIMD_float4::cos(SIMD_float4 floats) { return SIMD_float4(cos_ps(floats.data)); }
SIMD_float4 SIMD_float4::exp(SIMD_float4 floats) { return SIMD_float4(exp_ps(floats.data)); }

SIMD_float8 SIMD_float8::sin(SIMD_float8 floats) { return SIMD_float8(sin256_ps(floats.data)); }
SIMD_float8 SIMD_float8::cos(SIMD_float8 floats) { return SIMD_float8(cos256_ps(floats.data)); }
SIMD_float8 SIMD_float8::exp(SIMD_float8 floats) { return SIMD_float8(exp256_ps(floats.data)); }
