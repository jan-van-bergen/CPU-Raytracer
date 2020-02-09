#pragma once
#include "SIMD_floats.h"
#include "SIMD_ints.h"

#include "Config.h"

#if SIMD_LANE_SIZE == 1
typedef SIMD_float1 SIMD_float;
typedef SIMD_int1   SIMD_int;

inline FORCEINLINE SIMD_int   SIMD_float_to_int(SIMD_float floats) { return SIMD_int  (floats.data); }
inline FORCEINLINE SIMD_float SIMD_int_to_float(SIMD_int   ints)   { return SIMD_float(ints.data); }
#elif SIMD_LANE_SIZE == 4
typedef SIMD_float4 SIMD_float;
typedef SIMD_int4   SIMD_int;

inline FORCEINLINE SIMD_int   SIMD_float_to_int(SIMD_float floats) { return SIMD_int  (_mm_cvtps_epi32(floats.data)); }
inline FORCEINLINE SIMD_float SIMD_int_to_float(SIMD_int   ints)   { return SIMD_float(_mm_cvtepi32_ps(ints.data)); }
#elif SIMD_LANE_SIZE == 8
typedef SIMD_float8 SIMD_float;
typedef SIMD_int8   SIMD_int;

inline FORCEINLINE SIMD_int   SIMD_float_to_int(SIMD_float floats) { return SIMD_int  (_mm256_cvtps_epi32(floats.data)); }
inline FORCEINLINE SIMD_float SIMD_int_to_float(SIMD_int   ints)   { return SIMD_float(_mm256_cvtepi32_ps(ints.data)); }
#else
static_assert(false, "Unsupported Lane Size!");
#endif

inline FORCEINLINE SIMD_float SIMD_float::mod(const SIMD_float & v, const SIMD_float & m) { return SIMD_float(v - m * SIMD_float::floor(v / m)); }

inline FORCEINLINE SIMD_float SIMD_float::clamp(const SIMD_float & val, const SIMD_float & min, const SIMD_float & max) { return SIMD_float::min(SIMD_float::max(val, min), max); }
