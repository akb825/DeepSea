/*
 * Copyright 2018-2022 Aaron Barany
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
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Vector3.h>
#include <gtest/gtest.h>

static bool testHSVColor(uint8_t red, uint8_t green, uint8_t blue, float hue, float saturation,
	float value, float epsilon)
{
	dsColor expectedColor = {{red, green, blue, 128}};
	dsHSVColor expectedHSVColor = {{hue, saturation, value, 0.5f}};

	dsHSVColor hsvColor;
	dsHSVColor_fromColor(&hsvColor, expectedColor);
	bool success = dsHSVColor_epsilonEqual(&expectedHSVColor, &hsvColor, epsilon);
	EXPECT_NEAR(hue, hsvColor.h, epsilon);
	EXPECT_NEAR(saturation, hsvColor.s, epsilon);
	EXPECT_NEAR(value, hsvColor.v, epsilon);
	EXPECT_NEAR(0.5, hsvColor.a, epsilon);

	dsColor color = dsColor_fromHSVColor(&hsvColor);
	success = success && dsColor_equal(expectedColor, color);
	EXPECT_EQ(red, color.r);
	EXPECT_EQ(green, color.g);
	EXPECT_EQ(blue, color.b);
	EXPECT_EQ(128, color.a);

	return success;
}

static bool testHSVColor3f(float red, float green, float blue, float hue, float saturation,
	float value, float epsilon)
{
	dsColor3f expectedColor3f = {{red, green, blue}};
	dsHSVColor expectedHSVColor = {{hue, saturation, value, 1.0f}};

	dsHSVColor hsvColor;
	dsHSVColor_fromColor3f(&hsvColor, &expectedColor3f);
	bool success = dsHSVColor_epsilonEqual(&expectedHSVColor, &hsvColor, epsilon);
	EXPECT_NEAR(hue, hsvColor.h, epsilon);
	EXPECT_NEAR(saturation, hsvColor.s, epsilon);
	EXPECT_NEAR(value, hsvColor.v, epsilon);
	EXPECT_EQ(1.0f, hsvColor.a);

	dsColor3f color3f;
	dsColor3f_fromHSVColor(&color3f, &hsvColor);
	success = success && dsEpsilonEqualf(red, color3f.r, epsilon) &&
		dsEpsilonEqualf(green, color3f.g, epsilon) &&
		dsEpsilonEqualf(blue, color3f.b, epsilon);
	EXPECT_NEAR(red, color3f.r, epsilon);
	EXPECT_NEAR(green, color3f.g, epsilon);
	EXPECT_NEAR(blue, color3f.b, epsilon);

	return success;
}

static bool testHSVColor4f(float red, float green, float blue, float hue, float saturation,
	float value, float epsilon)
{
	dsColor4f expectedColor4f = {{red, green, blue, 0.5f}};
	dsHSVColor expectedHSVColor = {{hue, saturation, value, 0.5f}};

	dsHSVColor hsvColor;
	dsHSVColor_fromColor4f(&hsvColor, &expectedColor4f);
	bool success = dsHSVColor_epsilonEqual(&expectedHSVColor, &hsvColor, epsilon);
	EXPECT_NEAR(hue, hsvColor.h, epsilon);
	EXPECT_NEAR(saturation, hsvColor.s, epsilon);
	EXPECT_NEAR(value, hsvColor.v, epsilon);
	EXPECT_EQ(0.5f, hsvColor.a);

	dsColor4f color4f;
	dsColor4f_fromHSVColor(&color4f, &hsvColor);
	success = success && dsColor4f_epsilonEqual(&expectedColor4f, &color4f, epsilon);
	EXPECT_NEAR(red, color4f.r, epsilon);
	EXPECT_NEAR(green, color4f.g, epsilon);
	EXPECT_NEAR(blue, color4f.b, epsilon);
	EXPECT_EQ(0.5f, color4f.a);

	return success;
}

static bool testHSLColor(uint8_t red, uint8_t green, uint8_t blue, float hue, float saturation,
	float lightness, float epsilon)
{
	dsColor expectedColor = {{red, green, blue, 128}};
	dsHSLColor expectedHSLColor = {{hue, saturation, lightness, 0.5f}};

	dsHSLColor hslColor;
	dsHSLColor_fromColor(&hslColor, expectedColor);
	bool success = dsHSLColor_epsilonEqual(&expectedHSLColor, &hslColor, epsilon);
	EXPECT_NEAR(hue, hslColor.h, epsilon);
	EXPECT_NEAR(saturation, hslColor.s, epsilon);
	EXPECT_NEAR(lightness, hslColor.l, epsilon);
	EXPECT_NEAR(0.5, hslColor.a, epsilon);

	dsColor color = dsColor_fromHSLColor(&hslColor);
	success = success && dsColor_equal(expectedColor, color);
	EXPECT_EQ(red, color.r);
	EXPECT_EQ(green, color.g);
	EXPECT_EQ(blue, color.b);
	EXPECT_EQ(128, color.a);

	return success;
}

static bool testHSLColor3f(float red, float green, float blue, float hue, float saturation,
	float lightness, float epsilon)
{
	dsColor3f expectedColor3f = {{red, green, blue}};
	dsHSLColor expectedHSLColor = {{hue, saturation, lightness, 1.0f}};

	dsHSLColor hslColor;
	dsHSLColor_fromColor3f(&hslColor, &expectedColor3f);
	bool success = dsHSLColor_epsilonEqual(&expectedHSLColor, &hslColor, epsilon);
	EXPECT_NEAR(hue, hslColor.h, epsilon);
	EXPECT_NEAR(saturation, hslColor.s, epsilon);
	EXPECT_NEAR(lightness, hslColor.l, epsilon);
	EXPECT_EQ(1.0f, hslColor.a);

	dsColor3f color3f;
	dsColor3f_fromHSLColor(&color3f, &hslColor);
	success = success && dsEpsilonEqualf(red, color3f.r, epsilon) &&
		dsEpsilonEqualf(green, color3f.g, epsilon) &&
		dsEpsilonEqualf(blue, color3f.b, epsilon);
	EXPECT_NEAR(red, color3f.r, epsilon);
	EXPECT_NEAR(green, color3f.g, epsilon);
	EXPECT_NEAR(blue, color3f.b, epsilon);

	return success;
}

static bool testHSLColor4f(float red, float green, float blue, float hue, float saturation,
	float lightness, float epsilon)
{
	dsColor4f expectedColor4f = {{red, green, blue, 0.5f}};
	dsHSLColor expectedHSLColor = {{hue, saturation, lightness, 0.5f}};

	dsHSLColor hslColor;
	dsHSLColor_fromColor4f(&hslColor, &expectedColor4f);
	bool success = dsHSLColor_epsilonEqual(&expectedHSLColor, &hslColor, epsilon);
	EXPECT_NEAR(hue, hslColor.h, epsilon);
	EXPECT_NEAR(saturation, hslColor.s, epsilon);
	EXPECT_NEAR(lightness, hslColor.l, epsilon);
	EXPECT_EQ(0.5f, hslColor.a);

	dsColor4f color4f;
	dsColor4f_fromHSLColor(&color4f, &hslColor);
	success = success && dsColor4f_epsilonEqual(&expectedColor4f, &color4f, epsilon);
	EXPECT_NEAR(red, color4f.r, epsilon);
	EXPECT_NEAR(green, color4f.g, epsilon);
	EXPECT_NEAR(blue, color4f.b, epsilon);
	EXPECT_EQ(0.5f, color4f.a);

	return success;
}

TEST(ColorTest, YUVandRGBMatrices)
{
	dsMatrix33f identity;
	dsMatrix33_identity(identity);
	dsMatrix33f testMatrix;
	dsMatrix33_mul(testMatrix, dsYUVtoRGBTransform, dsRGBtoYUVTransform);
	for (unsigned int i = 0; i < 3; ++i)
		EXPECT_TRUE(dsVector3f_epsilonEqual(identity.columns + i, testMatrix.columns + i, 1e-4f));

	dsColor3f rgbColor;
	rgbColor.r = 1.0f;
	rgbColor.g = 0.0f;
	rgbColor.b = 0.0f;

	dsColor3f yuvColor;
	dsMatrix33_transform(yuvColor, dsRGBtoYUVTransform, rgbColor);
	EXPECT_EQ(0.2126f, yuvColor.x);
	EXPECT_EQ(-0.09991f, yuvColor.y);
	EXPECT_EQ(0.615f, yuvColor.z);

	rgbColor.r = 0.0f;
	rgbColor.g = 1.0f;
	rgbColor.b = 0.0f;

	dsMatrix33_transform(yuvColor, dsRGBtoYUVTransform, rgbColor);
	EXPECT_EQ(0.7152f, yuvColor.x);
	EXPECT_EQ(-0.33609f, yuvColor.y);
	EXPECT_EQ(-0.55861f, yuvColor.z);

	rgbColor.r = 0.0f;
	rgbColor.g = 0.0f;
	rgbColor.b = 1.0f;

	dsMatrix33_transform(yuvColor, dsRGBtoYUVTransform, rgbColor);
	EXPECT_EQ(0.0722f, yuvColor.x);
	EXPECT_EQ(0.436f, yuvColor.y);
	EXPECT_EQ(-0.05639f, yuvColor.z);

	rgbColor.r = 1.0f;
	rgbColor.g = 1.0f;
	rgbColor.b = 1.0f;

	dsMatrix33_transform(yuvColor, dsRGBtoYUVTransform, rgbColor);
	EXPECT_NEAR(1.0f, yuvColor.x, 1e-4f);
	EXPECT_NEAR(0.0f, yuvColor.y, 1e-4f);
	EXPECT_NEAR(0.0f, yuvColor.z, 1e-4f);
}

TEST(ColorTest, CreateHSVTransform)
{
	dsMatrix33f identity;
	dsMatrix33_identity(identity);
	dsMatrix33f transform;
	dsColor3f_createHSVTransform(&transform, 360, 1, 1);
	for (unsigned int i = 0; i < 3; ++i)
		EXPECT_TRUE(dsVector3f_epsilonEqual(identity.columns + i, transform.columns + i, 1e-4f));

	dsColor3f_createHSVTransform(&transform, 120, 1, 1);

	dsColor3f origColor;
	origColor.r = 1.0f;
	origColor.g = 0.0f;
	origColor.b = 0.0f;

	dsColor3f transformedColor;
	dsMatrix33_transform(transformedColor, transform, origColor);
	EXPECT_LT(transformedColor.r, transformedColor.g);
	EXPECT_LT(transformedColor.b, transformedColor.g);

	origColor.r = 0.0f;
	origColor.g = 1.0f;
	origColor.b = 0.0f;

	dsMatrix33_transform(transformedColor, transform, origColor);
	EXPECT_LT(transformedColor.r, transformedColor.b);
	EXPECT_LT(transformedColor.g, transformedColor.b);

	origColor.r = 0.0f;
	origColor.g = 0.0f;
	origColor.b = 1.0f;

	dsMatrix33_transform(transformedColor, transform, origColor);
	EXPECT_LT(transformedColor.g, transformedColor.r);
	EXPECT_LT(transformedColor.b, transformedColor.r);

	dsColor3f_createHSVTransform(&transform, 0, 0, 1);

	origColor.r = 0.1f;
	origColor.g = 0.2f;
	origColor.b = 0.3f;

	dsMatrix33_transform(transformedColor, transform, origColor);
	float grayscale = dsColor3f_grayscale(&origColor);
	EXPECT_NEAR(grayscale, transformedColor.r, 1e-4f);
	EXPECT_NEAR(grayscale, transformedColor.g, 1e-4f);
	EXPECT_NEAR(grayscale, transformedColor.b, 1e-4f);

	dsColor3f_createHSVTransform(&transform, 0, 1, 0.5f);

	dsMatrix33_transform(transformedColor, transform, origColor);
	EXPECT_NEAR(0.05f, transformedColor.r, 1e-4f);
	EXPECT_NEAR(0.1f, transformedColor.g, 1e-4f);
	EXPECT_NEAR(0.15f, transformedColor.b, 1e-4f);
}

TEST(ColorTest, ConvertColorAndColor3f)
{
	dsColor color = {{10, 20, 30, 40}};
	dsColor3f color3f;
	dsColor3f_fromColor(&color3f, color);
	EXPECT_EQ(10.0f/255.0f, color3f.r);
	EXPECT_EQ(20.0f/255.0f, color3f.g);
	EXPECT_EQ(30.0f/255.0f, color3f.b);

	color = dsColor_fromColor3f(&color3f);
	EXPECT_EQ(10, color.r);
	EXPECT_EQ(20, color.g);
	EXPECT_EQ(30, color.b);

	color3f.r = -1.0f;
	color3f.g = 0.499f;
	color3f.b = 2.0f;
	color = dsColor_fromColor3f(&color3f);
	EXPECT_EQ(0, color.r);
	EXPECT_EQ(127, color.g);
	EXPECT_EQ(255, color.b);
}

TEST(ColorTest, ConvertColorAndColor4f)
{
	dsColor color = {{10, 20, 30, 40}};
	dsColor4f color4f;
	dsColor4f_fromColor(&color4f, color);
	EXPECT_EQ(10.0f/255.0f, color4f.r);
	EXPECT_EQ(20.0f/255.0f, color4f.g);
	EXPECT_EQ(30.0f/255.0f, color4f.b);
	EXPECT_EQ(40.0f/255.0f, color4f.a);

	color = dsColor_fromColor4f(&color4f);
	EXPECT_EQ(10, color.r);
	EXPECT_EQ(20, color.g);
	EXPECT_EQ(30, color.b);
	EXPECT_EQ(40, color.a);

	color4f.r = -1.0f;
	color4f.g = 0.499f;
	color4f.b = 2.0f;
	color4f.a = 3.0f;
	color = dsColor_fromColor4f(&color4f);
	EXPECT_EQ(0, color.r);
	EXPECT_EQ(127, color.g);
	EXPECT_EQ(255, color.b);
	EXPECT_EQ(255, color.a);
}

TEST(ColorTest, ConvertColor3fAndColor4f)
{
	dsColor4f color4f = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsColor3f color3f;
	dsColor3f_fromColor4f(&color3f, &color4f);
	EXPECT_EQ(0.1f, color3f.r);
	EXPECT_EQ(0.2f, color3f.g);
	EXPECT_EQ(0.3f, color3f.b);

	dsColor4f_fromColor3f(&color4f, &color3f);
	EXPECT_EQ(0.1f, color4f.r);
	EXPECT_EQ(0.2f, color4f.g);
	EXPECT_EQ(0.3f, color4f.b);
	EXPECT_EQ(1.0f, color4f.a);
}

TEST(ColorTest, ConvertColorAndHSVColor)
{
	float epsilon = 1e-2f;
	EXPECT_TRUE(testHSVColor(0, 0, 0, 0.0f, 0.0f, 0.0f, epsilon));
	EXPECT_TRUE(testHSVColor(255, 255, 255, 0.0f, 0.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor(255, 0, 0, 0.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor(0, 255, 0, 120.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor(0, 0, 255, 240.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor(255, 255, 0, 60.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor(0, 255, 255, 180.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor(255, 0, 255, 300.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor(192, 192, 192, 0.0f, 0.0f, 0.75f, epsilon));
	EXPECT_TRUE(testHSVColor(128, 128, 128, 0.0f, 0.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(128, 0, 0, 0.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(128, 128, 0, 60.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(0, 128, 0, 120.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(128, 0, 128, 300.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(0, 128, 128, 180.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(0, 0, 128, 240.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(128, 64, 64, 0.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(64, 128, 128, 180.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(64, 64, 128, 240.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(128, 128, 64, 60.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(128, 64, 128, 300.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor(64, 128, 64, 120.0f, 0.5f, 0.5f, epsilon));
}

TEST(ColorTest, ConvertColor3fAndHSVColor)
{
	float epsilon = 1e-6f;
	EXPECT_TRUE(testHSVColor3f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.0f, 1.0f, 0.0f, 120.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.0f, 0.0f, 1.0f, 240.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(1.0f, 1.0f, 0.0f, 60.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.0f, 1.0f, 1.0f, 180.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(1.0f, 0.0f, 1.0f, 300.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.75f, 0.75f, 0.75f, 0.0f, 0.0f, 0.75f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.5f, 0.5f, 0.0f, 60.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.0f, 0.5f, 0.0f, 120.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.5f, 0.0f, 0.5f, 300.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.0f, 0.5f, 0.5f, 180.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.0f, 0.0f, 0.5f, 240.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.5f, 0.25f, 0.25f, 0.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.25f, 0.5f, 0.5f, 180.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.25f, 0.25f, 0.5f, 240.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.5f, 0.5f, 0.25f, 60.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.5f, 0.25f, 0.5f, 300.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor3f(0.25f, 0.5f, 0.25f, 120.0f, 0.5f, 0.5f, epsilon));
}

TEST(ColorTest, ConvertColor4fAndHSVColor)
{
	float epsilon = 1e-6f;
	EXPECT_TRUE(testHSVColor4f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.0f, 1.0f, 0.0f, 120.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.0f, 0.0f, 1.0f, 240.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(1.0f, 1.0f, 0.0f, 60.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.0f, 1.0f, 1.0f, 180.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(1.0f, 0.0f, 1.0f, 300.0f, 1.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.75f, 0.75f, 0.75f, 0.0f, 0.0f, 0.75f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.5f, 0.5f, 0.0f, 60.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.0f, 0.5f, 0.0f, 120.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.5f, 0.0f, 0.5f, 300.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.0f, 0.5f, 0.5f, 180.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.0f, 0.0f, 0.5f, 240.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.5f, 0.25f, 0.25f, 0.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.25f, 0.5f, 0.5f, 180.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.25f, 0.25f, 0.5f, 240.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.5f, 0.5f, 0.25f, 60.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.5f, 0.25f, 0.5f, 300.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSVColor4f(0.25f, 0.5f, 0.25f, 120.0f, 0.5f, 0.5f, epsilon));
}

TEST(ColorTest, ConvertColorAndHSLColor)
{
	float epsilon = 1e-2f;
	EXPECT_TRUE(testHSLColor(0, 0, 0, 0.0f, 0.0f, 0.0f, epsilon));
	EXPECT_TRUE(testHSLColor(255, 255, 255, 0.0f, 0.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSLColor(255, 0, 0, 0.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(0, 255, 0, 120.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(0, 0, 255, 240.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(255, 255, 0, 60.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(0, 255, 255, 180.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(255, 0, 255, 300.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(192, 192, 192, 0.0f, 0.0f, 0.75f, epsilon));
	EXPECT_TRUE(testHSLColor(128, 128, 128, 0.0f, 0.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(128, 0, 0, 0.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor(128, 128, 0, 60.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor(0, 128, 0, 120.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor(128, 0, 128, 300.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor(0, 128, 128, 180.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor(0, 0, 128, 240.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor(192, 64, 64, 0.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(64, 192, 192, 180.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(64, 64, 192, 240.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(192, 192, 64, 60.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(192, 64, 192, 300.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor(64, 192, 64, 120.0f, 0.5f, 0.5f, epsilon));
}

TEST(ColorTest, ConvertColor3fAndHSLColor)
{
	float epsilon = 1e-6f;
	EXPECT_TRUE(testHSLColor3f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, epsilon));
	EXPECT_TRUE(testHSLColor3f(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSLColor3f(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.0f, 1.0f, 0.0f, 120.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.0f, 0.0f, 1.0f, 240.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(1.0f, 1.0f, 0.0f, 60.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.0f, 1.0f, 1.0f, 180.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(1.0f, 0.0f, 1.0f, 300.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.75f, 0.75f, 0.75f, 0.0f, 0.0f, 0.75f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.5f, 0.5f, 0.0f, 60.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.0f, 0.5f, 0.0f, 120.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.5f, 0.0f, 0.5f, 300.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.0f, 0.5f, 0.5f, 180.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.0f, 0.0f, 0.5f, 240.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.75f, 0.25f, 0.25f, 0.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.25f, 0.75f, 0.75f, 180.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.25f, 0.25f, 0.75f, 240.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.75f, 0.75f, 0.25f, 60.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.75f, 0.25f, 0.75f, 300.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor3f(0.25f, 0.75f, 0.25f, 120.0f, 0.5f, 0.5f, epsilon));
}

TEST(ColorTest, ConvertColor4fAndHSLColor)
{
	float epsilon = 1e-6f;
	EXPECT_TRUE(testHSLColor4f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, epsilon));
	EXPECT_TRUE(testHSLColor4f(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, epsilon));
	EXPECT_TRUE(testHSLColor4f(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.0f, 1.0f, 0.0f, 120.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.0f, 0.0f, 1.0f, 240.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(1.0f, 1.0f, 0.0f, 60.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.0f, 1.0f, 1.0f, 180.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(1.0f, 0.0f, 1.0f, 300.0f, 1.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.75f, 0.75f, 0.75f, 0.0f, 0.0f, 0.75f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.5f, 0.5f, 0.0f, 60.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.0f, 0.5f, 0.0f, 120.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.5f, 0.0f, 0.5f, 300.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.0f, 0.5f, 0.5f, 180.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.0f, 0.0f, 0.5f, 240.0f, 1.0f, 0.25f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.75f, 0.25f, 0.25f, 0.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.25f, 0.75f, 0.75f, 180.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.25f, 0.25f, 0.75f, 240.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.75f, 0.75f, 0.25f, 60.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.75f, 0.25f, 0.75f, 300.0f, 0.5f, 0.5f, epsilon));
	EXPECT_TRUE(testHSLColor4f(0.25f, 0.75f, 0.25f, 120.0f, 0.5f, 0.5f, epsilon));
}

TEST(ColorTest, ConvertSRGBLinear)
{
	const float epsilon = 1e-6f;
	EXPECT_NEAR(0.0f, dsSRGBFromLinear(0.0f), epsilon);
	EXPECT_NEAR(0.0998528f, dsSRGBFromLinear(0.01f), epsilon);
	EXPECT_NEAR(0.5370987f, dsSRGBFromLinear(0.25f), epsilon);
	EXPECT_NEAR(0.8808250f, dsSRGBFromLinear(0.75f), epsilon);
	EXPECT_NEAR(1.0f, dsSRGBFromLinear(1.0f), epsilon);

	EXPECT_NEAR(0.0f, dsLinearFromSRGB(0.0f), epsilon);
	EXPECT_NEAR(0.0007740f, dsLinearFromSRGB(0.01f), epsilon);
	EXPECT_NEAR(0.0508761f, dsLinearFromSRGB(0.25f), epsilon);
	EXPECT_NEAR(0.5225216f, dsLinearFromSRGB(0.75f), epsilon);
	EXPECT_NEAR(1.0f, dsLinearFromSRGB(1.0f), epsilon);
}

TEST(ColorTest, ConvertSRGBLinearColor3f)
{
	const float epsilon = 1e-6f;
	dsColor3f color = {{0.01f, 0.25f, 0.75f}};
	dsColor3f convertedColor;

	dsColor3f_sRGBFromLinear(&convertedColor, &color);
	EXPECT_NEAR(0.0998528f, convertedColor.r, epsilon);
	EXPECT_NEAR(0.5370987f, convertedColor.g, epsilon);
	EXPECT_NEAR(0.8808250f, convertedColor.b, epsilon);

	dsColor3f_linearFromSRGB(&convertedColor, &color);
	EXPECT_NEAR(0.0007740f, convertedColor.r, epsilon);
	EXPECT_NEAR(0.0508761f, convertedColor.g, epsilon);
	EXPECT_NEAR(0.5225216f, convertedColor.b, epsilon);
}

TEST(ColorTest, ConvertSRGBLinearColor4f)
{
	const float epsilon = 1e-6f;
	dsColor4f color = {{0.01f, 0.25f, 0.75f, 0.5f}};
	dsColor4f convertedColor;

	dsColor4f_sRGBFromLinear(&convertedColor, &color);
	EXPECT_NEAR(0.0998528f, convertedColor.r, epsilon);
	EXPECT_NEAR(0.5370987f, convertedColor.g, epsilon);
	EXPECT_NEAR(0.8808250f, convertedColor.b, epsilon);
	EXPECT_EQ(0.5f, convertedColor.a);

	dsColor4f_linearFromSRGB(&convertedColor, &color);
	EXPECT_NEAR(0.0007740f, convertedColor.r, epsilon);
	EXPECT_NEAR(0.0508761f, convertedColor.g, epsilon);
	EXPECT_NEAR(0.5225216f, convertedColor.b, epsilon);
	EXPECT_EQ(0.5f, convertedColor.a);
}

TEST(ColorTest, ConvertSRGBLinearHSVColor)
{
	const float epsilon = 1e-6f;
	dsColor4f color4f = {{0.01f, 0.25f, 0.75f, 0.5f}};
	dsHSVColor color;
	dsHSVColor_fromColor4f(&color, &color4f);
	dsHSVColor convertedColor;

	dsHSVColor_sRGBFromLinear(&convertedColor, &color);
	dsColor4f_fromHSVColor(&color4f, &convertedColor);
	EXPECT_NEAR(0.0998528f, color4f.r, epsilon);
	EXPECT_NEAR(0.5370987f, color4f.g, epsilon);
	EXPECT_NEAR(0.8808250f, color4f.b, epsilon);
	EXPECT_EQ(0.5f, convertedColor.a);

	dsHSVColor_linearFromSRGB(&convertedColor, &color);
	dsColor4f_fromHSVColor(&color4f, &convertedColor);
	EXPECT_NEAR(0.0007740f, color4f.r, epsilon);
	EXPECT_NEAR(0.0508761f, color4f.g, epsilon);
	EXPECT_NEAR(0.5225216f, color4f.b, epsilon);
	EXPECT_EQ(0.5f, convertedColor.a);
}

TEST(ColorTest, ConvertSRGBLinearHSLColor)
{
	const float epsilon = 1e-6f;
	dsColor4f color4f = {{0.01f, 0.25f, 0.75f, 0.5f}};
	dsHSLColor color;
	dsHSLColor_fromColor4f(&color, &color4f);
	dsHSLColor convertedColor;

	dsHSLColor_sRGBFromLinear(&convertedColor, &color);
	dsColor4f_fromHSLColor(&color4f, &convertedColor);
	EXPECT_NEAR(0.0998528f, color4f.r, epsilon);
	EXPECT_NEAR(0.5370987f, color4f.g, epsilon);
	EXPECT_NEAR(0.8808250f, color4f.b, epsilon);
	EXPECT_EQ(0.5f, convertedColor.a);

	dsHSLColor_linearFromSRGB(&convertedColor, &color);
	dsColor4f_fromHSLColor(&color4f, &convertedColor);
	EXPECT_NEAR(0.0007740f, color4f.r, epsilon);
	EXPECT_NEAR(0.0508761f, color4f.g, epsilon);
	EXPECT_NEAR(0.5225216f, color4f.b, epsilon);
	EXPECT_EQ(0.5f, convertedColor.a);
}

TEST(ColorTest, ConvertGrayscale)
{
	dsColor color = {{10, 20, 30}};
	EXPECT_EQ(19U, dsColor_grayscale(color));

	dsColor3f color3f = {{0.1f, 0.2f, 0.3f}};
	EXPECT_FLOAT_EQ(0.18596f, dsColor3f_grayscale(&color3f));

	dsColor4f color4f = {{0.1f, 0.2f, 0.3f, 0.4f}};
	EXPECT_FLOAT_EQ(0.18596f, dsColor4f_grayscale(&color4f));

	dsHSVColor hsvColor;
	dsHSVColor_fromColor4f(&hsvColor, &color4f);
	EXPECT_FLOAT_EQ(0.18596f, dsHSVColor_grayscale(&hsvColor));

	dsHSLColor hslColor;
	dsHSLColor_fromColor4f(&hslColor, &color4f);
	EXPECT_FLOAT_EQ(0.18596f, dsHSLColor_grayscale(&hslColor));
}

TEST(ColorTest, LerpColor)
{
	dsColor color1 = {{10, 20, 30, 40}};
	dsColor color2 = {{110, 120, 130, 140}};

	dsColor color = dsColor_lerp(color1, color2, 0.3f);
	EXPECT_EQ(40, color.r);
	EXPECT_EQ(50, color.g);
	EXPECT_EQ(60, color.b);
	EXPECT_EQ(70, color.a);

	color = dsColor_lerpSRGB(color1, color2, 0.3f);
	dsColor3f linearColor1, linearColor2, expectedColor3f;
	dsColor3f_fromColor(&linearColor1, color1);
	dsColor3f_linearFromSRGB(&linearColor1, &linearColor1);
	dsColor3f_fromColor(&linearColor2, color2);
	dsColor3f_linearFromSRGB(&linearColor2, &linearColor2);
	dsColor3f_lerp(&expectedColor3f, &linearColor1, &linearColor2, 0.3f);
	dsColor3f_sRGBFromLinear(&expectedColor3f, &expectedColor3f);
	dsColor expectedColor = dsColor_fromColor3f(&expectedColor3f);
	EXPECT_EQ(expectedColor.r, color.r);
	EXPECT_EQ(expectedColor.g, color.g);
	EXPECT_EQ(expectedColor.b, color.b);
	EXPECT_EQ(70, color.a);
}

TEST(ColorTest, LerpColor3f)
{
	dsColor3f color1 = {{0.1f, 0.2f, 0.3f}};
	dsColor3f color2 = {{0.61f, 0.72f, 0.83f}};

	dsColor3f color;
	dsColor3f_lerp(&color, &color1, &color2, 0.3f);
	EXPECT_FLOAT_EQ(0.253f, color.r);
	EXPECT_FLOAT_EQ(0.356f, color.g);
	EXPECT_FLOAT_EQ(0.459f, color.b);

	dsColor3f_lerpSRGB(&color, &color1, &color2, 0.3f);
	dsColor3f linearColor1, linearColor2, expectedColor;
	dsColor3f_linearFromSRGB(&linearColor1, &color1);
	dsColor3f_linearFromSRGB(&linearColor2, &color2);
	dsColor3f_lerp(&expectedColor, &linearColor1, &linearColor2, 0.3f);
	dsColor3f_sRGBFromLinear(&expectedColor, &expectedColor);
	EXPECT_FLOAT_EQ(expectedColor.r, color.r);
	EXPECT_FLOAT_EQ(expectedColor.g, color.g);
	EXPECT_FLOAT_EQ(expectedColor.b, color.b);
}

TEST(ColorTest, LerpColor4f)
{
	dsColor4f color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsColor4f color2 = {{0.61f, 0.72f, 0.83f, 0.94f}};

	dsColor4f color;
	dsColor4f_lerp(&color, &color1, &color2, 0.3f);
	EXPECT_FLOAT_EQ(0.253f, color.r);
	EXPECT_FLOAT_EQ(0.356f, color.g);
	EXPECT_FLOAT_EQ(0.459f, color.b);
	EXPECT_FLOAT_EQ(0.562f, color.a);

	dsColor4f_lerpSRGB(&color, &color1, &color2, 0.3f);
	dsColor4f linearColor1, linearColor2, expectedColor;
	dsColor4f_linearFromSRGB(&linearColor1, &color1);
	dsColor4f_linearFromSRGB(&linearColor2, &color2);
	dsColor4f_lerp(&expectedColor, &linearColor1, &linearColor2, 0.3f);
	dsColor4f_sRGBFromLinear(&expectedColor, &expectedColor);
	EXPECT_FLOAT_EQ(expectedColor.r, color.r);
	EXPECT_FLOAT_EQ(expectedColor.g, color.g);
	EXPECT_FLOAT_EQ(expectedColor.b, color.b);
	EXPECT_FLOAT_EQ(0.562f, color.a);
}

TEST(ColorTest, LerpHSVColor)
{
	dsHSVColor color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsHSVColor color2 = {{0.61f, 0.72f, 0.83f, 0.94f}};

	dsHSVColor color;
	dsHSVColor_lerp(&color, &color1, &color2, 0.3f);
	EXPECT_FLOAT_EQ(0.253f, color.h);
	EXPECT_FLOAT_EQ(0.356f, color.s);
	EXPECT_FLOAT_EQ(0.459f, color.v);
	EXPECT_FLOAT_EQ(0.562f, color.a);

	dsHSVColor_lerpSRGB(&color, &color1, &color2, 0.3f);
	dsHSVColor linearColor1, linearColor2, expectedColor;
	dsHSVColor_linearFromSRGB(&linearColor1, &color1);
	dsHSVColor_linearFromSRGB(&linearColor2, &color2);
	dsHSVColor_lerp(&expectedColor, &linearColor1, &linearColor2, 0.3f);
	dsHSVColor_sRGBFromLinear(&expectedColor, &expectedColor);
	EXPECT_FLOAT_EQ(expectedColor.h, color.h);
	EXPECT_FLOAT_EQ(expectedColor.s, color.s);
	EXPECT_FLOAT_EQ(expectedColor.v, color.v);
	EXPECT_FLOAT_EQ(0.562f, color.a);
}

TEST(ColorTest, LerpHSLColor)
{
	dsHSLColor color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsHSLColor color2 = {{0.61f, 0.72f, 0.83f, 0.94f}};

	dsHSLColor color;
	dsHSLColor_lerp(&color, &color1, &color2, 0.3f);
	EXPECT_FLOAT_EQ(0.253f, color.h);
	EXPECT_FLOAT_EQ(0.356f, color.s);
	EXPECT_FLOAT_EQ(0.459f, color.l);
	EXPECT_FLOAT_EQ(0.562f, color.a);

	dsHSLColor_lerpSRGB(&color, &color1, &color2, 0.3f);
	dsHSLColor linearColor1, linearColor2, expectedColor;
	dsHSLColor_linearFromSRGB(&linearColor1, &color1);
	dsHSLColor_linearFromSRGB(&linearColor2, &color2);
	dsHSLColor_lerp(&expectedColor, &linearColor1, &linearColor2, 0.3f);
	dsHSLColor_sRGBFromLinear(&expectedColor, &expectedColor);
	EXPECT_FLOAT_EQ(expectedColor.h, color.h);
	EXPECT_FLOAT_EQ(expectedColor.s, color.s);
	EXPECT_FLOAT_EQ(expectedColor.l, color.l);
	EXPECT_FLOAT_EQ(0.562f, color.a);
}

TEST(ColorTest, EqualColor)
{
	dsColor color1 = {{10, 20, 30, 40}};
	dsColor color2 = {{0, 20, 30, 40}};
	dsColor color3 = {{10, 0, 30, 40}};
	dsColor color4 = {{10, 20, 0, 40}};
	dsColor color5 = {{10, 20, 30, 0}};

	EXPECT_TRUE(dsColor_equal(color1, color1));
	EXPECT_FALSE(dsColor_equal(color1, color2));
	EXPECT_FALSE(dsColor_equal(color1, color3));
	EXPECT_FALSE(dsColor_equal(color1, color4));
	EXPECT_FALSE(dsColor_equal(color1, color5));
}

TEST(ColorTest, EqualColor3f)
{
	dsColor3f color1 = {{0.1f, 0.2f, 0.3f}};
	dsColor3f color2 = {{0.0f, 0.2f, 0.3f}};
	dsColor3f color3 = {{0.1f, 0.0f, 0.3f}};
	dsColor3f color4 = {{0.1f, 0.2f, 0.0f}};

	EXPECT_TRUE(dsColor3f_equal(&color1, &color1));
	EXPECT_FALSE(dsColor3f_equal(&color1, &color2));
	EXPECT_FALSE(dsColor3f_equal(&color1, &color3));
	EXPECT_FALSE(dsColor3f_equal(&color1, &color4));
}

TEST(ColorTest, EqualColor4f)
{
	dsColor4f color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsColor4f color2 = {{0.0f, 0.2f, 0.3f, 0.4f}};
	dsColor4f color3 = {{0.1f, 0.0f, 0.3f, 0.4f}};
	dsColor4f color4 = {{0.1f, 0.2f, 0.0f, 0.4f}};
	dsColor4f color5 = {{0.1f, 0.2f, 0.3f, 0.0f}};

	EXPECT_TRUE(dsColor4f_equal(&color1, &color1));
	EXPECT_FALSE(dsColor4f_equal(&color1, &color2));
	EXPECT_FALSE(dsColor4f_equal(&color1, &color3));
	EXPECT_FALSE(dsColor4f_equal(&color1, &color4));
	EXPECT_FALSE(dsColor4f_equal(&color1, &color5));
}

TEST(ColorTest, EqualHSVColor)
{
	dsHSVColor color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsHSVColor color2 = {{0.0f, 0.2f, 0.3f, 0.4f}};
	dsHSVColor color3 = {{0.1f, 0.0f, 0.3f, 0.4f}};
	dsHSVColor color4 = {{0.1f, 0.2f, 0.0f, 0.4f}};
	dsHSVColor color5 = {{0.1f, 0.2f, 0.3f, 0.0f}};

	EXPECT_TRUE(dsHSVColor_equal(&color1, &color1));
	EXPECT_FALSE(dsHSVColor_equal(&color1, &color2));
	EXPECT_FALSE(dsHSVColor_equal(&color1, &color3));
	EXPECT_FALSE(dsHSVColor_equal(&color1, &color4));
	EXPECT_FALSE(dsHSVColor_equal(&color1, &color5));
}

TEST(ColorTest, EqualHSLColor)
{
	dsHSLColor color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsHSLColor color2 = {{0.0f, 0.2f, 0.3f, 0.4f}};
	dsHSLColor color3 = {{0.1f, 0.0f, 0.3f, 0.4f}};
	dsHSLColor color4 = {{0.1f, 0.2f, 0.0f, 0.4f}};
	dsHSLColor color5 = {{0.1f, 0.2f, 0.3f, 0.0f}};

	EXPECT_TRUE(dsHSLColor_equal(&color1, &color1));
	EXPECT_FALSE(dsHSLColor_equal(&color1, &color2));
	EXPECT_FALSE(dsHSLColor_equal(&color1, &color3));
	EXPECT_FALSE(dsHSLColor_equal(&color1, &color4));
	EXPECT_FALSE(dsHSLColor_equal(&color1, &color5));
}

TEST(ColorTest, EpsilonEqualColor3f)
{
	float epsilon = 1e-3f;
	dsColor3f color1 = {{0.1f, 0.2f, 0.3f}};
	dsColor3f color2 = {{0.1001f, 0.1999f, 0.3001f}};
	dsColor3f color3 = {{0.11f, 0.2f, 0.3f}};
	dsColor3f color4 = {{0.1f, 0.21f, 0.3f}};
	dsColor3f color5 = {{0.1f, 0.2f, 0.31f}};

	EXPECT_TRUE(dsColor3f_epsilonEqual(&color1, &color2, epsilon));
	EXPECT_FALSE(dsColor3f_epsilonEqual(&color1, &color3, epsilon));
	EXPECT_FALSE(dsColor3f_epsilonEqual(&color1, &color4, epsilon));
	EXPECT_FALSE(dsColor3f_epsilonEqual(&color1, &color5, epsilon));
}

TEST(ColorTest, EpsilonEqualColor4f)
{
	float epsilon = 1e-3f;
	dsColor4f color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsColor4f color2 = {{0.1001f, 0.1999f, 0.3001f, 0.3999f}};
	dsColor4f color3 = {{0.11f, 0.2f, 0.3f, 0.4f}};
	dsColor4f color4 = {{0.1f, 0.21f, 0.3f, 0.4f}};
	dsColor4f color5 = {{0.1f, 0.2f, 0.31f, 0.4f}};
	dsColor4f color6 = {{0.1f, 0.2f, 0.3f, 0.41f}};

	EXPECT_TRUE(dsColor4f_epsilonEqual(&color1, &color2, epsilon));
	EXPECT_FALSE(dsColor4f_epsilonEqual(&color1, &color3, epsilon));
	EXPECT_FALSE(dsColor4f_epsilonEqual(&color1, &color4, epsilon));
	EXPECT_FALSE(dsColor4f_epsilonEqual(&color1, &color5, epsilon));
	EXPECT_FALSE(dsColor4f_epsilonEqual(&color1, &color6, epsilon));
}

TEST(ColorTest, EpsilonEqualHSVColor)
{
	float epsilon = 1e-3f;
	dsHSVColor color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsHSVColor color2 = {{0.1001f, 0.1999f, 0.3001f, 0.3999f}};
	dsHSVColor color3 = {{0.11f, 0.2f, 0.3f, 0.4f}};
	dsHSVColor color4 = {{0.1f, 0.21f, 0.3f, 0.4f}};
	dsHSVColor color5 = {{0.1f, 0.2f, 0.31f, 0.4f}};
	dsHSVColor color6 = {{0.1f, 0.2f, 0.3f, 0.41f}};

	EXPECT_TRUE(dsHSVColor_epsilonEqual(&color1, &color2, epsilon));
	EXPECT_FALSE(dsHSVColor_epsilonEqual(&color1, &color3, epsilon));
	EXPECT_FALSE(dsHSVColor_epsilonEqual(&color1, &color4, epsilon));
	EXPECT_FALSE(dsHSVColor_epsilonEqual(&color1, &color5, epsilon));
	EXPECT_FALSE(dsHSVColor_epsilonEqual(&color1, &color6, epsilon));
}

TEST(ColorTest, EpsilonEqualHSLColor)
{
	float epsilon = 1e-3f;
	dsHSLColor color1 = {{0.1f, 0.2f, 0.3f, 0.4f}};
	dsHSLColor color2 = {{0.1001f, 0.1999f, 0.3001f, 0.3999f}};
	dsHSLColor color3 = {{0.11f, 0.2f, 0.3f, 0.4f}};
	dsHSLColor color4 = {{0.1f, 0.21f, 0.3f, 0.4f}};
	dsHSLColor color5 = {{0.1f, 0.2f, 0.31f, 0.4f}};
	dsHSLColor color6 = {{0.1f, 0.2f, 0.3f, 0.41f}};

	EXPECT_TRUE(dsHSLColor_epsilonEqual(&color1, &color2, epsilon));
	EXPECT_FALSE(dsHSLColor_epsilonEqual(&color1, &color3, epsilon));
	EXPECT_FALSE(dsHSLColor_epsilonEqual(&color1, &color4, epsilon));
	EXPECT_FALSE(dsHSLColor_epsilonEqual(&color1, &color5, epsilon));
	EXPECT_FALSE(dsHSLColor_epsilonEqual(&color1, &color6, epsilon));
}
