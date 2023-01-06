/*
 * Copyright 2016-2023 Aaron Barany
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

#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Math/Matrix44.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct OrientedBox3TypeSelector;

template <>
struct OrientedBox3TypeSelector<float>
{
	typedef dsVector3f Vector3Type;
	typedef dsVector4f Vector4Type;
	typedef dsAlignedBox3f AlignedBox3Type;
	typedef dsMatrix44f Matrix44Type;
	typedef dsOrientedBox3f OrientedBox3Type;
	static const float epsilon;
};

template <>
struct OrientedBox3TypeSelector<double>
{
	typedef dsVector3d Vector3Type;
	typedef dsVector4d Vector4Type;
	typedef dsAlignedBox3d AlignedBox3Type;
	typedef dsMatrix44d Matrix44Type;
	typedef dsOrientedBox3d OrientedBox3Type;
	static const double epsilon;
};

const float OrientedBox3TypeSelector<float>::epsilon = 1e-4f;
const double OrientedBox3TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class OrientedBox3Test : public testing::Test
{
};

using OrientedBox3Types = testing::Types<float, double>;
TYPED_TEST_SUITE(OrientedBox3Test, OrientedBox3Types);

inline void dsOrientedBox3_fromMatrix(dsOrientedBox3f* result, const dsMatrix44f* matrix)
{
	return dsOrientedBox3f_fromMatrix(result, matrix);
}

inline void dsOrientedBox3_fromMatrix(dsOrientedBox3d* result, const dsMatrix44d* matrix)
{
	return dsOrientedBox3d_fromMatrix(result, matrix);
}

inline bool dsOrientedBox3_transform(dsOrientedBox3f* box, const dsMatrix44f* transform)
{
	return dsOrientedBox3f_transform(box, transform);
}

inline bool dsOrientedBox3_transform(dsOrientedBox3d* box, const dsMatrix44d* transform)
{
	return dsOrientedBox3d_transform(box, transform);
}

inline void dsOrientedBox3_addPoint(dsOrientedBox3f* box, const dsVector3f* point)
{
	dsOrientedBox3f_addPoint(box, point);
}

inline void dsOrientedBox3_addPoint(dsOrientedBox3d* box, const dsVector3d* point)
{
	dsOrientedBox3d_addPoint(box, point);
}

inline bool dsOrientedBox3_addBox(dsOrientedBox3f* box, const dsOrientedBox3f* otherBox)
{
	return dsOrientedBox3f_addBox(box, otherBox);
}

inline bool dsOrientedBox3_addBox(dsOrientedBox3d* box, const dsOrientedBox3d* otherBox)
{
	return dsOrientedBox3d_addBox(box, otherBox);
}

inline bool dsOrientedBox3_corners(dsVector3f* corners, const dsOrientedBox3f* box)
{
	return dsOrientedBox3f_corners(corners, box);
}

inline bool dsOrientedBox3_corners(dsVector3d* corners, const dsOrientedBox3d* box)
{
	return dsOrientedBox3d_corners(corners, box);
}

inline bool dsOrientedBox3_intersects(const dsOrientedBox3f* box, const dsOrientedBox3f* otherBox)
{
	return dsOrientedBox3f_intersects(box, otherBox);
}

inline bool dsOrientedBox3_intersects(const dsOrientedBox3d* box, const dsOrientedBox3d* otherBox)
{
	return dsOrientedBox3d_intersects(box, otherBox);
}

inline bool dsOrientedBox3_containsPoint(const dsOrientedBox3f* box, const dsVector3f* point)
{
	return dsOrientedBox3f_containsPoint(box, point);
}

inline bool dsOrientedBox3_containsPoint(const dsOrientedBox3d* box, const dsVector3d* point)
{
	return dsOrientedBox3d_containsPoint(box, point);
}

inline bool dsOrientedBox3_closestPoint(dsVector3f* result, const dsOrientedBox3f* box,
	const dsVector3f* point)
{
	return dsOrientedBox3f_closestPoint(result, box, point);
}

inline bool dsOrientedBox3_closestPoint(dsVector3d* result, const dsOrientedBox3d* box,
	const dsVector3d* point)
{
	return dsOrientedBox3d_closestPoint(result, box, point);
}

inline float dsOrientedBox3_dist2(const dsOrientedBox3f* box, const dsVector3f* otherBox)
{
	return dsOrientedBox3f_dist2(box, otherBox);
}

inline double dsOrientedBox3_dist2(const dsOrientedBox3d* box, const dsVector3d* otherBox)
{
	return dsOrientedBox3d_dist2(box, otherBox);
}

inline float dsOrientedBox3_dist(const dsOrientedBox3f* box, const dsVector3f* otherBox)
{
	return dsOrientedBox3f_dist(box, otherBox);
}

inline double dsOrientedBox3_dist(const dsOrientedBox3d* box, const dsVector3d* otherBox)
{
	return dsOrientedBox3d_dist(box, otherBox);
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

TYPED_TEST(OrientedBox3Test, Initialize)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;

	OrientedBox3Type box =
	{
		{{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }},
		{{1, 2, 3}}, {{4, 5, 6}}
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

TYPED_TEST(OrientedBox3Test, IsValid)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;

	OrientedBox3Type box =
	{
		{{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }},
		{{1, 2, 3}}, {{4, 5, 6}}
	};

	EXPECT_TRUE(dsOrientedBox3_isValid(box));

	box.halfExtents.x = -1;
	EXPECT_FALSE(dsOrientedBox3_isValid(box));

	box.halfExtents.x = 4;
	box.halfExtents.y = -1;
	EXPECT_FALSE(dsOrientedBox3_isValid(box));

	box.halfExtents.y = 5;
	box.halfExtents.z = -1;
	EXPECT_FALSE(dsOrientedBox3_isValid(box));
}

TYPED_TEST(OrientedBox3Test, FromAlignedBox)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	AlignedBox3Type alignedBox = {{{0, 1, 2}}, {{4, 7, 10}}};

	dsOrientedBox3_fromAlignedBox(box, alignedBox);
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

TYPED_TEST(OrientedBox3Test, MakeInvalid)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;

	OrientedBox3Type box =
	{
		{{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }},
		{{1, 2, 3}}, {{4, 5, 6}}
	};

	EXPECT_TRUE(dsOrientedBox3_isValid(box));

	dsOrientedBox3_makeInvalid(box);
	EXPECT_FALSE(dsOrientedBox3_isValid(box));
}

TYPED_TEST(OrientedBox3Test, AddPoint)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type point1 = {{5, 6, 3}};
	Vector3Type point2 = {{1, 6, 3}};
	Vector3Type point3 = {{5, 0, 3}};
	Vector3Type point4 = {{5, 6, -1}};
	Vector3Type point5 = {{9, 6, 3}};
	Vector3Type point6 = {{5, 10, 3}};
	Vector3Type point7 = {{5, 6, 11}};

	dsOrientedBox3_addPoint(&box, &point1);
	EXPECT_EQ((TypeParam)6, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)4, box.center.z);
	EXPECT_EQ((TypeParam)3, box.halfExtents.x);
	EXPECT_EQ((TypeParam)2, box.halfExtents.y);
	EXPECT_EQ((TypeParam)1, box.halfExtents.z);

	dsOrientedBox3_addPoint(&box, &point2);
	EXPECT_EQ((TypeParam)4.5, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)4, box.center.z);
	EXPECT_EQ((TypeParam)3, box.halfExtents.x);
	EXPECT_EQ((TypeParam)3.5, box.halfExtents.y);
	EXPECT_EQ((TypeParam)1, box.halfExtents.z);

	dsOrientedBox3_addPoint(&box, &point3);
	EXPECT_EQ((TypeParam)4.5, box.center.x);
	EXPECT_EQ((TypeParam)3, box.center.y);
	EXPECT_EQ((TypeParam)4, box.center.z);
	EXPECT_EQ((TypeParam)3, box.halfExtents.x);
	EXPECT_EQ((TypeParam)3.5, box.halfExtents.y);
	EXPECT_EQ((TypeParam)3, box.halfExtents.z);

	dsOrientedBox3_addPoint(&box, &point4);
	EXPECT_EQ((TypeParam)4.5, box.center.x);
	EXPECT_EQ((TypeParam)3, box.center.y);
	EXPECT_EQ((TypeParam)3, box.center.z);
	EXPECT_EQ((TypeParam)4, box.halfExtents.x);
	EXPECT_EQ((TypeParam)3.5, box.halfExtents.y);
	EXPECT_EQ((TypeParam)3, box.halfExtents.z);

	dsOrientedBox3_addPoint(&box, &point5);
	EXPECT_EQ((TypeParam)5, box.center.x);
	EXPECT_EQ((TypeParam)3, box.center.y);
	EXPECT_EQ((TypeParam)3, box.center.z);
	EXPECT_EQ((TypeParam)4, box.halfExtents.x);
	EXPECT_EQ((TypeParam)4, box.halfExtents.y);
	EXPECT_EQ((TypeParam)3, box.halfExtents.z);

	dsOrientedBox3_addPoint(&box, &point6);
	EXPECT_EQ((TypeParam)5, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)3, box.center.z);
	EXPECT_EQ((TypeParam)4, box.halfExtents.x);
	EXPECT_EQ((TypeParam)4, box.halfExtents.y);
	EXPECT_EQ((TypeParam)5, box.halfExtents.z);

	dsOrientedBox3_addPoint(&box, &point7);
	EXPECT_EQ((TypeParam)5, box.center.x);
	EXPECT_EQ((TypeParam)5, box.center.y);
	EXPECT_EQ((TypeParam)5, box.center.z);
	EXPECT_EQ((TypeParam)6, box.halfExtents.x);
	EXPECT_EQ((TypeParam)4, box.halfExtents.y);
	EXPECT_EQ((TypeParam)5, box.halfExtents.z);
}

TYPED_TEST(OrientedBox3Test, Corners)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = OrientedBox3TypeSelector<TypeParam>::epsilon;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3_corners(corners, &box));

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

TYPED_TEST(OrientedBox3Test, ToMatrix)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector4Type Vector4Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = OrientedBox3TypeSelector<TypeParam>::epsilon;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3_corners(corners, &box));

	Matrix44Type matrix;
	dsOrientedBox3_toMatrix(matrix, box);

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

	OrientedBox3Type restoredBox;
	dsOrientedBox3_fromMatrix(&restoredBox, &matrix);
	EXPECT_NEAR(restoredBox.orientation.values[0][0], box.orientation.values[0][0], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[0][1], box.orientation.values[0][1], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[0][2], box.orientation.values[0][2], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[1][0], box.orientation.values[1][0], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[1][1], box.orientation.values[1][1], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[1][2], box.orientation.values[1][2], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[2][0], box.orientation.values[2][0], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[2][1], box.orientation.values[2][1], epsilon);
	EXPECT_NEAR(restoredBox.orientation.values[2][2], box.orientation.values[2][2], epsilon);
	EXPECT_NEAR(restoredBox.center.x, box.center.x, epsilon);
	EXPECT_NEAR(restoredBox.center.y, box.center.y, epsilon);
	EXPECT_NEAR(restoredBox.center.z, box.center.z, epsilon);
	EXPECT_NEAR(restoredBox.halfExtents.x, box.halfExtents.x, epsilon);
	EXPECT_NEAR(restoredBox.halfExtents.y, box.halfExtents.y, epsilon);
	EXPECT_NEAR(restoredBox.halfExtents.z, box.halfExtents.z, epsilon);
}

TYPED_TEST(OrientedBox3Test, Transform)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector4Type Vector4Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = OrientedBox3TypeSelector<TypeParam>::epsilon;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3_corners(corners, &box));

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

	EXPECT_TRUE(dsOrientedBox3_transform(&box, &transform));

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

	Vector3Type transformedCorners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3_corners(transformedCorners, &box));

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

TYPED_TEST(OrientedBox3Test, TransformIncremental)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector4Type Vector4Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = OrientedBox3TypeSelector<TypeParam>::epsilon;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type corners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3_corners(corners, &box));

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

	EXPECT_TRUE(dsOrientedBox3_transform(&box, &scale));
	EXPECT_TRUE(dsOrientedBox3_transform(&box, &rotate));
	EXPECT_TRUE(dsOrientedBox3_transform(&box, &translate));

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

	Vector3Type transformedCorners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3_corners(transformedCorners, &box));

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

TYPED_TEST(OrientedBox3Test, AddBox)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	TypeParam epsilon = OrientedBox3TypeSelector<TypeParam>::epsilon;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	OrientedBox3Type otherBox =
	{
		{{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }},
		{{1, 2, 3}}, {{4, 5, 6}}
	};

	Matrix44Type rotate, translate, scale, temp, transform;

	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -2, 5, -1);
	dsMatrix44_makeScale(&scale, 7, 8, 6);

	dsMatrix44_mul(temp, rotate, scale);
	dsMatrix44_mul(transform, translate, temp);

	dsOrientedBox3_transform(&otherBox, &transform);

	Vector3Type otherBoxCorners[DS_BOX3_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox3_corners(otherBoxCorners, &otherBox));

	OrientedBox3Type addPointsBox = box;
	dsOrientedBox3_addBox(&box, &otherBox);

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		dsOrientedBox3_addPoint(&addPointsBox, otherBoxCorners + i);

	EXPECT_NEAR(addPointsBox.center.x, box.center.x, epsilon);
	EXPECT_NEAR(addPointsBox.center.y, box.center.y, epsilon);
	EXPECT_NEAR(addPointsBox.center.z, box.center.z, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.x, box.halfExtents.x, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.y, box.halfExtents.y, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.z, box.halfExtents.z, epsilon);
}

TYPED_TEST(OrientedBox3Test, Intersects)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	OrientedBox3Type otherBox =
	{
		{{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }},
		{{1, 2, 3}}, {{4, 5, 6}}
	};

	Matrix44Type rotate;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsOrientedBox3_transform(&otherBox, &rotate);

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	// Pass (axes)
	otherBox.center.x = 1;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 11;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 0;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 10;
	otherBox.center.z = 4;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = -1;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = 9;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	// Pass (off-axis)
	otherBox.center.x = 3;
	otherBox.center.y = 2;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 3;
	otherBox.center.y = 2;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 3;
	otherBox.center.y = 8;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 3;
	otherBox.center.y = 8;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 2;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 2;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 8;
	otherBox.center.z = 1;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 9;
	otherBox.center.y = 8;
	otherBox.center.z = 7;
	EXPECT_TRUE(dsOrientedBox3_intersects(&box, &otherBox));

	// Fail (axes)
	otherBox.center.x = -6;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 18;
	otherBox.center.y = 5;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = -7;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 17;
	otherBox.center.z = 4;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = -8;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 6;
	otherBox.center.y = 5;
	otherBox.center.z = 16;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	// Fail (off-axis)
	otherBox.center.x = -4;
	otherBox.center.y = -5;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = -4;
	otherBox.center.y = -5;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = -4;
	otherBox.center.y = 15;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = -4;
	otherBox.center.y = 15;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = -5;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = -5;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = 15;
	otherBox.center.z = -6;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));

	otherBox.center.x = 16;
	otherBox.center.y = 15;
	otherBox.center.z = 14;
	EXPECT_FALSE(dsOrientedBox3_intersects(&box, &otherBox));
}

TYPED_TEST(OrientedBox3Test, ContainsPoint)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type point1 = {{5, 6, 3}};
	Vector3Type point2 = {{1, 6, 3}};
	Vector3Type point3 = {{5, 0, 3}};
	Vector3Type point4 = {{5, 6, -1}};
	Vector3Type point5 = {{11, 6, 3}};
	Vector3Type point6 = {{5, 10, 3}};
	Vector3Type point7 = {{5, 6, 9}};

	EXPECT_TRUE(dsOrientedBox3_containsPoint(&box, &box.center));
	EXPECT_TRUE(dsOrientedBox3_containsPoint(&box, &point1));
	EXPECT_FALSE(dsOrientedBox3_containsPoint(&box, &point2));
	EXPECT_FALSE(dsOrientedBox3_containsPoint(&box, &point3));
	EXPECT_FALSE(dsOrientedBox3_containsPoint(&box, &point4));
	EXPECT_FALSE(dsOrientedBox3_containsPoint(&box, &point5));
	EXPECT_FALSE(dsOrientedBox3_containsPoint(&box, &point6));
	EXPECT_FALSE(dsOrientedBox3_containsPoint(&box, &point7));
}

TYPED_TEST(OrientedBox3Test, ClosestPoint)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type point1 = {{5, 6, 3}};
	Vector3Type point2 = {{1, 6, 3}};
	Vector3Type point3 = {{5, 0, 3}};
	Vector3Type point4 = {{5, 6, -1}};
	Vector3Type point5 = {{11, 6, 3}};
	Vector3Type point6 = {{5, 10, 3}};
	Vector3Type point7 = {{5, 6, 9}};

	Vector3Type closest;
	dsOrientedBox3_closestPoint(&closest, &box, &box.center);
	EXPECT_EQ(box.center.x, closest.x);
	EXPECT_EQ(box.center.y, closest.y);
	EXPECT_EQ(box.center.z, closest.z);

	dsOrientedBox3_closestPoint(&closest, &box, &point1);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3_closestPoint(&closest, &box, &point2);
	EXPECT_EQ((TypeParam)4, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3_closestPoint(&closest, &box, &point3);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)4, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3_closestPoint(&closest, &box, &point4);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)1, closest.z);

	dsOrientedBox3_closestPoint(&closest, &box, &point5);
	EXPECT_EQ((TypeParam)8, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3_closestPoint(&closest, &box, &point6);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsOrientedBox3_closestPoint(&closest, &box, &point7);
	EXPECT_EQ((TypeParam)5, closest.x);
	EXPECT_EQ((TypeParam)6, closest.y);
	EXPECT_EQ((TypeParam)7, closest.z);
}

TYPED_TEST(OrientedBox3Test, Dist2)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type point1 = {{5, 6, 3}};
	Vector3Type point2 = {{1, 6, 3}};
	Vector3Type point3 = {{5, 0, 3}};
	Vector3Type point4 = {{5, 6, -1}};
	Vector3Type point5 = {{11, 6, 3}};
	Vector3Type point6 = {{5, 10, 3}};
	Vector3Type point7 = {{5, 6, 9}};

	EXPECT_EQ((TypeParam)0, dsOrientedBox3_dist2(&box, &box.center));
	EXPECT_EQ((TypeParam)0, dsOrientedBox3_dist2(&box, &point1));
	EXPECT_EQ((TypeParam)9, dsOrientedBox3_dist2(&box, &point2));
	EXPECT_EQ((TypeParam)16, dsOrientedBox3_dist2(&box, &point3));
	EXPECT_EQ((TypeParam)4, dsOrientedBox3_dist2(&box, &point4));
	EXPECT_EQ((TypeParam)9, dsOrientedBox3_dist2(&box, &point5));
	EXPECT_EQ((TypeParam)16, dsOrientedBox3_dist2(&box, &point6));
	EXPECT_EQ((TypeParam)4, dsOrientedBox3_dist2(&box, &point7));
}

TYPED_TEST(OrientedBox3Test, Dist)
{
	typedef typename OrientedBox3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename OrientedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Vector3Type point1 = {{5, 6, 3}};
	Vector3Type point2 = {{1, 6, 3}};
	Vector3Type point3 = {{5, 0, 3}};
	Vector3Type point4 = {{5, 6, -1}};
	Vector3Type point5 = {{11, 6, 3}};
	Vector3Type point6 = {{5, 10, 3}};
	Vector3Type point7 = {{5, 6, 9}};

	EXPECT_FLOAT_EQ(0.0f, (float)dsOrientedBox3_dist(&box, &box.center));
	EXPECT_FLOAT_EQ(0.0f, (float)dsOrientedBox3_dist(&box, &point1));
	EXPECT_FLOAT_EQ(3.0f, (float)dsOrientedBox3_dist(&box, &point2));
	EXPECT_FLOAT_EQ(4.0f, (float)dsOrientedBox3_dist(&box, &point3));
	EXPECT_FLOAT_EQ(2.0f, (float)dsOrientedBox3_dist(&box, &point4));
	EXPECT_FLOAT_EQ(3.0f, (float)dsOrientedBox3_dist(&box, &point5));
	EXPECT_FLOAT_EQ(4.0f, (float)dsOrientedBox3_dist(&box, &point6));
	EXPECT_FLOAT_EQ(2.0f, (float)dsOrientedBox3_dist(&box, &point7));
}

TEST(OrientedBox3, ConvertFloatToDouble)
{
	dsOrientedBox3f boxf =
	{
		{{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }},
		{{1, 2, 3}}, {{4, 5, 6}}
	};

	dsOrientedBox3d boxd;
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

TEST(OrientedBox3, ConvertDoubleToFloat)
{
	dsOrientedBox3d boxd =
	{
		{{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }},
		{{1, 2, 3}}, {{4, 5, 6}}
	};

	dsOrientedBox3f boxf;
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
