/*
 * Copyright 2018-2023 Aaron Barany
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

#include <DeepSea/Geometry/BezierCurve.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector3.h>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

namespace
{

template <typename T, int N>
struct CurveSelector {};

template <>
struct CurveSelector<float, 2>
{
	using RealType = float;
	using VectorType = dsVector2f;
	using CurveSampleFunctionType = dsCurveSampleFunctionf;
	using BezierCurveType = dsBezierCurvef;
	static constexpr uint32_t axisCount = 2;
	static constexpr float epsilon = 1e-6f;

	static VectorType createPoint(RealType x, RealType y, RealType)
	{
		VectorType point = {{x, y}};
		return point;
	}

	static RealType distance(const VectorType& p0, const VectorType& p1)
	{
		return dsVector2f_dist(&p0, &p1);
	}

	static VectorType middle(const VectorType& p0, const VectorType& p1)
	{
		VectorType middle;
		dsVector2_add(middle, p0, p1);
		dsVector2_scale(middle, middle, 0.5f);
		return middle;
	}
};

template <>
struct CurveSelector<double, 2>
{
	using RealType = double;
	using VectorType = dsVector2d;
	using CurveSampleFunctionType = dsCurveSampleFunctiond;
	using BezierCurveType = dsBezierCurved;
	static constexpr uint32_t axisCount = 2;
	static constexpr double epsilon = 1e-14;

	static VectorType createPoint(RealType x, RealType y, RealType)
	{
		VectorType point = {{x, y}};
		return point;
	}

	static RealType distance(const VectorType& p0, const VectorType& p1)
	{
		return dsVector2d_dist(&p0, &p1);
	}

	static VectorType middle(const VectorType& p0, const VectorType& p1)
	{
		VectorType middle;
		dsVector2_add(middle, p0, p1);
		dsVector2_scale(middle, middle, 0.5);
		return middle;
	}
};

template <>
struct CurveSelector<float, 3>
{
	using RealType = float;
	using VectorType = dsVector3f;
	using CurveSampleFunctionType = dsCurveSampleFunctionf;
	using BezierCurveType = dsBezierCurvef;
	static constexpr uint32_t axisCount = 3;
	static constexpr float epsilon = 1e-6f;

	static VectorType createPoint(RealType x, RealType y, RealType z)
	{
		VectorType point = {{x, y, z}};
		return point;
	}

	static RealType distance(const VectorType& p0, const VectorType& p1)
	{
		return dsVector3f_dist(&p0, &p1);
	}

	static VectorType middle(const VectorType& p0, const VectorType& p1)
	{
		VectorType middle;
		dsVector3_add(middle, p0, p1);
		dsVector3_scale(middle, middle, 0.5f);
		return middle;
	}
};

template <>
struct CurveSelector<double, 3>
{
	using RealType = double;
	using VectorType = dsVector3d;
	using CurveSampleFunctionType = dsCurveSampleFunctiond;
	using BezierCurveType = dsBezierCurved;
	static constexpr uint32_t axisCount = 3;
	static constexpr double epsilon = 1e-14;

	static VectorType createPoint(RealType x, RealType y, RealType z)
	{
		VectorType point = {{x, y, z}};
		return point;
	}

	static RealType distance(const VectorType& p0, const VectorType& p1)
	{
		return dsVector3d_dist(&p0, &p1);
	}

	static VectorType middle(const VectorType& p0, const VectorType& p1)
	{
		VectorType middle;
		dsVector3_add(middle, p0, p1);
		dsVector3_scale(middle, middle, 0.5);
		return middle;
	}
};

template <typename SelectorT>
class BezierCurveTest : public testing::Test
{
public:
	using RealType = typename SelectorT::RealType;
	using VectorType = typename SelectorT::VectorType;
	using CurveSampleFunctionType = typename SelectorT::CurveSampleFunctionType;
	using BezierCurveType = typename SelectorT::BezierCurveType;
	static constexpr uint32_t axisCount = SelectorT::axisCount;
	static constexpr RealType epsilon = SelectorT::epsilon;

	static VectorType createPoint(double x, double y, double z)
	{
		return SelectorT::createPoint(static_cast<RealType>(x), static_cast<RealType>(y),
			static_cast<RealType>(x));
	}

	static RealType distance(const VectorType& p0, const VectorType& p1)
	{
		return SelectorT::distance(p0, p1);
	}

	static VectorType middle(const VectorType& p0, const VectorType& p1)
	{
		return SelectorT::middle(p0, p1);
	}

	template <typename T>
	static bool lambdaAdapterImpl(void* userData, const void* point, uint32_t count, RealType t)
	{
		EXPECT_EQ((uint32_t)axisCount, count);
		(*(T*)userData)(*(const VectorType*)point, t);
		return true;
	}

	template <typename T>
	static CurveSampleFunctionType lambdaAdapter(const T&)
	{
		return lambdaAdapterImpl<T>;
	}
};

bool dsBezierCurve_initialize(dsBezierCurvef* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	return dsBezierCurvef_initialize(curve, axisCount, p0, p1, p2, p3);
}

bool dsBezierCurve_initialize(dsBezierCurved* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	return dsBezierCurved_initialize(curve, axisCount, p0, p1, p2, p3);
}

bool dsBezierCurve_initializeQuadratic(dsBezierCurvef* curve,
	uint32_t axisCount, const void* p0, const void* p1, const void* p2)
{
	return dsBezierCurvef_initializeQuadratic(curve, axisCount, p0, p1, p2);
}

bool dsBezierCurve_initializeQuadratic(dsBezierCurved* curve,
	uint32_t axisCount, const void* p0, const void* p1, const void* p2)
{
	return dsBezierCurved_initializeQuadratic(curve, axisCount, p0, p1, p2);
}

bool dsBezierCurve_evaluate(void* outPoint, const dsBezierCurvef* curve, float t)
{
	return dsBezierCurvef_evaluate(outPoint, curve, t);
}

bool dsBezierCurve_evaluate(void* outPoint, const dsBezierCurved* curve, double t)
{
	return dsBezierCurved_evaluate(outPoint, curve, t);
}

bool dsBezierCurve_evaluateTangent(void* outTangent, const dsBezierCurvef* curve, float t)
{
	return dsBezierCurvef_evaluateTangent(outTangent, curve, t);
}

bool dsBezierCurve_evaluateTangent(void* outTangent, const dsBezierCurved* curve, double t)
{
	return dsBezierCurved_evaluateTangent(outTangent, curve, t);
}

bool dsBezierCurve_tessellate(const dsBezierCurvef* curve, float chordalTolerance,
	uint32_t maxRecursions, dsCurveSampleFunctionf sampleFunc, void* userData)
{
	return dsBezierCurvef_tessellate(curve, chordalTolerance, maxRecursions, sampleFunc, userData);
}

bool dsBezierCurve_tessellate(const dsBezierCurved* curve, double chordalTolerance,
	uint32_t maxRecursions, dsCurveSampleFunctiond sampleFunc, void* userData)
{
	return dsBezierCurved_tessellate(curve, chordalTolerance, maxRecursions, sampleFunc, userData);
}

using CurveTypes = testing::Types<CurveSelector<float, 2>, CurveSelector<double, 2>,
	CurveSelector<float, 3>, CurveSelector<double, 3>>;
TYPED_TEST_SUITE(BezierCurveTest, CurveTypes);

TYPED_TEST(BezierCurveTest, EvaluateCubic)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using BezierCurveType = typename TestFixture::BezierCurveType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);
	VectorType p3 = TestFixture::createPoint(5.2, 0.9, 2.5);

	BezierCurveType curve;
	EXPECT_TRUE(dsBezierCurve_initialize(&curve, TestFixture::axisCount, &p0, &p1, &p2, &p3));

	VectorType point;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, RealType(0)));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_NEAR(p0.values[i], point.values[i], TestFixture::epsilon);

	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, RealType(1)));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_NEAR(p3.values[i], point.values[i], TestFixture::epsilon);

	VectorType tangent;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, RealType(0.3)));
	EXPECT_TRUE(dsBezierCurve_evaluateTangent(&tangent, &curve, RealType(0.3)));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
	{
		EXPECT_NEAR(dsPow3(0.7)*p0.values[i] + 3.0*dsPow2(0.7)*0.3*p1.values[i] +
			3.0*dsPow2(0.3)*0.7*p2.values[i] + dsPow3(0.3)*p3.values[i], point.values[i],
			TestFixture::epsilon);
		EXPECT_NEAR(3.0*dsPow2(0.7)*(p1.values[i] - p0.values[i]) +
			6.0*0.3*0.7*(p2.values[i] - p1.values[i]) +
			3.0*dsPow2(0.3)*(p3.values[i] - p2.values[i]), tangent.values[i],
			TestFixture::epsilon);
	}
}

TYPED_TEST(BezierCurveTest, EvaluateQuadratic)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using BezierCurveType = typename TestFixture::BezierCurveType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);

	BezierCurveType curve;
	EXPECT_TRUE(dsBezierCurve_initializeQuadratic(&curve, TestFixture::axisCount, &p0, &p1, &p2));

	VectorType point;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, RealType(0)));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_NEAR(p0.values[i], point.values[i], TestFixture::epsilon);

	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, RealType(1)));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_NEAR(p2.values[i], point.values[i], TestFixture::epsilon);

	VectorType tangent;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, RealType(0.3)));
	EXPECT_TRUE(dsBezierCurve_evaluateTangent(&tangent, &curve, RealType(0.3)));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
	{
		EXPECT_NEAR(dsPow2(0.7)*p0.values[i] + 2.0*0.7*0.3*p1.values[i] +
			dsPow2(0.3)*p2.values[i], point.values[i], TestFixture::epsilon);
		EXPECT_NEAR(2.0*0.7*(p1.values[i] - p0.values[i]) + 2.0*0.3*(p2.values[i] - p1.values[i]),
			tangent.values[i], TestFixture::epsilon);
	}
}

TYPED_TEST(BezierCurveTest, Tessellate)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using BezierCurveType = typename TestFixture::BezierCurveType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);
	VectorType p3 = TestFixture::createPoint(5.2, 0.9, 2.5);

	BezierCurveType curve;
	EXPECT_TRUE(dsBezierCurve_initialize(&curve, TestFixture::axisCount, &p0, &p1, &p2, &p3));

	std::vector<std::pair<VectorType, RealType>> points;
	auto pointFunc = [&curve, &points](const VectorType& point, RealType t)
	{
		VectorType expectedPoint;
		EXPECT_TRUE(dsBezierCurve_evaluate(&expectedPoint, &curve, t));
		for (uint32_t i = 0; i < curve.axisCount; ++i)
			EXPECT_NEAR(expectedPoint.values[i], point.values[i], TestFixture::epsilon);

		points.emplace_back(point, t);
	};

	EXPECT_TRUE(dsBezierCurve_tessellate(&curve, RealType(0.01), 0,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	ASSERT_EQ(2U, points.size());
	EXPECT_EQ(0, points[0].second);
	EXPECT_EQ(1, points[1].second);

	points.clear();
	EXPECT_TRUE(dsBezierCurve_tessellate(&curve, 10, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	ASSERT_EQ(3U, points.size());
	EXPECT_EQ(0, points[0].second);
	EXPECT_NEAR(0.5, points[1].second, TestFixture::epsilon);
	EXPECT_EQ(1, points[2].second);

	points.clear();
	const auto chordalTolerance = RealType(0.01);
	EXPECT_TRUE(dsBezierCurve_tessellate(&curve, chordalTolerance, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	for (uint32_t i = 0; i < points.size() - 1; ++i)
	{
		EXPECT_LT(points[i].second, points[i + 1].second);
		VectorType middle = TestFixture::middle(points[i].first, points[i + 1].first);
		VectorType curveMiddle;
		EXPECT_TRUE(dsBezierCurve_evaluate(&curveMiddle, &curve,
			(points[i].second + points[i + 1].second)/2));
		RealType distance = TestFixture::distance(middle, curveMiddle);
		EXPECT_GT(chordalTolerance + TestFixture::epsilon, distance);
	}
}

} // namespace
