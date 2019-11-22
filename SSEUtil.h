#pragma once

#include <immintrin.h>
#include <xmmintrin.h>

// Negate packed scalars
#define _mm_neg_ps(x) _mm_sub_ps(_mm_set1_ps(0.0f), x)

#define ALL_FALSE    0
#define ALL_TRUE_128 0xf
#define ALL_TRUE_256 0xff
