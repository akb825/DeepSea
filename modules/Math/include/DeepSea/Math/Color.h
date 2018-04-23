/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/Math/Types.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Math/Vector4.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Colors for converting between various color formats.
 *
 * You may note that there are no sRGB <-> linear conversion functions that use dsColor. This is
 * because you lose a lot of precision when storing a linear color in an 8-bit value.
 */

/**
 * @brief Converts to an sRGB value from a linear value.
 * @param value The value to convert.
 * @return The converted value.
 */
DS_MATH_EXPORT inline float dsSRGBFromLinear(float value);

/**
 * @brief Converts to a linear value from an sRGB value.
 * @param value The value to convert.
 * @return The converted value.
 */
DS_MATH_EXPORT inline float dsLinearFromSRGB(float value);

/**
 * @brief Gets the grayscale value.
 * @param r The red value.
 * @param g The green value.
 * @param b The blue value.
 * @return The grayscale value.
 */
DS_MATH_EXPORT inline float dsGrayscaleValue(float r, float g, float b);

/**
 * @brief Converts to a dsColor from a dsColor3f.
 * @param color The color to convert.
 * @return The converted color.
 */
DS_MATH_EXPORT inline dsColor dsColor_fromColor3f(const dsColor3f* color);

/**
 * @brief Converts to a dsColor from a dsColor4f.
 * @param color The color to convert.
 * @return The converted color.
 */
DS_MATH_EXPORT inline dsColor dsColor_fromColor4f(const dsColor4f* color);

/**
 * @brief Converts to a dsColor4f from a dsHSVColor.
 * @param color The color to convert.
 * @return The converted color.
 */
DS_MATH_EXPORT inline dsColor dsColor_fromHSVColor(const dsHSVColor* color);

/**
 * @brief Converts to a dsColor4f from a dsHSLColor.
 * @param color The color to convert.
 * @return The converted color.
 */
DS_MATH_EXPORT inline dsColor dsColor_fromHSLColor(const dsHSLColor* color);

/**
 * @brief Converts to a grayscale value from a dsColor.
 * @param color The color.
 * @return The grayscale value.
 */
DS_MATH_EXPORT inline uint8_t dsColor_grayscale(dsColor color);

/**
 * @brief Linearly interpolates between two colors.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 * @return The interpolated color.
 */
DS_MATH_EXPORT inline dsColor dsColor_lerp(dsColor x, dsColor y, float t);

/**
 * @brief Linearly interpolates between two sRGB colors in linear color space.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 * @return The interpolated color.
 */
DS_MATH_EXPORT inline dsColor dsColor_lerpSRGB(dsColor x, dsColor y, float t);

/**
 * @brief Get whether or not two colors are equal.
 * @param x The first color.
 * @param y The second color.
 * @return True if x == y.
 */
DS_MATH_EXPORT inline bool dsColor_equal(dsColor x, dsColor y);

/**
 * @brief Converts to a dsColor3f from a dsColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor3f_fromColor(dsColor3f* outColor, dsColor color);

/**
 * @brief Converts to a dsColor3f from a dsColor4f.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor3f_fromColor4f(dsColor3f* outColor, const dsColor4f* color);

/**
 * @brief Converts to a dsColor3f from a dsHSVColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT void dsColor3f_fromHSVColor(dsColor3f* outColor, const dsHSVColor* color);

/**
 * @brief Converts to a dsColor3f from a dsHSLColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT void dsColor3f_fromHSLColor(dsColor3f* outColor, const dsHSLColor* color);

/**
 * @brief Converts to an sRGB color from a linear color.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor3f_sRGBFromLinear(dsColor3f* outColor, const dsColor3f* color);

/**
 * @brief Converts to a linear color from an sRGB color.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor3f_linearFromSRGB(dsColor3f* outColor, const dsColor3f* color);

/**
 * @brief Converts to a grayscale value from a dsColor3f.
 * @param color The color.
 * @return The grayscale value.
 */
DS_MATH_EXPORT inline float dsColor3f_grayscale(const dsColor3f* color);

/**
 * @brief Linearly interpolates between two colors.
 * @remark This is the same as dsVector4f_lerp(), and is provided for completeness.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsColor3f_lerp(dsColor3f* outColor, const dsColor3f* x,
	const dsColor3f* y, float t);

/**
 * @brief Linearly interpolates between two sRGB colors in linear color space.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsColor3f_lerpSRGB(dsColor3f* outColor, const dsColor3f* x,
	const dsColor3f* y, float t);

/**
 * @brief Get whether or not two colors are equal.
 * @remark The is the same as dsVector3f_equal(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @return True if x == y.
 */
DS_MATH_EXPORT inline bool dsColor3f_equal(const dsColor3f* x, const dsColor3f* y);

/**
 * @brief Get whether or not two colors are equal within an epsilon.
 * @remark The is the same as dsVector3f_epsilonEqual(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @param epsilon The epsilon to check within.
 * @return True the values of x and y are within epsilon.
 */
DS_MATH_EXPORT inline bool dsColor3f_epsilonEqual(const dsColor3f* x, const dsColor3f* y,
	float epsilon);

/**
 * @brief Converts to a dsColor4f from a dsColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor4f_fromColor(dsColor4f* outColor, dsColor color);

/**
 * @brief Converts to a dsColor4f from a dsColor3f.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor4f_fromColor3f(dsColor4f* outColor, const dsColor3f* color);

/**
 * @brief Converts to a dsColor4f from a dsHSVColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor4f_fromHSVColor(dsColor4f* outColor, const dsHSVColor* color);

/**
 * @brief Converts to a dsColor4f from a dsHSLColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor4f_fromHSLColor(dsColor4f* outColor, const dsHSLColor* color);

/**
 * @brief Converts to an sRGB color from a linear color.
 * @param[out] outColor The color to convert to
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor4f_sRGBFromLinear(dsColor4f* outColor, const dsColor4f* color);

/**
 * @brief Converts to a linear color from an sRGB color.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsColor4f_linearFromSRGB(dsColor4f* outColor, const dsColor4f* color);

/**
 * @brief Converts to a grayscale value from a dsColor4f.
 * @param color The color.
 * @return The grayscale value.
 */
DS_MATH_EXPORT inline float dsColor4f_grayscale(const dsColor4f* color);

/**
 * @brief Linearly interpolates between two colors.
 * @remark This is the same as dsVector4f_lerp(), and is provided for completeness.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsColor4f_lerp(dsColor4f* outColor, const dsColor4f* x,
	const dsColor4f* y, float t);

/**
 * @brief Linearly interpolates between two sRGB colors in linear color space.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsColor4f_lerpSRGB(dsColor4f* outColor, const dsColor4f* x,
	const dsColor4f* y, float t);

/**
 * @brief Get whether or not two colors are equal.
 * @remark The is the same as dsVector4f_equal(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @return True if x == y.
 */
DS_MATH_EXPORT inline bool dsColor4f_equal(const dsColor4f* x, const dsColor4f* y);

/**
 * @brief Get whether or not two colors are equal within an epsilon.
 * @remark The is the same as dsVector4f_epsilonEqual(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @param epsilon The epsilon to check within.
 * @return True the values of x and y are within epsilon.
 */
DS_MATH_EXPORT inline bool dsColor4f_epsilonEqual(const dsColor4f* x, const dsColor4f* y,
	float epsilon);

/**
 * @brief Converts to a dsHSVColor from a dsColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSVColor_fromColor(dsHSVColor* outColor, dsColor color);

/**
 * @brief Converts to a dsHSVColor from a dsColor3f.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT void dsHSVColor_fromColor3f(dsHSVColor* outColor, const dsColor3f* color);

/**
 * @brief Converts to a dsHSVColor from a dsColor4f.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSVColor_fromColor4f(dsHSVColor* outColor, const dsColor4f* color);

/**
 * @brief Converts to a dsHSVColor from a dsHSLColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSVColor_fromHSLColor(dsHSVColor* outColor, const dsHSLColor* color);

/**
 * @brief Converts to an sRGB color from a linear color.
 * @param[out] outColor The color to convert to
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSVColor_sRGBFromLinear(dsHSVColor* outColor, const dsHSVColor* color);

/**
 * @brief Converts to a linear color from an sRGB color.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSVColor_linearFromSRGB(dsHSVColor* outColor, const dsHSVColor* color);

/**
 * @brief Converts to a grayscale value from a dsHSVColor.
 * @remark This will be the luminance based on the RGB value, which differs from just taking the
 *     value field from the color.
 * @param color The color.
 * @return The grayscale value.
 */
DS_MATH_EXPORT inline float dsHSVColor_grayscale(const dsHSVColor* color);

/**
 * @brief Linearly interpolates between two colors.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsHSVColor_lerp(dsHSVColor* outColor, const dsHSVColor* x,
	const dsHSVColor* y, float t);

/**
 * @brief Linearly interpolates between two sRGB colors in linear color space.
 * @remark This will be a rather slow operation. If called often, it would be better to convert to
 *     linear colors and interpolate those, then convert the result to sRGB.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsHSVColor_lerpSRGB(dsHSVColor* outColor, const dsHSVColor* x,
	const dsHSVColor* y, float t);

/**
 * @brief Get whether or not two colors are equal.
 * @remark The is the same as dsVector3f_equal(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @return True if x == y.
 */
DS_MATH_EXPORT inline bool dsHSVColor_equal(const dsHSVColor* x, const dsHSVColor* y);

/**
 * @brief Get whether or not two colors are equal within an epsilon.
 * @remark The is the same as dsVector3f_epsilonEqual(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @param epsilon The epsilon to check within.
 * @return True the values of x and y are within epsilon.
 */
DS_MATH_EXPORT inline bool dsHSVColor_epsilonEqual(const dsHSVColor* x, const dsHSVColor* y,
	float epsilon);

/**
 * @brief Converts to a dsHSLColor from a dsColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSLColor_fromColor(dsHSLColor* outColor, dsColor color);

/**
 * @brief Converts to a dsHSLColor from a dsColor3f.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT void dsHSLColor_fromColor3f(dsHSLColor* outColor, const dsColor3f* color);

/**
 * @brief Converts to a dsHSLColor from a dsColor4f.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSLColor_fromColor4f(dsHSLColor* outColor, const dsColor4f* color);

/**
 * @brief Converts to a dsHSLColor from a dsHSVColor.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSLColor_fromHSVColor(dsHSLColor* outColor, const dsHSVColor* color);

/**
 * @brief Converts to an sRGB color from a linear color.
 * @param[out] outColor The color to convert to
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSLColor_sRGBFromLinear(dsHSLColor* outColor, const dsHSLColor* color);

/**
 * @brief Converts to a linear color from an sRGB color.
 * @param[out] outColor The color to convert to.
 * @param color The color to convert.
 */
DS_MATH_EXPORT inline void dsHSLColor_linearFromSRGB(dsHSLColor* outColor, const dsHSLColor* color);

/**
 * @brief Converts to a grayscale value from a dsHSLColor.
 * @remark This will be the luminance based on the RGB value, which differs from just taking the
 *     lightness field from the color. This is because the HSL color standard doesn't take the fact
 *     that different color channels are weighted differently into account.
 * @param color The color.
 * @return The grayscale value.
 */
DS_MATH_EXPORT inline float dsHSLColor_grayscale(const dsHSLColor* color);

/**
 * @brief Linearly interpolates between two colors.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsHSLColor_lerp(dsHSLColor* outColor, const dsHSLColor* x,
	const dsHSLColor* y, float t);

/**
 * @brief Linearly interpolates between two sRGB colors in linear color space.
 * @remark This will be a rather slow operation. If called often, it would be better to convert to
 *     linear colors and interpolate those, then convert the result to sRGB.
 * @param[out] outColor The interpolated color.
 * @param x The first color.
 * @param y The second color.
 * @param t The interpolation value between x and y.
 */
DS_MATH_EXPORT inline void dsHSLColor_lerpSRGB(dsHSLColor* outColor, const dsHSLColor* x,
	const dsHSLColor* y, float t);

/**
 * @brief Get whether or not two colors are equal.
 * @remark The is the same as dsVector3f_equal(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @return True if x == y.
 */
DS_MATH_EXPORT inline bool dsHSLColor_equal(const dsHSLColor* x, const dsHSLColor* y);

/**
 * @brief Get whether or not two colors are equal within an epsilon.
 * @remark The is the same as dsVector3f_epsilonEqual(), nd is provided for completeness.
 * @param x The first color.
 * @param y The second color.
 * @param epsilon The epsilon to check within.
 * @return True the values of x and y are within epsilon.
 */
DS_MATH_EXPORT inline bool dsHSLColor_epsilonEqual(const dsHSLColor* x, const dsHSLColor* y,
	float epsilon);

inline float dsSRGBFromLinear(float value)
{
	if (value <= 0.0031308f)
		return value*12.92f;
	return 1.055f*powf(value, 1.0f/2.4f) - 0.055f;
}

inline float dsLinearFromSRGB(float c)
{
	if (c <= 0.04045f)
		return c/12.92f;
	return powf((c + 0.055f)/1.055f, 2.4f);
}

inline float dsGrayscaleValue(float r, float g, float b)
{
	// Rec. 709
	return r*0.2126f + g*0.7152f + b*0.0722f;
}

inline dsColor dsColor_fromColor3f(const dsColor3f* color)
{
	DS_ASSERT(color);

	dsColor outColor;
	outColor.r = (uint8_t)roundf(dsClamp(color->r, 0.0f, 1.0f)*255.0f);
	outColor.g = (uint8_t)roundf(dsClamp(color->g, 0.0f, 1.0f)*255.0f);
	outColor.b = (uint8_t)roundf(dsClamp(color->b, 0.0f, 1.0f)*255.0f);
	outColor.a = 255;
	return outColor;
}

inline dsColor dsColor_fromColor4f(const dsColor4f* color)
{
	DS_ASSERT(color);

	dsColor outColor;
	outColor.r = (uint8_t)roundf(dsClamp(color->r, 0.0f, 1.0f)*255.0f);
	outColor.g = (uint8_t)roundf(dsClamp(color->g, 0.0f, 1.0f)*255.0f);
	outColor.b = (uint8_t)roundf(dsClamp(color->b, 0.0f, 1.0f)*255.0f);
	outColor.a = (uint8_t)roundf(dsClamp(color->a, 0.0f, 1.0f)*255.0f);
	return outColor;
}

inline dsColor dsColor_fromHSVColor(const dsHSVColor* color)
{
	DS_ASSERT(color);

	dsColor4f color4f;
	dsColor4f_fromHSVColor(&color4f, color);
	return dsColor_fromColor4f(&color4f);
}

inline dsColor dsColor_fromHSLColor(const dsHSLColor* color)
{
	DS_ASSERT(color);

	dsColor4f color4f;
	dsColor4f_fromHSLColor(&color4f, color);
	return dsColor_fromColor4f(&color4f);
}

inline uint8_t dsColor_grayscale(dsColor color)
{
	return (uint8_t)roundf(dsGrayscaleValue(color.r, color.g, color.b));
}

inline dsColor dsColor_lerp(dsColor x, dsColor y, float t)
{
	dsColor outColor;
	outColor.r = (uint8_t)roundf(dsLerp((float)x.r, (float)y.r, t));
	outColor.g = (uint8_t)roundf(dsLerp((float)x.g, (float)y.g, t));
	outColor.b = (uint8_t)roundf(dsLerp((float)x.b, (float)y.b, t));
	outColor.a = (uint8_t)roundf(dsLerp((float)x.a, (float)y.a, t));
	return outColor;
}

inline dsColor dsColor_lerpSRGB(dsColor x, dsColor y, float t)
{
	dsColor4f x4f, y4f, outColor4f;
	dsColor4f_fromColor(&x4f, x);
	dsColor4f_fromColor(&y4f, y);
	dsColor4f_lerpSRGB(&outColor4f, &x4f, &y4f, t);

	// Do conversion directly to avoid unneeded clamping.
	dsColor outColor;
	outColor.r = (uint8_t)roundf(outColor4f.r*255.0f);
	outColor.g = (uint8_t)roundf(outColor4f.g*255.0f);
	outColor.b = (uint8_t)roundf(outColor4f.b*255.0f);
	outColor.a = (uint8_t)roundf(outColor4f.a*255.0f);
	return outColor;
}

inline bool dsColor_equal(dsColor x, dsColor y)
{
	return x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a;
}

inline void dsColor3f_fromColor(dsColor3f* outColor, dsColor color)
{
	DS_ASSERT(outColor);
	outColor->r = color.r/255.0f;
	outColor->g = color.g/255.0f;
	outColor->b = color.b/255.0f;
}

inline void dsColor3f_fromColor4f(dsColor3f* outColor, const dsColor4f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	outColor->r = color->r;
	outColor->g = color->g;
	outColor->b = color->b;
}

inline void dsColor3f_sRGBFromLinear(dsColor3f* outColor, const dsColor3f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	outColor->r = dsSRGBFromLinear(color->r);
	outColor->g = dsSRGBFromLinear(color->g);
	outColor->b = dsSRGBFromLinear(color->b);
}

inline float dsColor3f_grayscale(const dsColor3f* color)
{
	DS_ASSERT(color);
	return dsGrayscaleValue(color->r, color->g, color->b);
}

inline void dsColor3f_linearFromSRGB(dsColor3f* outColor, const dsColor3f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	outColor->r = dsLinearFromSRGB(color->r);
	outColor->g = dsLinearFromSRGB(color->g);
	outColor->b = dsLinearFromSRGB(color->b);
}

inline void dsColor3f_lerp(dsColor3f* outColor, const dsColor3f* x, const dsColor3f* y, float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsVector3_lerp(*outColor, *x, *y, t);
}

inline void dsColor3f_lerpSRGB(dsColor3f* outColor, const dsColor3f* x, const dsColor3f* y, float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsColor3f xLinear, yLinear;
	dsColor3f_linearFromSRGB(&xLinear, x);
	dsColor3f_linearFromSRGB(&yLinear, y);
	dsVector3_lerp(*outColor, xLinear, yLinear, t);
	dsColor3f_sRGBFromLinear(outColor, outColor);
}

inline bool dsColor3f_equal(const dsColor3f* x, const dsColor3f* y)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector3_equal(*x, *y);
}

inline bool dsColor3f_epsilonEqual(const dsColor3f* x, const dsColor3f* y, float epsilon)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector3f_epsilonEqual(x, y, epsilon);
}

inline void dsColor4f_fromHSVColor(dsColor4f* outColor, const dsHSVColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsColor3f_fromHSVColor((dsColor3f*)outColor, color);
	outColor->a = color->a;
}

inline void dsColor4f_fromHSLColor(dsColor4f* outColor, const dsHSLColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsColor3f_fromHSLColor((dsColor3f*)outColor, color);
	outColor->a = color->a;
}

inline void dsColor4f_fromColor(dsColor4f* outColor, dsColor color)
{
	DS_ASSERT(outColor);
	outColor->r = color.r/255.0f;
	outColor->g = color.g/255.0f;
	outColor->b = color.b/255.0f;
	outColor->a = color.a/255.0f;
}

inline void dsColor4f_fromColor3f(dsColor4f* outColor, const dsColor3f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	outColor->r = color->r;
	outColor->g = color->g;
	outColor->b = color->b;
	outColor->a = 1.0f;
}

inline void dsColor4f_sRGBFromLinear(dsColor4f* outColor, const dsColor4f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	outColor->r = dsSRGBFromLinear(color->r);
	outColor->g = dsSRGBFromLinear(color->g);
	outColor->b = dsSRGBFromLinear(color->b);
	outColor->a = color->a;
}

inline void dsColor4f_linearFromSRGB(dsColor4f* outColor, const dsColor4f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	outColor->r = dsLinearFromSRGB(color->r);
	outColor->g = dsLinearFromSRGB(color->g);
	outColor->b = dsLinearFromSRGB(color->b);
	outColor->a = color->a;
}

inline float dsColor4f_grayscale(const dsColor4f* color)
{
	DS_ASSERT(color);
	return dsGrayscaleValue(color->r, color->g, color->b);
}

inline void dsColor4f_lerp(dsColor4f* outColor, const dsColor4f* x, const dsColor4f* y, float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsVector4_lerp(*outColor, *x, *y, t);
}

inline void dsColor4f_lerpSRGB(dsColor4f* outColor, const dsColor4f* x, const dsColor4f* y,
	float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsColor4f xLinear, yLinear;
	dsColor4f_linearFromSRGB(&xLinear, x);
	dsColor4f_linearFromSRGB(&yLinear, y);
	dsVector4_lerp(*outColor, xLinear, yLinear, t);
	dsColor4f_sRGBFromLinear(outColor, outColor);
}

inline bool dsColor4f_equal(const dsColor4f* x, const dsColor4f* y)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector4_equal(*x, *y);
}

inline bool dsColor4f_epsilonEqual(const dsColor4f* x, const dsColor4f* y, float epsilon)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector4f_epsilonEqual(x, y, epsilon);
}

inline void dsHSVColor_fromColor(dsHSVColor* outColor, dsColor color)
{
	DS_ASSERT(outColor);

	dsColor4f color4f;
	dsColor4f_fromColor(&color4f, color);
	dsHSVColor_fromColor4f(outColor, &color4f);
}

inline void dsHSVColor_fromColor4f(dsHSVColor* outColor, const dsColor4f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsHSVColor_fromColor3f(outColor, (const dsColor3f*)color);
	outColor->a = color->a;
}

inline void dsHSVColor_fromHSLColor(dsHSVColor* outColor, const dsHSLColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsColor4f color4f;
	dsColor4f_fromHSLColor(&color4f, color);
	dsHSVColor_fromColor4f(outColor, &color4f);
}

inline void dsHSVColor_sRGBFromLinear(dsHSVColor* outColor, const dsHSVColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsVector4f color4f;
	dsColor4f_fromHSVColor(&color4f, color);
	dsColor4f_sRGBFromLinear(&color4f, &color4f);
	dsHSVColor_fromColor4f(outColor, &color4f);
}

inline void dsHSVColor_linearFromSRGB(dsHSVColor* outColor, const dsHSVColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsVector4f color4f;
	dsColor4f_fromHSVColor(&color4f, color);
	dsColor4f_linearFromSRGB(&color4f, &color4f);
	dsHSVColor_fromColor4f(outColor, &color4f);
}

inline float dsHSVColor_grayscale(const dsHSVColor* color)
{
	DS_ASSERT(color);

	dsColor3f color3f;
	dsColor3f_fromHSVColor(&color3f, color);
	return dsColor3f_grayscale(&color3f);
}

inline void dsHSVColor_lerp(dsHSVColor* outColor, const dsHSVColor* x, const dsHSVColor* y, float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsVector4_lerp(*outColor, *x, *y, t);
}

inline void dsHSVColor_lerpSRGB(dsHSVColor* outColor, const dsHSVColor* x, const dsHSVColor* y,
	float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsHSVColor xLinear, yLinear;
	dsHSVColor_linearFromSRGB(&xLinear, x);
	dsHSVColor_linearFromSRGB(&yLinear, y);
	dsVector4_lerp(*outColor, xLinear, yLinear, t);
	dsHSVColor_sRGBFromLinear(outColor, outColor);
}

inline bool dsHSVColor_equal(const dsHSVColor* x, const dsHSVColor* y)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector4_equal(*x, *y);
}

inline bool dsHSVColor_epsilonEqual(const dsHSVColor* x, const dsHSVColor* y, float epsilon)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector4f_epsilonEqual((const dsVector4f*)x, (const dsVector4f*)y, epsilon);
}

inline void dsHSLColor_fromColor(dsHSLColor* outColor, dsColor color)
{
	DS_ASSERT(outColor);

	dsColor4f color4f;
	dsColor4f_fromColor(&color4f, color);
	dsHSLColor_fromColor4f(outColor, &color4f);
}

inline void dsHSLColor_fromColor4f(dsHSLColor* outColor, const dsColor4f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsHSLColor_fromColor3f(outColor, (const dsColor3f*)color);
	outColor->a = color->a;
}

inline void dsHSLColor_fromHSVColor(dsHSLColor* outColor, const dsHSVColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsColor4f color4f;
	dsColor4f_fromHSVColor(&color4f, color);
	dsHSLColor_fromColor4f(outColor, &color4f);
}

inline void dsHSLColor_sRGBFromLinear(dsHSLColor* outColor, const dsHSLColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsVector4f color4f;
	dsColor4f_fromHSLColor(&color4f, color);
	dsColor4f_sRGBFromLinear(&color4f, &color4f);
	dsHSLColor_fromColor4f(outColor, &color4f);
}

inline void dsHSLColor_linearFromSRGB(dsHSLColor* outColor, const dsHSLColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	dsVector4f color4f;
	dsColor4f_fromHSLColor(&color4f, color);
	dsColor4f_linearFromSRGB(&color4f, &color4f);
	dsHSLColor_fromColor4f(outColor, &color4f);
}

inline float dsHSLColor_grayscale(const dsHSLColor* color)
{
	DS_ASSERT(color);

	dsColor3f color3f;
	dsColor3f_fromHSLColor(&color3f, color);
	return dsColor3f_grayscale(&color3f);
}

inline void dsHSLColor_lerp(dsHSLColor* outColor, const dsHSLColor* x, const dsHSLColor* y, float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsVector4_lerp(*outColor, *x, *y, t);
}

inline void dsHSLColor_lerpSRGB(dsHSLColor* outColor, const dsHSLColor* x, const dsHSLColor* y,
	float t)
{
	DS_ASSERT(outColor);
	DS_ASSERT(x);
	DS_ASSERT(y);

	dsHSLColor xLinear, yLinear;
	dsHSLColor_linearFromSRGB(&xLinear, x);
	dsHSLColor_linearFromSRGB(&yLinear, y);
	dsVector4_lerp(*outColor, xLinear, yLinear, t);
	dsHSLColor_sRGBFromLinear(outColor, outColor);
}

inline bool dsHSLColor_equal(const dsHSLColor* x, const dsHSLColor* y)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector4_equal(*x, *y);
}

inline bool dsHSLColor_epsilonEqual(const dsHSLColor* x, const dsHSLColor* y, float epsilon)
{
	DS_ASSERT(x);
	DS_ASSERT(y);

	return dsVector4f_epsilonEqual((const dsVector4f*)x, (const dsVector4f*)y, epsilon);
}

#ifdef __cplusplus
}
#endif
