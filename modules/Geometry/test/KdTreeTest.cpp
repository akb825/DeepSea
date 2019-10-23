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

#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Geometry/KdTree.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector3.h>
#include <gtest/gtest.h>
#include <cmath>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

namespace
{

template <int N, typename T>
struct KdTreeParamSelector
{
	static uint8_t axisCount;
	static dsGeometryElement element;
};

template <>
uint8_t KdTreeParamSelector<2, float>::axisCount = 2;
template <>
dsGeometryElement KdTreeParamSelector<2, float>::element = dsGeometryElement_Float;

template <>
uint8_t KdTreeParamSelector<2, double>::axisCount = 2;
template <>
dsGeometryElement KdTreeParamSelector<2, double>::element = dsGeometryElement_Double;

template <>
uint8_t KdTreeParamSelector<2, int>::axisCount = 2;
template <>
dsGeometryElement KdTreeParamSelector<2, int>::element = dsGeometryElement_Int;

template <>
uint8_t KdTreeParamSelector<3, float>::axisCount = 3;
template <>
dsGeometryElement KdTreeParamSelector<3, float>::element = dsGeometryElement_Float;

template <>
uint8_t KdTreeParamSelector<3, double>::axisCount = 3;
template <>
dsGeometryElement KdTreeParamSelector<3, double>::element = dsGeometryElement_Double;

template <>
uint8_t KdTreeParamSelector<3, int>::axisCount = 3;
template <>
dsGeometryElement KdTreeParamSelector<3, int>::element = dsGeometryElement_Int;

template <int N, typename T>
struct KdTreeSelector {};

template <>
struct KdTreeSelector<2, float> : public KdTreeParamSelector<2, float>
{
	typedef dsVector2f VectorType;

	static VectorType createPoint(int x, int y, int)
	{
		VectorType point = {{(float)x, (float)y}};
		return point;
	}
};

template <>
struct KdTreeSelector<2, double> : public KdTreeParamSelector<2, double>
{
	typedef dsVector2d VectorType;

	static VectorType createPoint(int x, int y, int)
	{
		VectorType point = {{(double)x, (double)y}};
		return point;
	}
};

template <>
struct KdTreeSelector<2, int> : public KdTreeParamSelector<2, int>
{
	typedef dsVector2i VectorType;

	static VectorType createPoint(int x, int y, int)
	{
		VectorType point = {{x, y}};
		return point;
	}
};

template <>
struct KdTreeSelector<3, float> : public KdTreeParamSelector<3, float>
{
	typedef dsVector3f VectorType;

	static VectorType createPoint(int x, int y, int z)
	{
		VectorType point = {{(float)x, (float)y, (float)z}};
		return point;
	}
};

template <>
struct KdTreeSelector<3, double> : public KdTreeParamSelector<3, double>
{
	typedef dsVector3d VectorType;

	static VectorType createPoint(int x, int y, int z)
	{
		VectorType point = {{(double)x, (double)y, (double)z}};
		return point;
	}
};

template <>
struct KdTreeSelector<3, int> : public KdTreeParamSelector<3, int>
{
	typedef dsVector3i VectorType;

	static VectorType createPoint(int x, int y, int z)
	{
		VectorType point = {{x, y, z}};
		return point;
	}
};

} // namespace

template <typename SelectorT>
class KdTreeTest : public testing::Test
{
public:
	using VectorType = typename SelectorT::VectorType;

	struct TestObject
	{
		VectorType point;
		int data;
	};

	void SetUp() override
	{
		ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
	}

	void TearDown() override
	{
		EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	}

	static uint8_t axisCount()
	{
		return SelectorT::axisCount;
	}

	static dsGeometryElement element()
	{
		return SelectorT::element;
	}

	static VectorType createPoint(int x, int y, int z)
	{
		return SelectorT::createPoint(x, y, z);
	}

	static bool getPoint(void* outPoint, const dsKdTree* kdTree, const void* object)
	{
		if (dsKdTree_getAxisCount(kdTree) != axisCount() ||
			dsKdTree_getElement(kdTree) != element())
		{
			return false;
		}

		*((VectorType*)outPoint) = ((TestObject*)object)->point;
		return true;
	}

	static bool getPointIndex(void* outPoint, const dsKdTree* kdTree, const void* object)
	{
		if (dsKdTree_getAxisCount(kdTree) != axisCount() ||
			dsKdTree_getElement(kdTree) != element())
		{
			return false;
		}

		auto objects = (const TestObject*)dsKdTree_getUserData(kdTree);
		*((VectorType*)outPoint) = objects[(size_t)object].point;
		return true;
	}

	template <typename T>
	static unsigned int lambdaAdapterImpl(void* userData, const dsKdTree*, const void* object,
		const void* point, uint8_t axis)
	{
		const auto& objectRef = *(const TestObject*)object;
		EXPECT_TRUE(dsVector2_equal(objectRef.point, *(const VectorType*)point));
		return (*(T*)userData)(objectRef, axis);
	}

	template <typename T>
	static dsKdTreeTraverseFunction lambdaAdapter(const T&)
	{
		return &lambdaAdapterImpl<T>;
	}

	template <typename T>
	static unsigned int indexLambdaAdapterImpl(void* userData, const dsKdTree* kdTree,
		const void* object, const void* point, uint8_t axis)
	{
		auto objects = (const TestObject*)dsKdTree_getUserData(kdTree);
		const auto& objectRef = objects[(size_t)object];
		EXPECT_TRUE(dsVector2_equal(objectRef.point, *(const VectorType*)point));
		return (*(T*)userData)(objectRef, axis);
	}

	template <typename T>
	static dsKdTreeTraverseFunction indexLambdaAdapter(const T&)
	{
		return &indexLambdaAdapterImpl<T>;
	}

	static uint32_t countElements(const dsKdTree* kdTree)
	{
		uint32_t elementCount = 0;
		auto traverseAllFunc = [&elementCount](const TestObject&, uint8_t)
		{
			++elementCount;
			return dsKdTreeSide_Both;
		};
		EXPECT_TRUE(dsKdTree_traverse(kdTree, lambdaAdapter(traverseAllFunc), &traverseAllFunc));
		return elementCount;
	}

	static uint32_t indexCountElements(const dsKdTree* kdTree)
	{
		uint32_t elementCount = 0;
		auto traverseAllFunc = [&elementCount](const TestObject&, uint8_t)
		{
			++elementCount;
			return dsKdTreeSide_Both;
		};
		EXPECT_TRUE(dsKdTree_traverse(kdTree, indexLambdaAdapter(traverseAllFunc),
			&traverseAllFunc));
		return elementCount;
	}

	static void findObject(const dsKdTree* kdTree, const TestObject& targetObject)
	{
		uint32_t foundCount = 0;
		auto findFunc = [&foundCount, &targetObject](const TestObject& object, uint8_t axis)
		{
			if (dsVector2_equal(targetObject.point, object.point))
			{
				++foundCount;
				EXPECT_EQ(targetObject.data, object.data);
				return dsKdTreeSide_None;
			}

			if (targetObject.point.values[axis] < object.point.values[axis])
				return dsKdTreeSide_Left;
			else if (targetObject.point.values[axis] > object.point.values[axis])
				return dsKdTreeSide_Right;
			return dsKdTreeSide_Both;
		};

		EXPECT_TRUE(dsKdTree_traverse(kdTree, lambdaAdapter(findFunc), &findFunc));
		EXPECT_EQ(1U, foundCount);
	}

	static void indexFindObject(const dsKdTree* kdTree, const TestObject& targetObject)
	{
		uint32_t foundCount = 0;
		auto findFunc = [&foundCount, &targetObject](const TestObject& object, uint8_t axis)
		{
			if (dsVector2_equal(targetObject.point, object.point))
			{
				++foundCount;
				EXPECT_EQ(targetObject.data, object.data);
				return dsKdTreeSide_None;
			}

			if (targetObject.point.values[axis] < object.point.values[axis])
				return dsKdTreeSide_Left;
			else if (targetObject.point.values[axis] > object.point.values[axis])
				return dsKdTreeSide_Right;
			return dsKdTreeSide_Both;
		};

		EXPECT_TRUE(dsKdTree_traverse(kdTree, indexLambdaAdapter(findFunc), &findFunc));
		EXPECT_EQ(1U, foundCount);
	}

	dsSystemAllocator allocator;
};

using KdTreeTypes = testing::Types<KdTreeSelector<2, float>, KdTreeSelector<2, double>,
	KdTreeSelector<2, int>, KdTreeSelector<3, float>, KdTreeSelector<3, double>,
	KdTreeSelector<3, int>>;
TYPED_TEST_SUITE(KdTreeTest, KdTreeTypes);

TYPED_TEST(KdTreeTest, Create)
{
	TestFixture* fixture = this;
	dsKdTree* kdTree = dsKdTree_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), fixture);
	ASSERT_TRUE(kdTree);
	EXPECT_EQ(TestFixture::axisCount(), dsKdTree_getAxisCount(kdTree));
	EXPECT_EQ(TestFixture::element(), dsKdTree_getElement(kdTree));
	EXPECT_EQ(fixture, dsKdTree_getUserData(kdTree));
	dsKdTree_destroy(kdTree);
}

TYPED_TEST(KdTreeTest, BuildAndTraverse)
{
	using TestObject = typename TestFixture::TestObject;

	TestFixture* fixture = this;
	dsKdTree* kdTree = dsKdTree_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(kdTree);

	TestObject data[] =
	{
		{TestFixture::createPoint(-2, -2, -2), 0},
		{TestFixture::createPoint(1, -2, 3), 1},
		{TestFixture::createPoint(-1, 2, -3), 2},
		{TestFixture::createPoint(1, 3, 3), 3},
		{TestFixture::createPoint(-1, -2, 3), 4},
		{TestFixture::createPoint(1, -3, -3), 5},
		{TestFixture::createPoint(1, 2, -3), 6},
		{TestFixture::createPoint(3, -2, 1), 7},
		{TestFixture::createPoint(-3, 2, -1), 8},
		{TestFixture::createPoint(2, -3, 1), 9},
		{TestFixture::createPoint(-2, 3, -1), 10}
	};

	for (uint32_t i = 0; i <= DS_ARRAY_SIZE(data); ++i)
	{
		EXPECT_TRUE(dsKdTree_build(kdTree, data, i, sizeof(TestObject),
			&TestFixture::getPoint));
		EXPECT_EQ(i, TestFixture::countElements(kdTree));

		for (uint32_t j = 0; j < i; ++j)
			TestFixture::findObject(kdTree, data[j]);
	}

	dsKdTree_destroy(kdTree);
}

TYPED_TEST(KdTreeTest, ObjectPointer)
{
	using TestObject = typename TestFixture::TestObject;

	TestFixture* fixture = this;
	dsKdTree* kdTree = dsKdTree_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(kdTree);

	TestObject* data[] =
	{
		new TestObject{TestFixture::createPoint(-2, -2, -2), 0},
		new TestObject{TestFixture::createPoint(1, -2, 3), 1},
		new TestObject{TestFixture::createPoint(-1, 2, -3), 2},
		new TestObject{TestFixture::createPoint(1, 3, 3), 3},
		new TestObject{TestFixture::createPoint(-1, -2, 3), 4},
		new TestObject{TestFixture::createPoint(1, -3, -3), 5},
		new TestObject{TestFixture::createPoint(1, 2, -3), 6},
		new TestObject{TestFixture::createPoint(3, -2, 1), 7},
		new TestObject{TestFixture::createPoint(-3, 2, -1), 8},
		new TestObject{TestFixture::createPoint(2, -3, 1), 9},
		new TestObject{TestFixture::createPoint(-2, 3, -1), 10}
	};

	EXPECT_TRUE(dsKdTree_build(kdTree, data, DS_ARRAY_SIZE(data), DS_GEOMETRY_OBJECT_POINTERS,
		&TestFixture::getPoint));
	EXPECT_EQ(DS_ARRAY_SIZE(data), TestFixture::countElements(kdTree));

	for (uint32_t i = 0; i < DS_ARRAY_SIZE(data); ++i)
		TestFixture::findObject(kdTree, *data[i]);

	dsKdTree_destroy(kdTree);

	for (TestObject* object : data)
		delete object;
}

TYPED_TEST(KdTreeTest, ObjectIndices)
{
	using TestObject = typename TestFixture::TestObject;

	TestObject data[] =
	{
		{TestFixture::createPoint(-2, -2, -2), 0},
		{TestFixture::createPoint(1, -2, 3), 1},
		{TestFixture::createPoint(-1, 2, -3), 2},
		{TestFixture::createPoint(1, 3, 3), 3},
		{TestFixture::createPoint(-1, -2, 3), 4},
		{TestFixture::createPoint(1, -3, -3), 5},
		{TestFixture::createPoint(1, 2, -3), 6},
		{TestFixture::createPoint(3, -2, 1), 7},
		{TestFixture::createPoint(-3, 2, -1), 8},
		{TestFixture::createPoint(2, -3, 1), 9},
		{TestFixture::createPoint(-2, 3, -1), 10}
	};

	TestFixture* fixture = this;
	dsKdTree* kdTree = dsKdTree_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), data);
	ASSERT_TRUE(kdTree);

	EXPECT_TRUE(dsKdTree_build(kdTree, NULL, DS_ARRAY_SIZE(data), DS_GEOMETRY_OBJECT_INDICES,
		&TestFixture::getPointIndex));
	EXPECT_EQ(DS_ARRAY_SIZE(data), TestFixture::indexCountElements(kdTree));

	for (uint32_t i = 0; i < DS_ARRAY_SIZE(data); ++i)
		TestFixture::indexFindObject(kdTree, data[i]);

	dsKdTree_destroy(kdTree);
}
