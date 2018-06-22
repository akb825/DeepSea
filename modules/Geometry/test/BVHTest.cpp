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
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/BVH.h>
#include <gtest/gtest.h>
#include <cmath>

namespace
{

template <int N, typename T>
struct BVHParamSelector
{
	static uint8_t axisCount;
	static dsGeometryElement element;
};

template <>
uint8_t BVHParamSelector<2, float>::axisCount = 2;
template <>
dsGeometryElement BVHParamSelector<2, float>::element = dsGeometryElement_Float;

template <>
uint8_t BVHParamSelector<2, double>::axisCount = 2;
template <>
dsGeometryElement BVHParamSelector<2, double>::element = dsGeometryElement_Double;

template <>
uint8_t BVHParamSelector<2, int>::axisCount = 2;
template <>
dsGeometryElement BVHParamSelector<2, int>::element = dsGeometryElement_Int;

template <>
uint8_t BVHParamSelector<3, float>::axisCount = 3;
template <>
dsGeometryElement BVHParamSelector<3, float>::element = dsGeometryElement_Float;

template <>
uint8_t BVHParamSelector<3, double>::axisCount = 3;
template <>
dsGeometryElement BVHParamSelector<3, double>::element = dsGeometryElement_Double;

template <>
uint8_t BVHParamSelector<3, int>::axisCount = 3;
template <>
dsGeometryElement BVHParamSelector<3, int>::element = dsGeometryElement_Int;

template <int N, typename T>
struct BVHSelector {};

template <>
struct BVHSelector<2, float> : public BVHParamSelector<2, float>
{
	typedef dsAlignedBox2f AlignedBoxType;

	static AlignedBoxType createBounds(int minX, int minY, int, int maxX, int maxY, int)
	{
		AlignedBoxType alignedBox = {{{(float)minX, (float)minY}}, {{(float)maxX, (float)maxY}}};
		return alignedBox;
	}
};

template <>
struct BVHSelector<2, double> : public BVHParamSelector<2, double>
{
	typedef dsAlignedBox2d AlignedBoxType;

	static AlignedBoxType createBounds(int minX, int minY, int, int maxX, int maxY, int)
	{
		AlignedBoxType alignedBox = {{{(double)minX, (double)minY}},
			{{(double)maxX, (double)maxY}}};
		return alignedBox;
	}
};

template <>
struct BVHSelector<2, int> : public BVHParamSelector<2, int>
{
	typedef dsAlignedBox2i AlignedBoxType;

	static AlignedBoxType createBounds(int minX, int minY, int, int maxX, int maxY, int)
	{
		AlignedBoxType alignedBox = {{{minX, minY}}, {{maxX, maxY}}};
		return alignedBox;
	}
};

template <>
struct BVHSelector<3, float> : public BVHParamSelector<3, float>
{
	typedef dsAlignedBox3f AlignedBoxType;

	static AlignedBoxType createBounds(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
	{
		AlignedBoxType alignedBox = {{{(float)minX, (float)minY, (float)minZ}},
			{{(float)maxX, (float)maxY, (float)maxZ}}};
		return alignedBox;
	}
};

template <>
struct BVHSelector<3, double> : public BVHParamSelector<3, double>
{
	typedef dsAlignedBox3d AlignedBoxType;

	static AlignedBoxType createBounds(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
	{
		AlignedBoxType alignedBox = {{{(double)minX, (double)minY, (double)minZ}},
			{{(double)maxX, (double)maxY, (double)maxZ}}};
		return alignedBox;
	}
};

template <>
struct BVHSelector<3, int> : public BVHParamSelector<3, int>
{
	typedef dsAlignedBox3i AlignedBoxType;

	static AlignedBoxType createBounds(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
	{
		AlignedBoxType alignedBox = {{{minX, minY, minZ}}, {{maxX, maxY, maxZ}}};
		return alignedBox;
	}
};

} // namespace

template <typename SelectorT>
class BVHTest : public testing::Test
{
public:
	using AlignedBoxType = typename SelectorT::AlignedBoxType;

	struct TestObject
	{
		AlignedBoxType bounds;
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

	static AlignedBoxType createBounds(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
	{
		return SelectorT::createBounds(minX, minY, minZ, maxX, maxY, maxZ);
	}

	static bool getBounds(void* outBounds, const dsBVH* bvh, const void* object)
	{
		if (dsBVH_getAxisCount(bvh) != axisCount() || dsBVH_getElement(bvh) != element())
			return false;

		*((AlignedBoxType*)outBounds) = ((TestObject*)object)->bounds;
		return true;
	}

	static bool getBoundsIndex(void* outBounds, const dsBVH* bvh, const void* object)
	{
		if (dsBVH_getAxisCount(bvh) != axisCount() || dsBVH_getElement(bvh) != element())
			return false;

		auto objects = (const TestObject*)dsBVH_getUserData(bvh);
		*((AlignedBoxType*)outBounds) = objects[(size_t)object].bounds;
		return true;
	}

	template <typename T>
	static bool lambdaAdapterImpl(void* userData, const dsBVH*, const void* object, const void*)
	{
		(*(T*)userData)(*(const TestObject*)object);
		return true;
	}

	template <typename T>
	static dsBVHVisitFunction lambdaAdapter(const T&)
	{
		return lambdaAdapterImpl<T>;
	}

	template <typename T>
	static bool indexLambdaAdapterImpl(void* userData, const dsBVH* bvh, const void* object,
		const void*)
	{
		auto objects = (const TestObject*)dsBVH_getUserData(bvh);
		(*(T*)userData)(objects[(size_t)object]);
		return true;
	}

	template <typename T>
	static dsBVHVisitFunction indexLambdaAdapter(const T&)
	{
		return indexLambdaAdapterImpl<T>;
	}

	static bool limitedVisits(void* userData, const dsBVH*, const void*, const void*)
	{
		auto* counts = reinterpret_cast<std::pair<int, int>*>(userData);
		return ++counts->first < counts->second;
	}

	dsSystemAllocator allocator;
};

using BVHTypes = testing::Types<BVHSelector<2, float>, BVHSelector<2, double>, BVHSelector<2, int>,
	BVHSelector<3, float>, BVHSelector<3, double>, BVHSelector<3, int>>;
TYPED_TEST_CASE(BVHTest, BVHTypes);

TYPED_TEST(BVHTest, Create)
{
	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), fixture);
	ASSERT_TRUE(bvh);
	EXPECT_EQ(TestFixture::axisCount(), dsBVH_getAxisCount(bvh));
	EXPECT_EQ(TestFixture::element(), dsBVH_getElement(bvh));
	EXPECT_EQ(fixture, dsBVH_getUserData(bvh));
	dsBVH_destroy(bvh);
}

TYPED_TEST(BVHTest, SeparateBoxes)
{
	using TestObject = typename TestFixture::TestObject;
	using AlignedBoxType = typename TestFixture::AlignedBoxType;

	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(bvh);

	TestObject data[] =
	{
		{TestFixture::createBounds(-2, -2, 0, -1, -1, 0), 0},
		{TestFixture::createBounds( 1, -2, 0,  2, -1, 0), 1},
		{TestFixture::createBounds(-2,  1, 0, -1,  2, 0), 2},
		{TestFixture::createBounds( 1,  1, 0,  2,  2, 0), 3}
	};

	EXPECT_TRUE(dsBVH_build(bvh, data, DS_ARRAY_SIZE(data), sizeof(TestObject),
		&TestFixture::getBounds, false));

	AlignedBoxType testBounds = TestFixture::createBounds(0, 0, 0, 0, 0, 0);
	EXPECT_EQ(0U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(0U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, -2, 0, 0, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(1U, object.data);
		};
		testBounds = TestFixture::createBounds(0, -2, 0, 2, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(2U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, 0, 0, 0, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(3U, object.data);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 2, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	testBounds = TestFixture::createBounds(-1, -1, 0, 1, 1, 0);
	EXPECT_EQ(4U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	std::pair<int, int> visitCounts = {0, 1};
	EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 2};
	EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 3};
	EXPECT_EQ(3U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	dsBVH_destroy(bvh);
}

TYPED_TEST(BVHTest, SeparateBoxesBalanced)
{
	using TestObject = typename TestFixture::TestObject;
	using AlignedBoxType = typename TestFixture::AlignedBoxType;

	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(bvh);

	TestObject data[] =
	{
		{TestFixture::createBounds(-2, -2, 0, -1, -1, 0), 0},
		{TestFixture::createBounds( 1, -2, 0,  2, -1, 0), 1},
		{TestFixture::createBounds(-2,  1, 0, -1,  2, 0), 2},
		{TestFixture::createBounds( 1,  1, 0,  2,  2, 0), 3}
	};

	EXPECT_TRUE(dsBVH_build(bvh, data, DS_ARRAY_SIZE(data), sizeof(TestObject),
		&TestFixture::getBounds, true));

	AlignedBoxType testBounds = TestFixture::createBounds(0, 0, 0, 0, 0, 0);
	EXPECT_EQ(0U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(0U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, -2, 0, 0, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(1U, object.data);
		};
		testBounds = TestFixture::createBounds(0, -2, 0, 2, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(2U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, 0, 0, 0, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(3U, object.data);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 2, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	testBounds = TestFixture::createBounds(-1, -1, 0, 1, 1, 0);
	EXPECT_EQ(4U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	std::pair<int, int> visitCounts = {0, 1};
	EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 2};
	EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 3};
	EXPECT_EQ(3U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	dsBVH_destroy(bvh);
}

TYPED_TEST(BVHTest, OverlappingBoxes)
{
	using TestObject = typename TestFixture::TestObject;
	using AlignedBoxType = typename TestFixture::AlignedBoxType;

	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(bvh);

	TestObject data[] =
	{
		{TestFixture::createBounds(-3, -3, 0, -1, -1, 0), 0},
		{TestFixture::createBounds( 1, -3, 0,  3, -1, 0), 1},
		{TestFixture::createBounds(-3,  1, 0, -1,  3, 0), 2},
		{TestFixture::createBounds( 1,  1, 0,  3,  3, 0), 3},
		{TestFixture::createBounds(-2, -2, 0,  2,  2, 0), 4}
	};

	EXPECT_TRUE(dsBVH_build(bvh, data, DS_ARRAY_SIZE(data), sizeof(TestObject),
		&TestFixture::getBounds, false));

	AlignedBoxType testBounds;
	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(4U, object.data);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 0, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 0 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(-2, -2, 0, 0, 0, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 1 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(0, -2, 0, 2, 0, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 2 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(-2, 0, 0, 0, 2, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 3 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 2, 2, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	testBounds = TestFixture::createBounds(-1, -1, 0, 1, 1, 0);
	EXPECT_EQ(5U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	std::pair<int, int> visitCounts = {0, 1};
	EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 2};
	EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 3};
	EXPECT_EQ(3U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 4};
	EXPECT_EQ(4U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	dsBVH_destroy(bvh);
}

TYPED_TEST(BVHTest, OverlappingBoxesBalanced)
{
	using TestObject = typename TestFixture::TestObject;
	using AlignedBoxType = typename TestFixture::AlignedBoxType;

	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(bvh);

	TestObject data[] =
	{
		{TestFixture::createBounds(-3, -3, 0, -1, -1, 0), 0},
		{TestFixture::createBounds( 1, -3, 0,  3, -1, 0), 1},
		{TestFixture::createBounds(-3,  1, 0, -1,  3, 0), 2},
		{TestFixture::createBounds( 1,  1, 0,  3,  3, 0), 3},
		{TestFixture::createBounds(-2, -2, 0,  2,  2, 0), 4}
	};

	EXPECT_TRUE(dsBVH_build(bvh, data, DS_ARRAY_SIZE(data), sizeof(TestObject),
		&TestFixture::getBounds, true));

	AlignedBoxType testBounds;
	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(4U, object.data);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 0, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 0 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(-2, -2, 0, 0, 0, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 1 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(0, -2, 0, 2, 0, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 2 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(-2, 0, 0, 0, 2, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_TRUE(object.data == 3 || object.data == 4);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 2, 2, 0);
		EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	testBounds = TestFixture::createBounds(-1, -1, 0, 1, 1, 0);
	EXPECT_EQ(5U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	std::pair<int, int> visitCounts = {0, 1};
	EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 2};
	EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 3};
	EXPECT_EQ(3U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 4};
	EXPECT_EQ(4U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	dsBVH_destroy(bvh);
}

TYPED_TEST(BVHTest, ObjectPointer)
{
	using TestObject = typename TestFixture::TestObject;
	using AlignedBoxType = typename TestFixture::AlignedBoxType;

	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(bvh);

	TestObject* data[] =
	{
		new TestObject{TestFixture::createBounds(-2, -2, 0, -1, -1, 0), 0},
		new TestObject{TestFixture::createBounds( 1, -2, 0,  2, -1, 0), 1},
		new TestObject{TestFixture::createBounds(-2,  1, 0, -1,  2, 0), 2},
		new TestObject{TestFixture::createBounds( 1,  1, 0,  2,  2, 0), 3}
	};

	EXPECT_TRUE(dsBVH_build(bvh, data, DS_ARRAY_SIZE(data), DS_GEOMETRY_OBJECT_POINTERS,
		&TestFixture::getBounds, false));

	AlignedBoxType testBounds = TestFixture::createBounds(0, 0, 0, 0, 0, 0);
	EXPECT_EQ(0U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(0U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, -2, 0, 0, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(1U, object.data);
		};
		testBounds = TestFixture::createBounds(0, -2, 0, 2, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(2U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, 0, 0, 0, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(3U, object.data);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 2, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	testBounds = TestFixture::createBounds(-1, -1, 0, 1, 1, 0);
	EXPECT_EQ(4U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	dsBVH_destroy(bvh);

	for (TestObject* object : data)
		delete object;
}

TYPED_TEST(BVHTest, ObjectIndex)
{
	using TestObject = typename TestFixture::TestObject;
	using AlignedBoxType = typename TestFixture::AlignedBoxType;

	TestObject data[] =
	{
		TestObject{TestFixture::createBounds(-2, -2, 0, -1, -1, 0), 0},
		TestObject{TestFixture::createBounds( 1, -2, 0,  2, -1, 0), 1},
		TestObject{TestFixture::createBounds(-2,  1, 0, -1,  2, 0), 2},
		TestObject{TestFixture::createBounds( 1,  1, 0,  2,  2, 0), 3}
	};

	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), data);
	ASSERT_TRUE(bvh);

	EXPECT_TRUE(dsBVH_build(bvh, NULL, DS_ARRAY_SIZE(data), DS_GEOMETRY_OBJECT_INDICES,
		&TestFixture::getBoundsIndex, false));

	AlignedBoxType testBounds = TestFixture::createBounds(0, 0, 0, 0, 0, 0);
	EXPECT_EQ(0U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(0U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, -2, 0, 0, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->indexLambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(1U, object.data);
		};
		testBounds = TestFixture::createBounds(0, -2, 0, 2, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->indexLambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(2U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, 0, 0, 0, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->indexLambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(3U, object.data);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 2, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->indexLambdaAdapter(testFunc),
			&testFunc));
	}

	testBounds = TestFixture::createBounds(-1, -1, 0, 1, 1, 0);
	EXPECT_EQ(4U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	dsBVH_destroy(bvh);
}

TYPED_TEST(BVHTest, Update)
{
	using TestObject = typename TestFixture::TestObject;
	using AlignedBoxType = typename TestFixture::AlignedBoxType;

	TestFixture* fixture = this;
	dsBVH* bvh = dsBVH_create((dsAllocator*)&fixture->allocator, TestFixture::axisCount(),
		TestFixture::element(), NULL);
	ASSERT_TRUE(bvh);

	TestObject data[] =
	{
		{TestFixture::createBounds(-2, -2, 0, -1, -1, 0), 0},
		{TestFixture::createBounds( 1, -2, 0,  2, -1, 0), 1},
		{TestFixture::createBounds(-2,  1, 0, -1,  2, 0), 2},
		{TestFixture::createBounds( 1,  1, 0,  2,  2, 0), 3}
	};

	EXPECT_TRUE(dsBVH_build(bvh, data, DS_ARRAY_SIZE(data), sizeof(TestObject),
		&TestFixture::getBounds, false));

	std::swap(data[0].bounds, data[1].bounds);
	std::swap(data[2].bounds, data[3].bounds);
	EXPECT_TRUE(dsBVH_update(bvh));

	AlignedBoxType testBounds = TestFixture::createBounds(0, 0, 0, 0, 0, 0);
	EXPECT_EQ(0U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(1U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, -2, 0, 0, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(0U, object.data);
		};
		testBounds = TestFixture::createBounds(0, -2, 0, 2, 0, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(3U, object.data);
		};
		testBounds = TestFixture::createBounds(-2, 0, 0, 0, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	{
		auto testFunc = [](const TestObject& object)
		{
			EXPECT_EQ(2U, object.data);
		};
		testBounds = TestFixture::createBounds(0, 0, 0, 2, 2, 0);
		EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, fixture->lambdaAdapter(testFunc),
			&testFunc));
	}

	testBounds = TestFixture::createBounds(-1, -1, 0, 1, 1, 0);
	EXPECT_EQ(4U, dsBVH_intersect(bvh, &testBounds, nullptr, nullptr));

	std::pair<int, int> visitCounts = {0, 1};
	EXPECT_EQ(1U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 2};
	EXPECT_EQ(2U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	visitCounts = {0, 3};
	EXPECT_EQ(3U, dsBVH_intersect(bvh, &testBounds, &TestFixture::limitedVisits, &visitCounts));

	dsBVH_destroy(bvh);
}
