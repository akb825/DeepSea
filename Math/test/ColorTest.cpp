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
#include <DeepSea/Math/Core.h>
#include <gtest/gtest.h>

static bool testHSVColor(uint8_t red, uint8_t green, uint8_t blue, float hue, float saturation,
	float value, float epsilon)
{
	dsColor color = {{red, green, blue, 128}};
	dsHSVColor hsvColor;
	dsHSVColor_fromColor(&hsvColor, color);
	bool success = dsEpsilonEqualf(hue, hsvColor.h, epsilon) &&
		dsEpsilonEqualf(saturation, hsvColor.s, epsilon) &&
		dsEpsilonEqualf(value, hsvColor.v, epsilon) &&
		dsEpsilonEqualf(0.5f, hsvColor.a, epsilon);
	EXPECT_NEAR(hue, hsvColor.h, epsilon);
	EXPECT_NEAR(saturation, hsvColor.s, epsilon);
	EXPECT_NEAR(value, hsvColor.v, epsilon);
	EXPECT_NEAR(0.5, hsvColor.a, epsilon);

	color = dsColor_fromHSVColor(&hsvColor);
	success = success && red == color.r && green == color.g && blue == color.b && 128 == color.a;
	EXPECT_EQ(red, color.r);
	EXPECT_EQ(green, color.g);
	EXPECT_EQ(blue, color.b);
	EXPECT_EQ(128, color.a);

	return success;
}

static bool testHSVColor3f(float red, float green, float blue, float hue, float saturation,
	float value, float epsilon)
{
	dsColor3f color3f = {{red, green, blue}};
	dsHSVColor hsvColor;
	dsHSVColor_fromColor3f(&hsvColor, &color3f);
	bool success = dsEpsilonEqualf(hue, hsvColor.h, epsilon) &&
		dsEpsilonEqualf(saturation, hsvColor.s, epsilon) &&
		dsEpsilonEqualf(value, hsvColor.v, epsilon) && hsvColor.a == 1.0f;
	EXPECT_NEAR(hue, hsvColor.h, epsilon);
	EXPECT_NEAR(saturation, hsvColor.s, epsilon);
	EXPECT_NEAR(value, hsvColor.v, epsilon);
	EXPECT_EQ(1.0f, hsvColor.a);

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
	dsColor4f color4f = {{red, green, blue, 0.5f}};
	dsHSVColor hsvColor;
	dsHSVColor_fromColor4f(&hsvColor, &color4f);
	bool success = dsEpsilonEqualf(hue, hsvColor.h, epsilon) &&
		dsEpsilonEqualf(saturation, hsvColor.s, epsilon) &&
		dsEpsilonEqualf(value, hsvColor.v, epsilon) && 0.5f == hsvColor.a;
	EXPECT_NEAR(hue, hsvColor.h, epsilon);
	EXPECT_NEAR(saturation, hsvColor.s, epsilon);
	EXPECT_NEAR(value, hsvColor.v, epsilon);
	EXPECT_EQ(0.5f, hsvColor.a);

	dsColor4f_fromHSVColor(&color4f, &hsvColor);
	success = success && dsEpsilonEqualf(red, color4f.r, epsilon) &&
		dsEpsilonEqualf(green, color4f.g, epsilon) &&
		dsEpsilonEqualf(blue, color4f.b, epsilon) && 0.5f == color4f.a;
	EXPECT_NEAR(red, color4f.r, epsilon);
	EXPECT_NEAR(green, color4f.g, epsilon);
	EXPECT_NEAR(blue, color4f.b, epsilon);
	EXPECT_EQ(0.5f, color4f.a);

	return success;
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

TEST(ColorTest, ConvertGrayscale)
{
	dsColor color = {{10, 20, 30}};
	EXPECT_EQ(19U, dsColor_grayscale(color));

	dsColor3f color3f = {{0.1f, 0.2f, 0.3f}};
	EXPECT_FLOAT_EQ(0.18596f, dsColor3f_grayscale(&color3f));

	dsColor4f color4f = {{0.1f, 0.2f, 0.3f, 0.4f}};
	EXPECT_FLOAT_EQ(0.18596f, dsColor4f_grayscale(&color4f));
}
