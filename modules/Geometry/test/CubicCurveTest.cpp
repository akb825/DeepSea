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

#include <DeepSea/Geometry/CubicCurve.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector3.h>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T, int N>
struct CurveSelector {};

template <>
struct CurveSelector<float, 2>
{
	using RealType = float;
	using VectorType = dsVector2f;
	using CurveSampleFunctionType = dsCurveSampleFunctionf;
	using CubicCurveType = dsCubicCurvef;
	static constexpr unsigned int axisCount = 2;
	static constexpr float epsilon = 1e-6f;
	static constexpr float relaxedEpsilon = 1e-5f;

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
	using CubicCurveType = dsCubicCurved;
	static constexpr unsigned int axisCount = 2;
	static constexpr double epsilon = 1e-14;
	static constexpr double relaxedEpsilon = 1e-7;

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
	using CubicCurveType = dsCubicCurvef;
	static constexpr unsigned int axisCount = 3;
	static constexpr float epsilon = 1e-6f;
	static constexpr float relaxedEpsilon = 1e-5f;

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
	using CubicCurveType = dsCubicCurved;
	static constexpr unsigned int axisCount = 3;
	static constexpr double epsilon = 1e-14;
	static constexpr double relaxedEpsilon = 1e-7;

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
class CubicCurveTest : public testing::Test
{
public:
	using RealType = typename SelectorT::RealType;
	using VectorType = typename SelectorT::VectorType;
	using CurveSampleFunctionType = typename SelectorT::CurveSampleFunctionType;
	using CubicCurveType = typename SelectorT::CubicCurveType;
	static constexpr uint32_t axisCount = SelectorT::axisCount;
	static constexpr RealType epsilon = SelectorT::epsilon;
	static constexpr RealType relaxedEpsilon = SelectorT::relaxedEpsilon;

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

bool dsCubicCurve_initializeBezier(dsCubicCurvef* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	return dsCubicCurvef_initializeBezier(curve, axisCount, p0, p1, p2, p3);
}

bool dsCubicCurve_initializeBezier(dsCubicCurved* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	return dsCubicCurved_initializeBezier(curve, axisCount, p0, p1, p2, p3);
}

bool dsCubicCurve_initializeQuadratic(dsCubicCurvef* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2)
{
	return dsCubicCurvef_initializeQuadratic(curve, axisCount, p0, p1, p2);
}

bool dsCubicCurve_initializeQuadratic(dsCubicCurved* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2)
{
	return dsCubicCurved_initializeQuadratic(curve, axisCount, p0, p1, p2);
}

bool dsCubicCurve_initializeHermite(dsCubicCurvef* curve, unsigned int axisCount,
	const void* p0, const void* t0, const void* p1, const void* t1)
{
	return dsCubicCurvef_initializeHermite(curve, axisCount, p0, t0, p1, t1);
}

bool dsCubicCurve_initializeHermite(dsCubicCurved* curve, unsigned int axisCount,
	const void* p0, const void* t0, const void* p1, const void* t1)
{
	return dsCubicCurved_initializeHermite(curve, axisCount, p0, t0, p1, t1);
}

bool dsCubicCurve_evaluate(void* outPoint, const dsCubicCurvef* curve, float t)
{
	return dsCubicCurvef_evaluate(outPoint, curve, t);
}

bool dsCubicCurve_evaluate(void* outPoint, const dsCubicCurved* curve, double t)
{
	return dsCubicCurved_evaluate(outPoint, curve, t);
}

bool dsCubicCurve_evaluateTangent(void* outTangent, const dsCubicCurvef* curve, float t)
{
	return dsCubicCurvef_evaluateTangent(outTangent, curve, t);
}

bool dsCubicCurve_evaluateTangent(void* outTangent, const dsCubicCurved* curve, double t)
{
	return dsCubicCurved_evaluateTangent(outTangent, curve, t);
}

bool dsCubicCurve_tessellate(const dsCubicCurvef* curve, float chordalTolerance,
	unsigned int maxRecursions, dsCurveSampleFunctionf sampleFunc, void* userData)
{
	return dsCubicCurvef_tessellate(curve, chordalTolerance, maxRecursions, sampleFunc, userData);
}

bool dsCubicCurve_tessellate(const dsCubicCurved* curve, double chordalTolerance,
	unsigned int maxRecursions, dsCurveSampleFunctiond sampleFunc, void* userData)
{
	return dsCubicCurved_tessellate(curve, chordalTolerance, maxRecursions, sampleFunc, userData);
}

using CurveTypes = testing::Types<CurveSelector<float, 2>, CurveSelector<double, 2>,
	CurveSelector<float, 3>, CurveSelector<double, 3>>;
TYPED_TEST_SUITE(CubicCurveTest, CurveTypes);

TEST(CubicCurveTest, FloatMtrices)
{
	dsMatrix44f forwardInverse;
	dsMatrix44f_mul(&forwardInverse, &dsCubicCurvef_cubicToBezier, &dsCubicCurvef_bezierToCubic);

	EXPECT_FLOAT_EQ(1, forwardInverse.values[0][0]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[0][1]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[0][2]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[0][3]);

	EXPECT_FLOAT_EQ(0, forwardInverse.values[1][0]);
	EXPECT_FLOAT_EQ(1, forwardInverse.values[1][1]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[1][2]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[1][3]);

	EXPECT_FLOAT_EQ(0, forwardInverse.values[2][0]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[2][1]);
	EXPECT_FLOAT_EQ(1, forwardInverse.values[2][2]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[2][3]);

	EXPECT_FLOAT_EQ(0, forwardInverse.values[3][0]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[3][1]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[3][2]);
	EXPECT_FLOAT_EQ(1, forwardInverse.values[3][3]);

	dsMatrix44f_mul(&forwardInverse, &dsCubicCurvef_cubicToHermite, &dsCubicCurvef_hermiteToCubic);

	EXPECT_FLOAT_EQ(1, forwardInverse.values[0][0]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[0][1]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[0][2]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[0][3]);

	EXPECT_FLOAT_EQ(0, forwardInverse.values[1][0]);
	EXPECT_FLOAT_EQ(1, forwardInverse.values[1][1]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[1][2]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[1][3]);

	EXPECT_FLOAT_EQ(0, forwardInverse.values[2][0]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[2][1]);
	EXPECT_FLOAT_EQ(1, forwardInverse.values[2][2]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[2][3]);

	EXPECT_FLOAT_EQ(0, forwardInverse.values[3][0]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[3][1]);
	EXPECT_FLOAT_EQ(0, forwardInverse.values[3][2]);
	EXPECT_FLOAT_EQ(1, forwardInverse.values[3][3]);
}

TEST(CubicCurveTest, DoubleMtrices)
{
	dsMatrix44d forwardInverse;
	dsMatrix44d_mul(&forwardInverse, &dsCubicCurved_cubicToBezier, &dsCubicCurved_bezierToCubic);

	EXPECT_DOUBLE_EQ(1, forwardInverse.values[0][0]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[0][1]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[0][2]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[0][3]);

	EXPECT_DOUBLE_EQ(0, forwardInverse.values[1][0]);
	EXPECT_DOUBLE_EQ(1, forwardInverse.values[1][1]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[1][2]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[1][3]);

	EXPECT_DOUBLE_EQ(0, forwardInverse.values[2][0]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[2][1]);
	EXPECT_DOUBLE_EQ(1, forwardInverse.values[2][2]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[2][3]);

	EXPECT_DOUBLE_EQ(0, forwardInverse.values[3][0]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[3][1]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[3][2]);
	EXPECT_DOUBLE_EQ(1, forwardInverse.values[3][3]);

	dsMatrix44d_mul(&forwardInverse, &dsCubicCurved_cubicToHermite, &dsCubicCurved_hermiteToCubic);

	EXPECT_DOUBLE_EQ(1, forwardInverse.values[0][0]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[0][1]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[0][2]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[0][3]);

	EXPECT_DOUBLE_EQ(0, forwardInverse.values[1][0]);
	EXPECT_DOUBLE_EQ(1, forwardInverse.values[1][1]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[1][2]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[1][3]);

	EXPECT_DOUBLE_EQ(0, forwardInverse.values[2][0]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[2][1]);
	EXPECT_DOUBLE_EQ(1, forwardInverse.values[2][2]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[2][3]);

	EXPECT_DOUBLE_EQ(0, forwardInverse.values[3][0]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[3][1]);
	EXPECT_DOUBLE_EQ(0, forwardInverse.values[3][2]);
	EXPECT_DOUBLE_EQ(1, forwardInverse.values[3][3]);
}

TYPED_TEST(CubicCurveTest, EvaluateBezier)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using CubicCurveType = typename TestFixture::CubicCurveType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);
	VectorType p3 = TestFixture::createPoint(5.2, 0.9, 2.5);

	CubicCurveType curve;
	EXPECT_TRUE(dsCubicCurve_initializeBezier(&curve, TestFixture::axisCount, &p0, &p1, &p2, &p3));

	VectorType point;
	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(0)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
		EXPECT_EQ(p0.values[i], point.values[i]);

	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(1)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
		EXPECT_EQ(p3.values[i], point.values[i]);

	VectorType tangent;
	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(0.3)));
	EXPECT_TRUE(dsCubicCurve_evaluateTangent(&tangent, &curve, RealType(0.3)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
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

TYPED_TEST(CubicCurveTest, EvaluateQuadratic)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using CubicCurveType = typename TestFixture::CubicCurveType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);

	CubicCurveType curve;
	EXPECT_TRUE(dsCubicCurve_initializeQuadratic(&curve, TestFixture::axisCount, &p0, &p1, &p2));

	VectorType point;
	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(0)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
		EXPECT_EQ(p0.values[i], point.values[i]);

	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(1)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
		EXPECT_EQ(p2.values[i], point.values[i]);

	VectorType tangent;
	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(0.3)));
	EXPECT_TRUE(dsCubicCurve_evaluateTangent(&tangent, &curve, RealType(0.3)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
	{
		EXPECT_NEAR(dsPow2(0.7)*p0.values[i] + 2.0*0.7*0.3*p1.values[i] +
			dsPow2(0.3)*p2.values[i], point.values[i], TestFixture::relaxedEpsilon);
		EXPECT_NEAR(2.0*0.7*(p1.values[i] - p0.values[i]) + 2.0*0.3*(p2.values[i] - p1.values[i]),
			tangent.values[i], TestFixture::relaxedEpsilon);
	}
}

TYPED_TEST(CubicCurveTest, EvaluateHermite)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using CubicCurveType = typename TestFixture::CubicCurveType;

	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType t0 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p1 = TestFixture::createPoint(5.2, 0.9, 2.5);
	VectorType t1 = TestFixture::createPoint(1.4, 3.2, -3.4);

	CubicCurveType curve;
	EXPECT_TRUE(dsCubicCurve_initializeHermite(&curve, TestFixture::axisCount, &p0, &t0, &p1, &t1));

	VectorType point;
	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(0)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
		EXPECT_EQ(p0.values[i], point.values[i]);

	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(1)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
		EXPECT_EQ(p1.values[i], point.values[i]);

	EXPECT_TRUE(dsCubicCurve_evaluate(&point, &curve, RealType(0.3)));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
	{
		EXPECT_NEAR((2*dsPow3(0.3) - 3*dsPow2(0.3) + 1)*p0.values[i] +
			(dsPow3(0.3) - 2*dsPow2(0.3) + 0.3)*t0.values[i] +
			(-2*dsPow3(0.3) + 3*dsPow2(0.3))*p1.values[i] +
			(dsPow3(0.3) - dsPow2(0.3))*t1.values[i], point.values[i], TestFixture::epsilon);
	}

	VectorType startTangent, endTangent;
	EXPECT_TRUE(dsCubicCurve_evaluateTangent(&startTangent, &curve, 0));
	EXPECT_TRUE(dsCubicCurve_evaluateTangent(&endTangent, &curve, 1));
	for (unsigned int i = 0; i < curve.axisCount; ++i)
	{
		EXPECT_NEAR(t0.values[i], startTangent.values[i], TestFixture::relaxedEpsilon);
		EXPECT_NEAR(t1.values[i], endTangent.values[i], TestFixture::relaxedEpsilon);
	}
}

TYPED_TEST(CubicCurveTest, Tessellate)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using CubicCurveType = typename TestFixture::CubicCurveType;

	const auto chordalTolerance = RealType(0.01);
	VectorType p0 = TestFixture::createPoint(0.0, 0.1, 0.2);
	VectorType p1 = TestFixture::createPoint(0.5, -0.3, 0.8);
	VectorType p2 = TestFixture::createPoint(1.4, 3.2, -3.4);
	VectorType p3 = TestFixture::createPoint(5.2, 0.9, 2.5);

	CubicCurveType curve;
	EXPECT_TRUE(dsCubicCurve_initializeBezier(&curve, TestFixture::axisCount, &p0, &p1, &p2, &p3));

	std::vector<std::pair<VectorType, RealType>> points;
	auto pointFunc = [&curve, &points](const VectorType& point, RealType t)
	{
		VectorType expectedPoint;
		EXPECT_TRUE(dsCubicCurve_evaluate(&expectedPoint, &curve, t));
		for (unsigned int i = 0; i < curve.axisCount; ++i)
			EXPECT_NEAR(expectedPoint.values[i], point.values[i], TestFixture::epsilon);

		points.emplace_back(point, t);
	};

	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, 0,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	ASSERT_EQ(2U, points.size());
	EXPECT_EQ(0, points[0].second);
	EXPECT_EQ(1, points[1].second);

	points.clear();
	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, 1,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	ASSERT_EQ(3U, points.size());
	EXPECT_EQ(0, points[0].second);
	EXPECT_NEAR(0.5, points[1].second, TestFixture::epsilon);
	EXPECT_EQ(1, points[2].second);

	points.clear();
	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, 2,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	ASSERT_EQ(5U, points.size());
	EXPECT_EQ(0, points[0].second);
	EXPECT_NEAR(0.25, points[1].second, TestFixture::epsilon);
	EXPECT_NEAR(0.5, points[2].second, TestFixture::epsilon);
	EXPECT_NEAR(0.75, points[3].second, TestFixture::epsilon);
	EXPECT_EQ(1, points[4].second);

	points.clear();
	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, 3,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	ASSERT_EQ(9U, points.size());
	EXPECT_EQ(0, points[0].second);
	EXPECT_NEAR(0.125, points[1].second, TestFixture::epsilon);
	EXPECT_NEAR(0.25, points[2].second, TestFixture::epsilon);
	EXPECT_NEAR(0.375, points[3].second, TestFixture::epsilon);
	EXPECT_NEAR(0.5, points[4].second, TestFixture::epsilon);
	EXPECT_NEAR(0.625, points[5].second, TestFixture::epsilon);
	EXPECT_NEAR(0.75, points[6].second, TestFixture::epsilon);
	EXPECT_NEAR(0.875, points[7].second, TestFixture::epsilon);
	EXPECT_EQ(1, points[8].second);

	points.clear();
	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	for (unsigned int i = 0; i < points.size() - 1; ++i)
	{
		EXPECT_LT(points[i].second, points[i + 1].second);
		VectorType middle = TestFixture::middle(points[i].first, points[i + 1].first);
		VectorType curveMiddle;
		EXPECT_TRUE(dsCubicCurve_evaluate(&curveMiddle, &curve,
			(points[i].second + points[i + 1].second)/2));
		RealType distance = TestFixture::distance(middle, curveMiddle);
		EXPECT_GT(chordalTolerance + TestFixture::epsilon, distance);
	}
}

TYPED_TEST(CubicCurveTest, TessellateChordalToleranceInsufficient)
{
	using RealType = typename TestFixture::RealType;
	using VectorType = typename TestFixture::VectorType;
	using CubicCurveType = typename TestFixture::CubicCurveType;

	const auto chordalTolerance = RealType(0.01);
	VectorType p0 = TestFixture::createPoint(-5.0, -5.0, 0.2);
	VectorType p1 = TestFixture::createPoint(5.2, 0.9, 2.5);
	VectorType t0 = TestFixture::createPoint(2.6, -2.0, 0.3);
	VectorType lineTangent = TestFixture::createPoint(10.2, 5.9, 2.3);
	VectorType zero = TestFixture::createPoint(0, 0, 0);

	CubicCurveType curve;
	std::vector<VectorType> points;
	auto pointFunc = [&curve, &points](const VectorType& point, RealType t)
	{
		VectorType expectedPoint;
		EXPECT_TRUE(dsCubicCurve_evaluate(&expectedPoint, &curve, t));
		for (unsigned int i = 0; i < curve.axisCount; ++i)
			EXPECT_NEAR(expectedPoint.values[i], point.values[i], TestFixture::epsilon);

		points.push_back(point);
	};

	// Passes through midpoint.
	EXPECT_TRUE(dsCubicCurve_initializeHermite(&curve, TestFixture::axisCount, &p0, &t0, &p1,
		&t0));
	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	EXPECT_LT(2U, points.size());

	// Passes through midpoint with tangent direction same as a straight line.
	points.clear();
	EXPECT_TRUE(dsCubicCurve_initializeHermite(&curve, TestFixture::axisCount, &p0, &zero, &p1,
		&zero));
	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	EXPECT_LT(2U, points.size());

	// Resolves into a line..
	points.clear();
	EXPECT_TRUE(dsCubicCurve_initializeHermite(&curve, TestFixture::axisCount, &p0, &lineTangent,
		&p1, &lineTangent));
	EXPECT_TRUE(dsCubicCurve_tessellate(&curve, chordalTolerance, DS_MAX_CURVE_RECURSIONS,
		TestFixture::lambdaAdapter(pointFunc), &pointFunc));
	EXPECT_EQ(2U, points.size());
}
