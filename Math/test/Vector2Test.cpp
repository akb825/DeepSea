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

#include <DeepSea/Math/Vector2.h>
#include <gtest/gtest.h>

template <typename T>
struct Vector2TypeSelector;

template <>
struct Vector2TypeSelector<float> {typedef dsVector2f Type;};

template <>
struct Vector2TypeSelector<double> {typedef dsVector2d Type;};

template <>
struct Vector2TypeSelector<int> {typedef dsVector2i Type;};

template <typename T>
class Vector2Test : public testing::Test
{
};

typedef testing::Types<float, double, int> Vector2Types;
TYPED_TEST_CASE(Vector2Test, Vector2Types);

TYPED_TEST(Vector2Test, Initialize)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type vector = {(TypeParam)-2.3, (TypeParam)4.5};

	EXPECT_EQ((TypeParam)-2.3, vector.x);
	EXPECT_EQ((TypeParam)4.5, vector.y);

	EXPECT_EQ((TypeParam)-2.3, vector.s);
	EXPECT_EQ((TypeParam)4.5, vector.t);

	EXPECT_EQ((TypeParam)-2.3, vector.r);
	EXPECT_EQ((TypeParam)4.5, vector.g);

	EXPECT_EQ((TypeParam)-2.3, vector.values[0]);
	EXPECT_EQ((TypeParam)4.5, vector.values[1]);
}
