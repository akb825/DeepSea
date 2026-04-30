/*
 * Copyright 2026 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/MathImpl.h>
#include <DeepSea/Math/Sqrt.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Custom implementations for trigonometric functions.
 *
 * Custom implementations are provided both for performance and cross-platform consistent results
 * when deterministic math is enabled. SIMD versions are also provided to allow for parallelization
 * at the instruction level.
 *
 * Certain assumptions are made based on the inputs to avoid runtime checks, such as inputs being in
 * the valid range. As a result, specific return values for degenerate or invalid inputs may not be
 * respected when compared to the standard library versions. Angles are also assumed to not have
 * large magnitudes, where precision may suffer at magnitudes larger than 8192 for floats and 1.07e9
 * for doubles compared to more robust (but slower) implementations.
 *
 * Implementations are adapted from the Cephes library by Stephen L. Moshier, found at
 * http://www.moshier.net/ From the original library's readme: may be used freely but it comes with
 * no support or guarantee.
 */

/**
 * @brief Takes the sine of an angle.
 * @param angle The angle in radians.
 * @return The sine of the angle.
 */
DS_MATH_EXPORT inline float dsSinf(float angle);

/** @copydoc dsSinf() */
DS_MATH_EXPORT inline double dsSind(double angle);

/**
 * @brief Takes the cosine of an angle.
 * @param angle The angle in radians.
 * @return The cosine of the angle.
 */
DS_MATH_EXPORT inline float dsCosf(float angle);

/** @copydoc dsCosf() */
DS_MATH_EXPORT inline double dsCosd(double angle);

/**
 * @brief Takes the sine and cosine of an angle.
 * @param[out] outSin The sine of the angle.
 * @param[out] outCos The cosine of the angle.
 * @param angle The angle in radians.
 */
DS_MATH_EXPORT inline void dsSinCosf(float* outSin, float* outCos, float angle);

/** @copydoc dsSinCosf() */
DS_MATH_EXPORT inline void dsSinCosd(double* outSin, double* outCos, double angle);

/**
 * @brief Takes the tangent of an angle.
 * @param angle The angle in radians.
 * @return The tangent of the angle.
 */
DS_MATH_EXPORT inline float dsTanf(float angle);

/** @copydoc dsTanf() */
DS_MATH_EXPORT inline double dsTand(double angle);

/**
 * @brief Takes the arc sine of a value.
 * @param x The sine of the angle. This is assumed to be in the range [-1, 1].
 * @return The angle in the range [-pi/2, pi/2] where the sine yields x.
 */
DS_MATH_EXPORT inline float dsASinf(float x);

/** @copydoc dsASinf() */
DS_MATH_EXPORT inline double dsASind(double x);

/**
 * @brief Takes the arc cosine of a value.
 * @param x The cosine of the angle. This is assumed to be in the range [-1, 1].
 * @return The angle in the range [0, pi] where the cosine yields x.
 */
DS_MATH_EXPORT inline float dsACosf(float x);

/** @copydoc dsACosf() */
DS_MATH_EXPORT inline double dsACosd(double x);

/**
 * @brief Takes the arc tangent of a value.
 * @param x The tangent of the angle.
 * @return The angle in the range [-pi/2, pi/2] where the tangent yields x.
 */
DS_MATH_EXPORT inline float dsATanf(float x);

/** @copydoc dsATanf() */
DS_MATH_EXPORT inline double dsATand(double x);

/**
 * @brief Takes the arc tangent of the ratio between two values.
 *
 * Unlike the other arc trig functions, this allows for retrieving the original angle within the
 * full range of a circle. This also properly handles the exact values when x or y is 0, where the
 * base atan functions don't check for infinite values.
 *
 * @param y The numerator for the tangent of the angle, or sine of the angle.
 * @param x The denomenator for the tangent of the angle, or cosine of the angle.
 * @return The angle in the range [-pi, pi] where the tangent yields y/x.
 */
DS_MATH_EXPORT inline float dsATan2f(float y, float x);

/** @copydoc dsATan2f() */
DS_MATH_EXPORT inline double dsATan2d(double y, double x);

#if DS_HAS_SIMD

/**
 * @brief Takes the sine of four angles.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param angles The angles in radians.
 * @return The sine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4f dsSinSIMD4f(dsSIMD4f angles);

/**
 * @brief Takes the cosine of four angles.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param angles The angles in radians.
 * @return The cosine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4f dsCosSIMD4f(dsSIMD4f angles);

/**
 * @brief Takes the sine and cosine of four angles.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param[out] outSin The sine of the angles.
 * @param[out] outCos The cosine of the angles.
 * @param angle The angles in radians.
 */
DS_MATH_EXPORT inline void dsSinCosSIMD4f(dsSIMD4f* outSin, dsSIMD4f* outCos, dsSIMD4f angle);

/**
 * @brief Takes the tangent of four angles.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param angles The angles in radians.
 * @return The tangent of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4f dsTanSIMD4f(dsSIMD4f angles);

/**
 * @brief Takes the arc sine of four values.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param x The sine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the sine yields x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsASinSIMD4f(dsSIMD4f x);

/**
 * @brief Takes the arc cosine of four values.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param x The cosine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [0, pi] where the cosine yields x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsACosSIMD4f(dsSIMD4f x);

/**
 * @brief Takes the arc tangent of four values.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param x The tangent of the angles.
 * @return The angles in the range [-pi/2, pi/2] where the tangent yields x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsATanSIMD4f(dsSIMD4f x);

/**
 * @brief Takes the arc tangent of the ratio between two sets of four values.
 *
 * Unlike the other arc trig functions, this allows for retrieving the original angle within the
 * full range of a circle. This also properly handles the exact values when x or y is 0, where the
 * base atan functions don't check for infinite values.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param y The numerator for the tangent of the angles, or sine of the angles.
 * @param x The denomenator for the tangent of the angles, or cosine of the angles.
 * @return The angles in the range [-pi, pi] where the tangent yields y/x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsATan2SIMD4f(dsSIMD4f y, dsSIMD4f x);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Takes the sine of four angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param angles The angles in radians.
 * @return The sine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4f dsSinFMA4f(dsSIMD4f angles);

/**
 * @brief Takes the cosine of four angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param angles The angles in radians.
 * @return The cosine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4f dsCosFMA4f(dsSIMD4f angles);

/**
 * @brief Takes the sine and cosine of four angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param[out] outSin The sine of the angles.
 * @param[out] outCos The cosine of the angles.
 * @param angle The angles in radians.
 */
DS_MATH_EXPORT inline void dsSinCosFMA4f(dsSIMD4f* outSin, dsSIMD4f* outCos, dsSIMD4f angle);

/**
 * @brief Takes the tangent of four angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param angles The angles in radians.
 * @return The tangent of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4f dsTanFMA4f(dsSIMD4f angles);

/**
 * @brief Takes the arc sine of four values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param x The sine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the sine yields x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsASinFMA4f(dsSIMD4f x);

/**
 * @brief Takes the arc cosine of four values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param x The cosine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [0, pi] where the cosine yields x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsACosFMA4f(dsSIMD4f x);

/**
 * @brief Takes the arc tangent of four values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param x The tangent of the angles.
 * @return The angles in the range [-pi/2, pi/2] where the tangent yields x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsATanFMA4f(dsSIMD4f x);

/**
 * @brief Takes the arc tangent of the ratio between two sets of four values with fused multiply-add
 *     operations.
 *
 * Unlike the other arc trig functions, this allows for retrieving the original angle within the
 * full range of a circle. This also properly handles the exact values when x or y is 0, where the
 * base atan functions don't check for infinite values.
 *
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param y The numerator for the tangent of the angles, or sine of the angles.
 * @param x The denomenator for the tangent of the angles, or cosine of the angles.
 * @return The angles in the range [-pi, pi] where the tangent yields y/x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsATan2FMA4f(dsSIMD4f y, dsSIMD4f x);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Takes the sine of two angles.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param angles The angles in radians.
 * @return The sine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD2d dsSinSIMD2d(dsSIMD2d angles);

/**
 * @brief Takes the cosine of two angles.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param angles The angles in radians.
 * @return The cosine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD2d dsCosSIMD2d(dsSIMD2d angles);

/**
 * @brief Takes the sine and cosine of two angles.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param[out] outSin The sine of the angles.
 * @param[out] outCos The cosine of the angles.
 * @param angle The angles in radians.
 */
DS_MATH_EXPORT inline void dsSinCosSIMD2d(dsSIMD2d* outSin, dsSIMD2d* outCos, dsSIMD2d angle);

/**
 * @brief Takes the tangent of two angles.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param angles The angles in radians.
 * @return The tangent of the angles.
 */
DS_MATH_EXPORT inline dsSIMD2d dsTanSIMD2d(dsSIMD2d angles);

/**
 * @brief Takes the arc sine of two values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param x The sine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the sine yields x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsASinSIMD2d(dsSIMD2d x);

/**
 * @brief Takes the arc cosine of two values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param x The sine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the cosine yields x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsACosSIMD2d(dsSIMD2d x);

/**
 * @brief Takes the arc tangent of two values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param x The tangent of the angles.
 * @return The angles in the range [-pi/2, pi/2] where the tangent yields x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsATanSIMD2d(dsSIMD2d x);

/**
 * @brief Takes the arc tangent of the ratio between two sets of two values.
 *
 * Unlike the other arc trig functions, this allows for retrieving the original angle within the
 * full range of a circle. This also properly handles the exact values when x or y is 0, where the
 * base atan functions don't check for infinite values.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param y The numerator for the tangent of the angles, or sine of the angles.
 * @param x The denomenator for the tangent of the angles, or cosine of the angles.
 * @return The angles in the range [-pi, pi] where the tangent yields y/x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsATan2SIMD2d(dsSIMD2d y, dsSIMD2d x);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Takes the sine of two angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param angles The angles in radians.
 * @return The sine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD2d dsSinFMA2d(dsSIMD2d angles);

/**
 * @brief Takes the cosine of two angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param angles The angles in radians.
 * @return The cosine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD2d dsCosFMA2d(dsSIMD2d angles);

/**
 * @brief Takes the sine and cosine of two angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param[out] outSin The sine of the angles.
 * @param[out] outCos The cosine of the angles.
 * @param angle The angles in radians.
 */
DS_MATH_EXPORT inline void dsSinCosFMA2d(dsSIMD2d* outSin, dsSIMD2d* outCos, dsSIMD2d angle);

/**
 * @brief Takes the tangent of two angles with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param angles The angles in radians.
 * @return The tangent of the angles.
 */
DS_MATH_EXPORT inline dsSIMD2d dsTanFMA2d(dsSIMD2d angles);

/**
 * @brief Takes the arc sine of two values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param x The sine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the sine yields x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsASinFMA2d(dsSIMD2d x);

/**
 * @brief Takes the arc cosine of two values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param x The cosine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the cosine yields x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsACosFMA2d(dsSIMD2d x);

/**
 * @brief Takes the arc tangent of two values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param x The tangent of the angles.
 * @return The angles in the range [-pi/2, pi/2] where the tangent yields x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsATanFMA2d(dsSIMD2d x);

/**
 * @brief Takes the arc tangent of the ratio between two sets of two values with fused multiply-add
 *     operations.
 *
 * Unlike the other arc trig functions, this allows for retrieving the original angle within the
 * full range of a circle. This also properly handles the exact values when x or y is 0, where the
 * base atan functions don't check for infinite values.
 *
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and dsSIMDFeatures_FMA
 *     are available.
 * @param y The numerator for the tangent of the angles, or sine of the angles.
 * @param x The denomenator for the tangent of the angles, or cosine of the angles.
 * @return The angles in the range [-pi, pi] where the tangent yields y/x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsATan2FMA2d(dsSIMD2d y, dsSIMD2d x);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Takes the sine of four angles.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param angles The angles in radians.
 * @return The sine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4d dsSinSIMD4d(dsSIMD4d angles);

/**
 * @brief Takes the cosine of four angles.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param angles The angles in radians.
 * @return The cosine of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4d dsCosSIMD4d(dsSIMD4d angles);

/**
 * @brief Takes the sine and cosine of four angles.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param[out] outSin The sine of the angles.
 * @param[out] outCos The cosine of the angles.
 * @param angle The angles in radians.
 */
DS_MATH_EXPORT inline void dsSinCosSIMD4d(dsSIMD4d* outSin, dsSIMD4d* outCos, dsSIMD4d angle);

/**
 * @brief Takes the tangent of four angles.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param angles The angles in radians.
 * @return The tangent of the angles.
 */
DS_MATH_EXPORT inline dsSIMD4d dsTanSIMD4d(dsSIMD4d angles);

/**
 * @brief Takes the arc sine of four values.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param x The sine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the sine yields x.
 */
DS_MATH_EXPORT inline dsSIMD4d dsASinSIMD4d(dsSIMD4d x);

/**
 * @brief Takes the arc cosine of four values.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param x The cosine of the angles. These are assumed to be in the range [-1, 1].
 * @return The angles in the range [-pi/2, pi/2] where the cosine yields x.
 */
DS_MATH_EXPORT inline dsSIMD4d dsACosSIMD4d(dsSIMD4d x);

/**
 * @brief Takes the arc tangent of four values.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param x The tangent of the angles.
 * @return The angles in the range [-pi/2, pi/2] where the tangent yields x.
 */
DS_MATH_EXPORT inline dsSIMD4d dsATanSIMD4d(dsSIMD4d x);

/**
 * @brief Takes the arc tangent of the ratio between two sets of four values.
 *
 * Unlike the other arc trig functions, this allows for retrieving the original angle within the
 * full range of a circle. This also properly handles the exact values when x or y is 0, where the
 * base atan functions don't check for infinite values.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available, and
 *     will use FMA if not disabled through enabling determinisitic math.
 * @param y The numerator for the tangent of the angles, or sine of the angles.
 * @param x The denomenator for the tangent of the angles, or cosine of the angles.
 * @return The angles in the range [-pi, pi] where the tangent yields y/x.
 */
DS_MATH_EXPORT inline dsSIMD4d dsATan2SIMD4d(dsSIMD4d y, dsSIMD4d x);

#endif // DS_HAS_SIMD

/*
 * The angle values are normalized within the range [-pi/4, pi/4], and an integer is used to
 * determine which of these four pi/2 ranges is on the circle. Two Taylor expansions are used:
 * one for sine and the other for cosine, each in the angle range [-pi/4, pi/4]. As sine and cosine
 * are offsets of each-other, choosing which to use based on the angle quadrant and whether to
 * negate the result can give full coverage for both sine and cosine.
 *
 * The original code used octants rather than quadrants. However, these were used largely for
 * rounding purposes: an odd octant gets rounded to the next even octant. This can be simplified by
 * adding 1/2 before converting to an integer as a poor-man's round. This is more optimal on modern
 * hardware, where branches are much more expensive than floating-point operations, and can be used
 * for SIMD operations.
 */

/// @cond
/*
 * Factors for Cody-Waite argument reduction to map to the range [-pi/4, pi/4] while reducing
 * precision loss. Double values from Cephes as we divide into 4ths instead of 8ths.
 *
 * See:
 * https://stackoverflow.com/questions/42455143/sine-cosine-modular-extended-precision-arithmetic
 */
#define DS_PI_2f_1 1.5703125f
#define DS_PI_2f_2 4.83751296997e-4f
#define DS_PI_2f_3 7.54978995489e-8f

#define DS_PI_2d_1 1.5707962512969971
#define DS_PI_2d_2 7.5497894158615964e-8
#define DS_PI_2d_3 5.390302858158119e-15

#define DS_PI_2_BIT_EXTENSIONd 5.721188726109831840122e-18

#define DS_TAN_3_PI_8f 2.414213562373095f
#define DS_TAN_PI_8f 0.4142135623730950f

#define DS_TAN_3_PI_8d 2.41421356237309504880

// Taylor corefficients.
#define DS_SIN_TAYLOR_1f -1.9515295891e-4f
#define DS_SIN_TAYLOR_2f 8.3321608736e-3f
#define DS_SIN_TAYLOR_3f -1.6666654611e-1f

#define DS_SIN_TAYLOR_1d 1.58962301576546568060e-10
#define DS_SIN_TAYLOR_2d -2.50507477628578072866e-8
#define DS_SIN_TAYLOR_3d 2.75573136213857245213e-6
#define DS_SIN_TAYLOR_4d -1.98412698295895385996E-4
#define DS_SIN_TAYLOR_5d 8.33333333332211858878e-3
#define DS_SIN_TAYLOR_6d -1.66666666666666307295e-1

#define DS_COS_TAYLOR_1f 2.443315711809948e-5f
#define DS_COS_TAYLOR_2f -1.388731625493765e-3f
#define DS_COS_TAYLOR_3f 4.166664568298827e-2f

#define DS_COS_TAYLOR_1d -1.13585365213876817300E-11
#define DS_COS_TAYLOR_2d 2.08757008419747316778E-9
#define DS_COS_TAYLOR_3d -2.75573141792967388112E-7
#define DS_COS_TAYLOR_4d 2.48015872888517045348E-5
#define DS_COS_TAYLOR_5d -1.38888888888730564116E-3
#define DS_COS_TAYLOR_6d 4.16666666666665929218E-2

#define DS_TAN_TAYLOR_1f 9.38540185543e-3f
#define DS_TAN_TAYLOR_2f 3.11992232697e-3f
#define DS_TAN_TAYLOR_3f 2.44301354525e-2f
#define DS_TAN_TAYLOR_4f 5.34112807005e-2f
#define DS_TAN_TAYLOR_5f 1.33387994085e-1f
#define DS_TAN_TAYLOR_6f 3.33331568548e-1f

#define DS_TAN_TAYLOR_P_1d -1.30936939181383777646e4
#define DS_TAN_TAYLOR_P_2d 1.15351664838587416140e6
#define DS_TAN_TAYLOR_P_3d -1.79565251976484877988e7

#define DS_TAN_TAYLOR_Q_1d 1.36812963470692954678e4
#define DS_TAN_TAYLOR_Q_2d -1.32089234440210967447e6
#define DS_TAN_TAYLOR_Q_3d 2.50083801823357915839e7
#define DS_TAN_TAYLOR_Q_4d -5.38695755929454629881e7

#define DS_ASIN_TAYLOR_1f 4.2163199048e-2f
#define DS_ASIN_TAYLOR_2f 2.4181311049e-2f
#define DS_ASIN_TAYLOR_3f 4.5470025998e-2f
#define DS_ASIN_TAYLOR_4f 7.4953002686e-2f
#define DS_ASIN_TAYLOR_5f 1.6666752422e-1f

#define DS_ASIN_TAYLOR_P_1d 4.253011369004428248960e-3
#define DS_ASIN_TAYLOR_P_2d -6.019598008014123785661e-1
#define DS_ASIN_TAYLOR_P_3d 5.444622390564711410273e0
#define DS_ASIN_TAYLOR_P_4d -1.626247967210700244449e1
#define DS_ASIN_TAYLOR_P_5d 1.956261983317594739197e1
#define DS_ASIN_TAYLOR_P_6d -8.198089802484824371615

#define DS_ASIN_TAYLOR_Q_1d -1.474091372988853791896e1
#define DS_ASIN_TAYLOR_Q_2d 7.049610280856842141659e1
#define DS_ASIN_TAYLOR_Q_3d -1.471791292232726029859e2
#define DS_ASIN_TAYLOR_Q_4d 1.395105614657485689735e2
#define DS_ASIN_TAYLOR_Q_5d -4.918853881490881290097e1

#define DS_ASIN_TAYLOR_R_1d 2.967721961301243206100e-3
#define DS_ASIN_TAYLOR_R_2d -5.634242780008963776856e-1
#define DS_ASIN_TAYLOR_R_3d 6.968710824104713396794
#define DS_ASIN_TAYLOR_R_4d -2.556901049652824852289e1
#define DS_ASIN_TAYLOR_R_5d 2.853665548261061424989e1

#define DS_ASIN_TAYLOR_S_1d -2.194779531642920639778e1
#define DS_ASIN_TAYLOR_S_2d 1.470656354026814941758e2
#define DS_ASIN_TAYLOR_S_3d -3.838770957603691357202e2
#define DS_ASIN_TAYLOR_S_4d 3.424398657913078477438e2

#define DS_ATAN_TAYLOR_1f 8.05374449538e-2f
#define DS_ATAN_TAYLOR_2f -1.38776856032e-1f
#define DS_ATAN_TAYLOR_3f 1.99777106478e-1f
#define DS_ATAN_TAYLOR_4f -3.33329491539e-1f

#define DS_ATAN_TAYLOR_P_1d -8.750608600031904122785e-1
#define DS_ATAN_TAYLOR_P_2d -1.615753718733365076637e1
#define DS_ATAN_TAYLOR_P_3d -7.500855792314704667340e1
#define DS_ATAN_TAYLOR_P_4d -1.228866684490136173410e2
#define DS_ATAN_TAYLOR_P_5d -6.485021904942025371773e1

#define DS_ATAN_TAYLOR_Q_1d 2.485846490142306297962e1
#define DS_ATAN_TAYLOR_Q_2d 1.650270098316988542046e2
#define DS_ATAN_TAYLOR_Q_3d 4.328810604912902668951e2
#define DS_ATAN_TAYLOR_Q_4d 4.853903996359136964868e2
#define DS_ATAN_TAYLOR_Q_5d 1.945506571482613964425e2

DS_ALWAYS_INLINE float dsTrigQuadrantAnglef(float absAngle, float quadrant)
{
	return ((absAngle - quadrant*DS_PI_2f_1) - quadrant*DS_PI_2f_2) - quadrant*DS_PI_2f_3;
}

DS_ALWAYS_INLINE double dsTrigQuadrantAngled(double absAngle, double quadrant)
{
	return ((absAngle - quadrant*DS_PI_2d_1) - quadrant*DS_PI_2d_2) - quadrant*DS_PI_2d_3;
}

DS_ALWAYS_INLINE void dsSinCosfImpl(
	float* outSinTaylor, float* outCosTaylor, uint32_t* outQuadrant, float angle)
{
	float absAngle = fabsf(angle);
	// Use truncation to perform rounding.
	*outQuadrant = (uint32_t)(absAngle*M_2_PIf + 0.5f);

	float quadrantAngle = dsTrigQuadrantAnglef(absAngle, (float)*outQuadrant);
	float quadrantAngle2 = dsPow2(quadrantAngle);
	*outSinTaylor = ((DS_SIN_TAYLOR_1f*quadrantAngle2 + DS_SIN_TAYLOR_2f)*quadrantAngle2 +
		DS_SIN_TAYLOR_3f)*quadrantAngle2*quadrantAngle + quadrantAngle;
	*outCosTaylor = ((DS_COS_TAYLOR_1f*quadrantAngle2 + DS_COS_TAYLOR_2f)*quadrantAngle2 +
		DS_COS_TAYLOR_3f)*quadrantAngle2*quadrantAngle2 - 0.5f*quadrantAngle2 + 1.0f;
}

DS_ALWAYS_INLINE void dsSinCosdImpl(
	double* outSinTaylor, double* outCosTaylor, uint64_t* outQuadrant, double angle)
{
	double absAngle = fabs(angle);
	// Use truncation to perform rounding.
	*outQuadrant = (uint64_t)(absAngle*M_2_PI + 0.5);

	double quadrantAngle = dsTrigQuadrantAngled(absAngle, (double)*outQuadrant);
	double quadrantAngle2 = dsPow2(quadrantAngle);
	*outSinTaylor = (((((DS_SIN_TAYLOR_1d*quadrantAngle2 + DS_SIN_TAYLOR_2d)*quadrantAngle2 +
		DS_SIN_TAYLOR_3d)*quadrantAngle2 + DS_SIN_TAYLOR_4d)*quadrantAngle2 +
		DS_SIN_TAYLOR_5d)*quadrantAngle2 + DS_SIN_TAYLOR_6d)*quadrantAngle2*quadrantAngle +
		quadrantAngle;
	*outCosTaylor = (((((DS_COS_TAYLOR_1d*quadrantAngle2 + DS_COS_TAYLOR_2d)*quadrantAngle2 +
		DS_COS_TAYLOR_3d)*quadrantAngle2 + DS_COS_TAYLOR_4d)*quadrantAngle2 +
		DS_COS_TAYLOR_5d)*quadrantAngle2 + DS_COS_TAYLOR_6d)*quadrantAngle2*quadrantAngle2 -
		0.5*quadrantAngle2 + 1.0;
}
/// @endcond

DS_MATH_EXPORT inline float dsSinf(float angle)
{
	float sinTaylor, cosTaylor;
	uint32_t quadrant;
	dsSinCosfImpl(&sinTaylor, &cosTaylor, &quadrant, angle);

	// Ordering: sin, cos, -sin, -cos; negate if angle is negated.
	uint32_t evenOdd = quadrant & 0x1;
	uint32_t halfSign = (quadrant & 0x2) << 30;
	uint32_t sign = halfSign ^ dsMathImplExtractSignBitf(angle);
	return dsMathImplConditionalNegatef(dsMathImplSelectf(evenOdd, cosTaylor, sinTaylor), sign);
}

DS_MATH_EXPORT inline double dsSind(double angle)
{
	double sinTaylor, cosTaylor;
	uint64_t quadrant;
	dsSinCosdImpl(&sinTaylor, &cosTaylor, &quadrant, angle);

	// Ordering: sin, cos, -sin, -cos; negate if angle is negated.
	uint64_t evenOdd = quadrant & 0x1;
	uint64_t halfSign = (quadrant & 0x2) << 62;
	uint64_t sign = halfSign ^ dsMathImplExtractSignBitd(angle);
	return dsMathImplConditionalNegated(dsMathImplSelectd(evenOdd, cosTaylor, sinTaylor), sign);
}

DS_MATH_EXPORT inline float dsCosf(float angle)
{
	float sinTaylor, cosTaylor;
	uint32_t quadrant;
	dsSinCosfImpl(&sinTaylor, &cosTaylor, &quadrant, angle);

	// Ordering: cos, -sin, -cos, sin
	uint32_t evenOdd = quadrant & 0x1;
	uint32_t halfSign = (quadrant & 0x2) << 30;
	uint32_t sign = halfSign ^ (evenOdd << 31);
	return dsMathImplConditionalNegatef(dsMathImplSelectf(evenOdd, sinTaylor, cosTaylor), sign);
}

DS_MATH_EXPORT inline double dsCosd(double angle)
{
	double sinTaylor, cosTaylor;
	uint64_t quadrant;
	dsSinCosdImpl(&sinTaylor, &cosTaylor, &quadrant, angle);

	// Ordering: cos, -sin, -cos, sin
	uint64_t evenOdd = quadrant & 0x1;
	uint64_t halfSign = (quadrant & 0x2) << 62;
	uint64_t sign = halfSign ^ (evenOdd << 63);
	return dsMathImplConditionalNegated(dsMathImplSelectd(evenOdd, sinTaylor, cosTaylor), sign);
}

DS_MATH_EXPORT inline void dsSinCosf(float* outSin, float* outCos, float angle)
{
	DS_ASSERT(outSin);
	DS_ASSERT(outCos);

	float sinTaylor, cosTaylor;
	uint32_t quadrant;
	dsSinCosfImpl(&sinTaylor, &cosTaylor, &quadrant, angle);

	// Ordering for quadrants is:
	// Sine: sin, cos, -sin, -cos
	// Cosine: cos, -sin, -cos, sin
	uint32_t evenOdd = quadrant & 0x1;
	uint32_t halfSign = (quadrant & 0x2) << 30;
	// Match against 2 or 3.
	uint32_t sinSign = halfSign ^ dsMathImplExtractSignBitf(angle);
	// Match against 1 or 2.
	uint32_t cosSign = halfSign ^ (evenOdd << 31);

	*outSin = dsMathImplConditionalNegatef(
		dsMathImplSelectf(evenOdd, cosTaylor, sinTaylor), sinSign);
	*outCos = dsMathImplConditionalNegatef(
		dsMathImplSelectf(evenOdd, sinTaylor, cosTaylor), cosSign);
}

DS_MATH_EXPORT inline void dsSinCosd(double* outSin, double* outCos, double angle)
{
	DS_ASSERT(outSin);
	DS_ASSERT(outCos);

	double sinTaylor, cosTaylor;
	uint64_t quadrant;
	dsSinCosdImpl(&sinTaylor, &cosTaylor, &quadrant, angle);

	// Ordering for quadrants is:
	// Sine: sin, cos, -sin, -cos
	// Cosine: cos, -sin, -cos, sin
	uint64_t evenOdd = quadrant & 0x1;
	uint64_t halfSign = (quadrant & 0x2) << 62;
	// Match against 2 or 3.
	uint64_t sinSign = halfSign ^ dsMathImplExtractSignBitd(angle);
	// Match against 1 or 2.
	uint64_t cosSign = halfSign ^ (evenOdd << 63);

	*outSin = dsMathImplConditionalNegated(
		dsMathImplSelectd(evenOdd, cosTaylor, sinTaylor), sinSign);
	*outCos = dsMathImplConditionalNegated(
		dsMathImplSelectd(evenOdd, sinTaylor, cosTaylor), cosSign);
}

DS_MATH_EXPORT inline float dsTanf(float angle)
{
	float absAngle = fabsf(angle);
	// Use truncation to perform rounding.
	uint32_t quadrant = (uint32_t)(absAngle*M_2_PIf + 0.5f);

	float quadrantAngle = dsTrigQuadrantAnglef(absAngle, (float)quadrant);
	float quadrantAngle2 = dsPow2(quadrantAngle);

	float tanTaylor = (((((DS_TAN_TAYLOR_1f*quadrantAngle2 + DS_TAN_TAYLOR_2f)*quadrantAngle2 +
		DS_TAN_TAYLOR_3f)*quadrantAngle2 + DS_TAN_TAYLOR_4f)*quadrantAngle2 +
		DS_TAN_TAYLOR_5f)*quadrantAngle2 + DS_TAN_TAYLOR_6f)*quadrantAngle2*quadrantAngle +
		quadrantAngle;

	// Ordering of quadrants is: tan, -1/tan, tan, -1/tan
	uint32_t evenOdd = quadrant & 0x1;
	return dsMathImplConditionalNegatef(dsMathImplSelectf(evenOdd, -1.0f/tanTaylor, tanTaylor),
		dsMathImplExtractSignBitf(angle));
}

DS_MATH_EXPORT inline double dsTand(double angle)
{
	double absAngle = fabs(angle);
	// Use truncation to perform rounding.
	uint64_t quadrant = (uint64_t)(absAngle*M_2_PI + 0.5);

	double quadrantAngle = dsTrigQuadrantAngled(absAngle, (double)quadrant);
	double quadrantAngle2 = dsPow2(quadrantAngle);

	// Use a rational function rather than raw Taylor expansion.
	double pTaylor = (DS_TAN_TAYLOR_P_1d*quadrantAngle2 + DS_TAN_TAYLOR_P_2d)*quadrantAngle2 +
		DS_TAN_TAYLOR_P_3d;
	double qTaylor = (((quadrantAngle2 + DS_TAN_TAYLOR_Q_1d)*quadrantAngle2 +
		DS_TAN_TAYLOR_Q_2d)*quadrantAngle2 + DS_TAN_TAYLOR_Q_3d)*quadrantAngle2 +
		DS_TAN_TAYLOR_Q_4d;
	double tanRational = pTaylor/qTaylor*quadrantAngle2*quadrantAngle + quadrantAngle;

	// Ordering of quadrants is: tan, -1/tan, tan, -1/tan
	uint64_t evenOdd = quadrant & 0x1;
	return dsMathImplConditionalNegated(dsMathImplSelectd(evenOdd, -1.0/tanRational, tanRational),
		dsMathImplExtractSignBitd(angle));
}

DS_MATH_EXPORT inline float dsASinf(float x)
{
	DS_ASSERT(x >= -1.0f && x <= 1.0f);

	float absX = fabsf(x);

	// Remap the range for larger values based on the identity
	// asin(x) = pi/2 - 2*asin(sqrt((1 - x)/2)).
	uint32_t adjustRange = absX > 0.5f;
	float adjustedX = dsSqrtf(0.5f*(1.0f - absX));
	absX = dsMathImplSelectf(adjustRange, adjustedX, absX);
	float x2 = dsPow2(absX);

	float asinTaylor = ((((DS_ASIN_TAYLOR_1f*x2 + DS_ASIN_TAYLOR_2f)*x2 + DS_ASIN_TAYLOR_3f)*x2 +
		DS_ASIN_TAYLOR_4f)*x2 + DS_ASIN_TAYLOR_5f)*x2*absX + absX;
	float adjustedASinTaylor = M_PI_2f - (asinTaylor + asinTaylor);

	return dsMathImplConditionalNegatef(
		dsMathImplSelectf(adjustRange, adjustedASinTaylor, asinTaylor),
		dsMathImplExtractSignBitf(x));
}

DS_MATH_EXPORT inline double dsASind(double x)
{
	DS_ASSERT(x >= -1.0 && x <= 1.0);

	double absX = fabs(x);

	// Choose which rational function to use based on the value.
	uint64_t adjustRange = absX > 0.625;

	double x2 = dsPow2(x);
	double pTaylor = ((((DS_ASIN_TAYLOR_P_1d*x2 + DS_ASIN_TAYLOR_P_2d)*x2 +
		DS_ASIN_TAYLOR_P_3d)*x2 + DS_ASIN_TAYLOR_P_4d)*x2 + DS_ASIN_TAYLOR_P_5d)*x2 +
		DS_ASIN_TAYLOR_P_6d;
	double qTaylor = ((((x2 + DS_ASIN_TAYLOR_Q_1d)*x2 + DS_ASIN_TAYLOR_Q_2d)*x2 +
		DS_ASIN_TAYLOR_Q_3d)*x2 + DS_ASIN_TAYLOR_Q_4d)*x2 + DS_ASIN_TAYLOR_Q_5d;
	double asinRational = pTaylor/qTaylor*x2*absX + absX;

	// Compute based on asin(1 - x).
	double adjustedX = 1.0 - absX;
	double rTaylor = (((DS_ASIN_TAYLOR_R_1d*adjustedX + DS_ASIN_TAYLOR_R_2d)*adjustedX +
		DS_ASIN_TAYLOR_R_3d)*adjustedX + DS_ASIN_TAYLOR_R_4d)*adjustedX + DS_ASIN_TAYLOR_R_5d;
	double sTaylor = (((adjustedX + DS_ASIN_TAYLOR_S_1d)*adjustedX +
		DS_ASIN_TAYLOR_S_2d)*adjustedX + DS_ASIN_TAYLOR_S_3d)*adjustedX + DS_ASIN_TAYLOR_S_4d;
	double adjustedBaseRational = rTaylor/sTaylor*adjustedX;
	double adjustedSqrt2X = dsSqrtd(adjustedX + adjustedX);
	// Take pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational) in a way that extends the bits
	// of precision.
	double adjustedASinRational = M_PI_4 - adjustedSqrt2X -
		(adjustedSqrt2X*adjustedBaseRational - DS_PI_2_BIT_EXTENSIONd) + M_PI_4;

	return dsMathImplConditionalNegated(
		dsMathImplSelectd(adjustRange, adjustedASinRational, asinRational),
		dsMathImplExtractSignBitd(x));
}

DS_MATH_EXPORT inline float dsACosf(float x)
{
	DS_ASSERT(x >= -1.0f && x <= 1.0f);

	float absX = fabsf(x);
	uint32_t signBit = dsMathImplExtractSignBitf(x);

	// Remap the range for larger values based on the identity
	// asin(x) = pi/2 - 2*asin(sqrt((1 - x)/2)).
	uint32_t adjustRange = absX > 0.5f;
	float adjustedX = dsSqrtf(0.5f*(1.0f - absX));
	x = dsMathImplSelectf(adjustRange, dsMathImplConditionalNegatef(adjustedX, signBit), x);
	float x2 = dsPow2(x);

	float asinTaylor = ((((DS_ASIN_TAYLOR_1f*x2 + DS_ASIN_TAYLOR_2f)*x2 + DS_ASIN_TAYLOR_3f)*x2 +
		DS_ASIN_TAYLOR_4f)*x2 + DS_ASIN_TAYLOR_5f)*x2*x + x;

	// Use the identity that acos(x) = pi/2 - asin(x), applying it directly rather than calling
	// dsASinf() as that could introduce error for the adjusted range.
	uint32_t signMask = -(signBit >> 31);
	float acosTaylor = M_PI_2f - asinTaylor;
	// Make sure to add the pi/2 factors together first to reduce precision errors.
	float adjustedACosTaylor = dsMathImplMaskf(signMask, M_PIf) + (asinTaylor + asinTaylor);

	return dsMathImplSelectf(adjustRange, adjustedACosTaylor, acosTaylor);
}

DS_MATH_EXPORT inline double dsACosd(double x)
{
	DS_ASSERT(x >= -1.0 && x <= 1.0);

	double absX = fabs(x);

	// Choose which rational function to use based on the value.
	// Use the identity that acos(x) = pi/2 - asin(x), applying it directly rather than calling
	// dsASinf() as that could introduce error for the adjusted range.
	uint64_t adjustRange = absX > 0.625;

	double x2 = dsPow2(x);
	double pTaylor = ((((DS_ASIN_TAYLOR_P_1d*x2 + DS_ASIN_TAYLOR_P_2d)*x2 +
		DS_ASIN_TAYLOR_P_3d)*x2 + DS_ASIN_TAYLOR_P_4d)*x2 + DS_ASIN_TAYLOR_P_5d)*x2 +
		DS_ASIN_TAYLOR_P_6d;
	double qTaylor = ((((x2 + DS_ASIN_TAYLOR_Q_1d)*x2 + DS_ASIN_TAYLOR_Q_2d)*x2 +
		DS_ASIN_TAYLOR_Q_3d)*x2 + DS_ASIN_TAYLOR_Q_4d)*x2 + DS_ASIN_TAYLOR_Q_5d;
	double acosRational = M_PI_2 - pTaylor/qTaylor*x2*x - x;

	// Compute based on asin(1 - x).
	uint64_t signBit = dsMathImplExtractSignBitd(x);
	uint64_t signMask = -(signBit >> 63);
	double adjustedX = 1.0 - absX;

	double rTaylor = (((DS_ASIN_TAYLOR_R_1d*adjustedX + DS_ASIN_TAYLOR_R_2d)*adjustedX +
		DS_ASIN_TAYLOR_R_3d)*adjustedX + DS_ASIN_TAYLOR_R_4d)*adjustedX + DS_ASIN_TAYLOR_R_5d;
	double sTaylor = (((adjustedX + DS_ASIN_TAYLOR_S_1d)*adjustedX +
		DS_ASIN_TAYLOR_S_2d)*adjustedX + DS_ASIN_TAYLOR_S_3d)*adjustedX + DS_ASIN_TAYLOR_S_4d;
	double adjustedBaseRational = rTaylor/sTaylor*adjustedX;
	double adjustedSqrt2X = dsMathImplConditionalNegated(dsSqrtd(adjustedX + adjustedX), signBit);
	// Take pi/2 - (applySign(pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational))) in a way that
	// extends the bits of precision. The sign adjustment for the factor with the bit extension
	// is provided by adjustedSqrt2X. As a result, need do
	// "- adjustSign(extesnion - adjustSign(extension))", which resolvees to
	// "+ extension - adjustSign(extension)", or conditionally adding 2x extension.
	double halfConstantFactor = dsMathImplMaskd(signMask, M_PI_2);
	double bitExtensionFactor = dsMathImplMaskd(signMask, DS_PI_2_BIT_EXTENSIONd*2);
	double adjustedACosRational = halfConstantFactor + adjustedSqrt2X +
		(adjustedSqrt2X*adjustedBaseRational + bitExtensionFactor) + halfConstantFactor;

	return dsMathImplSelectd(adjustRange, adjustedACosRational, acosRational);
}

DS_MATH_EXPORT inline float dsATanf(float x)
{
	float absX = fabsf(x);

	// Choose a range to compute the taylor on.
	float largeX = -1.0f/absX;
	float midX = (absX - 1.0f)/(absX + 1.0f);

	uint32_t isLarge = absX > DS_TAN_3_PI_8f;
	uint32_t atLeastMid = absX > DS_TAN_PI_8f;
	uint32_t atLeastMidMask = ~(atLeastMid - 1);
	absX = dsMathImplSelectf(isLarge, largeX, dsMathImplSelectf(atLeastMid, midX, absX));
	float offset = dsMathImplMaskf(atLeastMidMask, dsMathImplSelectf(isLarge, M_PI_2f, M_PI_4f));

	float x2 = dsPow2(absX);
	float atanTaylor = (((DS_ATAN_TAYLOR_1f*x2 + DS_ATAN_TAYLOR_2f)*x2 +
		DS_ATAN_TAYLOR_3f)*x2 + DS_ATAN_TAYLOR_4f)*x2*absX + absX + offset;
	return dsMathImplConditionalNegatef(atanTaylor, dsMathImplExtractSignBitf(x));
}

DS_MATH_EXPORT inline double dsATand(double x)
{
	double absX = fabs(x);

	// Choose a range to compute the taylor on.
	double largeX = -1.0/absX;
	double midX = (absX - 1.0)/(absX + 1.0);

	uint64_t isLarge = absX > DS_TAN_3_PI_8d;
	uint64_t atLeastMid = absX > 0.66;
	uint64_t atLeastMidMask = ~(atLeastMid - 1);
	absX = dsMathImplSelectd(isLarge, largeX, dsMathImplSelectd(atLeastMid, midX, absX));
	double offset = dsMathImplMaskd(atLeastMidMask, dsMathImplSelectd(isLarge, M_PI_2, M_PI_4));
	double bitExtension = dsMathImplMaskd(atLeastMidMask,
		dsMathImplSelectd(isLarge, DS_PI_2_BIT_EXTENSIONd, DS_PI_2_BIT_EXTENSIONd*0.5));

	double x2 = dsPow2(absX);
	double pTaylor = (((DS_ATAN_TAYLOR_P_1d*x2 + DS_ATAN_TAYLOR_P_2d)*x2 + DS_ATAN_TAYLOR_P_3d)*x2 +
		DS_ATAN_TAYLOR_P_4d)*x2 + DS_ATAN_TAYLOR_P_5d;
	double qTaylor = ((((x2 + DS_ATAN_TAYLOR_Q_1d)*x2 + DS_ATAN_TAYLOR_Q_2d)*x2 +
		DS_ATAN_TAYLOR_Q_3d)*x2 + DS_ATAN_TAYLOR_Q_4d)*x2 + DS_ATAN_TAYLOR_Q_5d;
	double atanRational = pTaylor/qTaylor*x2*absX + absX + bitExtension + offset;
	return dsMathImplConditionalNegated(atanRational, dsMathImplExtractSignBitd(x));
}

DS_MATH_EXPORT inline float dsATan2f(float y, float x)
{
	uint32_t ySign = dsMathImplExtractSignBitf(y);
	uint32_t xSign = dsMathImplExtractSignBitf(x);
	uint32_t xNegativeMask = ~((xSign >> 31) - 1);
	// Branches are faster for zero checks with branch predictor due to being very infrequent.
	if (y == 0)
		return dsMathImplConditionalNegatef(dsMathImplMaskf(xNegativeMask, M_PIf), ySign);
	if (x == 0)
		return dsMathImplConditionalNegatef(M_PI_2f, ySign);

	// When x is positive, take the raw atan value. If x is negative, add or subtract pi when y is
	// positive or negative.
	float offset = dsMathImplMaskf(xNegativeMask, dsMathImplConditionalNegatef(M_PIf, ySign));
	return dsATanf(y/x) + offset;
}

DS_MATH_EXPORT inline double dsATan2d(double y, double x)
{
	uint64_t xSign = dsMathImplExtractSignBitd(x);
	uint64_t ySign = dsMathImplExtractSignBitd(y);
	uint64_t xNegativeMask = ~((xSign >> 63) - 1);
	// Branches are faster for zero checks with branch predictor due to being very infrequent.
	if (y == 0)
		return dsMathImplConditionalNegated(dsMathImplMaskd(xNegativeMask, M_PI), ySign);
	if (x == 0)
		return dsMathImplConditionalNegated(M_PI_2, ySign);

	// When x is positive, take the raw atan value. If x is negative, add or subtract pi when y is
	// positive or negative.
	double offset = dsMathImplMaskd(xNegativeMask, dsMathImplConditionalNegated(M_PI, ySign));
	return dsATand(y/x) + offset;
}

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT)

/// @cond
DS_ALWAYS_INLINE dsSIMD4f dsTrigQuadrantAngleSIMD4f(dsSIMD4f absAngles, dsSIMD4f quadrants)
{
	return dsSIMD4f_sub(dsSIMD4f_sub(dsSIMD4f_sub(absAngles,
		dsSIMD4f_mul(quadrants, dsSIMD4f_set1(DS_PI_2f_1))),
		dsSIMD4f_mul(quadrants, dsSIMD4f_set1(DS_PI_2f_2))),
		dsSIMD4f_mul(quadrants, dsSIMD4f_set1(DS_PI_2f_3)));
}

DS_ALWAYS_INLINE void dsSinCosSIMD4fImpl(
	dsSIMD4f* outSinTaylor, dsSIMD4f* outCosTaylor, dsSIMD4fb* outQuadrants, dsSIMD4f angles)
{
	dsSIMD4f twoOverPi = dsSIMD4f_set1(M_2_PIf);
	dsSIMD4f half = dsSIMD4f_set1(0.5f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f sinTaylor1 = dsSIMD4f_set1(DS_SIN_TAYLOR_1f);
	dsSIMD4f sinTaylor2 = dsSIMD4f_set1(DS_SIN_TAYLOR_2f);
	dsSIMD4f sinTaylor3 = dsSIMD4f_set1(DS_SIN_TAYLOR_3f);

	dsSIMD4f cosTaylor1 = dsSIMD4f_set1(DS_COS_TAYLOR_1f);
	dsSIMD4f cosTaylor2 = dsSIMD4f_set1(DS_COS_TAYLOR_2f);
	dsSIMD4f cosTaylor3 = dsSIMD4f_set1(DS_COS_TAYLOR_3f);

	dsSIMD4f absAngles = dsSIMD4f_abs(angles);
#if DS_DETERMINISTIC_MATH
	// Not all platforms will use the same rounding mode, so round through truncation.
	*outQuadrants = dsSIMD4fb_fromFloat(dsSIMD4f_add(dsSIMD4f_mul(absAngles, twoOverPi), half));
#else
	*outQuadrants = dsSIMD4fb_round(dsSIMD4f_mul(absAngles, twoOverPi));
#endif

	dsSIMD4f quadrantAngles = dsTrigQuadrantAngleSIMD4f(
		absAngles, dsSIMD4fb_toFloat(*outQuadrants));
	dsSIMD4f quadrantAngles2 = dsSIMD4f_mul(quadrantAngles, quadrantAngles);
	*outSinTaylor = dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(
		dsSIMD4f_mul(sinTaylor1, quadrantAngles2), sinTaylor2), quadrantAngles2), sinTaylor3),
		quadrantAngles2), quadrantAngles), quadrantAngles);
	*outCosTaylor = dsSIMD4f_add(dsSIMD4f_sub(dsSIMD4f_mul(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		dsSIMD4f_add(dsSIMD4f_mul(cosTaylor1, quadrantAngles2), cosTaylor2), quadrantAngles2),
		cosTaylor3), quadrantAngles2), quadrantAngles2), dsSIMD4f_mul(half, quadrantAngles2)), one);
}
/// @endcond

DS_MATH_EXPORT inline dsSIMD4f dsSinSIMD4f(dsSIMD4f angles)
{
	dsSIMD4f sinTaylor, cosTaylor;
	dsSIMD4fb quadrants;
	dsSinCosSIMD4fImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: sin, cos, -sin, -cos; negate if angle is negated.
	dsSIMD4fb one = dsSIMD4fb_set1(1);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(dsSIMD4fb_and(quadrants, one), one); // As full bool bitmask.
	dsSIMD4fb halfSign = dsSIMD4fb_shiftLeftConst(
		dsSIMD4fb_and(quadrants, dsSIMD4fb_set1(0x2)), 30);
	dsSIMD4fb sign = dsSIMD4fb_xor(halfSign, dsMathImplExtractSignBitSIMD4f(angles));
	return dsMathImplConditionalNegateSIMD4f(dsSIMD4f_select(oddEven, sinTaylor, cosTaylor), sign);
}

DS_MATH_EXPORT inline dsSIMD4f dsCosSIMD4f(dsSIMD4f angles)
{
	dsSIMD4f sinTaylor, cosTaylor;
	dsSIMD4fb quadrants;
	dsSinCosSIMD4fImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: cos, -sin, -cos, sin
	dsSIMD4fb one = dsSIMD4fb_set1(1);
	dsSIMD4fb evenOdd = dsSIMD4fb_and(quadrants, one);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD4fb halfSign = dsSIMD4fb_shiftLeftConst(
		dsSIMD4fb_and(quadrants, dsSIMD4fb_set1(0x2)), 30);
	dsSIMD4fb sign = dsSIMD4fb_xor(halfSign, dsSIMD4fb_shiftLeftConst(evenOdd, 31));
	return dsMathImplConditionalNegateSIMD4f(dsSIMD4f_select(oddEven, cosTaylor, sinTaylor), sign);
}

DS_MATH_EXPORT inline void dsSinCosSIMD4f(dsSIMD4f* outSin, dsSIMD4f* outCos, dsSIMD4f angles)
{
	DS_ASSERT(outSin);
	DS_ASSERT(outCos);

	dsSIMD4f sinTaylor, cosTaylor;
	dsSIMD4fb quadrants;
	dsSinCosSIMD4fImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering for quadrants is:
	// Sine: sin, cos, -sin, -cos
	// Cosine: cos, -sin, -cos, sin
	dsSIMD4fb one = dsSIMD4fb_set1(1);
	dsSIMD4fb evenOdd = dsSIMD4fb_and(quadrants, one);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD4fb halfSign = dsSIMD4fb_shiftLeftConst(
		dsSIMD4fb_and(quadrants, dsSIMD4fb_set1(0x2)), 30);
	// Match against 2 or 3.
	dsSIMD4fb sinSign = dsSIMD4fb_xor(halfSign, dsMathImplExtractSignBitSIMD4f(angles));
	// Match against 1 or 2.
	dsSIMD4fb cosSign = dsSIMD4fb_xor(halfSign, dsSIMD4fb_shiftLeftConst(evenOdd, 31));

	*outSin = dsMathImplConditionalNegateSIMD4f(
		dsSIMD4f_select(oddEven, sinTaylor, cosTaylor), sinSign);
	*outCos = dsMathImplConditionalNegateSIMD4f(
		dsSIMD4f_select(oddEven, cosTaylor, sinTaylor), cosSign);
}

DS_MATH_EXPORT inline dsSIMD4f dsTanSIMD4f(dsSIMD4f angles)
{
	dsSIMD4f twoOverPi = dsSIMD4f_set1(M_2_PIf);
	dsSIMD4f negOne = dsSIMD4f_set1(-1.0f);
	dsSIMD4fb oneb = dsSIMD4fb_set1(1);

	dsSIMD4f tanTaylor1 = dsSIMD4f_set1(DS_TAN_TAYLOR_1f);
	dsSIMD4f tanTaylor2 = dsSIMD4f_set1(DS_TAN_TAYLOR_2f);
	dsSIMD4f tanTaylor3 = dsSIMD4f_set1(DS_TAN_TAYLOR_3f);
	dsSIMD4f tanTaylor4 = dsSIMD4f_set1(DS_TAN_TAYLOR_4f);
	dsSIMD4f tanTaylor5 = dsSIMD4f_set1(DS_TAN_TAYLOR_5f);
	dsSIMD4f tanTaylor6 = dsSIMD4f_set1(DS_TAN_TAYLOR_6f);

	dsSIMD4f absAngles = dsSIMD4f_abs(angles);
#if DS_DETERMINISTIC_MATH
	// Not all platforms will use the same rounding mode, so round through truncation.
	dsSIMD4fb quadrants = dsSIMD4fb_fromFloat(
		dsSIMD4f_add(dsSIMD4f_mul(absAngles, twoOverPi), dsSIMD4f_set1(0.5f)));
#else
	dsSIMD4fb quadrants = dsSIMD4fb_round(dsSIMD4f_mul(absAngles, twoOverPi));
#endif

	dsSIMD4f quadrantAngles = dsTrigQuadrantAngleSIMD4f(absAngles, dsSIMD4fb_toFloat(quadrants));
	dsSIMD4f quadrantAngles2 = dsSIMD4f_mul(quadrantAngles, quadrantAngles);

	dsSIMD4f tanTaylor = dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(
		dsSIMD4f_mul(tanTaylor1, quadrantAngles2), tanTaylor2), quadrantAngles2), tanTaylor3),
		quadrantAngles2), tanTaylor4), quadrantAngles2), tanTaylor5), quadrantAngles2), tanTaylor6),
		quadrantAngles2), quadrantAngles), quadrantAngles);

	// Ordering of quadrants is: tan, -1/tan, tan, -1/tan
	dsSIMD4fb evenOdd = dsSIMD4fb_and(quadrants, oneb);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(evenOdd, oneb);
	return dsMathImplConditionalNegateSIMD4f(dsSIMD4f_select(oddEven, tanTaylor,
		dsSIMD4f_div(negOne, tanTaylor)), dsMathImplExtractSignBitSIMD4f(angles));
}

DS_MATH_EXPORT inline dsSIMD4f dsASinSIMD4f(dsSIMD4f x)
{
	DS_ASSERT(dsSIMD4fb_all(dsSIMD4fb_and(
		dsSIMD4f_cmpge(x, dsSIMD4f_set1(-1.0f)), dsSIMD4f_cmple(x, dsSIMD4f_set1(1.0f)))));

	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f half =  dsSIMD4f_set1(0.5f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f asinTaylor1 = dsSIMD4f_set1(DS_ASIN_TAYLOR_1f);
	dsSIMD4f asinTaylor2 = dsSIMD4f_set1(DS_ASIN_TAYLOR_2f);
	dsSIMD4f asinTaylor3 = dsSIMD4f_set1(DS_ASIN_TAYLOR_3f);
	dsSIMD4f asinTaylor4 = dsSIMD4f_set1(DS_ASIN_TAYLOR_4f);
	dsSIMD4f asinTaylor5 = dsSIMD4f_set1(DS_ASIN_TAYLOR_5f);

	dsSIMD4f absX = dsSIMD4f_abs(x);

	// Remap the range for larger values based on the identity
	// asin(x) = pi/2 - 2*asin(sqrt((1 - x)/2)).
	dsSIMD4fb adjustRange = dsSIMD4f_cmpgt(absX, half);
	dsSIMD4f adjustedX = dsSIMD4f_sqrt(dsSIMD4f_mul(half, dsSIMD4f_sub(one, absX)));
	absX = dsSIMD4f_select(adjustRange, adjustedX, absX);
	dsSIMD4f x2 = dsSIMD4f_mul(absX, absX);

	dsSIMD4f asinTaylor = dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		asinTaylor1, x2), asinTaylor2), x2), asinTaylor3), x2), asinTaylor4), x2), asinTaylor5),
		x2), absX), absX);
	dsSIMD4f adjustedASinTaylor = dsSIMD4f_sub(pi2, dsSIMD4f_add(asinTaylor, asinTaylor));

	return dsMathImplConditionalNegateSIMD4f(
		dsSIMD4f_select(adjustRange, adjustedASinTaylor, asinTaylor),
		dsMathImplExtractSignBitSIMD4f(x));
}

DS_MATH_EXPORT inline dsSIMD4f dsACosSIMD4f(dsSIMD4f x)
{
	DS_ASSERT(dsSIMD4fb_all(dsSIMD4fb_and(
		dsSIMD4f_cmpge(x, dsSIMD4f_set1(-1.0f)), dsSIMD4f_cmple(x, dsSIMD4f_set1(1.0f)))));

	dsSIMD4f pi = dsSIMD4f_set1(M_PIf);
	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f half =  dsSIMD4f_set1(0.5f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f asinTaylor1 = dsSIMD4f_set1(DS_ASIN_TAYLOR_1f);
	dsSIMD4f asinTaylor2 = dsSIMD4f_set1(DS_ASIN_TAYLOR_2f);
	dsSIMD4f asinTaylor3 = dsSIMD4f_set1(DS_ASIN_TAYLOR_3f);
	dsSIMD4f asinTaylor4 = dsSIMD4f_set1(DS_ASIN_TAYLOR_4f);
	dsSIMD4f asinTaylor5 = dsSIMD4f_set1(DS_ASIN_TAYLOR_5f);

	dsSIMD4f absX = dsSIMD4f_abs(x);
	dsSIMD4fb signBit = dsMathImplExtractSignBitSIMD4f(x);

	// Remap the range for larger values based on the identity
	// asin(x) = pi/2 - 2*asin(sqrt((1 - x)/2)).
	dsSIMD4fb adjustRange = dsSIMD4f_cmpgt(absX, half);
	dsSIMD4f adjustedX = dsSIMD4f_sqrt(dsSIMD4f_mul(half, dsSIMD4f_sub(one, absX)));
	x = dsSIMD4f_select(adjustRange, dsMathImplConditionalNegateSIMD4f(adjustedX, signBit), x);
	dsSIMD4f x2 = dsSIMD4f_mul(x, x);

	dsSIMD4f asinTaylor = dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		asinTaylor1, x2), asinTaylor2), x2), asinTaylor3), x2), asinTaylor4), x2), asinTaylor5),
		x2), x), x);

	// Use the identity that acos(x) = pi/2 - asin(x), applying it directly rather than calling
	// dsASinf() as that could introduce error for the adjusted range.
	dsSIMD4fb signMask = dsSIMD4fb_neg(dsSIMD4fb_shiftRightConst(signBit, 31));
	dsSIMD4f acosTaylor = dsSIMD4f_sub(pi2, asinTaylor);
	// Make sure to add the pi/2 factors together first to reduce precision errors.
	dsSIMD4f adjustedACosTaylor = dsSIMD4f_add(dsMathImplMaskSIMD4f(signMask, pi),
		dsSIMD4f_add(asinTaylor, asinTaylor));

	return dsSIMD4f_select(adjustRange, adjustedACosTaylor, acosTaylor);
}

DS_MATH_EXPORT inline dsSIMD4f dsATanSIMD4f(dsSIMD4f x)
{
	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f pi4 = dsSIMD4f_set1(M_PI_4f);
	dsSIMD4f tan3Pi8 = dsSIMD4f_set1(DS_TAN_3_PI_8f);
	dsSIMD4f tanPi8 = dsSIMD4f_set1(DS_TAN_PI_8f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f atanTaylor1 = dsSIMD4f_set1(DS_ATAN_TAYLOR_1f);
	dsSIMD4f atanTaylor2 = dsSIMD4f_set1(DS_ATAN_TAYLOR_2f);
	dsSIMD4f atanTaylor3 = dsSIMD4f_set1(DS_ATAN_TAYLOR_3f);
	dsSIMD4f atanTaylor4 = dsSIMD4f_set1(DS_ATAN_TAYLOR_4f);

	dsSIMD4f absX = dsSIMD4f_abs(x);

	// Choose a range to compute the taylor on.
	dsSIMD4f largeX = dsSIMD4f_neg(dsSIMD4f_rcp(absX));
	dsSIMD4f midX = dsSIMD4f_div(dsSIMD4f_sub(absX, one), dsSIMD4f_add(absX, one));

	dsSIMD4fb isLarge = dsSIMD4f_cmpgt(absX, tan3Pi8);
	dsSIMD4fb atLeastMid = dsSIMD4f_cmpgt(absX, tanPi8);
	absX = dsSIMD4f_select(isLarge, largeX, dsSIMD4f_select(atLeastMid, midX, absX));
	dsSIMD4f offset = dsMathImplMaskSIMD4f(atLeastMid, dsSIMD4f_select(isLarge, pi2, pi4));

	dsSIMD4f x2 = dsSIMD4f_mul(absX, absX);
	dsSIMD4f atanTaylor = dsSIMD4f_add(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_mul(dsSIMD4f_add(
		dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(atanTaylor1, x2),
		atanTaylor2), x2), atanTaylor3), x2), atanTaylor4), x2), absX), absX), offset);
	return dsMathImplConditionalNegateSIMD4f(atanTaylor, dsMathImplExtractSignBitSIMD4f(x));
}

DS_MATH_EXPORT inline dsSIMD4f dsATan2SIMD4f(dsSIMD4f y, dsSIMD4f x)
{
	dsSIMD4f pi = dsSIMD4f_set1(M_PIf);
	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	dsSIMD4fb oneb = dsSIMD4fb_set1(1);

	dsSIMD4fb ySign = dsMathImplExtractSignBitSIMD4f(y);
	dsSIMD4fb xSign = dsMathImplExtractSignBitSIMD4f(x);
	dsSIMD4fb xNegativeMask = dsSIMD4fb_not(
		dsSIMD4fb_sub(dsSIMD4fb_shiftRightConst(xSign, 31), oneb));
	dsSIMD4fb yZero = dsSIMD4f_cmpeq(y, zero);
	dsSIMD4fb xZero = dsSIMD4f_cmpeq(x, zero);

	// When x is positive, take the raw atan value. If x is negative, add or subtract pi when y is
	// positive or negative.
	dsSIMD4f offset = dsMathImplMaskSIMD4f(
		xNegativeMask, dsMathImplConditionalNegateSIMD4f(pi, ySign));
	dsSIMD4f result = dsSIMD4f_add(dsATanSIMD4f(dsSIMD4f_div(y, x)), offset);
	result = dsSIMD4f_select(xZero, dsMathImplConditionalNegateSIMD4f(pi2, ySign), result);
	return dsSIMD4f_select(yZero,
		dsMathImplConditionalNegateSIMD4f(dsMathImplMaskSIMD4f(xNegativeMask, pi), ySign), result);
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT,DS_SIMD_FMA)

/// @cond
DS_ALWAYS_INLINE dsSIMD4f dsTrigQuadrantAngleFMA4f(dsSIMD4f absAngles, dsSIMD4f quadrants)
{
	return dsSIMD4f_fnmadd(quadrants, dsSIMD4f_set1(DS_PI_2f_3),
		dsSIMD4f_fnmadd(quadrants, dsSIMD4f_set1(DS_PI_2f_2),
		dsSIMD4f_fnmadd(quadrants, dsSIMD4f_set1(DS_PI_2f_1), absAngles)));
}

DS_ALWAYS_INLINE void dsSinCosFMA4fImpl(
	dsSIMD4f* outSinTaylor, dsSIMD4f* outCosTaylor, dsSIMD4fb* outQuadrants, dsSIMD4f angles)
{
	dsSIMD4f twoOverPi = dsSIMD4f_set1(M_2_PIf);
	dsSIMD4f half = dsSIMD4f_set1(0.5f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f sinTaylor1 = dsSIMD4f_set1(DS_SIN_TAYLOR_1f);
	dsSIMD4f sinTaylor2 = dsSIMD4f_set1(DS_SIN_TAYLOR_2f);
	dsSIMD4f sinTaylor3 = dsSIMD4f_set1(DS_SIN_TAYLOR_3f);

	dsSIMD4f cosTaylor1 = dsSIMD4f_set1(DS_COS_TAYLOR_1f);
	dsSIMD4f cosTaylor2 = dsSIMD4f_set1(DS_COS_TAYLOR_2f);
	dsSIMD4f cosTaylor3 = dsSIMD4f_set1(DS_COS_TAYLOR_3f);

	dsSIMD4f absAngles = dsSIMD4f_abs(angles);
	*outQuadrants = dsSIMD4fb_round(dsSIMD4f_mul(absAngles, twoOverPi));

	dsSIMD4f quadrantAngles = dsTrigQuadrantAngleFMA4f(
		absAngles, dsSIMD4fb_toFloat(*outQuadrants));
	dsSIMD4f quadrantAngles2 = dsSIMD4f_mul(quadrantAngles, quadrantAngles);
	*outSinTaylor = dsSIMD4f_fmadd(dsSIMD4f_mul(dsSIMD4f_fmadd(dsSIMD4f_fmadd(
		sinTaylor1, quadrantAngles2, sinTaylor2), quadrantAngles2, sinTaylor3), quadrantAngles2),
		quadrantAngles, quadrantAngles);
	*outCosTaylor = dsSIMD4f_add(dsSIMD4f_fmsub(dsSIMD4f_mul(dsSIMD4f_fmadd(dsSIMD4f_fmadd(
		cosTaylor1, quadrantAngles2, cosTaylor2), quadrantAngles2, cosTaylor3), quadrantAngles2),
		quadrantAngles2, dsSIMD4f_mul(half, quadrantAngles2)), one);
}
/// @endcond

DS_MATH_EXPORT inline dsSIMD4f dsSinFMA4f(dsSIMD4f angles)
{
	dsSIMD4f sinTaylor, cosTaylor;
	dsSIMD4fb quadrants;
	dsSinCosFMA4fImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: sin, cos, -sin, -cos; negate if angle is negated.
	dsSIMD4fb one = dsSIMD4fb_set1(1);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(dsSIMD4fb_and(quadrants, one), one); // As full bool bitmask.
	dsSIMD4fb halfSign = dsSIMD4fb_shiftLeftConst(
		dsSIMD4fb_and(quadrants, dsSIMD4fb_set1(0x2)), 30);
	dsSIMD4fb sign = dsSIMD4fb_xor(halfSign, dsMathImplExtractSignBitSIMD4f(angles));
	return dsMathImplConditionalNegateSIMD4f(dsSIMD4f_select(oddEven, sinTaylor, cosTaylor), sign);
}

DS_MATH_EXPORT inline dsSIMD4f dsCosFMA4f(dsSIMD4f angles)
{
	dsSIMD4f sinTaylor, cosTaylor;
	dsSIMD4fb quadrants;
	dsSinCosFMA4fImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: cos, -sin, -cos, sin
	dsSIMD4fb one = dsSIMD4fb_set1(1);
	dsSIMD4fb evenOdd = dsSIMD4fb_and(quadrants, one);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD4fb halfSign = dsSIMD4fb_shiftLeftConst(
		dsSIMD4fb_and(quadrants, dsSIMD4fb_set1(0x2)), 30);
	dsSIMD4fb sign = dsSIMD4fb_xor(halfSign, dsSIMD4fb_shiftLeftConst(evenOdd, 31));
	return dsMathImplConditionalNegateSIMD4f(dsSIMD4f_select(oddEven, cosTaylor, sinTaylor), sign);
}

DS_MATH_EXPORT inline void dsSinCosFMA4f(dsSIMD4f* outSin, dsSIMD4f* outCos, dsSIMD4f angles)
{
	DS_ASSERT(outSin);
	DS_ASSERT(outCos);

	dsSIMD4f sinTaylor, cosTaylor;
	dsSIMD4fb quadrants;
	dsSinCosFMA4fImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering for quadrants is:
	// Sine: sin, cos, -sin, -cos
	// Cosine: cos, -sin, -cos, sin
	dsSIMD4fb one = dsSIMD4fb_set1(1);
	dsSIMD4fb evenOdd = dsSIMD4fb_and(quadrants, one);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD4fb halfSign = dsSIMD4fb_shiftLeftConst(
		dsSIMD4fb_and(quadrants, dsSIMD4fb_set1(0x2)), 30);
	// Match against 2 or 3.
	dsSIMD4fb sinSign = dsSIMD4fb_xor(halfSign, dsMathImplExtractSignBitSIMD4f(angles));
	// Match against 1 or 2.
	dsSIMD4fb cosSign = dsSIMD4fb_xor(halfSign, dsSIMD4fb_shiftLeftConst(evenOdd, 31));

	*outSin = dsMathImplConditionalNegateSIMD4f(
		dsSIMD4f_select(oddEven, sinTaylor, cosTaylor), sinSign);
	*outCos = dsMathImplConditionalNegateSIMD4f(
		dsSIMD4f_select(oddEven, cosTaylor, sinTaylor), cosSign);
}

DS_MATH_EXPORT inline dsSIMD4f dsTanFMA4f(dsSIMD4f angles)
{
	dsSIMD4f twoOverPi = dsSIMD4f_set1(M_2_PIf);
	dsSIMD4f negOne = dsSIMD4f_set1(-1.0f);
	dsSIMD4fb oneb = dsSIMD4fb_set1(1);

	dsSIMD4f tanTaylor1 = dsSIMD4f_set1(DS_TAN_TAYLOR_1f);
	dsSIMD4f tanTaylor2 = dsSIMD4f_set1(DS_TAN_TAYLOR_2f);
	dsSIMD4f tanTaylor3 = dsSIMD4f_set1(DS_TAN_TAYLOR_3f);
	dsSIMD4f tanTaylor4 = dsSIMD4f_set1(DS_TAN_TAYLOR_4f);
	dsSIMD4f tanTaylor5 = dsSIMD4f_set1(DS_TAN_TAYLOR_5f);
	dsSIMD4f tanTaylor6 = dsSIMD4f_set1(DS_TAN_TAYLOR_6f);

	dsSIMD4f absAngles = dsSIMD4f_abs(angles);
	dsSIMD4fb quadrants = dsSIMD4fb_round(dsSIMD4f_mul(absAngles, twoOverPi));

	dsSIMD4f quadrantAngles = dsTrigQuadrantAngleSIMD4f(absAngles, dsSIMD4fb_toFloat(quadrants));
	dsSIMD4f quadrantAngles2 = dsSIMD4f_mul(quadrantAngles, quadrantAngles);

	dsSIMD4f tanTaylor = dsSIMD4f_fmadd(dsSIMD4f_mul(dsSIMD4f_fmadd(dsSIMD4f_fmadd(dsSIMD4f_fmadd(
		dsSIMD4f_fmadd(dsSIMD4f_fmadd(tanTaylor1, quadrantAngles2, tanTaylor2), quadrantAngles2,
		tanTaylor3), quadrantAngles2, tanTaylor4), quadrantAngles2, tanTaylor5), quadrantAngles2,
		tanTaylor6), quadrantAngles2), quadrantAngles, quadrantAngles);

	// Ordering of quadrants is: tan, -1/tan, tan, -1/tan
	dsSIMD4fb evenOdd = dsSIMD4fb_and(quadrants, oneb);
	dsSIMD4fb oddEven = dsSIMD4fb_sub(evenOdd, oneb);
	return dsMathImplConditionalNegateSIMD4f(dsSIMD4f_select(oddEven, tanTaylor,
		dsSIMD4f_div(negOne, tanTaylor)), dsMathImplExtractSignBitSIMD4f(angles));
}

DS_MATH_EXPORT inline dsSIMD4f dsASinFMA4f(dsSIMD4f x)
{
	DS_ASSERT(dsSIMD4fb_all(dsSIMD4fb_and(
		dsSIMD4f_cmpge(x, dsSIMD4f_set1(-1.0f)), dsSIMD4f_cmple(x, dsSIMD4f_set1(1.0f)))));

	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f half =  dsSIMD4f_set1(0.5f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f asinTaylor1 = dsSIMD4f_set1(DS_ASIN_TAYLOR_1f);
	dsSIMD4f asinTaylor2 = dsSIMD4f_set1(DS_ASIN_TAYLOR_2f);
	dsSIMD4f asinTaylor3 = dsSIMD4f_set1(DS_ASIN_TAYLOR_3f);
	dsSIMD4f asinTaylor4 = dsSIMD4f_set1(DS_ASIN_TAYLOR_4f);
	dsSIMD4f asinTaylor5 = dsSIMD4f_set1(DS_ASIN_TAYLOR_5f);

	dsSIMD4f absX = dsSIMD4f_abs(x);

	// Remap the range for larger values based on the identity
	// asin(x) = pi/2 - 2*asin(sqrt((1 - x)/2)).
	dsSIMD4fb adjustRange = dsSIMD4f_cmpgt(absX, half);
	dsSIMD4f adjustedX = dsSIMD4f_sqrt(dsSIMD4f_mul(half, dsSIMD4f_sub(one, absX)));
	absX = dsSIMD4f_select(adjustRange, adjustedX, absX);
	dsSIMD4f x2 = dsSIMD4f_mul(absX, absX);

	dsSIMD4f asinTaylor = dsSIMD4f_fmadd(dsSIMD4f_mul(dsSIMD4f_fmadd(dsSIMD4f_fmadd(dsSIMD4f_fmadd(
		dsSIMD4f_fmadd(asinTaylor1, x2, asinTaylor2), x2, asinTaylor3), x2, asinTaylor4), x2,
		asinTaylor5), x2), absX, absX);
	dsSIMD4f adjustedASinTaylor = dsSIMD4f_sub(pi2, dsSIMD4f_add(asinTaylor, asinTaylor));

	return dsMathImplConditionalNegateSIMD4f(
		dsSIMD4f_select(adjustRange, adjustedASinTaylor, asinTaylor),
		dsMathImplExtractSignBitSIMD4f(x));
}

DS_MATH_EXPORT inline dsSIMD4f dsACosFMA4f(dsSIMD4f x)
{
	DS_ASSERT(dsSIMD4fb_all(dsSIMD4fb_and(
		dsSIMD4f_cmpge(x, dsSIMD4f_set1(-1.0f)), dsSIMD4f_cmple(x, dsSIMD4f_set1(1.0f)))));

	dsSIMD4f pi = dsSIMD4f_set1(M_PIf);
	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f half =  dsSIMD4f_set1(0.5f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f asinTaylor1 = dsSIMD4f_set1(DS_ASIN_TAYLOR_1f);
	dsSIMD4f asinTaylor2 = dsSIMD4f_set1(DS_ASIN_TAYLOR_2f);
	dsSIMD4f asinTaylor3 = dsSIMD4f_set1(DS_ASIN_TAYLOR_3f);
	dsSIMD4f asinTaylor4 = dsSIMD4f_set1(DS_ASIN_TAYLOR_4f);
	dsSIMD4f asinTaylor5 = dsSIMD4f_set1(DS_ASIN_TAYLOR_5f);

	dsSIMD4f absX = dsSIMD4f_abs(x);
	dsSIMD4fb signBit = dsMathImplExtractSignBitSIMD4f(x);

	// Remap the range for larger values based on the identity
	// asin(x) = pi/2 - 2*asin(sqrt((1 - x)/2)).
	dsSIMD4fb adjustRange = dsSIMD4f_cmpgt(absX, half);
	dsSIMD4f adjustedX = dsSIMD4f_sqrt(dsSIMD4f_mul(half, dsSIMD4f_sub(one, absX)));
	x = dsSIMD4f_select(adjustRange, dsMathImplConditionalNegateSIMD4f(adjustedX, signBit), x);
	dsSIMD4f x2 = dsSIMD4f_mul(x, x);

	dsSIMD4f asinTaylor = dsSIMD4f_fmadd(dsSIMD4f_mul(dsSIMD4f_fmadd(dsSIMD4f_fmadd(dsSIMD4f_fmadd(
		dsSIMD4f_fmadd(asinTaylor1, x2, asinTaylor2), x2, asinTaylor3), x2, asinTaylor4), x2,
		asinTaylor5), x2), x, x);

	// Use the identity that acos(x) = pi/2 - asin(x), applying it directly rather than calling
	// dsASinf() as that could introduce error for the adjusted range.
	dsSIMD4fb signMask = dsSIMD4fb_neg(dsSIMD4fb_shiftRightConst(signBit, 31));
	dsSIMD4f acosTaylor = dsSIMD4f_sub(pi2, asinTaylor);
	// Make sure to add the pi/2 factors together first to reduce precision errors.
	dsSIMD4f adjustedACosTaylor = dsSIMD4f_add(dsMathImplMaskSIMD4f(signMask, pi),
		dsSIMD4f_add(asinTaylor, asinTaylor));

	return dsSIMD4f_select(adjustRange, adjustedACosTaylor, acosTaylor);
}

DS_MATH_EXPORT inline dsSIMD4f dsATanFMA4f(dsSIMD4f x)
{
	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f pi4 = dsSIMD4f_set1(M_PI_4f);
	dsSIMD4f tan3Pi8 = dsSIMD4f_set1(DS_TAN_3_PI_8f);
	dsSIMD4f tanPi8 = dsSIMD4f_set1(DS_TAN_PI_8f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);
	dsSIMD4f negOne = dsSIMD4f_set1(-1.0f);

	dsSIMD4f atanTaylor1 = dsSIMD4f_set1(DS_ATAN_TAYLOR_1f);
	dsSIMD4f atanTaylor2 = dsSIMD4f_set1(DS_ATAN_TAYLOR_2f);
	dsSIMD4f atanTaylor3 = dsSIMD4f_set1(DS_ATAN_TAYLOR_3f);
	dsSIMD4f atanTaylor4 = dsSIMD4f_set1(DS_ATAN_TAYLOR_4f);

	dsSIMD4f absX = dsSIMD4f_abs(x);

	// Choose a range to compute the taylor on.
	dsSIMD4f largeX = dsSIMD4f_div(negOne, absX);
	dsSIMD4f midX = dsSIMD4f_div(dsSIMD4f_sub(absX, one), dsSIMD4f_add(absX, one));

	dsSIMD4fb isLarge = dsSIMD4f_cmpgt(absX, tan3Pi8);
	dsSIMD4fb atLeastMid = dsSIMD4f_cmpgt(absX, tanPi8);
	absX = dsSIMD4f_select(isLarge, largeX, dsSIMD4f_select(atLeastMid, midX, absX));
	dsSIMD4f offset = dsMathImplMaskSIMD4f(atLeastMid, dsSIMD4f_select(isLarge, pi2, pi4));

	dsSIMD4f x2 = dsSIMD4f_mul(absX, absX);
	dsSIMD4f atanTaylor = dsSIMD4f_add(dsSIMD4f_fmadd(dsSIMD4f_mul(dsSIMD4f_fmadd(dsSIMD4f_fmadd(
		dsSIMD4f_fmadd(atanTaylor1, x2, atanTaylor2), x2, atanTaylor3), x2, atanTaylor4), x2), absX,
		absX), offset);
	return dsMathImplConditionalNegateSIMD4f(atanTaylor, dsMathImplExtractSignBitSIMD4f(x));
}

DS_MATH_EXPORT inline dsSIMD4f dsATan2FMA4f(dsSIMD4f y, dsSIMD4f x)
{
	dsSIMD4f pi = dsSIMD4f_set1(M_PIf);
	dsSIMD4f pi2 = dsSIMD4f_set1(M_PI_2f);
	dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	dsSIMD4fb oneb = dsSIMD4fb_set1(1);

	dsSIMD4fb ySign = dsMathImplExtractSignBitSIMD4f(y);
	dsSIMD4fb xSign = dsMathImplExtractSignBitSIMD4f(x);
	dsSIMD4fb xNegativeMask = dsSIMD4fb_not(
		dsSIMD4fb_sub(dsSIMD4fb_shiftRightConst(xSign, 31), oneb));
	dsSIMD4fb yZero = dsSIMD4f_cmpeq(y, zero);
	dsSIMD4fb xZero = dsSIMD4f_cmpeq(x, zero);

	// When x is positive, take the raw atan value. If x is negative, add or subtract pi when y is
	// positive or negative.
	dsSIMD4f offset = dsMathImplMaskSIMD4f(
		xNegativeMask, dsMathImplConditionalNegateSIMD4f(pi, ySign));
	dsSIMD4f result = dsSIMD4f_add(dsATanFMA4f(dsSIMD4f_div(y, x)), offset);
	result = dsSIMD4f_select(xZero, dsMathImplConditionalNegateSIMD4f(pi2, ySign), result);
	return dsSIMD4f_select(yZero,
		dsMathImplConditionalNegateSIMD4f(dsMathImplMaskSIMD4f(xNegativeMask, pi), ySign), result);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT)

/// @cond
DS_ALWAYS_INLINE dsSIMD2d dsTrigQuadrantAngleSIMD2d(dsSIMD2d absAngles, dsSIMD2d quadrants)
{
	return dsSIMD2d_sub(dsSIMD2d_sub(dsSIMD2d_sub(absAngles,
		dsSIMD2d_mul(quadrants, dsSIMD2d_set1(DS_PI_2d_1))),
		dsSIMD2d_mul(quadrants, dsSIMD2d_set1(DS_PI_2d_2))),
		dsSIMD2d_mul(quadrants, dsSIMD2d_set1(DS_PI_2d_3)));
}

DS_ALWAYS_INLINE void dsSinCosSIMD2dImpl(
	dsSIMD2d* outSinTaylor, dsSIMD2d* outCosTaylor, dsSIMD2db* outQuadrants, dsSIMD2d angles)
{
	dsSIMD2d twoOverPi = dsSIMD2d_set1(M_2_PI);
	dsSIMD2d half = dsSIMD2d_set1(0.5);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d sinTaylor1 = dsSIMD2d_set1(DS_SIN_TAYLOR_1d);
	dsSIMD2d sinTaylor2 = dsSIMD2d_set1(DS_SIN_TAYLOR_2d);
	dsSIMD2d sinTaylor3 = dsSIMD2d_set1(DS_SIN_TAYLOR_3d);
	dsSIMD2d sinTaylor4 = dsSIMD2d_set1(DS_SIN_TAYLOR_4d);
	dsSIMD2d sinTaylor5 = dsSIMD2d_set1(DS_SIN_TAYLOR_5d);
	dsSIMD2d sinTaylor6 = dsSIMD2d_set1(DS_SIN_TAYLOR_6d);

	dsSIMD2d cosTaylor1 = dsSIMD2d_set1(DS_COS_TAYLOR_1d);
	dsSIMD2d cosTaylor2 = dsSIMD2d_set1(DS_COS_TAYLOR_2d);
	dsSIMD2d cosTaylor3 = dsSIMD2d_set1(DS_COS_TAYLOR_3d);
	dsSIMD2d cosTaylor4 = dsSIMD2d_set1(DS_COS_TAYLOR_4d);
	dsSIMD2d cosTaylor5 = dsSIMD2d_set1(DS_COS_TAYLOR_5d);
	dsSIMD2d cosTaylor6 = dsSIMD2d_set1(DS_COS_TAYLOR_6d);

	dsSIMD2d absAngles = dsSIMD2d_abs(angles);
	*outQuadrants = dsSIMD2db_round(dsSIMD2d_mul(absAngles, twoOverPi));

	dsSIMD2d quadrantAngles = dsTrigQuadrantAngleSIMD2d(
		absAngles, dsSIMD2db_toDouble(*outQuadrants));
	dsSIMD2d quadrantAngles2 = dsSIMD2d_mul(quadrantAngles, quadrantAngles);
	*outSinTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(
		sinTaylor1, quadrantAngles2), sinTaylor2), quadrantAngles2), sinTaylor3), quadrantAngles2),
		sinTaylor4), quadrantAngles2), sinTaylor5), quadrantAngles2), sinTaylor6), quadrantAngles2),
		quadrantAngles), quadrantAngles);
	*outCosTaylor = dsSIMD2d_add(dsSIMD2d_sub(dsSIMD2d_mul(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(
		dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(cosTaylor1, quadrantAngles2), cosTaylor2), quadrantAngles2), cosTaylor3),
		quadrantAngles2), cosTaylor4), quadrantAngles2), cosTaylor5), quadrantAngles2),
		cosTaylor6), quadrantAngles2), quadrantAngles2), dsSIMD2d_mul(half, quadrantAngles2)), one);
}
/// @endcond

DS_MATH_EXPORT inline dsSIMD2d dsSinSIMD2d(dsSIMD2d angles)
{
	dsSIMD2d sinTaylor, cosTaylor;
	dsSIMD2db quadrants;
	dsSinCosSIMD2dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: sin, cos, -sin, -cos; negate if angle is negated.
	dsSIMD2db one = dsSIMD2db_set1(1);
	dsSIMD2db oddEven = dsSIMD2db_sub(dsSIMD2db_and(quadrants, one), one); // As full bool bitmask.
	dsSIMD2db halfSign = dsSIMD2db_shiftLeftConst(
		dsSIMD2db_and(quadrants, dsSIMD2db_set1(0x2)), 62);
	dsSIMD2db sign = dsSIMD2db_xor(halfSign, dsMathImplExtractSignBitSIMD2d(angles));
	return dsMathImplConditionalNegateSIMD2d(dsSIMD2d_select(oddEven, sinTaylor, cosTaylor), sign);
}

DS_MATH_EXPORT inline dsSIMD2d dsCosSIMD2d(dsSIMD2d angles)
{
	dsSIMD2d sinTaylor, cosTaylor;
	dsSIMD2db quadrants;
	dsSinCosSIMD2dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: cos, -sin, -cos, sin
	dsSIMD2db one = dsSIMD2db_set1(1);
	dsSIMD2db evenOdd = dsSIMD2db_and(quadrants, one);
	dsSIMD2db oddEven = dsSIMD2db_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD2db halfSign = dsSIMD2db_shiftLeftConst(
		dsSIMD2db_and(quadrants, dsSIMD2db_set1(0x2)), 62);
	dsSIMD2db sign = dsSIMD2db_xor(halfSign, dsSIMD2db_shiftLeftConst(evenOdd, 63));
	return dsMathImplConditionalNegateSIMD2d(dsSIMD2d_select(oddEven, cosTaylor, sinTaylor), sign);
}

DS_MATH_EXPORT inline void dsSinCosSIMD2d(dsSIMD2d* outSin, dsSIMD2d* outCos, dsSIMD2d angles)
{
	DS_ASSERT(outSin);
	DS_ASSERT(outCos);

	dsSIMD2d sinTaylor, cosTaylor;
	dsSIMD2db quadrants;
	dsSinCosSIMD2dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering for quadrants is:
	// Sine: sin, cos, -sin, -cos
	// Cosine: cos, -sin, -cos, sin
	dsSIMD2db one = dsSIMD2db_set1(1);
	dsSIMD2db evenOdd = dsSIMD2db_and(quadrants, one);
	dsSIMD2db oddEven = dsSIMD2db_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD2db halfSign = dsSIMD2db_shiftLeftConst(
		dsSIMD2db_and(quadrants, dsSIMD2db_set1(0x2)), 62);
	// Match against 2 or 3.
	dsSIMD2db sinSign = dsSIMD2db_xor(halfSign, dsMathImplExtractSignBitSIMD2d(angles));
	// Match against 1 or 2.
	dsSIMD2db cosSign = dsSIMD2db_xor(halfSign, dsSIMD2db_shiftLeftConst(evenOdd, 63));

	*outSin = dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_select(oddEven, sinTaylor, cosTaylor), sinSign);
	*outCos = dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_select(oddEven, cosTaylor, sinTaylor), cosSign);
}

DS_MATH_EXPORT inline dsSIMD2d dsTanSIMD2d(dsSIMD2d angles)
{
	dsSIMD2d twoOverPi = dsSIMD2d_set1(M_2_PI);
	dsSIMD2d negOne = dsSIMD2d_set1(-1.0);
	dsSIMD2db oneb = dsSIMD2db_set1(1);

	dsSIMD2d tanTaylorP1 = dsSIMD2d_set1(DS_TAN_TAYLOR_P_1d);
	dsSIMD2d tanTaylorP2 = dsSIMD2d_set1(DS_TAN_TAYLOR_P_2d);
	dsSIMD2d tanTaylorP3 = dsSIMD2d_set1(DS_TAN_TAYLOR_P_3d);

	dsSIMD2d tanTaylorQ1 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_1d);
	dsSIMD2d tanTaylorQ2 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_2d);
	dsSIMD2d tanTaylorQ3 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_3d);
	dsSIMD2d tanTaylorQ4 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_4d);

	dsSIMD2d absAngles = dsSIMD2d_abs(angles);
	dsSIMD2db quadrants = dsSIMD2db_round(dsSIMD2d_mul(absAngles, twoOverPi));

	dsSIMD2d quadrantAngles = dsTrigQuadrantAngleSIMD2d(absAngles, dsSIMD2db_toDouble(quadrants));
	dsSIMD2d quadrantAngles2 = dsSIMD2d_mul(quadrantAngles, quadrantAngles);

	// Use a rational function rather than raw Taylor expansion.
	dsSIMD2d pTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(
		tanTaylorP1, quadrantAngles2), tanTaylorP2), quadrantAngles2), tanTaylorP3);
	dsSIMD2d qTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(quadrantAngles2, tanTaylorQ1), quadrantAngles2), tanTaylorQ2),
		quadrantAngles2), tanTaylorQ3), quadrantAngles2), tanTaylorQ4);
	dsSIMD2d tanRational = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_mul(dsSIMD2d_div(pTaylor, qTaylor),
		quadrantAngles2), quadrantAngles), quadrantAngles);

	// Ordering of quadrants is: tan, -1/tan, tan, -1/tan
	dsSIMD2db evenOdd = dsSIMD2db_and(quadrants, oneb);
	dsSIMD2db oddEven = dsSIMD2db_sub(evenOdd, oneb);
	return dsMathImplConditionalNegateSIMD2d(dsSIMD2d_select(oddEven, tanRational,
		dsSIMD2d_div(negOne, tanRational)), dsMathImplExtractSignBitSIMD2d(angles));
}

DS_MATH_EXPORT inline dsSIMD2d dsASinSIMD2d(dsSIMD2d x)
{
	DS_ASSERT(dsSIMD2db_all(dsSIMD2db_and(
		dsSIMD2d_cmpge(x, dsSIMD2d_set1(-1.0)), dsSIMD2d_cmple(x, dsSIMD2d_set1(1.0)))));

	dsSIMD2d pi4 = dsSIMD2d_set1(M_PI_4);
	dsSIMD2d pi2BitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd);
	dsSIMD2d threshold = dsSIMD2d_set1(0.625);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d asinTaylorP1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_1d);
	dsSIMD2d asinTaylorP2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_2d);
	dsSIMD2d asinTaylorP3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_3d);
	dsSIMD2d asinTaylorP4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_4d);
	dsSIMD2d asinTaylorP5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_5d);
	dsSIMD2d asinTaylorP6 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_6d);

	dsSIMD2d asinTaylorQ1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_1d);
	dsSIMD2d asinTaylorQ2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_2d);
	dsSIMD2d asinTaylorQ3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_3d);
	dsSIMD2d asinTaylorQ4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_4d);
	dsSIMD2d asinTaylorQ5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_5d);

	dsSIMD2d absX = dsSIMD2d_abs(x);

	// Choose which rational function to use based on the value.
	dsSIMD2db adjustRange = dsSIMD2d_cmpgt(absX, threshold);

	dsSIMD2d x2 = dsSIMD2d_mul(x, x);
	dsSIMD2d pTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(asinTaylorP1, x2),
		asinTaylorP2), x2), asinTaylorP3), x2), asinTaylorP4), x2), asinTaylorP5), x2),
		asinTaylorP6);
	dsSIMD2d qTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(x2, asinTaylorQ1), x2), asinTaylorQ2),
		x2), asinTaylorQ3), x2), asinTaylorQ4), x2), asinTaylorQ5);
	dsSIMD2d asinRational = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_mul(
		dsSIMD2d_div(pTaylor, qTaylor), x2), absX), absX);

	// Compute based on asin(1 - x).
	dsSIMD2d asinTaylorR1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_1d);
	dsSIMD2d asinTaylorR2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_2d);
	dsSIMD2d asinTaylorR3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_3d);
	dsSIMD2d asinTaylorR4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_4d);
	dsSIMD2d asinTaylorR5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_5d);

	dsSIMD2d asinTaylorS1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_1d);
	dsSIMD2d asinTaylorS2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_2d);
	dsSIMD2d asinTaylorS3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_3d);
	dsSIMD2d asinTaylorS4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_4d);

	dsSIMD2d adjustedX = dsSIMD2d_sub(one, absX);
	dsSIMD2d rTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(asinTaylorR1, adjustedX), asinTaylorR2), adjustedX),
		asinTaylorR3), adjustedX), asinTaylorR4), adjustedX), asinTaylorR5);
	dsSIMD2d sTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(adjustedX, asinTaylorS1), adjustedX), asinTaylorS2), adjustedX),
		asinTaylorS3), adjustedX), asinTaylorS4);
	dsSIMD2d adjustedBaseRational = dsSIMD2d_mul(dsSIMD2d_div(rTaylor, sTaylor), adjustedX);
	dsSIMD2d adjustedSqrt2X = dsSIMD2d_sqrt(dsSIMD2d_add(adjustedX, adjustedX));
	// Take pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational) in a way that extends the bits
	// of precision.
	dsSIMD2d adjustedASinRational = dsSIMD2d_add(dsSIMD2d_sub(dsSIMD2d_sub(pi4, adjustedSqrt2X),
		dsSIMD2d_sub(dsSIMD2d_mul(adjustedSqrt2X, adjustedBaseRational), pi2BitExtension)), pi4);

	return dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_select(adjustRange, adjustedASinRational, asinRational),
		dsMathImplExtractSignBitSIMD2d(x));
}

DS_MATH_EXPORT inline dsSIMD2d dsACosSIMD2d(dsSIMD2d x)
{
	DS_ASSERT(dsSIMD2db_all(dsSIMD2db_and(
		dsSIMD2d_cmpge(x, dsSIMD2d_set1(-1.0)), dsSIMD2d_cmple(x, dsSIMD2d_set1(1.0)))));

	dsSIMD2d pi2 = dsSIMD2d_set1(M_PI_2);
	dsSIMD2d piBitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd*2);
	dsSIMD2d threshold = dsSIMD2d_set1(0.625);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d asinTaylorP1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_1d);
	dsSIMD2d asinTaylorP2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_2d);
	dsSIMD2d asinTaylorP3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_3d);
	dsSIMD2d asinTaylorP4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_4d);
	dsSIMD2d asinTaylorP5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_5d);
	dsSIMD2d asinTaylorP6 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_6d);

	dsSIMD2d asinTaylorQ1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_1d);
	dsSIMD2d asinTaylorQ2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_2d);
	dsSIMD2d asinTaylorQ3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_3d);
	dsSIMD2d asinTaylorQ4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_4d);
	dsSIMD2d asinTaylorQ5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_5d);

	dsSIMD2d absX = dsSIMD2d_abs(x);

	// Choose which rational function to use based on the value.
	// Use the identity that acos(x) = pi/2 - asin(x), applying it directly rather than calling
	// dsASinf() as that could introduce error for the adjusted range.
	dsSIMD2db adjustRange = dsSIMD2d_cmpgt(absX, threshold);

	dsSIMD2d x2 = dsSIMD2d_mul(x, x);
	dsSIMD2d pTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(asinTaylorP1, x2),
		asinTaylorP2), x2), asinTaylorP3), x2), asinTaylorP4), x2), asinTaylorP5), x2),
		asinTaylorP6);
	dsSIMD2d qTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(x2, asinTaylorQ1), x2), asinTaylorQ2),
		x2), asinTaylorQ3), x2), asinTaylorQ4), x2), asinTaylorQ5);
	dsSIMD2d acosRational = dsSIMD2d_sub(dsSIMD2d_sub(pi2, dsSIMD2d_mul(dsSIMD2d_mul(
		dsSIMD2d_div(pTaylor, qTaylor), x2), x)), x);

	// Compute based on asin(1 - x).
	dsSIMD2d asinTaylorR1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_1d);
	dsSIMD2d asinTaylorR2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_2d);
	dsSIMD2d asinTaylorR3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_3d);
	dsSIMD2d asinTaylorR4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_4d);
	dsSIMD2d asinTaylorR5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_5d);

	dsSIMD2d asinTaylorS1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_1d);
	dsSIMD2d asinTaylorS2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_2d);
	dsSIMD2d asinTaylorS3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_3d);
	dsSIMD2d asinTaylorS4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_4d);

	dsSIMD2db signBit = dsMathImplExtractSignBitSIMD2d(x);
	dsSIMD2db signMask = dsSIMD2db_neg(dsSIMD2db_shiftRightConst(signBit, 63));
	dsSIMD2d adjustedX = dsSIMD2d_sub(one, absX);

	dsSIMD2d rTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(asinTaylorR1, adjustedX), asinTaylorR2), adjustedX),
		asinTaylorR3), adjustedX), asinTaylorR4), adjustedX), asinTaylorR5);
	dsSIMD2d sTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(adjustedX, asinTaylorS1), adjustedX), asinTaylorS2), adjustedX),
		asinTaylorS3), adjustedX), asinTaylorS4);
	dsSIMD2d adjustedBaseRational = dsSIMD2d_mul(dsSIMD2d_div(rTaylor, sTaylor), adjustedX);
	dsSIMD2d adjustedSqrt2X = dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_sqrt(dsSIMD2d_add(adjustedX, adjustedX)), signBit);
	// Take pi/2 - (applySign(pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational))) in a way that
	// extends the bits of precision. The sign adjustment for the factor with the bit extension
	// is provided by adjustedSqrt2X. As a result, need do
	// "- adjustSign(extesnion - adjustSign(extension))", which resolvees to
	// "+ extension - adjustSign(extension)", or conditionally adding 2x extension.
	dsSIMD2d halfConstantFactor = dsMathImplMaskSIMD2d(signMask, pi2);
	dsSIMD2d bitExtensionFactor = dsMathImplMaskSIMD2d(signMask, piBitExtension);
	dsSIMD2d adjustedACosRational = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_add(halfConstantFactor, adjustedSqrt2X),
		dsSIMD2d_add(dsSIMD2d_mul(adjustedSqrt2X, adjustedBaseRational), bitExtensionFactor)),
		halfConstantFactor);

	return dsSIMD2d_select(adjustRange, adjustedACosRational, acosRational);
}

DS_MATH_EXPORT inline dsSIMD2d dsATanSIMD2d(dsSIMD2d x)
{
	dsSIMD2d pi2 = dsSIMD2d_set1(M_PI_2);
	dsSIMD2d pi4 = dsSIMD2d_set1(M_PI_4);
	dsSIMD2d pi2BitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd);
	dsSIMD2d pi4BitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd*0.5);
	dsSIMD2d tan3Pi8 = dsSIMD2d_set1(DS_TAN_3_PI_8d);
	dsSIMD2d midThreshold = dsSIMD2d_set1(0.66);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d atanTaylorP1 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_1d);
	dsSIMD2d atanTaylorP2 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_2d);
	dsSIMD2d atanTaylorP3 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_3d);
	dsSIMD2d atanTaylorP4 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_4d);
	dsSIMD2d atanTaylorP5 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_5d);

	dsSIMD2d atanTaylorQ1 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_1d);
	dsSIMD2d atanTaylorQ2 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_2d);
	dsSIMD2d atanTaylorQ3 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_3d);
	dsSIMD2d atanTaylorQ4 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_4d);
	dsSIMD2d atanTaylorQ5 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_5d);

	dsSIMD2d absX = dsSIMD2d_abs(x);

	// Choose a range to compute the taylor on.
	dsSIMD2d largeX = dsSIMD2d_neg(dsSIMD2d_rcp(absX));
	dsSIMD2d midX = dsSIMD2d_div(dsSIMD2d_sub(absX, one), dsSIMD2d_add(absX, one));

	dsSIMD2db isLarge = dsSIMD2d_cmpgt(absX, tan3Pi8);
	dsSIMD2db atLeastMid = dsSIMD2d_cmpgt(absX, midThreshold);
	absX = dsSIMD2d_select(isLarge, largeX, dsSIMD2d_select(atLeastMid, midX, absX));
	dsSIMD2d offset = dsMathImplMaskSIMD2d(atLeastMid, dsSIMD2d_select(isLarge, pi2, pi4));
	dsSIMD2d bitExtension = dsMathImplMaskSIMD2d(atLeastMid,
		dsSIMD2d_select(isLarge, pi2BitExtension, pi4BitExtension));

	dsSIMD2d x2 = dsSIMD2d_mul(absX, absX);
	dsSIMD2d pTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(atanTaylorP1, x2), atanTaylorP2), x2), atanTaylorP3),
		x2), atanTaylorP4), x2), atanTaylorP5);
	dsSIMD2d qTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(x2, atanTaylorQ1), x2), atanTaylorQ2),
		x2), atanTaylorQ3), x2), atanTaylorQ4), x2), atanTaylorQ5);
	dsSIMD2d atanRational = dsSIMD2d_add(dsSIMD2d_add(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_mul(
		dsSIMD2d_div(pTaylor, qTaylor), x2), absX), absX), bitExtension), offset);
	return dsMathImplConditionalNegateSIMD2d(atanRational, dsMathImplExtractSignBitSIMD2d(x));
}

DS_MATH_EXPORT inline dsSIMD2d dsATan2SIMD2d(dsSIMD2d y, dsSIMD2d x)
{
	dsSIMD2d pi = dsSIMD2d_set1(M_PI);
	dsSIMD2d pi2 = dsSIMD2d_set1(M_PI_2);
	dsSIMD2d zero = dsSIMD2d_set1(0.0);
	dsSIMD2db oneb = dsSIMD2db_set1(1);

	dsSIMD2db ySign = dsMathImplExtractSignBitSIMD2d(y);
	dsSIMD2db xSign = dsMathImplExtractSignBitSIMD2d(x);
	dsSIMD2db xNegativeMask = dsSIMD2db_not(
		dsSIMD2db_sub(dsSIMD2db_shiftRightConst(xSign, 63), oneb));
	dsSIMD2db yZero = dsSIMD2d_cmpeq(y, zero);
	dsSIMD2db xZero = dsSIMD2d_cmpeq(x, zero);

	// When x is positive, take the raw atan value. If x is negative, add or subtract pi when y is
	// positive or negative.
	dsSIMD2d offset = dsMathImplMaskSIMD2d(
		xNegativeMask, dsMathImplConditionalNegateSIMD2d(pi, ySign));
	dsSIMD2d result = dsSIMD2d_add(dsATanSIMD2d(dsSIMD2d_div(y, x)), offset);
	result = dsSIMD2d_select(xZero, dsMathImplConditionalNegateSIMD2d(pi2, ySign), result);
	return dsSIMD2d_select(yZero,
		dsMathImplConditionalNegateSIMD2d(dsMathImplMaskSIMD2d(xNegativeMask, pi), ySign), result);
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT,DS_SIMD_FMA)

/// @cond
DS_ALWAYS_INLINE dsSIMD2d dsTrigQuadrantAngleFMA2d(dsSIMD2d absAngles, dsSIMD2d quadrants)
{
	return dsSIMD2d_fnmadd(quadrants, dsSIMD2d_set1(DS_PI_2d_3),
		dsSIMD2d_fnmadd(quadrants, dsSIMD2d_set1(DS_PI_2d_2),
		dsSIMD2d_fnmadd(quadrants, dsSIMD2d_set1(DS_PI_2d_1), absAngles)));
}

DS_ALWAYS_INLINE void dsSinCosFMA2dImpl(
	dsSIMD2d* outSinTaylor, dsSIMD2d* outCosTaylor, dsSIMD2db* outQuadrants, dsSIMD2d angles)
{
	dsSIMD2d twoOverPi = dsSIMD2d_set1(M_2_PI);
	dsSIMD2d half = dsSIMD2d_set1(0.5);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d sinTaylor1 = dsSIMD2d_set1(DS_SIN_TAYLOR_1d);
	dsSIMD2d sinTaylor2 = dsSIMD2d_set1(DS_SIN_TAYLOR_2d);
	dsSIMD2d sinTaylor3 = dsSIMD2d_set1(DS_SIN_TAYLOR_3d);
	dsSIMD2d sinTaylor4 = dsSIMD2d_set1(DS_SIN_TAYLOR_4d);
	dsSIMD2d sinTaylor5 = dsSIMD2d_set1(DS_SIN_TAYLOR_5d);
	dsSIMD2d sinTaylor6 = dsSIMD2d_set1(DS_SIN_TAYLOR_6d);

	dsSIMD2d cosTaylor1 = dsSIMD2d_set1(DS_COS_TAYLOR_1d);
	dsSIMD2d cosTaylor2 = dsSIMD2d_set1(DS_COS_TAYLOR_2d);
	dsSIMD2d cosTaylor3 = dsSIMD2d_set1(DS_COS_TAYLOR_3d);
	dsSIMD2d cosTaylor4 = dsSIMD2d_set1(DS_COS_TAYLOR_4d);
	dsSIMD2d cosTaylor5 = dsSIMD2d_set1(DS_COS_TAYLOR_5d);
	dsSIMD2d cosTaylor6 = dsSIMD2d_set1(DS_COS_TAYLOR_6d);

	dsSIMD2d absAngles = dsSIMD2d_abs(angles);
	*outQuadrants = dsSIMD2db_round(dsSIMD2d_mul(absAngles, twoOverPi));

	dsSIMD2d quadrantAngles = dsTrigQuadrantAngleFMA2d(
		absAngles, dsSIMD2db_toDouble(*outQuadrants));
	dsSIMD2d quadrantAngles2 = dsSIMD2d_mul(quadrantAngles, quadrantAngles);
	*outSinTaylor = dsSIMD2d_fmadd(dsSIMD2d_mul(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(
		dsSIMD2d_fmadd(dsSIMD2d_fmadd(sinTaylor1, quadrantAngles2, sinTaylor2), quadrantAngles2,
		sinTaylor3), quadrantAngles2, sinTaylor4), quadrantAngles2, sinTaylor5), quadrantAngles2,
		sinTaylor6), quadrantAngles2), quadrantAngles, quadrantAngles);
	*outCosTaylor = dsSIMD2d_add(dsSIMD2d_fmsub(dsSIMD2d_mul(dsSIMD2d_fmadd(dsSIMD2d_fmadd(
		dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(cosTaylor1, quadrantAngles2, cosTaylor2),
		quadrantAngles2, cosTaylor3), quadrantAngles2, cosTaylor4), quadrantAngles2, cosTaylor5),
		quadrantAngles2, cosTaylor6), quadrantAngles2), quadrantAngles2,
		dsSIMD2d_mul(half, quadrantAngles2)), one);
}
/// @endcond

DS_MATH_EXPORT inline dsSIMD2d dsSinFMA2d(dsSIMD2d angles)
{
	dsSIMD2d sinTaylor, cosTaylor;
	dsSIMD2db quadrants;
	dsSinCosFMA2dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: sin, cos, -sin, -cos; negate if angle is negated.
	dsSIMD2db one = dsSIMD2db_set1(1);
	dsSIMD2db oddEven = dsSIMD2db_sub(dsSIMD2db_and(quadrants, one), one); // As full bool bitmask.
	dsSIMD2db halfSign = dsSIMD2db_shiftLeftConst(
		dsSIMD2db_and(quadrants, dsSIMD2db_set1(0x2)), 62);
	dsSIMD2db sign = dsSIMD2db_xor(halfSign, dsMathImplExtractSignBitSIMD2d(angles));
	return dsMathImplConditionalNegateSIMD2d(dsSIMD2d_select(oddEven, sinTaylor, cosTaylor), sign);
}

DS_MATH_EXPORT inline dsSIMD2d dsCosFMA2d(dsSIMD2d angles)
{
	dsSIMD2d sinTaylor, cosTaylor;
	dsSIMD2db quadrants;
	dsSinCosFMA2dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: cos, -sin, -cos, sin
	dsSIMD2db one = dsSIMD2db_set1(1);
	dsSIMD2db evenOdd = dsSIMD2db_and(quadrants, one);
	dsSIMD2db oddEven = dsSIMD2db_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD2db halfSign = dsSIMD2db_shiftLeftConst(
		dsSIMD2db_and(quadrants, dsSIMD2db_set1(0x2)), 62);
	dsSIMD2db sign = dsSIMD2db_xor(halfSign, dsSIMD2db_shiftLeftConst(evenOdd, 63));
	return dsMathImplConditionalNegateSIMD2d(dsSIMD2d_select(oddEven, cosTaylor, sinTaylor), sign);
}

DS_MATH_EXPORT inline void dsSinCosFMA2d(dsSIMD2d* outSin, dsSIMD2d* outCos, dsSIMD2d angles)
{
	DS_ASSERT(outSin);
	DS_ASSERT(outCos);

	dsSIMD2d sinTaylor, cosTaylor;
	dsSIMD2db quadrants;
	dsSinCosFMA2dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering for quadrants is:
	// Sine: sin, cos, -sin, -cos
	// Cosine: cos, -sin, -cos, sin
	dsSIMD2db one = dsSIMD2db_set1(1);
	dsSIMD2db evenOdd = dsSIMD2db_and(quadrants, one);
	dsSIMD2db oddEven = dsSIMD2db_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD2db halfSign = dsSIMD2db_shiftLeftConst(
		dsSIMD2db_and(quadrants, dsSIMD2db_set1(0x2)), 62);
	// Match against 2 or 3.
	dsSIMD2db sinSign = dsSIMD2db_xor(halfSign, dsMathImplExtractSignBitSIMD2d(angles));
	// Match against 1 or 2.
	dsSIMD2db cosSign = dsSIMD2db_xor(halfSign, dsSIMD2db_shiftLeftConst(evenOdd, 63));

	*outSin = dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_select(oddEven, sinTaylor, cosTaylor), sinSign);
	*outCos = dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_select(oddEven, cosTaylor, sinTaylor), cosSign);
}

DS_MATH_EXPORT inline dsSIMD2d dsTanFMA2d(dsSIMD2d angles)
{
	dsSIMD2d twoOverPi = dsSIMD2d_set1(M_2_PI);
	dsSIMD2d negOne = dsSIMD2d_set1(-1.0);
	dsSIMD2db oneb = dsSIMD2db_set1(1);

	dsSIMD2d tanTaylorP1 = dsSIMD2d_set1(DS_TAN_TAYLOR_P_1d);
	dsSIMD2d tanTaylorP2 = dsSIMD2d_set1(DS_TAN_TAYLOR_P_2d);
	dsSIMD2d tanTaylorP3 = dsSIMD2d_set1(DS_TAN_TAYLOR_P_3d);

	dsSIMD2d tanTaylorQ1 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_1d);
	dsSIMD2d tanTaylorQ2 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_2d);
	dsSIMD2d tanTaylorQ3 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_3d);
	dsSIMD2d tanTaylorQ4 = dsSIMD2d_set1(DS_TAN_TAYLOR_Q_4d);

	dsSIMD2d absAngles = dsSIMD2d_abs(angles);
	dsSIMD2db quadrants = dsSIMD2db_round(dsSIMD2d_mul(absAngles, twoOverPi));

	dsSIMD2d quadrantAngles = dsTrigQuadrantAngleSIMD2d(absAngles, dsSIMD2db_toDouble(quadrants));
	dsSIMD2d quadrantAngles2 = dsSIMD2d_mul(quadrantAngles, quadrantAngles);

	// Use a rational function rather than raw Taylor expansion.
	dsSIMD2d pTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(tanTaylorP1, quadrantAngles2, tanTaylorP2),
		quadrantAngles2, tanTaylorP3);
	dsSIMD2d qTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_add(
		quadrantAngles2, tanTaylorQ1), quadrantAngles2, tanTaylorQ2), quadrantAngles2, tanTaylorQ3),
		quadrantAngles2, tanTaylorQ4);
	dsSIMD2d tanRational = dsSIMD2d_fmadd(dsSIMD2d_mul(dsSIMD2d_div(pTaylor, qTaylor),
		quadrantAngles2), quadrantAngles, quadrantAngles);

	// Ordering of quadrants is: tan, -1/tan, tan, -1/tan
	dsSIMD2db evenOdd = dsSIMD2db_and(quadrants, oneb);
	dsSIMD2db oddEven = dsSIMD2db_sub(evenOdd, oneb);
	return dsMathImplConditionalNegateSIMD2d(dsSIMD2d_select(oddEven, tanRational,
		dsSIMD2d_div(negOne, tanRational)), dsMathImplExtractSignBitSIMD2d(angles));
}

DS_MATH_EXPORT inline dsSIMD2d dsASinFMA2d(dsSIMD2d x)
{
	DS_ASSERT(dsSIMD2db_all(dsSIMD2db_and(
		dsSIMD2d_cmpge(x, dsSIMD2d_set1(-1.0)), dsSIMD2d_cmple(x, dsSIMD2d_set1(1.0)))));

	dsSIMD2d pi4 = dsSIMD2d_set1(M_PI_4);
	dsSIMD2d pi2BitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd);
	dsSIMD2d threshold = dsSIMD2d_set1(0.625);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d asinTaylorP1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_1d);
	dsSIMD2d asinTaylorP2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_2d);
	dsSIMD2d asinTaylorP3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_3d);
	dsSIMD2d asinTaylorP4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_4d);
	dsSIMD2d asinTaylorP5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_5d);
	dsSIMD2d asinTaylorP6 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_6d);

	dsSIMD2d asinTaylorQ1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_1d);
	dsSIMD2d asinTaylorQ2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_2d);
	dsSIMD2d asinTaylorQ3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_3d);
	dsSIMD2d asinTaylorQ4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_4d);
	dsSIMD2d asinTaylorQ5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_5d);

	dsSIMD2d absX = dsSIMD2d_abs(x);

	// Choose which rational function to use based on the value.
	dsSIMD2db adjustRange = dsSIMD2d_cmpgt(absX, threshold);

	dsSIMD2d x2 = dsSIMD2d_mul(x, x);
	dsSIMD2d pTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(
		asinTaylorP1, x2, asinTaylorP2), x2, asinTaylorP3), x2, asinTaylorP4), x2, asinTaylorP5),
		x2, asinTaylorP6);
	dsSIMD2d qTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_add(
		x2, asinTaylorQ1), x2, asinTaylorQ2), x2, asinTaylorQ3), x2, asinTaylorQ4), x2,
		asinTaylorQ5);
	dsSIMD2d asinRational = dsSIMD2d_fmadd(dsSIMD2d_mul(
		dsSIMD2d_div(pTaylor, qTaylor), x2), absX, absX);

	// Compute based on asin(1 - x).
	dsSIMD2d asinTaylorR1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_1d);
	dsSIMD2d asinTaylorR2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_2d);
	dsSIMD2d asinTaylorR3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_3d);
	dsSIMD2d asinTaylorR4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_4d);
	dsSIMD2d asinTaylorR5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_5d);

	dsSIMD2d asinTaylorS1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_1d);
	dsSIMD2d asinTaylorS2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_2d);
	dsSIMD2d asinTaylorS3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_3d);
	dsSIMD2d asinTaylorS4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_4d);

	dsSIMD2d adjustedX = dsSIMD2d_sub(one, absX);
	dsSIMD2d rTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(
		asinTaylorR1, adjustedX, asinTaylorR2), adjustedX, asinTaylorR3), adjustedX, asinTaylorR4),
		adjustedX, asinTaylorR5);
	dsSIMD2d sTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_add(adjustedX,
		asinTaylorS1), adjustedX, asinTaylorS2), adjustedX, asinTaylorS3), adjustedX, asinTaylorS4);
	dsSIMD2d adjustedBaseRational = dsSIMD2d_mul(dsSIMD2d_div(rTaylor, sTaylor), adjustedX);
	dsSIMD2d adjustedSqrt2X = dsSIMD2d_sqrt(dsSIMD2d_add(adjustedX, adjustedX));
	// Take pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational) in a way that extends the bits
	// of precision.
	dsSIMD2d adjustedASinRational = dsSIMD2d_add(dsSIMD2d_sub(dsSIMD2d_sub(pi4, adjustedSqrt2X),
		dsSIMD2d_fmsub(adjustedSqrt2X, adjustedBaseRational, pi2BitExtension)), pi4);

	return dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_select(adjustRange, adjustedASinRational, asinRational),
		dsMathImplExtractSignBitSIMD2d(x));
}

DS_MATH_EXPORT inline dsSIMD2d dsACosFMA2d(dsSIMD2d x)
{
	DS_ASSERT(dsSIMD2db_all(dsSIMD2db_and(
		dsSIMD2d_cmpge(x, dsSIMD2d_set1(-1.0)), dsSIMD2d_cmple(x, dsSIMD2d_set1(1.0)))));

	dsSIMD2d pi2 = dsSIMD2d_set1(M_PI_2);
	dsSIMD2d piBitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd*2);
	dsSIMD2d threshold = dsSIMD2d_set1(0.625);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d asinTaylorP1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_1d);
	dsSIMD2d asinTaylorP2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_2d);
	dsSIMD2d asinTaylorP3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_3d);
	dsSIMD2d asinTaylorP4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_4d);
	dsSIMD2d asinTaylorP5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_5d);
	dsSIMD2d asinTaylorP6 = dsSIMD2d_set1(DS_ASIN_TAYLOR_P_6d);

	dsSIMD2d asinTaylorQ1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_1d);
	dsSIMD2d asinTaylorQ2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_2d);
	dsSIMD2d asinTaylorQ3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_3d);
	dsSIMD2d asinTaylorQ4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_4d);
	dsSIMD2d asinTaylorQ5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_Q_5d);

	dsSIMD2d absX = dsSIMD2d_abs(x);

	// Choose which rational function to use based on the value.
	// Use the identity that acos(x) = pi/2 - asin(x), applying it directly rather than calling
	// dsASinf() as that could introduce error for the adjusted range.
	dsSIMD2db adjustRange = dsSIMD2d_cmpgt(absX, threshold);

	dsSIMD2d x2 = dsSIMD2d_mul(x, x);
	dsSIMD2d pTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(
		asinTaylorP1, x2, asinTaylorP2), x2, asinTaylorP3), x2, asinTaylorP4), x2, asinTaylorP5),
		x2, asinTaylorP6);
	dsSIMD2d qTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_add(
		x2, asinTaylorQ1), x2, asinTaylorQ2), x2, asinTaylorQ3), x2, asinTaylorQ4), x2,
		asinTaylorQ5);
	dsSIMD2d acosRational = dsSIMD2d_sub(dsSIMD2d_fnmadd(dsSIMD2d_mul(
		dsSIMD2d_div(pTaylor, qTaylor), x2), x, pi2), x);

	// Compute based on asin(1 - x).
	dsSIMD2d asinTaylorR1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_1d);
	dsSIMD2d asinTaylorR2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_2d);
	dsSIMD2d asinTaylorR3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_3d);
	dsSIMD2d asinTaylorR4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_4d);
	dsSIMD2d asinTaylorR5 = dsSIMD2d_set1(DS_ASIN_TAYLOR_R_5d);

	dsSIMD2d asinTaylorS1 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_1d);
	dsSIMD2d asinTaylorS2 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_2d);
	dsSIMD2d asinTaylorS3 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_3d);
	dsSIMD2d asinTaylorS4 = dsSIMD2d_set1(DS_ASIN_TAYLOR_S_4d);

	dsSIMD2db signBit = dsMathImplExtractSignBitSIMD2d(x);
	dsSIMD2db signMask = dsSIMD2db_neg(dsSIMD2db_shiftRightConst(signBit, 63));
	dsSIMD2d adjustedX = dsSIMD2d_sub(one, absX);

	dsSIMD2d rTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(
		asinTaylorR1, adjustedX, asinTaylorR2), adjustedX, asinTaylorR3), adjustedX, asinTaylorR4),
		adjustedX, asinTaylorR5);
	dsSIMD2d sTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_add(adjustedX,
		asinTaylorS1), adjustedX, asinTaylorS2), adjustedX, asinTaylorS3), adjustedX, asinTaylorS4);
	dsSIMD2d adjustedBaseRational = dsSIMD2d_mul(dsSIMD2d_div(rTaylor, sTaylor), adjustedX);
	dsSIMD2d adjustedSqrt2X = dsMathImplConditionalNegateSIMD2d(
		dsSIMD2d_sqrt(dsSIMD2d_add(adjustedX, adjustedX)), signBit);
	// Take pi/2 - (applySign(pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational))) in a way that
	// extends the bits of precision. The sign adjustment for the factor with the bit extension
	// is provided by adjustedSqrt2X. As a result, need do
	// "- adjustSign(extesnion - adjustSign(extension))", which resolvees to
	// "+ extension - adjustSign(extension)", or conditionally adding 2x extension.
	dsSIMD2d halfConstantFactor = dsMathImplMaskSIMD2d(signMask, pi2);
	dsSIMD2d bitExtensionFactor = dsMathImplMaskSIMD2d(signMask, piBitExtension);
	dsSIMD2d adjustedACosRational = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_add(halfConstantFactor, adjustedSqrt2X),
		dsSIMD2d_fmadd(adjustedSqrt2X, adjustedBaseRational, bitExtensionFactor)),
		halfConstantFactor);

	return dsSIMD2d_select(adjustRange, adjustedACosRational, acosRational);
}

DS_MATH_EXPORT inline dsSIMD2d dsATanFMA2d(dsSIMD2d x)
{
	dsSIMD2d pi2 = dsSIMD2d_set1(M_PI_2);
	dsSIMD2d pi4 = dsSIMD2d_set1(M_PI_4);
	dsSIMD2d pi2BitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd);
	dsSIMD2d pi4BitExtension = dsSIMD2d_set1(DS_PI_2_BIT_EXTENSIONd*0.5);
	dsSIMD2d tan3Pi8 = dsSIMD2d_set1(DS_TAN_3_PI_8d);
	dsSIMD2d midThreshold = dsSIMD2d_set1(0.66);
	dsSIMD2d one = dsSIMD2d_set1(1.0);

	dsSIMD2d atanTaylorP1 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_1d);
	dsSIMD2d atanTaylorP2 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_2d);
	dsSIMD2d atanTaylorP3 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_3d);
	dsSIMD2d atanTaylorP4 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_4d);
	dsSIMD2d atanTaylorP5 = dsSIMD2d_set1(DS_ATAN_TAYLOR_P_5d);

	dsSIMD2d atanTaylorQ1 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_1d);
	dsSIMD2d atanTaylorQ2 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_2d);
	dsSIMD2d atanTaylorQ3 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_3d);
	dsSIMD2d atanTaylorQ4 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_4d);
	dsSIMD2d atanTaylorQ5 = dsSIMD2d_set1(DS_ATAN_TAYLOR_Q_5d);

	dsSIMD2d absX = dsSIMD2d_abs(x);

	// Choose a range to compute the taylor on.
	dsSIMD2d largeX = dsSIMD2d_neg(dsSIMD2d_rcp(absX));
	dsSIMD2d midX = dsSIMD2d_div(dsSIMD2d_sub(absX, one), dsSIMD2d_add(absX, one));

	dsSIMD2db isLarge = dsSIMD2d_cmpgt(absX, tan3Pi8);
	dsSIMD2db atLeastMid = dsSIMD2d_cmpgt(absX, midThreshold);
	absX = dsSIMD2d_select(isLarge, largeX, dsSIMD2d_select(atLeastMid, midX, absX));
	dsSIMD2d offset = dsMathImplMaskSIMD2d(atLeastMid, dsSIMD2d_select(isLarge, pi2, pi4));
	dsSIMD2d bitExtension = dsMathImplMaskSIMD2d(atLeastMid,
		dsSIMD2d_select(isLarge, pi2BitExtension, pi4BitExtension));

	dsSIMD2d x2 = dsSIMD2d_mul(absX, absX);
	dsSIMD2d pTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(atanTaylorP1, x2,
		atanTaylorP2), x2, atanTaylorP3), x2, atanTaylorP4), x2, atanTaylorP5);
	dsSIMD2d qTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_add(
		x2, atanTaylorQ1), x2, atanTaylorQ2), x2, atanTaylorQ3), x2, atanTaylorQ4), x2,
		atanTaylorQ5);
	dsSIMD2d atanRational = dsSIMD2d_add(dsSIMD2d_add(dsSIMD2d_fmadd(dsSIMD2d_mul(
		dsSIMD2d_div(pTaylor, qTaylor), x2), absX, absX), bitExtension), offset);
	return dsMathImplConditionalNegateSIMD2d(atanRational, dsMathImplExtractSignBitSIMD2d(x));
}

DS_MATH_EXPORT inline dsSIMD2d dsATan2FMA2d(dsSIMD2d y, dsSIMD2d x)
{
	dsSIMD2d pi = dsSIMD2d_set1(M_PI);
	dsSIMD2d pi2 = dsSIMD2d_set1(M_PI_2);
	dsSIMD2d zero = dsSIMD2d_set1(0.0);
	dsSIMD2db oneb = dsSIMD2db_set1(1);

	dsSIMD2db ySign = dsMathImplExtractSignBitSIMD2d(y);
	dsSIMD2db xSign = dsMathImplExtractSignBitSIMD2d(x);
	dsSIMD2db xNegativeMask = dsSIMD2db_not(
		dsSIMD2db_sub(dsSIMD2db_shiftRightConst(xSign, 63), oneb));
	dsSIMD2db yZero = dsSIMD2d_cmpeq(y, zero);
	dsSIMD2db xZero = dsSIMD2d_cmpeq(x, zero);

	// When x is positive, take the raw atan value. If x is negative, add or subtract pi when y is
	// positive or negative.
	dsSIMD2d offset = dsMathImplMaskSIMD2d(
		xNegativeMask, dsMathImplConditionalNegateSIMD2d(pi, ySign));
	dsSIMD2d result = dsSIMD2d_add(dsATanFMA2d(dsSIMD2d_div(y, x)), offset);
	result = dsSIMD2d_select(xZero, dsMathImplConditionalNegateSIMD2d(pi2, ySign), result);
	return dsSIMD2d_select(yZero,
		dsMathImplConditionalNegateSIMD2d(dsMathImplMaskSIMD2d(xNegativeMask, pi), ySign), result);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_INT,DS_SIMD_FMA)

/// @cond
DS_ALWAYS_INLINE dsSIMD4d dsTrigQuadrantAngleSIMD4d(dsSIMD4d absAngles, dsSIMD4d quadrants)
{
#if DS_DETERMINISTIC_MATH
	return dsSIMD4d_sub(dsSIMD4d_sub(dsSIMD4d_sub(absAngles,
		dsSIMD4d_mul(quadrants, dsSIMD4d_set1(DS_PI_2d_1))),
		dsSIMD4d_mul(quadrants, dsSIMD4d_set1(DS_PI_2d_2))),
		dsSIMD4d_mul(quadrants, dsSIMD4d_set1(DS_PI_2d_3)));
#else
	return dsSIMD4d_fnmadd(quadrants, dsSIMD4d_set1(DS_PI_2d_3),
		dsSIMD4d_fnmadd(quadrants, dsSIMD4d_set1(DS_PI_2d_2),
		dsSIMD4d_fnmadd(quadrants, dsSIMD4d_set1(DS_PI_2d_1), absAngles)));
#endif
}

DS_ALWAYS_INLINE void dsSinCosSIMD4dImpl(
	dsSIMD4d* outSinTaylor, dsSIMD4d* outCosTaylor, dsSIMD4db* outQuadrants, dsSIMD4d angles)
{
	dsSIMD4d twoOverPi = dsSIMD4d_set1(M_2_PI);
	dsSIMD4d half = dsSIMD4d_set1(0.5);
	dsSIMD4d one = dsSIMD4d_set1(1.0);

	dsSIMD4d sinTaylor1 = dsSIMD4d_set1(DS_SIN_TAYLOR_1d);
	dsSIMD4d sinTaylor2 = dsSIMD4d_set1(DS_SIN_TAYLOR_2d);
	dsSIMD4d sinTaylor3 = dsSIMD4d_set1(DS_SIN_TAYLOR_3d);
	dsSIMD4d sinTaylor4 = dsSIMD4d_set1(DS_SIN_TAYLOR_4d);
	dsSIMD4d sinTaylor5 = dsSIMD4d_set1(DS_SIN_TAYLOR_5d);
	dsSIMD4d sinTaylor6 = dsSIMD4d_set1(DS_SIN_TAYLOR_6d);

	dsSIMD4d cosTaylor1 = dsSIMD4d_set1(DS_COS_TAYLOR_1d);
	dsSIMD4d cosTaylor2 = dsSIMD4d_set1(DS_COS_TAYLOR_2d);
	dsSIMD4d cosTaylor3 = dsSIMD4d_set1(DS_COS_TAYLOR_3d);
	dsSIMD4d cosTaylor4 = dsSIMD4d_set1(DS_COS_TAYLOR_4d);
	dsSIMD4d cosTaylor5 = dsSIMD4d_set1(DS_COS_TAYLOR_5d);
	dsSIMD4d cosTaylor6 = dsSIMD4d_set1(DS_COS_TAYLOR_6d);

	dsSIMD4d absAngles = dsSIMD4d_abs(angles);
	*outQuadrants = dsSIMD4db_round(dsSIMD4d_mul(absAngles, twoOverPi));

	dsSIMD4d quadrantAngles = dsTrigQuadrantAngleSIMD4d(
		absAngles, dsSIMD4db_toDouble(*outQuadrants));
	dsSIMD4d quadrantAngles2 = dsSIMD4d_mul(quadrantAngles, quadrantAngles);
#if DS_DETERMINISTIC_MATH
	*outSinTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(
		sinTaylor1, quadrantAngles2), sinTaylor2), quadrantAngles2), sinTaylor3), quadrantAngles2),
		sinTaylor4), quadrantAngles2), sinTaylor5), quadrantAngles2), sinTaylor6), quadrantAngles2),
		quadrantAngles), quadrantAngles);
	*outCosTaylor = dsSIMD4d_add(dsSIMD4d_sub(dsSIMD4d_mul(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(
		dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(cosTaylor1, quadrantAngles2), cosTaylor2), quadrantAngles2), cosTaylor3),
		quadrantAngles2), cosTaylor4), quadrantAngles2), cosTaylor5), quadrantAngles2),
		cosTaylor6), quadrantAngles2), quadrantAngles2), dsSIMD4d_mul(half, quadrantAngles2)), one);
#else
	*outSinTaylor = dsSIMD4d_fmadd(dsSIMD4d_mul(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(
		dsSIMD4d_fmadd(dsSIMD4d_fmadd(sinTaylor1, quadrantAngles2, sinTaylor2), quadrantAngles2,
		sinTaylor3), quadrantAngles2, sinTaylor4), quadrantAngles2, sinTaylor5), quadrantAngles2,
		sinTaylor6), quadrantAngles2), quadrantAngles, quadrantAngles);
	*outCosTaylor = dsSIMD4d_add(dsSIMD4d_fmsub(dsSIMD4d_mul(dsSIMD4d_fmadd(dsSIMD4d_fmadd(
		dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(cosTaylor1, quadrantAngles2, cosTaylor2),
		quadrantAngles2, cosTaylor3), quadrantAngles2, cosTaylor4), quadrantAngles2, cosTaylor5),
		quadrantAngles2, cosTaylor6), quadrantAngles2), quadrantAngles2,
		dsSIMD4d_mul(half, quadrantAngles2)), one);
#endif
}
/// @endcond

DS_MATH_EXPORT inline dsSIMD4d dsSinSIMD4d(dsSIMD4d angles)
{
	dsSIMD4d sinTaylor, cosTaylor;
	dsSIMD4db quadrants;
	dsSinCosSIMD4dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: sin, cos, -sin, -cos; negate if angle is negated.
	dsSIMD4db one = dsSIMD4db_set1(1);
	dsSIMD4db oddEven = dsSIMD4db_sub(dsSIMD4db_and(quadrants, one), one); // As full bool bitmask.
	dsSIMD4db halfSign = dsSIMD4db_shiftLeftConst(
		dsSIMD4db_and(quadrants, dsSIMD4db_set1(0x2)), 62);
	dsSIMD4db sign = dsSIMD4db_xor(halfSign, dsMathImplExtractSignBitSIMD4d(angles));
	return dsMathImplConditionalNegateSIMD4d(dsSIMD4d_select(oddEven, sinTaylor, cosTaylor), sign);
}

DS_MATH_EXPORT inline dsSIMD4d dsCosSIMD4d(dsSIMD4d angles)
{
	dsSIMD4d sinTaylor, cosTaylor;
	dsSIMD4db quadrants;
	dsSinCosSIMD4dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering: cos, -sin, -cos, sin
	dsSIMD4db one = dsSIMD4db_set1(1);
	dsSIMD4db evenOdd = dsSIMD4db_and(quadrants, one);
	dsSIMD4db oddEven = dsSIMD4db_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD4db halfSign = dsSIMD4db_shiftLeftConst(
		dsSIMD4db_and(quadrants, dsSIMD4db_set1(0x2)), 62);
	dsSIMD4db sign = dsSIMD4db_xor(halfSign, dsSIMD4db_shiftLeftConst(evenOdd, 63));
	return dsMathImplConditionalNegateSIMD4d(dsSIMD4d_select(oddEven, cosTaylor, sinTaylor), sign);
}

DS_MATH_EXPORT inline void dsSinCosSIMD4d(dsSIMD4d* outSin, dsSIMD4d* outCos, dsSIMD4d angles)
{
	DS_ASSERT(outSin);
	DS_ASSERT(outCos);

	dsSIMD4d sinTaylor, cosTaylor;
	dsSIMD4db quadrants;
	dsSinCosSIMD4dImpl(&sinTaylor, &cosTaylor, &quadrants, angles);

	// Ordering for quadrants is:
	// Sine: sin, cos, -sin, -cos
	// Cosine: cos, -sin, -cos, sin
	dsSIMD4db one = dsSIMD4db_set1(1);
	dsSIMD4db evenOdd = dsSIMD4db_and(quadrants, one);
	dsSIMD4db oddEven = dsSIMD4db_sub(evenOdd, one); // As full bool bitmask.
	dsSIMD4db halfSign = dsSIMD4db_shiftLeftConst(
		dsSIMD4db_and(quadrants, dsSIMD4db_set1(0x2)), 62);
	// Match against 2 or 3.
	dsSIMD4db sinSign = dsSIMD4db_xor(halfSign, dsMathImplExtractSignBitSIMD4d(angles));
	// Match against 1 or 2.
	dsSIMD4db cosSign = dsSIMD4db_xor(halfSign, dsSIMD4db_shiftLeftConst(evenOdd, 63));

	*outSin = dsMathImplConditionalNegateSIMD4d(
		dsSIMD4d_select(oddEven, sinTaylor, cosTaylor), sinSign);
	*outCos = dsMathImplConditionalNegateSIMD4d(
		dsSIMD4d_select(oddEven, cosTaylor, sinTaylor), cosSign);
}

DS_MATH_EXPORT inline dsSIMD4d dsTanSIMD4d(dsSIMD4d angles)
{
	dsSIMD4d twoOverPi = dsSIMD4d_set1(M_2_PI);
	dsSIMD4d negOne = dsSIMD4d_set1(-1.0);
	dsSIMD4db oneb = dsSIMD4db_set1(0x1);

	dsSIMD4d tanTaylorP1 = dsSIMD4d_set1(DS_TAN_TAYLOR_P_1d);
	dsSIMD4d tanTaylorP2 = dsSIMD4d_set1(DS_TAN_TAYLOR_P_2d);
	dsSIMD4d tanTaylorP3 = dsSIMD4d_set1(DS_TAN_TAYLOR_P_3d);

	dsSIMD4d tanTaylorQ1 = dsSIMD4d_set1(DS_TAN_TAYLOR_Q_1d);
	dsSIMD4d tanTaylorQ2 = dsSIMD4d_set1(DS_TAN_TAYLOR_Q_2d);
	dsSIMD4d tanTaylorQ3 = dsSIMD4d_set1(DS_TAN_TAYLOR_Q_3d);
	dsSIMD4d tanTaylorQ4 = dsSIMD4d_set1(DS_TAN_TAYLOR_Q_4d);

	dsSIMD4d absAngles = dsSIMD4d_abs(angles);
	dsSIMD4db quadrants = dsSIMD4db_round(dsSIMD4d_mul(absAngles, twoOverPi));

	dsSIMD4d quadrantAngles = dsTrigQuadrantAngleSIMD4d(absAngles, dsSIMD4db_toDouble(quadrants));
	dsSIMD4d quadrantAngles2 = dsSIMD4d_mul(quadrantAngles, quadrantAngles);

	// Use a rational function rather than raw Taylor expansion.
#if DS_DETERMINISTIC_MATH
	dsSIMD4d pTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(
		tanTaylorP1, quadrantAngles2), tanTaylorP2), quadrantAngles2), tanTaylorP3);
	dsSIMD4d qTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(quadrantAngles2, tanTaylorQ1), quadrantAngles2), tanTaylorQ2),
		quadrantAngles2), tanTaylorQ3), quadrantAngles2), tanTaylorQ4);
	dsSIMD4d tanRational = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_mul(dsSIMD4d_div(pTaylor, qTaylor),
		quadrantAngles2), quadrantAngles), quadrantAngles);
#else
	dsSIMD4d pTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(tanTaylorP1, quadrantAngles2, tanTaylorP2),
		quadrantAngles2, tanTaylorP3);
	dsSIMD4d qTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_add(
		quadrantAngles2, tanTaylorQ1), quadrantAngles2, tanTaylorQ2), quadrantAngles2, tanTaylorQ3),
		quadrantAngles2, tanTaylorQ4);
	dsSIMD4d tanRational = dsSIMD4d_fmadd(dsSIMD4d_mul(dsSIMD4d_div(pTaylor, qTaylor),
		quadrantAngles2), quadrantAngles, quadrantAngles);
#endif

	// Ordering of quadrants is: tan, -1/tan, tan, -1/tan
	dsSIMD4db evenOdd = dsSIMD4db_and(quadrants, oneb);
	dsSIMD4db oddEven = dsSIMD4db_sub(evenOdd, oneb);
	return dsMathImplConditionalNegateSIMD4d(dsSIMD4d_select(oddEven, tanRational,
		dsSIMD4d_div(negOne, tanRational)), dsMathImplExtractSignBitSIMD4d(angles));
}

DS_MATH_EXPORT inline dsSIMD4d dsASinSIMD4d(dsSIMD4d x)
{
	DS_ASSERT(dsSIMD4db_all(dsSIMD4db_and(
		dsSIMD4d_cmpge(x, dsSIMD4d_set1(-1.0)), dsSIMD4d_cmple(x, dsSIMD4d_set1(1.0)))));

	dsSIMD4d pi4 = dsSIMD4d_set1(M_PI_4);
	dsSIMD4d pi2BitExtension = dsSIMD4d_set1(DS_PI_2_BIT_EXTENSIONd);
	dsSIMD4d threshold = dsSIMD4d_set1(0.625);
	dsSIMD4d one = dsSIMD4d_set1(1.0);

	dsSIMD4d asinTaylorP1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_1d);
	dsSIMD4d asinTaylorP2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_2d);
	dsSIMD4d asinTaylorP3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_3d);
	dsSIMD4d asinTaylorP4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_4d);
	dsSIMD4d asinTaylorP5 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_5d);
	dsSIMD4d asinTaylorP6 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_6d);

	dsSIMD4d asinTaylorQ1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_1d);
	dsSIMD4d asinTaylorQ2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_2d);
	dsSIMD4d asinTaylorQ3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_3d);
	dsSIMD4d asinTaylorQ4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_4d);
	dsSIMD4d asinTaylorQ5 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_5d);

	dsSIMD4d absX = dsSIMD4d_abs(x);

	// Choose which rational function to use based on the value.
	dsSIMD4db adjustRange = dsSIMD4d_cmpgt(absX, threshold);

	dsSIMD4d x2 = dsSIMD4d_mul(x, x);
#if DS_DETERMINISTIC_MATH
	dsSIMD4d pTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(asinTaylorP1, x2),
		asinTaylorP2), x2), asinTaylorP3), x2), asinTaylorP4), x2), asinTaylorP5), x2),
		asinTaylorP6);
	dsSIMD4d qTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(x2, asinTaylorQ1), x2), asinTaylorQ2),
		x2), asinTaylorQ3), x2), asinTaylorQ4), x2), asinTaylorQ5);
#else
	dsSIMD4d pTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(
		asinTaylorP1, x2, asinTaylorP2), x2, asinTaylorP3), x2, asinTaylorP4), x2, asinTaylorP5),
		x2, asinTaylorP6);
	dsSIMD4d qTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_add(
		x2, asinTaylorQ1), x2, asinTaylorQ2), x2, asinTaylorQ3), x2, asinTaylorQ4), x2,
		asinTaylorQ5);
#endif
	dsSIMD4d asinRational = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_mul(
		dsSIMD4d_div(pTaylor, qTaylor), x2), absX), absX);

	// Compute based on asin(1 - x).
	dsSIMD4d asinTaylorR1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_1d);
	dsSIMD4d asinTaylorR2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_2d);
	dsSIMD4d asinTaylorR3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_3d);
	dsSIMD4d asinTaylorR4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_4d);
	dsSIMD4d asinTaylorR5 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_5d);

	dsSIMD4d asinTaylorS1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_1d);
	dsSIMD4d asinTaylorS2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_2d);
	dsSIMD4d asinTaylorS3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_3d);
	dsSIMD4d asinTaylorS4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_4d);

	dsSIMD4d adjustedX = dsSIMD4d_sub(one, absX);
#if DS_DETERMINISTIC_MATH
	dsSIMD4d rTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(asinTaylorR1, adjustedX), asinTaylorR2), adjustedX),
		asinTaylorR3), adjustedX), asinTaylorR4), adjustedX), asinTaylorR5);
	dsSIMD4d sTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(adjustedX, asinTaylorS1), adjustedX), asinTaylorS2), adjustedX),
		asinTaylorS3), adjustedX), asinTaylorS4);
#else
	dsSIMD4d rTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(
		asinTaylorR1, adjustedX, asinTaylorR2), adjustedX, asinTaylorR3), adjustedX, asinTaylorR4),
		adjustedX, asinTaylorR5);
	dsSIMD4d sTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_add(adjustedX,
		asinTaylorS1), adjustedX, asinTaylorS2), adjustedX, asinTaylorS3), adjustedX, asinTaylorS4);
#endif
	dsSIMD4d adjustedBaseRational = dsSIMD4d_mul(dsSIMD4d_div(rTaylor, sTaylor), adjustedX);
	dsSIMD4d adjustedSqrt2X = dsSIMD4d_sqrt(dsSIMD4d_add(adjustedX, adjustedX));
	// Take pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational) in a way that extends the bits
	// of precision.
#if DS_DETERMINISTIC_MATH
	dsSIMD4d adjustedASinRational = dsSIMD4d_add(dsSIMD4d_sub(dsSIMD4d_sub(pi4, adjustedSqrt2X),
		dsSIMD4d_sub(dsSIMD4d_mul(adjustedSqrt2X, adjustedBaseRational), pi2BitExtension)), pi4);
#else
	dsSIMD4d adjustedASinRational = dsSIMD4d_add(dsSIMD4d_sub(dsSIMD4d_sub(pi4, adjustedSqrt2X),
		dsSIMD4d_fmsub(adjustedSqrt2X, adjustedBaseRational, pi2BitExtension)), pi4);
#endif

	return dsMathImplConditionalNegateSIMD4d(
		dsSIMD4d_select(adjustRange, adjustedASinRational, asinRational),
		dsMathImplExtractSignBitSIMD4d(x));
}

DS_MATH_EXPORT inline dsSIMD4d dsACosSIMD4d(dsSIMD4d x)
{
	DS_ASSERT(dsSIMD4db_all(dsSIMD4db_and(
		dsSIMD4d_cmpge(x, dsSIMD4d_set1(-1.0)), dsSIMD4d_cmple(x, dsSIMD4d_set1(1.0)))));

	dsSIMD4d pi2 = dsSIMD4d_set1(M_PI_2);
	dsSIMD4d piBitExtension = dsSIMD4d_set1(DS_PI_2_BIT_EXTENSIONd*2);
	dsSIMD4d threshold = dsSIMD4d_set1(0.625);
	dsSIMD4d one = dsSIMD4d_set1(1.0);

	dsSIMD4d asinTaylorP1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_1d);
	dsSIMD4d asinTaylorP2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_2d);
	dsSIMD4d asinTaylorP3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_3d);
	dsSIMD4d asinTaylorP4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_4d);
	dsSIMD4d asinTaylorP5 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_5d);
	dsSIMD4d asinTaylorP6 = dsSIMD4d_set1(DS_ASIN_TAYLOR_P_6d);

	dsSIMD4d asinTaylorQ1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_1d);
	dsSIMD4d asinTaylorQ2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_2d);
	dsSIMD4d asinTaylorQ3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_3d);
	dsSIMD4d asinTaylorQ4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_4d);
	dsSIMD4d asinTaylorQ5 = dsSIMD4d_set1(DS_ASIN_TAYLOR_Q_5d);

	dsSIMD4d absX = dsSIMD4d_abs(x);

	// Choose which rational function to use based on the value.
	// Use the identity that acos(x) = pi/2 - asin(x), applying it directly rather than calling
	// dsASinf() as that could introduce error for the adjusted range.
	dsSIMD4db adjustRange = dsSIMD4d_cmpgt(absX, threshold);

	dsSIMD4d x2 = dsSIMD4d_mul(x, x);
#if DS_DETERMINISTIC_MATH
	dsSIMD4d pTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(asinTaylorP1, x2),
		asinTaylorP2), x2), asinTaylorP3), x2), asinTaylorP4), x2), asinTaylorP5), x2),
		asinTaylorP6);
	dsSIMD4d qTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(x2, asinTaylorQ1), x2), asinTaylorQ2),
		x2), asinTaylorQ3), x2), asinTaylorQ4), x2), asinTaylorQ5);
	dsSIMD4d acosRational = dsSIMD4d_sub(dsSIMD4d_sub(pi2, dsSIMD4d_mul(dsSIMD4d_mul(
		dsSIMD4d_div(pTaylor, qTaylor), x2), x)), x);
#else
	dsSIMD4d pTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(
		asinTaylorP1, x2, asinTaylorP2), x2, asinTaylorP3), x2, asinTaylorP4), x2, asinTaylorP5),
		x2, asinTaylorP6);
	dsSIMD4d qTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_add(
		x2, asinTaylorQ1), x2, asinTaylorQ2), x2, asinTaylorQ3), x2, asinTaylorQ4), x2,
		asinTaylorQ5);
	dsSIMD4d acosRational = dsSIMD4d_sub(dsSIMD4d_fnmadd(dsSIMD4d_mul(
		dsSIMD4d_div(pTaylor, qTaylor), x2), x, pi2), x);
#endif

	// Compute based on asin(1 - x).
	dsSIMD4d asinTaylorR1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_1d);
	dsSIMD4d asinTaylorR2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_2d);
	dsSIMD4d asinTaylorR3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_3d);
	dsSIMD4d asinTaylorR4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_4d);
	dsSIMD4d asinTaylorR5 = dsSIMD4d_set1(DS_ASIN_TAYLOR_R_5d);

	dsSIMD4d asinTaylorS1 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_1d);
	dsSIMD4d asinTaylorS2 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_2d);
	dsSIMD4d asinTaylorS3 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_3d);
	dsSIMD4d asinTaylorS4 = dsSIMD4d_set1(DS_ASIN_TAYLOR_S_4d);

	dsSIMD4db signBit = dsMathImplExtractSignBitSIMD4d(x);
	dsSIMD4db signMask = dsSIMD4db_neg(dsSIMD4db_shiftRightConst(signBit, 63));
	dsSIMD4d adjustedX = dsSIMD4d_sub(one, absX);

#if DS_DETERMINISTIC_MATH
	dsSIMD4d rTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(asinTaylorR1, adjustedX), asinTaylorR2), adjustedX),
		asinTaylorR3), adjustedX), asinTaylorR4), adjustedX), asinTaylorR5);
	dsSIMD4d sTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(adjustedX, asinTaylorS1), adjustedX), asinTaylorS2), adjustedX),
		asinTaylorS3), adjustedX), asinTaylorS4);
#else
	dsSIMD4d rTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(
		asinTaylorR1, adjustedX, asinTaylorR2), adjustedX, asinTaylorR3), adjustedX, asinTaylorR4),
		adjustedX, asinTaylorR5);
	dsSIMD4d sTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_add(adjustedX,
		asinTaylorS1), adjustedX, asinTaylorS2), adjustedX, asinTaylorS3), adjustedX, asinTaylorS4);
#endif
	dsSIMD4d adjustedBaseRational = dsSIMD4d_mul(dsSIMD4d_div(rTaylor, sTaylor), adjustedX);
	dsSIMD4d adjustedSqrt2X = dsMathImplConditionalNegateSIMD4d(
		dsSIMD4d_sqrt(dsSIMD4d_add(adjustedX, adjustedX)), signBit);
	// Take pi/2 - (applySign(pi/2 - adjustedSqrt2X*(1 + adjustedBaseRational))) in a way that
	// extends the bits of precision. The sign adjustment for the factor with the bit extension
	// is provided by adjustedSqrt2X. As a result, need do
	// "- adjustSign(extesnion - adjustSign(extension))", which resolvees to
	// "+ extension - adjustSign(extension)", or conditionally adding 2x extension.
	dsSIMD4d halfConstantFactor = dsMathImplMaskSIMD4d(signMask, pi2);
	dsSIMD4d bitExtensionFactor = dsMathImplMaskSIMD4d(signMask, piBitExtension);
#if DS_DETERMINISTIC_MATH
	dsSIMD4d adjustedACosRational = dsSIMD4d_add(dsSIMD4d_add(
		dsSIMD4d_add(halfConstantFactor, adjustedSqrt2X),
		dsSIMD4d_add(dsSIMD4d_mul(adjustedSqrt2X, adjustedBaseRational), bitExtensionFactor)),
		halfConstantFactor);
#else
	dsSIMD4d adjustedACosRational = dsSIMD4d_add(dsSIMD4d_add(
		dsSIMD4d_add(halfConstantFactor, adjustedSqrt2X),
		dsSIMD4d_fmadd(adjustedSqrt2X, adjustedBaseRational, bitExtensionFactor)),
		halfConstantFactor);
#endif

	return dsSIMD4d_select(adjustRange, adjustedACosRational, acosRational);
}

DS_MATH_EXPORT inline dsSIMD4d dsATanSIMD4d(dsSIMD4d x)
{
	dsSIMD4d pi2 = dsSIMD4d_set1(M_PI_2);
	dsSIMD4d pi4 = dsSIMD4d_set1(M_PI_4);
	dsSIMD4d pi2BitExtension = dsSIMD4d_set1(DS_PI_2_BIT_EXTENSIONd);
	dsSIMD4d pi4BitExtension = dsSIMD4d_set1(DS_PI_2_BIT_EXTENSIONd*0.5);
	dsSIMD4d tan3Pi8 = dsSIMD4d_set1(DS_TAN_3_PI_8d);
	dsSIMD4d midThreshold = dsSIMD4d_set1(0.66);
	dsSIMD4d one = dsSIMD4d_set1(1.0);

	dsSIMD4d atanTaylorP1 = dsSIMD4d_set1(DS_ATAN_TAYLOR_P_1d);
	dsSIMD4d atanTaylorP2 = dsSIMD4d_set1(DS_ATAN_TAYLOR_P_2d);
	dsSIMD4d atanTaylorP3 = dsSIMD4d_set1(DS_ATAN_TAYLOR_P_3d);
	dsSIMD4d atanTaylorP4 = dsSIMD4d_set1(DS_ATAN_TAYLOR_P_4d);
	dsSIMD4d atanTaylorP5 = dsSIMD4d_set1(DS_ATAN_TAYLOR_P_5d);

	dsSIMD4d atanTaylorQ1 = dsSIMD4d_set1(DS_ATAN_TAYLOR_Q_1d);
	dsSIMD4d atanTaylorQ2 = dsSIMD4d_set1(DS_ATAN_TAYLOR_Q_2d);
	dsSIMD4d atanTaylorQ3 = dsSIMD4d_set1(DS_ATAN_TAYLOR_Q_3d);
	dsSIMD4d atanTaylorQ4 = dsSIMD4d_set1(DS_ATAN_TAYLOR_Q_4d);
	dsSIMD4d atanTaylorQ5 = dsSIMD4d_set1(DS_ATAN_TAYLOR_Q_5d);

	dsSIMD4d absX = dsSIMD4d_abs(x);

	// Choose a range to compute the taylor on.
	dsSIMD4d largeX = dsSIMD4d_neg(dsSIMD4d_rcp(absX));
	dsSIMD4d midX = dsSIMD4d_div(dsSIMD4d_sub(absX, one), dsSIMD4d_add(absX, one));

	dsSIMD4db isLarge = dsSIMD4d_cmpgt(absX, tan3Pi8);
	dsSIMD4db atLeastMid = dsSIMD4d_cmpgt(absX, midThreshold);
	absX = dsSIMD4d_select(isLarge, largeX, dsSIMD4d_select(atLeastMid, midX, absX));
	dsSIMD4d offset = dsMathImplMaskSIMD4d(atLeastMid, dsSIMD4d_select(isLarge, pi2, pi4));
	dsSIMD4d bitExtension = dsMathImplMaskSIMD4d(atLeastMid,
		dsSIMD4d_select(isLarge, pi2BitExtension, pi4BitExtension));

	dsSIMD4d x2 = dsSIMD4d_mul(absX, absX);
#if DS_DETERMINISTIC_MATH
	dsSIMD4d pTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(atanTaylorP1, x2), atanTaylorP2), x2), atanTaylorP3),
		x2), atanTaylorP4), x2), atanTaylorP5);
	dsSIMD4d qTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(x2, atanTaylorQ1), x2), atanTaylorQ2),
		x2), atanTaylorQ3), x2), atanTaylorQ4), x2), atanTaylorQ5);
	dsSIMD4d atanRational = dsSIMD4d_add(dsSIMD4d_add(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_mul(
		dsSIMD4d_div(pTaylor, qTaylor), x2), absX), absX), bitExtension), offset);
#else
	dsSIMD4d pTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(atanTaylorP1, x2,
		atanTaylorP2), x2, atanTaylorP3), x2, atanTaylorP4), x2, atanTaylorP5);
	dsSIMD4d qTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_add(
		x2, atanTaylorQ1), x2, atanTaylorQ2), x2, atanTaylorQ3), x2, atanTaylorQ4), x2,
		atanTaylorQ5);
	dsSIMD4d atanRational = dsSIMD4d_add(dsSIMD4d_add(dsSIMD4d_fmadd(dsSIMD4d_mul(
		dsSIMD4d_div(pTaylor, qTaylor), x2), absX, absX), bitExtension), offset);
#endif
	return dsMathImplConditionalNegateSIMD4d(atanRational, dsMathImplExtractSignBitSIMD4d(x));
}

DS_MATH_EXPORT inline dsSIMD4d dsATan2SIMD4d(dsSIMD4d y, dsSIMD4d x)
{
	dsSIMD4d pi = dsSIMD4d_set1(M_PI);
	dsSIMD4d pi2 = dsSIMD4d_set1(M_PI_2);
	dsSIMD4d zero = dsSIMD4d_set1(0.0);
	dsSIMD4db oneb = dsSIMD4db_set1(1);

	dsSIMD4db ySign = dsMathImplExtractSignBitSIMD4d(y);
	dsSIMD4db xSign = dsMathImplExtractSignBitSIMD4d(x);
	dsSIMD4db xNegativeMask = dsSIMD4db_not(
		dsSIMD4db_sub(dsSIMD4db_shiftRightConst(xSign, 63), oneb));
	dsSIMD4db yZero = dsSIMD4d_cmpeq(y, zero);
	dsSIMD4db xZero = dsSIMD4d_cmpeq(x, zero);

	// When x is positive, take the raw atan value. If x is negative, add or subtract pi when y is
	// positive or negative.
	dsSIMD4d offset = dsMathImplMaskSIMD4d(
		xNegativeMask, dsMathImplConditionalNegateSIMD4d(pi, ySign));
	dsSIMD4d result = dsSIMD4d_add(dsATanSIMD4d(dsSIMD4d_div(y, x)), offset);
	result = dsSIMD4d_select(xZero, dsMathImplConditionalNegateSIMD4d(pi2, ySign), result);
	return dsSIMD4d_select(yZero,
		dsMathImplConditionalNegateSIMD4d(dsMathImplMaskSIMD4d(xNegativeMask, pi), ySign), result);
}

DS_SIMD_END()

#endif // DS_HAS_SIMD

#undef DS_PI_2f_1
#undef DS_PI_2f_2
#undef DS_PI_2f_3

#undef DS_PI_2d_1
#undef DS_PI_2d_2
#undef DS_PI_2d_3

#undef DS_PI_2_BIT_EXTENSIONd

#undef DS_TAN_3_PI_8f
#undef DS_TAN_PI_8f

#undef DS_TAN_3_PI_8d

#undef DS_SIN_TAYLOR_1f
#undef DS_SIN_TAYLOR_2f
#undef DS_SIN_TAYLOR_3f

#undef DS_SIN_TAYLOR_1d
#undef DS_SIN_TAYLOR_2d
#undef DS_SIN_TAYLOR_3d
#undef DS_SIN_TAYLOR_4d
#undef DS_SIN_TAYLOR_5d
#undef DS_SIN_TAYLOR_6d

#undef DS_COS_TAYLOR_1f
#undef DS_COS_TAYLOR_2f
#undef DS_COS_TAYLOR_3f

#undef DS_COS_TAYLOR_1d
#undef DS_COS_TAYLOR_2d
#undef DS_COS_TAYLOR_3d
#undef DS_COS_TAYLOR_4d
#undef DS_COS_TAYLOR_5d
#undef DS_COS_TAYLOR_6d

#undef DS_TAN_TAYLOR_1f
#undef DS_TAN_TAYLOR_2f
#undef DS_TAN_TAYLOR_3f
#undef DS_TAN_TAYLOR_4f
#undef DS_TAN_TAYLOR_5f
#undef DS_TAN_TAYLOR_6f

#undef DS_TAN_TAYLOR_P_1d
#undef DS_TAN_TAYLOR_P_2d
#undef DS_TAN_TAYLOR_P_3d

#undef DS_TAN_TAYLOR_Q_1d
#undef DS_TAN_TAYLOR_Q_2d
#undef DS_TAN_TAYLOR_Q_3d
#undef DS_TAN_TAYLOR_Q_4d

#undef DS_ASIN_TAYLOR_1f
#undef DS_ASIN_TAYLOR_2f
#undef DS_ASIN_TAYLOR_3f
#undef DS_ASIN_TAYLOR_4f
#undef DS_ASIN_TAYLOR_5f

#undef DS_ASIN_TAYLOR_P_1d
#undef DS_ASIN_TAYLOR_P_2d
#undef DS_ASIN_TAYLOR_P_3d
#undef DS_ASIN_TAYLOR_P_4d
#undef DS_ASIN_TAYLOR_P_5d
#undef DS_ASIN_TAYLOR_P_6d

#undef DS_ASIN_TAYLOR_Q_1d
#undef DS_ASIN_TAYLOR_Q_2d
#undef DS_ASIN_TAYLOR_Q_3d
#undef DS_ASIN_TAYLOR_Q_4d

#undef DS_ASIN_TAYLOR_R_1d
#undef DS_ASIN_TAYLOR_R_2d
#undef DS_ASIN_TAYLOR_R_3d
#undef DS_ASIN_TAYLOR_R_4d
#undef DS_ASIN_TAYLOR_R_5d

#undef DS_ASIN_TAYLOR_S_1d
#undef DS_ASIN_TAYLOR_S_2d
#undef DS_ASIN_TAYLOR_S_3d
#undef DS_ASIN_TAYLOR_S_4d

#undef DS_ATAN_TAYLOR_1f
#undef DS_ATAN_TAYLOR_2f
#undef DS_ATAN_TAYLOR_3f
#undef DS_ATAN_TAYLOR_4f

#undef DS_ATAN_TAYLOR_P_1d
#undef DS_ATAN_TAYLOR_P_2d
#undef DS_ATAN_TAYLOR_P_3d
#undef DS_ATAN_TAYLOR_P_4d
#undef DS_ATAN_TAYLOR_P_5d

#undef DS_ATAN_TAYLOR_Q_1d
#undef DS_ATAN_TAYLOR_Q_2d
#undef DS_ATAN_TAYLOR_Q_3d
#undef DS_ATAN_TAYLOR_Q_4d
#undef DS_ATAN_TAYLOR_Q_5d

#ifdef __cplusplus
}
#endif
