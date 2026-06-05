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

#include <DeepSea/Geometry/OrientedBox3x.h>
#include <DeepSea/Math/Matrix44.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct OrientedBox3xTypeSelector;

template <>
struct OrientedBox3xTypeSelector<float>
{
	typedef dsVector3xf Vector3xType;
	typedef dsVector4f Vector4Type;
	typedef dsAlignedBox3xf AlignedBox3xType;
	typedef dsMatrix44f Matrix44Type;
	typedef dsOrientedBox3xf OrientedBox3xType;
	static const float epsilon;
};

template <>
struct OrientedBox3xTypeSelector<double>
{
	typedef dsVector3xd Vector3xType;
	typedef dsVector4d Vector4Type;
	typedef dsAlignedBox3xd AlignedBox3xType;
	typedef dsMatrix44d Matrix44Type;
	typedef dsOrientedBox3xd OrientedBox3xType;
	static const double epsilon;
};

const float OrientedBox3xTypeSelector<float>::epsilon = 1e-4f;
const double OrientedBox3xTypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class OrientedBox3xTest : public testing::Test
{
};

using OrientedBox3xTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(OrientedBox3xTest, OrientedBox3xTypes);

inline bool dsOrientedBox3x_isValid(const dsOrientedBox3xf* box)
{
	return dsOrientedBox3xf_isValid(box);
}

inline bool dsOrientedBox3x_isValid(const dsOrientedBox3xd* box)
{
	return dsOrientedBox3xd_isValid(box);
}

inline void dsOrientedBox3x_fromAlignedBox(
	dsOrientedBox3xf* result, const dsAlignedBox3xf* alignedBox)
{
	dsOrientedBox3xf_fromAlignedBox(result, alignedBox);
}

inline void dsOrientedBox3x_fromAlignedBox(
	dsOrientedBox3xd* result, const dsAlignedBox3xd* alignedBox)
{
	dsOrientedBox3xd_fromAlignedBox(result, alignedBox);
}

inline void dsOrientedBox3x_toMatrix(dsMatrix44f* result, const dsOrientedBox3xf* box)
{
	dsOrientedBox3xf_toMatrix(result, box);
}

inline void dsOrientedBox3x_toMatrix(dsMatrix44d* result, const dsOrientedBox3xd* box)
{
	dsOrientedBox3xd_toMatrix(result, box);
}

inline void dsOrientedBox3x_toMatrixTranspose(dsMatrix44f* result, const dsOrientedBox3xf* box)
{
	dsOrientedBox3xf_toMatrixTranspose(result, box);
}

inline void dsOrientedBox3x_toMatrixTranspose(dsMatrix44d* result, const dsOrientedBox3xd* box)
{
	dsOrientedBox3xd_toMatrixTranspose(result, box);
}

inline void dsOrientedBox3x_makeInvalid(dsOrientedBox3xf* result)
{
	dsOrientedBox3xf_makeInvalid(result);
}

inline void dsOrientedBox3x_makeInvalid(dsOrientedBox3xd* result)
{
	dsOrientedBox3xd_makeInvalid(result);
}

inline void dsOrientedBox3x_fromMatrix(dsOrientedBox3xf* result, const dsMatrix44f* matrix)
{
	dsOrientedBox3xf_fromMatrix(result, matrix);
}

inline void dsOrientedBox3x_fromMatrix(dsOrientedBox3xd* result, const dsMatrix44d* matrix)
{
	dsOrientedBox3xd_fromMatrix(result, matrix);
}

inline bool dsOrientedBox3x_transform(dsOrientedBox3xf* box, const dsMatrix44f* transform)
{
	return dsOrientedBox3xf_transform(box, transform);
}

inline bool dsOrientedBox3x_transform(dsOrientedBox3xd* box, const dsMatrix44d* transform)
{
	return dsOrientedBox3xd_transform(box, transform);
}

inline void dsOrientedBox3x_addPoint(dsOrientedBox3xf* box, const dsVector3xf* point)
{
	dsOrientedBox3xf_addPoint(box, point);
}

inline void dsOrientedBox3x_addPoint(dsOrientedBox3xd* box, const dsVector3xd* point)
{
	dsOrientedBox3xd_addPoint(box, point);
}

inline bool dsOrientedBox3x_addBox(dsOrientedBox3xf* box, const dsOrientedBox3xf* otherBox)
{
	return dsOrientedBox3xf_addBox(box, otherBox);
}

inline bool dsOrientedBox3x_addBox(dsOrientedBox3xd* box, const dsOrientedBox3xd* otherBox)
{
	return dsOrientedBox3xd_addBox(box, otherBox);
}

inline bool dsOrientedBox3x_corners(
	dsVector3xf corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3xf* box)
{
	return dsOrientedBox3xf_corners(corners, box);
}

inline bool dsOrientedBox3x_corners(
	dsVector3xd corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3xd* box)
{
	return dsOrientedBox3xd_corners(corners, box);
}

inline bool dsOrientedBox3x_intersects(
	const dsOrientedBox3xf* box, const dsOrientedBox3xf* otherBox)
{
	return dsOrientedBox3xf_intersects(box, otherBox);
}

inline bool dsOrientedBox3x_intersects(
	const dsOrientedBox3xd* box, const dsOrientedBox3xd* otherBox)
{
	return dsOrientedBox3xd_intersects(box, otherBox);
}

inline bool dsOrientedBox3x_containsPoint(const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	return dsOrientedBox3xf_containsPoint(box, point);
}

inline bool dsOrientedBox3x_containsPoint(const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	return dsOrientedBox3xd_containsPoint(box, point);
}

inline bool dsOrientedBox3x_closestPoint(
	dsVector3xf* result, const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	return dsOrientedBox3xf_closestPoint(result, box, point);
}

inline bool dsOrientedBox3x_closestPoint(
	dsVector3xd* result, const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	return dsOrientedBox3xd_closestPoint(result, box, point);
}

inline float dsOrientedBox3x_dist2(const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	return dsOrientedBox3xf_dist2(box, point);
}

inline double dsOrientedBox3x_dist2(const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	return dsOrientedBox3xd_dist2(box, point);
}

inline float dsOrientedBox3x_dist(const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	return dsOrientedBox3xf_dist(box, point);
}

inline double dsOrientedBox3x_dist(const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	return dsOrientedBox3xd_dist(box, point);
}

inline void dsMatrix44_makeRotate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeRotate(result, x, y, z);
}

inline void dsMatrix44_makeRotate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeRotate(result, x, y, z);
}

inline void dsMatrix44_makeTranslate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_makeTranslate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_makeScale(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeScale(result, x, y, z);
}

inline void dsMatrix44_makeScale(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeScale(result, x, y, z);
}

TYPED_TEST(OrientedBox3xTest, Initialize)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;

	OrientedBox3xType box =
	{
		{{ {1, 0, 0, 7}, {0, 1, 0, 8}, {0, 0, 1, 9} }},
		{{1, 2, 3, 10}}, {{4, 5, 6, 11}}
	};

	EXPECT_EQ((TypeParam)1, box.orientation.values[0][0]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[0][1]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[0][2]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[1][0]);
	EXPECT_EQ((TypeParam)1, box.orientation.values[1][1]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[1][2]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[2][0]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[2][1]);
	EXPECT_EQ((TypeParam)1, box.orientation.values[2][2]);

	EXPECT_EQ((TypeParam)1, box.center.x);
	EXPECT_EQ((TypeParam)2, box.center.y);
	EXPECT_EQ((TypeParam)3, box.center.z);

	EXPECT_EQ((TypeParam)4, box.halfExtents.x);
	EXPECT_EQ((TypeParam)5, box.halfExtents.y);
	EXPECT_EQ((TypeParam)6, box.halfExtents.z);
}

TYPED_TEST(OrientedBox3xTest, IsValid)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;

	OrientedBox3xType box =
	{
		{{ {1, 0, 0, 7}, {0, 1, 0, 8}, {0, 0, 1, 9} }},
		{{1, 2, 3, 10}}, {{4, 5, 6, 11}}
	};

	EXPECT_TRUE(dsOrientedBox3x_isValid(&box));

	box.halfExtents.x = -1;
	EXPECT_FALSE(dsOrientedBox3x_isValid(&box));

	box.halfExtents.x = 4;
	box.halfExtents.y = -1;
	EXPECT_FALSE(dsOrientedBox3x_isValid(&box));

	box.halfExtents.y = 5;
	box.halfExtents.z = -1;
	EXPECT_FALSE(dsOrientedBox3x_isValid(&box));
}

TYPED_TEST(OrientedBox3xTest, FromAlignedBox)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	AlignedBox3xType alignedBox = {{{0, 1, 2, 10}}, {{4, 7, 10, 11}}};

	dsOrientedBox3x_fromAlignedBox(&box, &alignedBox);
	EXPECT_EQ((TypeParam)1, box.orientation.values[0][0]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[0][1]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[0][2]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[1][0]);
	EXPECT_EQ((TypeParam)1, box.orientation.values[1][1]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[1][2]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[2][0]);
	EXPECT_EQ((TypeParam)0, box.orientation.values[2][1]);
	EXPECT_EQ((TypeParam)1, box.orientation.values[2][2]);

	EXPECT_EQ((TypeParam)2, box.center.x);
	EXPECT_EQ((TypeParam)4, box.center.y);
	EXPECT_EQ((TypeParam)6, box.center.z);

	EXPECT_EQ((TypeParam)2, box.halfExtents.x);
	EXPECT_EQ((TypeParam)3, box.halfExtents.y);
	EXPECT_EQ((TypeParam)4, box.halfExtents.z);
}

TYPED_TEST(OrientedBox3xTest, MakeInvalid)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;

	OrientedBox3xType box =
	{
		{{ {1, 0, 0, 7}, {0, 1, 0, 8}, {0, 0, 1, 9} }},
		{{1, 2, 3, 10}}, {{4, 5, 6, 11}}
	};

	EXPECT_TRUE(dsOrientedBox3x_isValid(&box));

	dsOrientedBox3x_makeInvalid(&box);
	EXPECT_FALSE(dsOrientedBox3x_isValid(&box));
}

TYPED_TEST(OrientedBox3xTest, AddPoint)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType point1 = {{5, 6, 3, 10}};
	Vector3xType point2 = {{1, 6, 3, 11}};
	Vector3xType point3 = {{5, 0, 3, 12}};
	Vector3xType point4 = {{5, 6, -1, 13}};
	Vector3xType point5 = {{9, 6, 3, 14}};
	Vector3xType point6 = {{5, 10, 3, 15}};
	Vector3xType point7 = {{5, 6, 11, 16}};

	dsOrientedBox3x_addPoint(&box, &point1);
	EXPECT_EQ((TypeParam)6, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)4, box.center.z);
	EXPECT_EQ((TypeParam)3, box.halfExtents.x);
	EXPECT_EQ((TypeParam)2, box.halfExtents.y);
	EXPECT_EQ((TypeParam)1, box.halfExtents.z);

	dsOrientedBox3x_addPoint(&box, &point2);
	EXPECT_EQ((TypeParam)4.5, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)4, box.center.z);
	EXPECT_EQ((TypeParam)3, box.halfExtents.x);
	EXPECT_EQ((TypeParam)3.5, box.halfExtents.y);
	EXPECT_EQ((TypeParam)1, box.halfExtents.z);

	dsOrientedBox3x_addPoint(&box, &point3);
	EXPECT_EQ((TypeParam)4.5, box.center.x);
	EXPECT_EQ((TypeParam)3, box.center.y);
	EXPECT_EQ((TypeParam)4, box.center.z);
	EXPECT_EQ((TypeParam)3, box.halfExtents.x);
	EXPECT_EQ((TypeParam)3.5, box.halfExtents.y);
	EXPECT_EQ((TypeParam)3, box.halfExtents.z);

	dsOrientedBox3x_addPoint(&box, &point4);
	EXPECT_EQ((TypeParam)4.5, box.center.x);
	EXPECT_EQ((TypeParam)3, box.center.y);
	EXPECT_EQ((TypeParam)3, box.center.z);
	EXPECT_EQ((TypeParam)4, box.halfExtents.x);
	EXPECT_EQ((TypeParam)3.5, box.halfExtents.y);
	EXPECT_EQ((TypeParam)3, box.halfExtents.z);

	dsOrientedBox3x_addPoint(&box, &point5);
	EXPECT_EQ((TypeParam)5, box.center.x);
	EXPECT_EQ((TypeParam)3, box.center.y);
	EXPECT_EQ((TypeParam)3, box.center.z);
	EXPECT_EQ((TypeParam)4, box.halfExtents.x);
	EXPECT_EQ((TypeParam)4, box.halfExtents.y);
	EXPECT_EQ((TypeParam)3, box.halfExtents.z);

	dsOrientedBox3x_addPoint(&box, &point6);
	EXPECT_EQ((TypeParam)5, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)3, box.center.z);
	EXPECT_EQ((TypeParam)4, box.halfExtents.x);
	EXPECT_EQ((TypeParam)4, box.halfExtents.y);
	EXPECT_EQ((TypeParam)5, box.halfExtents.z);

	dsOrientedBox3x_addPoint(&box, &point7);
	EXPECT_EQ((TypeParam)5, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)5, box.center.z);
	EXPECT_EQ((TypeParam)6, box.halfExtents.x);
	EXPECT_EQ((TypeParam)4, box.halfExtents.y);
	EXPECT_EQ((TypeParam)5, box.halfExtents.z);
}

TYPED_TEST(OrientedBox3xTest, Corners)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;
	TypeParam epsilon = OrientedBox3xTypeSelector<TypeParam>::epsilon;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3x_corners(corners, &box));

	EXPECT_NEAR(8, corners[0].x, epsilon);
	EXPECT_NEAR(4, corners[0].y, epsilon);
	EXPECT_NEAR(1, corners[0].z, epsilon);

	EXPECT_NEAR(8, corners[1].x, epsilon);
	EXPECT_NEAR(6, corners[1].y, epsilon);
	EXPECT_NEAR(1, corners[1].z, epsilon);

	EXPECT_NEAR(4, corners[2].x, epsilon);
	EXPECT_NEAR(4, corners[2].y, epsilon);
	EXPECT_NEAR(1, corners[2].z, epsilon);

	EXPECT_NEAR(4, corners[3].x, epsilon);
	EXPECT_NEAR(6, corners[3].y, epsilon);
	EXPECT_NEAR(1, corners[3].z, epsilon);

	EXPECT_NEAR(8, corners[4].x, epsilon);
	EXPECT_NEAR(4, corners[4].y, epsilon);
	EXPECT_NEAR(7, corners[4].z, epsilon);

	EXPECT_NEAR(8, corners[5].x, epsilon);
	EXPECT_NEAR(6, corners[5].y, epsilon);
	EXPECT_NEAR(7, corners[5].z, epsilon);

	EXPECT_NEAR(4, corners[6].x, epsilon);
	EXPECT_NEAR(4, corners[6].y, epsilon);
	EXPECT_NEAR(7, corners[6].z, epsilon);

	EXPECT_NEAR(4, corners[7].x, epsilon);
	EXPECT_NEAR(6, corners[7].y, epsilon);
	EXPECT_NEAR(7, corners[7].z, epsilon);
}

TYPED_TEST(OrientedBox3xTest, ToMatrix)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector4Type Vector4Type;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;
	TypeParam epsilon = OrientedBox3xTypeSelector<TypeParam>::epsilon;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3x_corners(corners, &box));

	Matrix44Type matrix;
	dsOrientedBox3x_toMatrix(&matrix, &box);

	Vector4Type lowerLeft = {{-1, -1, -1, 1}};
	Vector4Type boxPoint;
	dsMatrix44_transform(boxPoint, matrix, lowerLeft);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].z, boxPoint.z, epsilon);

	Vector4Type upperRight = {{1, 1, 1, 1}};
	dsMatrix44_transform(boxPoint, matrix, upperRight);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].z, boxPoint.z, epsilon);

	OrientedBox3xType restoredBox;
	dsOrientedBox3x_fromMatrix(&restoredBox, &matrix);
	EXPECT_NEAR(box.orientation.values[0][0], restoredBox.orientation.values[0][0], epsilon);
	EXPECT_NEAR(box.orientation.values[0][1], restoredBox.orientation.values[0][1], epsilon);
	EXPECT_NEAR(box.orientation.values[0][2], restoredBox.orientation.values[0][2], epsilon);
	EXPECT_NEAR(box.orientation.values[1][0], restoredBox.orientation.values[1][0], epsilon);
	EXPECT_NEAR(box.orientation.values[1][1], restoredBox.orientation.values[1][1], epsilon);
	EXPECT_NEAR(box.orientation.values[1][2], restoredBox.orientation.values[1][2], epsilon);
	EXPECT_NEAR(box.orientation.values[2][0], restoredBox.orientation.values[2][0], epsilon);
	EXPECT_NEAR(box.orientation.values[2][1], restoredBox.orientation.values[2][1], epsilon);
	EXPECT_NEAR(box.orientation.values[2][2], restoredBox.orientation.values[2][2], epsilon);
	EXPECT_NEAR(box.center.x, restoredBox.center.x, epsilon);
	EXPECT_NEAR(box.center.y, restoredBox.center.y, epsilon);
	EXPECT_NEAR(box.center.z, restoredBox.center.z, epsilon);
	EXPECT_NEAR(box.halfExtents.x, restoredBox.halfExtents.x, epsilon);
	EXPECT_NEAR(box.halfExtents.y, restoredBox.halfExtents.y, epsilon);
	EXPECT_NEAR(box.halfExtents.z, restoredBox.halfExtents.z, epsilon);
}

TYPED_TEST(OrientedBox3xTest, ToMatrixTranspose)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Matrix44Type matrix, transposedMatrix;
	dsOrientedBox3x_toMatrix(&matrix, &box);
	dsOrientedBox3x_toMatrixTranspose(&transposedMatrix, &box);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			EXPECT_EQ(matrix.values[j][i], transposedMatrix.values[i][j]);
	}
}

TYPED_TEST(OrientedBox3xTest, Transform)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector4Type Vector4Type;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;
	TypeParam epsilon = OrientedBox3xTypeSelector<TypeParam>::epsilon;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3x_corners(corners, &box));

	Matrix44Type rotate, translate, scale, temp, transform;

	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -2, 5, -1);
	dsMatrix44_makeScale(&scale, 7, 8, 6);

	dsMatrix44_mul(temp, rotate, scale);
	dsMatrix44_mul(transform, translate, temp);

	Vector4Type originalCenter = {{box.center.x, box.center.y, box.center.z, 1}};
	Vector4Type center;
	dsMatrix44_transform(center, transform, originalCenter);

	EXPECT_TRUE(dsOrientedBox3x_transform(&box, &transform));

	EXPECT_NEAR(rotate.values[2][0], box.orientation.values[0][0], epsilon);
	EXPECT_NEAR(rotate.values[2][1], box.orientation.values[0][1], epsilon);
	EXPECT_NEAR(rotate.values[2][2], box.orientation.values[0][2], epsilon);

	EXPECT_NEAR(-rotate.values[0][0], box.orientation.values[1][0], epsilon);
	EXPECT_NEAR(-rotate.values[0][1], box.orientation.values[1][1], epsilon);
	EXPECT_NEAR(-rotate.values[0][2], box.orientation.values[1][2], epsilon);

	EXPECT_NEAR(rotate.values[1][0], box.orientation.values[2][0], epsilon);
	EXPECT_NEAR(rotate.values[1][1], box.orientation.values[2][1], epsilon);
	EXPECT_NEAR(rotate.values[1][2], box.orientation.values[2][2], epsilon);

	EXPECT_NEAR(center.x, box.center.x, epsilon);
	EXPECT_NEAR(center.y, box.center.y, epsilon);
	EXPECT_NEAR(center.z, box.center.z, epsilon);

	EXPECT_NEAR(18, box.halfExtents.x, epsilon);
	EXPECT_NEAR(14, box.halfExtents.y, epsilon);
	EXPECT_NEAR(8, box.halfExtents.z, epsilon);

	Vector3xType transformedCorners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3x_corners(transformedCorners, &box));

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		Vector4Type curCorner = {{corners[i].x, corners[i].y, corners[i].z, 1}};
		Vector4Type expectedCorner;
		dsMatrix44_transform(expectedCorner, transform, curCorner);

		EXPECT_NEAR(expectedCorner.x, transformedCorners[i].x, epsilon);
		EXPECT_NEAR(expectedCorner.y, transformedCorners[i].y, epsilon);
		EXPECT_NEAR(expectedCorner.z, transformedCorners[i].z, epsilon);
	}
}

TYPED_TEST(OrientedBox3xTest, TransformIncremental)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector4Type Vector4Type;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;
	TypeParam epsilon = OrientedBox3xTypeSelector<TypeParam>::epsilon;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3x_corners(corners, &box));

	Matrix44Type rotate, translate, scale, temp, transform;

	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -2, 5, -1);
	dsMatrix44_makeScale(&scale, 7, 8, 6);

	dsMatrix44_mul(temp, rotate, scale);
	dsMatrix44_mul(transform, translate, temp);

	Vector4Type originalCenter = {{box.center.x, box.center.y, box.center.z, 1}};
	Vector4Type center;
	dsMatrix44_transform(center, transform, originalCenter);

	EXPECT_TRUE(dsOrientedBox3x_transform(&box, &scale));
	EXPECT_TRUE(dsOrientedBox3x_transform(&box, &rotate));
	EXPECT_TRUE(dsOrientedBox3x_transform(&box, &translate));

	EXPECT_NEAR(rotate.values[2][0], box.orientation.values[0][0], epsilon);
	EXPECT_NEAR(rotate.values[2][1], box.orientation.values[0][1], epsilon);
	EXPECT_NEAR(rotate.values[2][2], box.orientation.values[0][2], epsilon);

	EXPECT_NEAR(-rotate.values[0][0], box.orientation.values[1][0], epsilon);
	EXPECT_NEAR(-rotate.values[0][1], box.orientation.values[1][1], epsilon);
	EXPECT_NEAR(-rotate.values[0][2], box.orientation.values[1][2], epsilon);

	EXPECT_NEAR(rotate.values[1][0], box.orientation.values[2][0], epsilon);
	EXPECT_NEAR(rotate.values[1][1], box.orientation.values[2][1], epsilon);
	EXPECT_NEAR(rotate.values[1][2], box.orientation.values[2][2], epsilon);

	EXPECT_NEAR(center.x, box.center.x, epsilon);
	EXPECT_NEAR(center.y, box.center.y, epsilon);
	EXPECT_NEAR(center.z, box.center.z, epsilon);

	EXPECT_NEAR(18, box.halfExtents.x, epsilon);
	EXPECT_NEAR(14, box.halfExtents.y, epsilon);
	EXPECT_NEAR(8, box.halfExtents.z, epsilon);

	Vector3xType transformedCorners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3x_corners(transformedCorners, &box));

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		Vector4Type curCorner = {{corners[i].x, corners[i].y, corners[i].z, 1}};
		Vector4Type expectedCorner;
		dsMatrix44_transform(expectedCorner, transform, curCorner);

		EXPECT_NEAR(expectedCorner.x, transformedCorners[i].x, epsilon);
		EXPECT_NEAR(expectedCorner.y, transformedCorners[i].y, epsilon);
		EXPECT_NEAR(expectedCorner.z, transformedCorners[i].z, epsilon);
	}
}

TYPED_TEST(OrientedBox3xTest, AddBox)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	TypeParam epsilon = OrientedBox3xTypeSelector<TypeParam>::epsilon;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	OrientedBox3xType otherBox =
	{
		{{ {1, 0, 0, 10}, {0, 1, 0, 11}, {0, 0, 1, 12} }},
		{{1, 2, 3, 13}}, {{4, 5, 6, 14}}
	};

	Matrix44Type rotate, translate, scale, temp, transform;

	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -2, 5, -1);
	dsMatrix44_makeScale(&scale, 7, 8, 6);

	dsMatrix44_mul(temp, rotate, scale);
	dsMatrix44_mul(transform, translate, temp);

	dsOrientedBox3x_transform(&otherBox, &transform);

	Vector3xType otherBoxCorners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3x_corners(otherBoxCorners, &otherBox));

	OrientedBox3xType addPointsBox = box;
	dsOrientedBox3x_addBox(&box, &otherBox);

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		dsOrientedBox3x_addPoint(&addPointsBox, otherBoxCorners + i);

	EXPECT_NEAR(addPointsBox.center.x, box.center.x, epsilon);
	EXPECT_NEAR(addPointsBox.center.y, box.center.y, epsilon);
	EXPECT_NEAR(addPointsBox.center.z, box.center.z, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.x, box.halfExtents.x, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.y, box.halfExtents.y, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.z, box.halfExtents.z, epsilon);
}

TYPED_TEST(OrientedBox3xTest, Intersects)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	OrientedBox3xType otherBox =
	{
		{{ {1, 0, 0, 10}, {0, 1, 0, 11}, {0, 0, 1, 12} }},
		{{1, 2, 3, 13}}, {{4, 5, 6, 14}}
	};

	Matrix44Type rotate;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsOrientedBox3x_transform(&otherBox, &rotate);

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	// Pass (axes)
	otherBox.center.x = 1;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 11;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 0;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 10;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = -1;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = 9;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	// Pass (off-axis)
	otherBox.center.x = 3;
	otherBox.center.y = 2;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 3;
	otherBox.center.y = 2;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 3;
	otherBox.center.y = 8;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 3;
	otherBox.center.y = 8;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 2;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 2;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 8;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 8;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3x_intersects(&box, &otherBox));

	// Fail (axes)
	otherBox.center.x = -6;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 18;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = -7;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 17;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = -8;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = 16;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	// Fail (off-axis)
	otherBox.center.x = -4;
	otherBox.center.y = -5;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = -4;
	otherBox.center.y = -5;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = -4;
	otherBox.center.y = 15;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = -4;
	otherBox.center.y = 15;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = -5;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = -5;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = 15;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = 15;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3x_intersects(&box, &otherBox));
}

TYPED_TEST(OrientedBox3xTest, ContainsPoint)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType point1 = {{5, 6, 3, 10}};
	Vector3xType point2 = {{1, 6, 3, 11}};
	Vector3xType point3 = {{5, 0, 3, 12}};
	Vector3xType point4 = {{5, 6, -1, 13}};
	Vector3xType point5 = {{11, 6, 3, 14}};
	Vector3xType point6 = {{5, 10, 3, 15}};
	Vector3xType point7 = {{5, 6, 9, 16}};

	EXPECT_TRUE(dsOrientedBox3x_containsPoint(&box, &box.center));
	EXPECT_TRUE(dsOrientedBox3x_containsPoint(&box, &point1));
	EXPECT_FALSE(dsOrientedBox3x_containsPoint(&box, &point2));
	EXPECT_FALSE(dsOrientedBox3x_containsPoint(&box, &point3));
	EXPECT_FALSE(dsOrientedBox3x_containsPoint(&box, &point4));
	EXPECT_FALSE(dsOrientedBox3x_containsPoint(&box, &point5));
	EXPECT_FALSE(dsOrientedBox3x_containsPoint(&box, &point6));
	EXPECT_FALSE(dsOrientedBox3x_containsPoint(&box, &point7));
}

TYPED_TEST(OrientedBox3xTest, ClosestPoint)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType point1 = {{5, 6, 3, 10}};
	Vector3xType point2 = {{1, 6, 3, 11}};
	Vector3xType point3 = {{5, 0, 3, 12}};
	Vector3xType point4 = {{5, 6, -1, 13}};
	Vector3xType point5 = {{11, 6, 3, 14}};
	Vector3xType point6 = {{5, 10, 3, 15}};
	Vector3xType point7 = {{5, 6, 9, 16}};

	Vector3xType closest;
	dsOrientedBox3x_closestPoint(&closest, &box, &box.center);
	EXPECT_EQ(box.center.x, closest.x);
	EXPECT_EQ(box.center.y, closest.y);
	EXPECT_EQ(box.center.z, closest.z);

	dsOrientedBox3x_closestPoint(&closest, &box, &point1);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3x_closestPoint(&closest, &box, &point2);
	EXPECT_EQ((TypeParam)4, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3x_closestPoint(&closest, &box, &point3);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)4, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3x_closestPoint(&closest, &box, &point4);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)1, closest.z);

	dsOrientedBox3x_closestPoint(&closest, &box, &point5);
	EXPECT_EQ((TypeParam)8, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3x_closestPoint(&closest, &box, &point6);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3x_closestPoint(&closest, &box, &point7);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)7, closest.z);
}

TYPED_TEST(OrientedBox3xTest, Dist2)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType point1 = {{5, 6, 3, 10}};
	Vector3xType point2 = {{1, 6, 3, 11}};
	Vector3xType point3 = {{5, 0, 3, 12}};
	Vector3xType point4 = {{5, 6, -1, 13}};
	Vector3xType point5 = {{11, 6, 3, 14}};
	Vector3xType point6 = {{5, 10, 3, 15}};
	Vector3xType point7 = {{5, 6, 9, 16}};

	EXPECT_EQ((TypeParam)0, dsOrientedBox3x_dist2(&box, &box.center));
	EXPECT_EQ((TypeParam)0, dsOrientedBox3x_dist2(&box, &point1));
	EXPECT_EQ((TypeParam)9, dsOrientedBox3x_dist2(&box, &point2));
	EXPECT_EQ((TypeParam)16, dsOrientedBox3x_dist2(&box, &point3));
	EXPECT_EQ((TypeParam)4, dsOrientedBox3x_dist2(&box, &point4));
	EXPECT_EQ((TypeParam)9, dsOrientedBox3x_dist2(&box, &point5));
	EXPECT_EQ((TypeParam)16, dsOrientedBox3x_dist2(&box, &point6));
	EXPECT_EQ((TypeParam)4, dsOrientedBox3x_dist2(&box, &point7));
}

TYPED_TEST(OrientedBox3xTest, Dist)
{
	typedef typename OrientedBox3xTypeSelector<TypeParam>::OrientedBox3xType OrientedBox3xType;
	typedef typename OrientedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	OrientedBox3xType box =
	{
		{{ {0, 0, 1, 5}, {-1, 0, 0, 6}, {0, 1, 0, 7} }},
		{{6, 5, 4, 8}}, {{3, 2, 1, 9}}
	};

	Vector3xType point1 = {{5, 6, 3, 10}};
	Vector3xType point2 = {{1, 6, 3, 11}};
	Vector3xType point3 = {{5, 0, 3, 12}};
	Vector3xType point4 = {{5, 6, -1, 13}};
	Vector3xType point5 = {{11, 6, 3, 14}};
	Vector3xType point6 = {{5, 10, 3, 15}};
	Vector3xType point7 = {{5, 6, 9, 16}};

	EXPECT_FLOAT_EQ(0.0f, (float)dsOrientedBox3x_dist(&box, &box.center));
	EXPECT_FLOAT_EQ(0.0f, (float)dsOrientedBox3x_dist(&box, &point1));
	EXPECT_FLOAT_EQ(3.0f, (float)dsOrientedBox3x_dist(&box, &point2));
	EXPECT_FLOAT_EQ(4.0f, (float)dsOrientedBox3x_dist(&box, &point3));
	EXPECT_FLOAT_EQ(2.0f, (float)dsOrientedBox3x_dist(&box, &point4));
	EXPECT_FLOAT_EQ(3.0f, (float)dsOrientedBox3x_dist(&box, &point5));
	EXPECT_FLOAT_EQ(4.0f, (float)dsOrientedBox3x_dist(&box, &point6));
	EXPECT_FLOAT_EQ(2.0f, (float)dsOrientedBox3x_dist(&box, &point7));
}

#if DS_HAS_SIMD

TEST(OrientedBox3xfTest, ToMatrixSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = OrientedBox3xTypeSelector<float>::epsilon;

	dsOrientedBox3xf box =
	{
		{{ {0.0f, 0.0f, 1.0f, 5.0f}, {-1.0f, 0.0f, 0.0f, 6.0f}, {0.0f, 1.0f, 0.0f, 7.0f} }},
		{{6.0f, 5.0f, 4.0f, 8.0f}}, {{3.0f, 2.0f, 1.0f, 9.0f}}
	};

	dsVector3xf corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3xf_corners(corners, &box));

	dsMatrix44f matrix;
	dsOrientedBox3xf_toMatrixSIMD(&matrix, &box);

	dsVector4f lowerLeft = {{-1.0f, -1.0f, -1.0f, 1.0f}};
	dsVector4f boxPoint;
	dsMatrix44_transform(boxPoint, matrix, lowerLeft);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].z, boxPoint.z, epsilon);

	dsVector4f upperRight = {{1.0f, 1.0f, 1.0f, 1.0f}};
	dsMatrix44_transform(boxPoint, matrix, upperRight);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].z, boxPoint.z, epsilon);

	dsOrientedBox3xf restoredBox;
	dsOrientedBox3xf_fromMatrixSIMD(&restoredBox, &matrix);
	EXPECT_NEAR(box.orientation.values[0][0], restoredBox.orientation.values[0][0], epsilon);
	EXPECT_NEAR(box.orientation.values[0][1], restoredBox.orientation.values[0][1], epsilon);
	EXPECT_NEAR(box.orientation.values[0][2], restoredBox.orientation.values[0][2], epsilon);
	EXPECT_NEAR(box.orientation.values[1][0], restoredBox.orientation.values[1][0], epsilon);
	EXPECT_NEAR(box.orientation.values[1][1], restoredBox.orientation.values[1][1], epsilon);
	EXPECT_NEAR(box.orientation.values[1][2], restoredBox.orientation.values[1][2], epsilon);
	EXPECT_NEAR(box.orientation.values[2][0], restoredBox.orientation.values[2][0], epsilon);
	EXPECT_NEAR(box.orientation.values[2][1], restoredBox.orientation.values[2][1], epsilon);
	EXPECT_NEAR(box.orientation.values[2][2], restoredBox.orientation.values[2][2], epsilon);
	EXPECT_NEAR(box.center.x, restoredBox.center.x, epsilon);
	EXPECT_NEAR(box.center.y, restoredBox.center.y, epsilon);
	EXPECT_NEAR(box.center.z, restoredBox.center.z, epsilon);
	EXPECT_NEAR(box.halfExtents.x, restoredBox.halfExtents.x, epsilon);
	EXPECT_NEAR(box.halfExtents.y, restoredBox.halfExtents.y, epsilon);
	EXPECT_NEAR(box.halfExtents.z, restoredBox.halfExtents.z, epsilon);
}

TEST(OrientedBox3xfTest, ToMatrixTransposeSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsOrientedBox3xf box =
	{
		{{ {0.0f, 0.0f, 1.0f, 5.0f}, {-1.0f, 0.0f, 0.0f, 6.0f}, {0.0f, 1.0f, 0.0f, 7.0f} }},
		{{6.0f, 5.0f, 4.0f, 8.0f}}, {{3.0f, 2.0f, 1.0f, 9.0f}}
	};

	dsMatrix44f matrix, transposedMatrix;
	dsOrientedBox3xf_toMatrixSIMD(&matrix, &box);
	dsOrientedBox3xf_toMatrixTransposeSIMD(&transposedMatrix, &box);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			EXPECT_EQ(matrix.values[j][i], transposedMatrix.values[i][j]);
	}
}

#if !DS_DETERMINISTIC_MATH
TEST(OrientedBox3xfTest, FromMatrixFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = OrientedBox3xTypeSelector<float>::epsilon;

	dsOrientedBox3xf box =
	{
		{{ {0.0f, 0.0f, 1.0f, 5.0f}, {-1.0f, 0.0f, 0.0f, 6.0f}, {0.0f, 1.0f, 0.0f, 7.0f} }},
		{{6.0f, 5.0f, 4.0f, 8.0f}}, {{3.0f, 2.0f, 1.0f, 9.0f}}
	};

	dsMatrix44f matrix;
	dsOrientedBox3xf_toMatrixSIMD(&matrix, &box);

	dsOrientedBox3xf restoredBox;
	dsOrientedBox3xf_fromMatrixFMA(&restoredBox, &matrix);
	EXPECT_NEAR(box.orientation.values[0][0], restoredBox.orientation.values[0][0], epsilon);
	EXPECT_NEAR(box.orientation.values[0][1], restoredBox.orientation.values[0][1], epsilon);
	EXPECT_NEAR(box.orientation.values[0][2], restoredBox.orientation.values[0][2], epsilon);
	EXPECT_NEAR(box.orientation.values[1][0], restoredBox.orientation.values[1][0], epsilon);
	EXPECT_NEAR(box.orientation.values[1][1], restoredBox.orientation.values[1][1], epsilon);
	EXPECT_NEAR(box.orientation.values[1][2], restoredBox.orientation.values[1][2], epsilon);
	EXPECT_NEAR(box.orientation.values[2][0], restoredBox.orientation.values[2][0], epsilon);
	EXPECT_NEAR(box.orientation.values[2][1], restoredBox.orientation.values[2][1], epsilon);
	EXPECT_NEAR(box.orientation.values[2][2], restoredBox.orientation.values[2][2], epsilon);
	EXPECT_NEAR(box.center.x, restoredBox.center.x, epsilon);
	EXPECT_NEAR(box.center.y, restoredBox.center.y, epsilon);
	EXPECT_NEAR(box.center.z, restoredBox.center.z, epsilon);
	EXPECT_NEAR(box.halfExtents.x, restoredBox.halfExtents.x, epsilon);
	EXPECT_NEAR(box.halfExtents.y, restoredBox.halfExtents.y, epsilon);
	EXPECT_NEAR(box.halfExtents.z, restoredBox.halfExtents.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(OrientedBox3xdTest, ToMatrixSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = OrientedBox3xTypeSelector<double>::epsilon;

	dsOrientedBox3xd box =
	{
		{{ {0.0, 0.0, 1.0, 5.0}, {-1.0, 0.0, 0.0, 6.0}, {0.0, 1.0, 0.0, 7.0} }},
		{{6.0, 5.0, 4.0, 8.0}}, {{3.0, 2.0, 1.0, 9.0}}
	};

	dsVector3xd corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3xd_corners(corners, &box));

	dsMatrix44d matrix;
	dsOrientedBox3xd_toMatrixSIMD2(&matrix, &box);

	dsVector4d lowerLeft = {{-1.0, -1.0, -1.0, 1.0}};
	dsVector4d boxPoint;
	dsMatrix44_transform(boxPoint, matrix, lowerLeft);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].z, boxPoint.z, epsilon);

	dsVector4d upperRight = {{1.0, 1.0, 1.0, 1.0}};
	dsMatrix44_transform(boxPoint, matrix, upperRight);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].z, boxPoint.z, epsilon);

	dsOrientedBox3xd restoredBox;
	dsOrientedBox3xd_fromMatrixSIMD2(&restoredBox, &matrix);
	EXPECT_NEAR(box.orientation.values[0][0], restoredBox.orientation.values[0][0], epsilon);
	EXPECT_NEAR(box.orientation.values[0][1], restoredBox.orientation.values[0][1], epsilon);
	EXPECT_NEAR(box.orientation.values[0][2], restoredBox.orientation.values[0][2], epsilon);
	EXPECT_NEAR(box.orientation.values[1][0], restoredBox.orientation.values[1][0], epsilon);
	EXPECT_NEAR(box.orientation.values[1][1], restoredBox.orientation.values[1][1], epsilon);
	EXPECT_NEAR(box.orientation.values[1][2], restoredBox.orientation.values[1][2], epsilon);
	EXPECT_NEAR(box.orientation.values[2][0], restoredBox.orientation.values[2][0], epsilon);
	EXPECT_NEAR(box.orientation.values[2][1], restoredBox.orientation.values[2][1], epsilon);
	EXPECT_NEAR(box.orientation.values[2][2], restoredBox.orientation.values[2][2], epsilon);
	EXPECT_NEAR(box.center.x, restoredBox.center.x, epsilon);
	EXPECT_NEAR(box.center.y, restoredBox.center.y, epsilon);
	EXPECT_NEAR(box.center.z, restoredBox.center.z, epsilon);
	EXPECT_NEAR(box.halfExtents.x, restoredBox.halfExtents.x, epsilon);
	EXPECT_NEAR(box.halfExtents.y, restoredBox.halfExtents.y, epsilon);
	EXPECT_NEAR(box.halfExtents.z, restoredBox.halfExtents.z, epsilon);
}

TEST(OrientedBox3xdTest, ToMatrixTransposeSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	dsOrientedBox3xd box =
	{
		{{ {0.0, 0.0, 1.0, 5.0}, {-1.0, 0.0, 0.0, 6.0}, {0.0, 1.0, 0.0, 7.0} }},
		{{6.0, 5.0, 4.0, 8.0}}, {{3.0, 2.0, 1.0, 9.0}}
	};

	dsMatrix44d matrix, transposedMatrix;
	dsOrientedBox3xd_toMatrixSIMD2(&matrix, &box);
	dsOrientedBox3xd_toMatrixTransposeSIMD2(&transposedMatrix, &box);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			EXPECT_EQ(matrix.values[j][i], transposedMatrix.values[i][j]);
	}
}

#if !DS_DETERMINISTIC_MATH
TEST(OrientedBox3xdTest, FromMatrixFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = OrientedBox3xTypeSelector<double>::epsilon;

	dsOrientedBox3xd box =
	{
		{{ {0.0, 0.0, 1.0, 5.0}, {-1.0, 0.0, 0.0, 6.0}, {0.0, 1.0, 0.0, 7.0} }},
		{{6.0, 5.0, 4.0, 8.0}}, {{3.0, 2.0, 1.0, 9.0}}
	};

	dsMatrix44d matrix;
	dsOrientedBox3xd_toMatrixSIMD2(&matrix, &box);

	dsOrientedBox3xd restoredBox;
	dsOrientedBox3xd_fromMatrixFMA2(&restoredBox, &matrix);
	EXPECT_NEAR(box.orientation.values[0][0], restoredBox.orientation.values[0][0], epsilon);
	EXPECT_NEAR(box.orientation.values[0][1], restoredBox.orientation.values[0][1], epsilon);
	EXPECT_NEAR(box.orientation.values[0][2], restoredBox.orientation.values[0][2], epsilon);
	EXPECT_NEAR(box.orientation.values[1][0], restoredBox.orientation.values[1][0], epsilon);
	EXPECT_NEAR(box.orientation.values[1][1], restoredBox.orientation.values[1][1], epsilon);
	EXPECT_NEAR(box.orientation.values[1][2], restoredBox.orientation.values[1][2], epsilon);
	EXPECT_NEAR(box.orientation.values[2][0], restoredBox.orientation.values[2][0], epsilon);
	EXPECT_NEAR(box.orientation.values[2][1], restoredBox.orientation.values[2][1], epsilon);
	EXPECT_NEAR(box.orientation.values[2][2], restoredBox.orientation.values[2][2], epsilon);
	EXPECT_NEAR(box.center.x, restoredBox.center.x, epsilon);
	EXPECT_NEAR(box.center.y, restoredBox.center.y, epsilon);
	EXPECT_NEAR(box.center.z, restoredBox.center.z, epsilon);
	EXPECT_NEAR(box.halfExtents.x, restoredBox.halfExtents.x, epsilon);
	EXPECT_NEAR(box.halfExtents.y, restoredBox.halfExtents.y, epsilon);
	EXPECT_NEAR(box.halfExtents.z, restoredBox.halfExtents.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(OrientedBox3xdTest, ToMatrixSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = OrientedBox3xTypeSelector<double>::epsilon;

	DS_ALIGN(32) dsOrientedBox3xd box =
	{
		{{ {0.0, 0.0, 1.0, 5.0}, {-1.0, 0.0, 0.0, 6.0}, {0.0, 1.0, 0.0, 7.0} }},
		{{6.0, 5.0, 4.0, 8.0}}, {{3.0, 2.0, 1.0, 9.0}}
	};

	dsVector3xd corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3xd_corners(corners, &box));

	DS_ALIGN(32) dsMatrix44d matrix;
	dsOrientedBox3xd_toMatrixSIMD4(&matrix, &box);

	dsVector4d lowerLeft = {{-1.0, -1.0, -1.0, 1.0}};
	dsVector4d boxPoint;
	dsMatrix44_transform(boxPoint, matrix, lowerLeft);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_xyz].z, boxPoint.z, epsilon);

	dsVector4d upperRight = {{1.0, 1.0, 1.0, 1.0}};
	dsMatrix44_transform(boxPoint, matrix, upperRight);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].x, boxPoint.x, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].y, boxPoint.y, epsilon);
	EXPECT_NEAR(corners[dsBox3Corner_XYZ].z, boxPoint.z, epsilon);

	DS_ALIGN(32) dsOrientedBox3xd restoredBox;
	dsOrientedBox3xd_fromMatrixSIMD4(&restoredBox, &matrix);
	EXPECT_NEAR(box.orientation.values[0][0], restoredBox.orientation.values[0][0], epsilon);
	EXPECT_NEAR(box.orientation.values[0][1], restoredBox.orientation.values[0][1], epsilon);
	EXPECT_NEAR(box.orientation.values[0][2], restoredBox.orientation.values[0][2], epsilon);
	EXPECT_NEAR(box.orientation.values[1][0], restoredBox.orientation.values[1][0], epsilon);
	EXPECT_NEAR(box.orientation.values[1][1], restoredBox.orientation.values[1][1], epsilon);
	EXPECT_NEAR(box.orientation.values[1][2], restoredBox.orientation.values[1][2], epsilon);
	EXPECT_NEAR(box.orientation.values[2][0], restoredBox.orientation.values[2][0], epsilon);
	EXPECT_NEAR(box.orientation.values[2][1], restoredBox.orientation.values[2][1], epsilon);
	EXPECT_NEAR(box.orientation.values[2][2], restoredBox.orientation.values[2][2], epsilon);
	EXPECT_NEAR(box.center.x, restoredBox.center.x, epsilon);
	EXPECT_NEAR(box.center.y, restoredBox.center.y, epsilon);
	EXPECT_NEAR(box.center.z, restoredBox.center.z, epsilon);
	EXPECT_NEAR(box.halfExtents.x, restoredBox.halfExtents.x, epsilon);
	EXPECT_NEAR(box.halfExtents.y, restoredBox.halfExtents.y, epsilon);
	EXPECT_NEAR(box.halfExtents.z, restoredBox.halfExtents.z, epsilon);
}

TEST(OrientedBox3xdTest, ToMatrixTransposeSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	DS_ALIGN(32) dsOrientedBox3xd box =
	{
		{{ {0.0, 0.0, 1.0, 5.0}, {-1.0, 0.0, 0.0, 6.0}, {0.0, 1.0, 0.0, 7.0} }},
		{{6.0, 5.0, 4.0, 8.0}}, {{3.0, 2.0, 1.0, 9.0}}
	};

	DS_ALIGN(32) dsMatrix44d matrix, transposedMatrix;
	dsOrientedBox3xd_toMatrixSIMD4(&matrix, &box);
	dsOrientedBox3xd_toMatrixTransposeSIMD4(&transposedMatrix, &box);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			EXPECT_EQ(matrix.values[j][i], transposedMatrix.values[i][j]);
	}
}

#endif // DS_HAS_SIMD

TEST(OrientedBox3x, ConvertFloatToDouble)
{
	dsOrientedBox3xf boxf =
	{
		{{ {1, 0, 0, 7}, {0, 1, 0, 8}, {0, 0, 1, 9} }},
		{{1, 2, 3, 10}}, {{4, 5, 6, 11}}
	};

	dsOrientedBox3xd boxd;
	dsConvertFloatToDouble(boxd, boxf);

	EXPECT_FLOAT_EQ(boxf.orientation.values[0][0], (float)boxd.orientation.values[0][0]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[0][1], (float)boxd.orientation.values[0][1]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[0][2], (float)boxd.orientation.values[0][2]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[1][0], (float)boxd.orientation.values[1][0]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[1][1], (float)boxd.orientation.values[1][1]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[1][2], (float)boxd.orientation.values[1][2]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[2][0], (float)boxd.orientation.values[2][0]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[2][1], (float)boxd.orientation.values[2][1]);
	EXPECT_FLOAT_EQ(boxf.orientation.values[2][2], (float)boxd.orientation.values[2][2]);

	EXPECT_FLOAT_EQ(boxf.center.x, (float)boxd.center.x);
	EXPECT_FLOAT_EQ(boxf.center.y, (float)boxd.center.y);
	EXPECT_FLOAT_EQ(boxf.center.z, (float)boxd.center.z);

	EXPECT_FLOAT_EQ(boxf.halfExtents.x, (float)boxd.halfExtents.x);
	EXPECT_FLOAT_EQ(boxf.halfExtents.y, (float)boxd.halfExtents.y);
	EXPECT_FLOAT_EQ(boxf.halfExtents.z, (float)boxd.halfExtents.z);
}

TEST(OrientedBox3x, ConvertDoubleToFloat)
{
	dsOrientedBox3xd boxd =
	{
		{{ {1, 0, 0, 7}, {0, 1, 0, 8}, {0, 0, 1, 9} }},
		{{1, 2, 3, 10}}, {{4, 5, 6, 11}}
	};

	dsOrientedBox3xf boxf;
	dsConvertDoubleToFloat(boxf, boxd);

	EXPECT_FLOAT_EQ((float)boxd.orientation.values[0][0], boxf.orientation.values[0][0]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[0][1], boxf.orientation.values[0][1]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[0][2], boxf.orientation.values[0][2]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[1][0], boxf.orientation.values[1][0]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[1][1], boxf.orientation.values[1][1]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[1][2], boxf.orientation.values[1][2]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[2][0], boxf.orientation.values[2][0]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[2][1], boxf.orientation.values[2][1]);
	EXPECT_FLOAT_EQ((float)boxd.orientation.values[2][2], boxf.orientation.values[2][2]);

	EXPECT_FLOAT_EQ((float)boxd.center.x, boxf.center.x);
	EXPECT_FLOAT_EQ((float)boxd.center.y, boxf.center.y);
	EXPECT_FLOAT_EQ((float)boxd.center.z, boxf.center.z);

	EXPECT_FLOAT_EQ((float)boxd.halfExtents.x, boxf.halfExtents.x);
	EXPECT_FLOAT_EQ((float)boxd.halfExtents.y, boxf.halfExtents.y);
	EXPECT_FLOAT_EQ((float)boxd.halfExtents.z, boxf.halfExtents.z);
}
