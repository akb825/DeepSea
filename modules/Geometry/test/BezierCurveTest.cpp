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

template <int N>
struct CurveSelector {};

template <>
struct CurveSelector<2>
{
	typedef dsVector2d VectorType;
	static const uint32_t axisCount = 2;

	static VectorType createPoint(double x, double y, double)
	{
		VectorType point = {{x, y}};
		return point;
	}

	static double distance(const VectorType& p0, const VectorType& p1)
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
struct CurveSelector<3>
{
	typedef dsVector3d VectorType;
	static const uint32_t axisCount = 3;

	static VectorType createPoint(double x, double y, double z)
	{
		VectorType point = {{x, y, z}};
		return point;
	}

	static double distance(const VectorType& p0, const VectorType& p1)
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
	using VectorType = typename SelectorT::VectorType;
	static const uint32_t axisCount = SelectorT::axisCount;

	static VectorType createPoint(double x, double y, double z)
	{
		return SelectorT::createPoint(x, y, z);
	}

	static double distance(const VectorType& p0, const VectorType& p1)
	{
		return SelectorT::distance(p0, p1);
	}

	static VectorType middle(const VectorType& p0, const VectorType& p1)
	{
		return SelectorT::middle(p0, p1);
	}

	template <typename T>
	static bool lambdaAdapterImpl(void* userData, const void* point, uint32_t count, double t)
	{
		EXPECT_EQ((uint32_t)axisCount, count);
		(*(T*)userData)(*(const VectorType*)point, t);
		return true;
	}

	template <typename T>
	static dsCurveSampleFunction lambdaAdapter(const T&)
	{
		return lambdaAdapterImpl<T>;
	}
};

using CurveTypes = testing::Types<CurveSelector<2>, CurveSelector<3>>;
TYPED_TEST_SUITE(BezierCurveTest, CurveTypes);

TYPED_TEST(BezierCurveTest, EvaluateCubic)
{
	using VectorType = typename TestFixture::VectorType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);
	VectorType p3 = TestFixture::createPoint(5.2, 0.9, 2.5);

	dsBezierCurve curve;
	EXPECT_TRUE(dsBezierCurve_initialize(&curve, TestFixture::axisCount, &p0, &p1, &p2, &p3));

	VectorType point;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, 0.0));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_DOUBLE_EQ(point.values[i], p0.values[i]);

	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, 1.0));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_DOUBLE_EQ(point.values[i], p3.values[i]);

	VectorType tangent;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, 0.3));
	EXPECT_TRUE(dsBezierCurve_evaluateTangent(&tangent, &curve, 0.3));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
	{
		EXPECT_DOUBLE_EQ(point.values[i], dsPow3(0.7)*p0.values[i] +
			3.0*dsPow2(0.7)*0.3*p1.values[i] + 3.0*dsPow2(0.3)*0.7*p2.values[i] +
			dsPow3(0.3)*p3.values[i]);
		EXPECT_DOUBLE_EQ(tangent.values[i], 3.0*dsPow2(0.7)*(p1.values[i] - p0.values[i]) +
			6.0*0.3*0.7*(p2.values[i] - p1.values[i]) +
			3.0*dsPow2(0.3)*(p3.values[i] - p2.values[i]));
	}
}

TYPED_TEST(BezierCurveTest, EvaluateQuadratic)
{
	using VectorType = typename TestFixture::VectorType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);

	dsBezierCurve curve;
	EXPECT_TRUE(dsBezierCurve_initializeQuadratic(&curve, TestFixture::axisCount, &p0, &p1, &p2));

	VectorType point;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, 0.0));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_DOUBLE_EQ(point.values[i], p0.values[i]);

	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, 1.0));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
		EXPECT_DOUBLE_EQ(point.values[i], p2.values[i]);

	VectorType tangent;
	EXPECT_TRUE(dsBezierCurve_evaluate(&point, &curve, 0.3));
	EXPECT_TRUE(dsBezierCurve_evaluateTangent(&tangent, &curve, 0.3));
	for (uint32_t i = 0; i < curve.axisCount; ++i)
	{
		EXPECT_NEAR(point.values[i], dsPow2(0.7)*p0.values[i] +
			2.0*0.7*0.3*p1.values[i] + dsPow2(0.3)*p2.values[i], 1e-7);
		EXPECT_NEAR(tangent.values[i], 2.0*0.7*(p1.values[i] - p0.values[i]) +
			2.0*0.3*(p2.values[i] - p1.values[i]), 1e-7);
	}
}

TYPED_TEST(BezierCurveTest, Tessellate)
{
	using VectorType = typename TestFixture::VectorType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);
	VectorType p3 = TestFixture::createPoint(5.2, 0.9, 2.5);

	dsBezierCurve curve;
	EXPECT_TRUE(dsBezierCurve_initialize(&curve, TestFixture::axisCount, &p0, &p1, &p2, &p3));

	std::vector<std::pair<VectorType, double>> points;
	auto pointFunc = [&curve, &points](const VectorType& point, double t)
	{
		VectorType expectedPoint;
		EXPECT_TRUE(dsBezierCurve_evaluate(&expectedPoint, &curve, t));
		for (uint32_t i = 0; i < curve.axisCount; ++i)
			EXPECT_NEAR(expectedPoint.values[i], point.values[i], 1e-10);

		points.emplace_back(point, t);
	};

	EXPECT_TRUE(dsBezierCurve_tessellate(&curve, 0.01, 0, TestFixture::lambdaAdapter(pointFunc),
		&pointFunc));
	ASSERT_EQ(2U, points.size());
	EXPECT_EQ(0.0, points[0].second);
	EXPECT_EQ(1.0, points[1].second);

	points.clear();
	EXPECT_TRUE(dsBezierCurve_tessellate(&curve, 10.0, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	ASSERT_EQ(3U, points.size());
	EXPECT_EQ(0.0, points[0].second);
	EXPECT_EQ(0.5, points[1].second);
	EXPECT_EQ(1.0, points[2].second);

	points.clear();
	const double chordalTolerance = 0.01;
	const double epsilon = 1e-7;
	EXPECT_TRUE(dsBezierCurve_tessellate(&curve, chordalTolerance, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	for (uint32_t i = 0; i < points.size() - 1; ++i)
	{
		EXPECT_LT(points[i].second, points[i + 1].second);
		VectorType middle = TestFixture::middle(points[i].first, points[i + 1].first);
		VectorType curveMiddle;
		EXPECT_TRUE(dsBezierCurve_evaluate(&curveMiddle, &curve,
			(points[i].second + points[i + 1].second)*0.5));
		double distance = TestFixture::distance(middle, curveMiddle);
		EXPECT_GT(chordalTolerance + epsilon, distance);
	}
}

} // namespace
