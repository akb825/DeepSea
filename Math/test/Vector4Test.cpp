/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Math/Vector4.h>
#include <gtest/gtest.h>

template <typename T>
struct Vector4TypeSelector;

template <>
struct Vector4TypeSelector<float> {typedef dsVector4f Type;};

template <>
struct Vector4TypeSelector<double> {typedef dsVector4d Type;};

template <>
struct Vector4TypeSelector<int> {typedef dsVector4i Type;};

template <typename T>
class Vector4Test : public testing::Test
{
};

typedef testing::Types<float, double, int> Vector4Types;
TYPED_TEST_CASE(Vector4Test, Vector4Types);

TYPED_TEST(Vector4Test, Initialize)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type vector = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};

	EXPECT_EQ((TypeParam)-2.3, vector.x);
	EXPECT_EQ((TypeParam)4.5, vector.y);
	EXPECT_EQ((TypeParam)-6.7, vector.z);
	EXPECT_EQ((TypeParam)8.9, vector.w);

	EXPECT_EQ((TypeParam)-2.3, vector.s);
	EXPECT_EQ((TypeParam)4.5, vector.t);
	EXPECT_EQ((TypeParam)-6.7, vector.p);
	EXPECT_EQ((TypeParam)8.9, vector.q);

	EXPECT_EQ((TypeParam)-2.3, vector.r);
	EXPECT_EQ((TypeParam)4.5, vector.g);
	EXPECT_EQ((TypeParam)-6.7, vector.b);
	EXPECT_EQ((TypeParam)8.9, vector.a);

	EXPECT_EQ((TypeParam)-2.3, vector.values[0]);
	EXPECT_EQ((TypeParam)4.5, vector.values[1]);
	EXPECT_EQ((TypeParam)-6.7, vector.values[2]);
	EXPECT_EQ((TypeParam)8.9, vector.values[3]);
}
