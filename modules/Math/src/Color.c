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

#include <DeepSea/Math/Color.h>

float dsSRGBFromLinear(float value);
float dsLinearFromSRGB(float value);
float dsGrayscaleValue(float r, float g, float b);

dsColor dsColor_fromColor3f(const dsColor3f* color);
dsColor dsColor_fromColor4f(const dsColor4f* color);
dsColor dsColor_fromHSVColor(const dsHSVColor* color);
dsColor dsColor_fromHSLColor(const dsHSLColor* color);
uint8_t dsColor_grayscale(dsColor color);
dsColor dsColor_lerp(dsColor x, dsColor y, float t);
dsColor dsColor_lerpSRGB(dsColor x, dsColor y, float t);
bool dsColor_equal(dsColor x, dsColor y);

void dsColor3f_fromColor(dsColor3f* outColor, dsColor color);
void dsColor3f_fromColor4f(dsColor3f* outColor, const dsColor4f* color);

void dsColor3f_fromHSVColor(dsColor3f* outColor, const dsHSVColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	// https://www.rapidtables.com/convert/color/hsv-to-rgb.html
	float hue = dsWrapf(color->h, 0.0f, 360.0f);
	float c = color->v*color->s;
	float x = c*(1.0f - fabsf(dsWrapf(hue/60.0f, 0.0f, 2.0f) - 1.0f));
	float m = color->v - c;
	c += m;
	x += m;

	if (hue >= 0.0f && hue < 60.0f)
	{
		outColor->r = c;
		outColor->g = x;
		outColor->b = m;
	}
	else if (hue >= 60.0f && hue < 120.0f)
	{
		outColor->r = x;
		outColor->g = c;
		outColor->b = m;
	}
	else if (hue >= 120.0f && hue < 180.0f)
	{
		outColor->r = m;
		outColor->g = c;
		outColor->b = x;
	}
	else if (hue >= 180.0f && hue < 240.0f)
	{
		outColor->r = m;
		outColor->g = x;
		outColor->b = c;
	}
	else if (hue >= 240.0f && hue < 300.0f)
	{
		outColor->r = x;
		outColor->g = m;
		outColor->b = c;
	}
	else
	{
		outColor->r = c;
		outColor->g = m;
		outColor->b = x;
	}
}

void dsColor3f_fromHSLColor(dsColor3f* outColor, const dsHSLColor* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	// https://www.rapidtables.com/convert/color/hsl-to-rgb.html
	float hue = dsWrapf(color->h, 0.0f, 360.0f);
	float c = (1.0f - fabsf(2.0f*color->l - 1.0f))*color->s;
	float x = c*(1.0f - fabsf(dsWrapf(hue/60.0f, 0.0f, 2.0f) - 1.0f));
	float m = color->l - c*0.5f;
	c += m;
	x += m;

	if (hue >= 0.0f && hue < 60.0f)
	{
		outColor->r = c;
		outColor->g = x;
		outColor->b = m;
	}
	else if (hue >= 60.0f && hue < 120.0f)
	{
		outColor->r = x;
		outColor->g = c;
		outColor->b = m;
	}
	else if (hue >= 120.0f && hue < 180.0f)
	{
		outColor->r = m;
		outColor->g = c;
		outColor->b = x;
	}
	else if (hue >= 180.0f && hue < 240.0f)
	{
		outColor->r = m;
		outColor->g = x;
		outColor->b = c;
	}
	else if (hue >= 240.0f && hue < 300.0f)
	{
		outColor->r = x;
		outColor->g = m;
		outColor->b = c;
	}
	else
	{
		outColor->r = c;
		outColor->g = m;
		outColor->b = x;
	}
}

void dsColor3f_sRGBFromLinear(dsColor3f* outColor, const dsColor3f* color);
void dsColor3f_linearFromSRGB(dsColor3f* outColor, const dsColor3f* color);
float dsColor3f_grayscale(const dsColor3f* color);
void dsColor3f_lerp(dsColor3f* outColor, const dsColor3f* x, const dsColor3f* y, float t);
void dsColor3f_lerpSRGB(dsColor3f* outColor, const dsColor3f* x, const dsColor3f* y, float t);
bool dsColor3f_equal(const dsColor3f* x, const dsColor3f* y);
bool dsColor3f_epsilonEqual(const dsColor3f* x, const dsColor3f* y, float epsilon);

void dsColor4f_fromColor(dsColor4f* outColor, dsColor color);
void dsColor4f_fromColor3f(dsColor4f* outColor, const dsColor3f* color);
void dsColor4f_fromHSVColor(dsColor4f* outColor, const dsHSVColor* color);
void dsColor4f_fromHSLColor(dsColor4f* outColor, const dsHSLColor* color);
void dsColor4f_sRGBFromLinear(dsColor4f* outColor, const dsColor4f* color);
void dsColor4f_linearFromSRGB(dsColor4f* outColor, const dsColor4f* color);
float dsColor4f_grayscale(const dsColor4f* color);
void dsColor4f_lerp(dsColor4f* outColor, const dsColor4f* x, const dsColor4f* y, float t);
void dsColor4f_lerpSRGB(dsColor4f* outColor, const dsColor4f* x, const dsColor4f* y, float t);
bool dsColor4f_equal(const dsColor4f* x, const dsColor4f* y);
bool dsColor4f_epsilonEqual(const dsColor4f* x, const dsColor4f* y, float epsilon);

void dsHSVColor_fromColor(dsHSVColor* outColor, dsColor color);

void dsHSVColor_fromColor3f(dsHSVColor* outColor, const dsColor3f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	// https://www.rapidtables.com/convert/color/rgb-to-hsv.html
	float cMax = dsMax(color->r, color->g);
	cMax = dsMax(cMax, color->b);
	float cMin = dsMin(color->r, color->g);
	cMin = dsMin(cMin, color->b);
	float delta = cMax - cMin;

	const float epsilon = 1e-6f;
	if (delta < epsilon)
	{
		outColor->h = 0.0f;
		outColor->s = 0.0f;
	}
	else
	{
		if (cMax == color->r)
			outColor->h = 60.0f*(color->g - color->b)/delta;
		else if (cMax == color->g)
			outColor->h = 120.0f + 60.0f*(color->b - color->r)/delta;
		else
			outColor->h = 240.0f + 60.0f*(color->r - color->g)/delta;
		if (outColor->h < 0.0f)
			outColor->h += 360.0f;
		outColor->s = delta/cMax;
	}

	outColor->v = cMax;
	outColor->a = 1.0f;
}

void dsHSVColor_fromColor4f(dsHSVColor* outColor, const dsColor4f* color);
void dsHSVColor_fromHSLColor(dsHSVColor* outColor, const dsHSLColor* color);
void dsHSVColor_sRGBFromLinear(dsHSVColor* outColor, const dsHSVColor* color);
void dsHSVColor_linearFromSRGB(dsHSVColor* outColor, const dsHSVColor* color);
float dsHSLColor_grayscale(const dsHSLColor* color);
void dsHSVColor_lerp(dsHSVColor* outColor, const dsHSVColor* x, const dsHSVColor* y, float t);
void dsHSVColor_lerpSRGB(dsHSVColor* outColor, const dsHSVColor* x, const dsHSVColor* y, float t);
bool dsHSVColor_equal(const dsHSVColor* x, const dsHSVColor* y);
bool dsHSVColor_epsilonEqual(const dsHSVColor* x, const dsHSVColor* y, float epsilon);

void dsHSLColor_fromColor3f(dsHSLColor* outColor, const dsColor3f* color)
{
	DS_ASSERT(outColor);
	DS_ASSERT(color);

	// https://www.rapidtables.com/convert/color/rgb-to-hsl.html
	float cMax = dsMax(color->r, color->g);
	cMax = dsMax(cMax, color->b);
	float cMin = dsMin(color->r, color->g);
	cMin = dsMin(cMin, color->b);
	float delta = cMax - cMin;
	outColor->l = (cMax + cMin)*0.5f;

	const float epsilon = 1e-6f;
	if (delta < epsilon)
	{
		outColor->h = 0.0f;
		outColor->s = 0.0f;
	}
	else
	{
		if (cMax == color->r)
			outColor->h = 60.0f*(color->g - color->b)/delta;
		else if (cMax == color->g)
			outColor->h = 120.0f + 60.0f*(color->b - color->r)/delta;
		else
			outColor->h = 240.0f + 60.0f*(color->r - color->g)/delta;
		if (outColor->h < 0.0f)
			outColor->h += 360.0f;
		outColor->s = delta/(1.0f - fabsf(2.0f*outColor->l - 1.0f));
	}

	outColor->a = 1.0f;
}

void dsHSLColor_fromColor4f(dsHSLColor* outColor, const dsColor4f* color);
void dsHSLColor_fromHSVColor(dsHSLColor* outColor, const dsHSVColor* color);
void dsHSLColor_sRGBFromLinear(dsHSLColor* outColor, const dsHSLColor* color);
void dsHSLColor_linearFromSRGB(dsHSLColor* outColor, const dsHSLColor* color);
float dsHSLColor_grayscale(const dsHSLColor* color);
void dsHSLColor_lerp(dsHSLColor* outColor, const dsHSLColor* x, const dsHSLColor* y, float t);
void dsHSLColor_lerpSRGB(dsHSLColor* outColor, const dsHSLColor* x, const dsHSLColor* y, float t);
bool dsHSLColor_equal(const dsHSLColor* x, const dsHSLColor* y);
bool dsHSLColor_epsilonEqual(const dsHSLColor* x, const dsHSLColor* y, float epsilon);
