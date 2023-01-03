/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/Math/SIMD/Matrix44x4.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>
#include <gtest/gtest.h>

#if DS_HAS_SIMD

static constexpr float epsilon = 2e-5f;

TEST(Matrix44x4Test, Multiply)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44f b0 = a1;
	dsMatrix44f b1 = a0;
	dsMatrix44f b2 = a3;
	dsMatrix44f b3 = a2;

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f b;
	dsMatrix44x4f_load(&b, &b0, &b1, &b2, &b3);

	dsMatrix44x4f result;
	dsMatrix44x4f_mul(&result, &a, &b);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44_mul(expected0, a0, b0);
	dsMatrix44_mul(expected1, a1, b1);
	dsMatrix44_mul(expected2, a2, b2);
	dsMatrix44_mul(expected3, a3, b3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, MultiplyFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44f b0 = a1;
	dsMatrix44f b1 = a0;
	dsMatrix44f b2 = a3;
	dsMatrix44f b3 = a2;

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f b;
	dsMatrix44x4f_load(&b, &b0, &b1, &b2, &b3);

	dsMatrix44x4f result;
	dsMatrix44x4f_mulFMA(&result, &a, &b);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44_mul(expected0, a0, b0);
	dsMatrix44_mul(expected1, a1, b1);
	dsMatrix44_mul(expected2, a2, b2);
	dsMatrix44_mul(expected3, a3, b3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, AffineMultiply)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44f b0 = a1;
	dsMatrix44f b1 = a0;
	dsMatrix44f b2 = a3;
	dsMatrix44f b3 = a2;

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f b;
	dsMatrix44x4f_load(&b, &b0, &b1, &b2, &b3);

	dsMatrix44x4f result;
	dsMatrix44x4f_affineMul(&result, &a, &b);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44_affineMul(expected0, a0, b0);
	dsMatrix44_affineMul(expected1, a1, b1);
	dsMatrix44_affineMul(expected2, a2, b2);
	dsMatrix44_affineMul(expected3, a3, b3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, AffineMultiplyFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44f b0 = a1;
	dsMatrix44f b1 = a0;
	dsMatrix44f b2 = a3;
	dsMatrix44f b3 = a2;

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f b;
	dsMatrix44x4f_load(&b, &b0, &b1, &b2, &b3);

	dsMatrix44x4f result;
	dsMatrix44x4f_affineMulFMA(&result, &a, &b);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44_affineMul(expected0, a0, b0);
	dsMatrix44_affineMul(expected1, a1, b1);
	dsMatrix44_affineMul(expected2, a2, b2);
	dsMatrix44_affineMul(expected3, a3, b3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, Transpose)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_transpose(&result, &a);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44_transpose(expected0, a0);
	dsMatrix44_transpose(expected1, a1);
	dsMatrix44_transpose(expected2, a2);
	dsMatrix44_transpose(expected3, a3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, FastInvert)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_fastInvert(&result, &a);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44_fastInvert(expected0, a0);
	dsMatrix44_fastInvert(expected1, a1);
	dsMatrix44_fastInvert(expected2, a2);
	dsMatrix44_fastInvert(expected3, a3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, FastInvertFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_fastInvertFMA(&result, &a);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44_fastInvert(expected0, a0);
	dsMatrix44_fastInvert(expected1, a1);
	dsMatrix44_fastInvert(expected2, a2);
	dsMatrix44_fastInvert(expected3, a3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, DISABLED_AffineInvert)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_affineInvert(&result, &a);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44f_affineInvert(&expected0, &a0);
	dsMatrix44f_affineInvert(&expected1, &a1);
	dsMatrix44f_affineInvert(&expected2, &a2);
	dsMatrix44f_affineInvert(&expected3, &a3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, DISABLED_AffineInvertFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_affineInvertFMA(&result, &a);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44f_affineInvert(&expected0, &a0);
	dsMatrix44f_affineInvert(&expected1, &a1);
	dsMatrix44f_affineInvert(&expected2, &a2);
	dsMatrix44f_affineInvert(&expected3, &a3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, Invert)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_invert(&result, &a);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44f_invert(&expected0, &a0);
	dsMatrix44f_invert(&expected1, &a1);
	dsMatrix44f_invert(&expected2, &a2);
	dsMatrix44f_invert(&expected3, &a3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, InvertFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_invertFMA(&result, &a);

	dsMatrix44f expected0, expected1, expected2, expected3;
	dsMatrix44f_invert(&expected0, &a0);
	dsMatrix44f_invert(&expected1, &a1);
	dsMatrix44f_invert(&expected2, &a2);
	dsMatrix44f_invert(&expected3, &a3);

	dsMatrix44f result0, result1, result2, result3;
	dsMatrix44x4f_store(&result0, &result1, &result2, &result3, &result);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0.values[i][j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1.values[i][j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2.values[i][j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3.values[i][j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, InverseTranspose)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_inverseTranspose(&result, &a);

	dsMatrix33f expected0, expected1, expected2, expected3;
	dsMatrix44f_inverseTranspose(&expected0, &a0);
	dsMatrix44f_inverseTranspose(&expected1, &a1);
	dsMatrix44f_inverseTranspose(&expected2, &a2);
	dsMatrix44f_inverseTranspose(&expected3, &a3);

	dsVector4f result0[3], result1[3], result2[3], result3[3];
	dsMatrix44x4f_store33(result0, result1, result2, result3, &result);

	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0[i].values[j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1[i].values[j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2[i].values[j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3[i].values[j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, InverseTransposeFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_inverseTransposeFMA(&result, &a);

	dsMatrix33f expected0, expected1, expected2, expected3;
	dsMatrix44f_inverseTranspose(&expected0, &a0);
	dsMatrix44f_inverseTranspose(&expected1, &a1);
	dsMatrix44f_inverseTranspose(&expected2, &a2);
	dsMatrix44f_inverseTranspose(&expected3, &a3);

	dsVector4f result0[3], result1[3], result2[3], result3[3];
	dsMatrix44x4f_store33(result0, result1, result2, result3, &result);

	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0[i].values[j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1[i].values[j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2[i].values[j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3[i].values[j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, Invert33)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_invert33(&result, &a);

	dsMatrix33f temp0, temp1, temp2, temp3;
	dsMatrix33f expected0, expected1, expected2, expected3;
	dsMatrix33_copy(temp0, a0);
	dsMatrix33f_invert(&expected0, &temp0);
	dsMatrix33_copy(temp1, a1);
	dsMatrix33f_invert(&expected1, &temp1);
	dsMatrix33_copy(temp2, a2);
	dsMatrix33f_invert(&expected2, &temp2);
	dsMatrix33_copy(temp3, a3);
	dsMatrix33f_invert(&expected3, &temp3);

	dsVector4f result0[3], result1[3], result2[3], result3[3];
	dsMatrix44x4f_store33(result0, result1, result2, result3, &result);

	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0[i].values[j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1[i].values[j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2[i].values[j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3[i].values[j], epsilon);
		}
	}
}

TEST(Matrix44x4Test, Invert33FMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	dsMatrix44f a0 =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44f a1 =
	{{
		{1.0f, -3.2f, -5.4f, 7.6f},
		{-9.8f, 1.0f, -3.2f, 5.4f},
		{7.6f, -9.8f, 1.0f, -3.2f},
		{-5.4f, 7.6f, 9.8f, -1.0f}
	}};

	dsMatrix44f a2 =
	{{
		{0.1f, -2.3f, 4.5f, -6.7f},
		{-8.9f, 0.1f, -2.3f, 4.5f},
		{6.7f, -8.9f, -0.1f, 2.3f},
		{-4.5f, 6.7f, 8.9f, -0.1f}
	}};

	dsMatrix44f a3 =
	{{
		{-1.0f, 3.2f, 5.4f, -7.6f},
		{9.8f, -1.0f, 3.2f, -5.4f},
		{-7.6f, 9.8f, -1.0f, 3.2f},
		{5.4f, -7.6f, -9.8f, 1.0f}
	}};

	dsMatrix44x4f a;
	dsMatrix44x4f_load(&a, &a0, &a1, &a2, &a3);

	dsMatrix44x4f result;
	dsMatrix44x4f_invert33FMA(&result, &a);

	dsMatrix33f temp0, temp1, temp2, temp3;
	dsMatrix33f expected0, expected1, expected2, expected3;
	dsMatrix33_copy(temp0, a0);
	dsMatrix33f_invert(&expected0, &temp0);
	dsMatrix33_copy(temp1, a1);
	dsMatrix33f_invert(&expected1, &temp1);
	dsMatrix33_copy(temp2, a2);
	dsMatrix33f_invert(&expected2, &temp2);
	dsMatrix33_copy(temp3, a3);
	dsMatrix33f_invert(&expected3, &temp3);

	dsVector4f result0[3], result1[3], result2[3], result3[3];
	dsMatrix44x4f_store33(result0, result1, result2, result3, &result);

	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(expected0.values[i][j], result0[i].values[j], epsilon);
			EXPECT_NEAR(expected1.values[i][j], result1[i].values[j], epsilon);
			EXPECT_NEAR(expected2.values[i][j], result2[i].values[j], epsilon);
			EXPECT_NEAR(expected3.values[i][j], result3[i].values[j], epsilon);
		}
	}
}

#endif // DS_HAS_SIMD
