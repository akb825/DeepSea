/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/Math/JacobiEigenvalues.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Types.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct JacobiEigenvaluesTypeSelector;

template <>
struct JacobiEigenvaluesTypeSelector<float>
{
	typedef dsMatrix44f Matrix44Type;
	typedef dsVector4f Vector4Type;

	static constexpr float epsilon = 3e-4f;
};

template <>
struct JacobiEigenvaluesTypeSelector<double>
{
	typedef dsMatrix44d Matrix44Type;
	typedef dsVector4d Vector4Type;

	static constexpr double epsilon = 2e-12;
};

template <typename T>
class JacobiEigenvaluesTest : public testing::Test
{
};

using JacobiEigenvaluesTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(JacobiEigenvaluesTest, JacobiEigenvaluesTypes);

inline bool dsJacobiEigenvaluesClassic(dsMatrix44f& outEigenvectors, dsVector4f& outEigenvalues,
	const dsMatrix44f& matrix, unsigned int maxSweeps)
{
	return dsJacobiEigenvaluesClassicf(outEigenvectors.values[0], outEigenvalues.values,
		matrix.values[0], 4, maxSweeps);
}

inline bool dsJacobiEigenvaluesClassic(dsMatrix44d& outEigenvectors, dsVector4d& outEigenvalues,
	const dsMatrix44d& matrix, unsigned int maxSweeps)
{
	return dsJacobiEigenvaluesClassicd(outEigenvectors.values[0], outEigenvalues.values,
		matrix.values[0], 4, maxSweeps);
}

inline bool dsJacobiEigenvaluesCyclic(dsMatrix44f& outEigenvectors, dsVector4f& outEigenvalues,
	const dsMatrix44f& matrix, unsigned int maxSweeps)
{
	return dsJacobiEigenvaluesCyclicf(outEigenvectors.values[0], outEigenvalues.values,
		matrix.values[0], 4, maxSweeps);
}

inline bool dsJacobiEigenvaluesCyclic(dsMatrix44d& outEigenvectors, dsVector4d& outEigenvalues,
	const dsMatrix44d& matrix, unsigned int maxSweeps)
{
	return dsJacobiEigenvaluesCyclicd(outEigenvectors.values[0], outEigenvalues.values,
		matrix.values[0], 4, maxSweeps);
}

inline void dsSortEigenvalues(dsMatrix44f& eigenvectors, dsVector4f& eigenvalues)
{
	dsSortEigenvaluesf(eigenvectors.values[0], eigenvalues.values, 4);
}

inline void dsSortEigenvalues(dsMatrix44d& eigenvectors, dsVector4d& eigenvalues)
{
	dsSortEigenvaluesd(eigenvectors.values[0], eigenvalues.values, 4);
}

TYPED_TEST(JacobiEigenvaluesTest, Classic)
{
	typedef typename JacobiEigenvaluesTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename JacobiEigenvaluesTypeSelector<TypeParam>::Vector4Type Vector4Type;
	constexpr auto epsilon = JacobiEigenvaluesTypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix =
	{{
		{4, -30, 60, -35},
		{-30, 300, -675, 420},
		{60, -675, 1620, -1050},
		{-35, 420, -1050, 700}
	}};

	Matrix44Type eigenvectors;
	Vector4Type eigenvalues;
	ASSERT_TRUE(dsJacobiEigenvaluesClassic(eigenvectors, eigenvalues, matrix, 5));
	dsSortEigenvalues(eigenvectors, eigenvalues);

	Matrix44Type eigenvectorsTrans;
	dsMatrix44_transpose(eigenvectorsTrans, eigenvectors);
	Matrix44Type eigenvaluesDiag =
	{{
		{eigenvalues.x, 0, 0, 0},
		{0, eigenvalues.y, 0, 0},
		{0, 0, eigenvalues.z, 0},
		{0, 0, 0, eigenvalues.w}
	}};

	Matrix44Type temp, restored;
	dsMatrix44_mul(temp, eigenvaluesDiag, eigenvectorsTrans);
	dsMatrix44_mul(restored, eigenvectors, temp);
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
			EXPECT_NEAR(matrix.values[i][j], restored.values[i][j], epsilon) << i << ", " << j;
	}

	EXPECT_NEAR(2585.25381092892231, eigenvalues.x, epsilon);
	EXPECT_NEAR(37.1014913651276582, eigenvalues.y, epsilon);
	EXPECT_NEAR(1.4780548447781369, eigenvalues.z, epsilon);
	EXPECT_NEAR(0.1666428611718905, eigenvalues.w, epsilon);

	EXPECT_NEAR(0.0291933231647860588, eigenvectors.columns[0].x, epsilon);
	EXPECT_NEAR(-0.328712055763188997, eigenvectors.columns[0].y, epsilon);
	EXPECT_NEAR(0.791411145833126331, eigenvectors.columns[0].z, epsilon);
	EXPECT_NEAR(-0.514552749997152907, eigenvectors.columns[0].w, epsilon);

	EXPECT_NEAR(-0.179186290535454826, eigenvectors.columns[1].x, epsilon);
	EXPECT_NEAR(0.741917790628453435, eigenvectors.columns[1].y, epsilon);
	EXPECT_NEAR(-0.100228136947192199, eigenvectors.columns[1].z, epsilon);
	EXPECT_NEAR(-0.638282528193614892, eigenvectors.columns[1].w, epsilon);

	EXPECT_NEAR(-0.582075699497237650, eigenvectors.columns[2].x, epsilon);
	EXPECT_NEAR(0.370502185067093058, eigenvectors.columns[2].y, epsilon);
	EXPECT_NEAR(0.509578634501799626, eigenvectors.columns[2].z, epsilon);
	EXPECT_NEAR(0.514048272222164294, eigenvectors.columns[2].w, epsilon);

	EXPECT_NEAR(0.792608291163763583, eigenvectors.columns[3].x, epsilon);
	EXPECT_NEAR(0.451923120901599794, eigenvectors.columns[3].y, epsilon);
	EXPECT_NEAR(0.322416398581824992, eigenvectors.columns[3].z, epsilon);
	EXPECT_NEAR(0.252161169688241933, eigenvectors.columns[3].w, epsilon);
}

TYPED_TEST(JacobiEigenvaluesTest, Cyclic)
{
	typedef typename JacobiEigenvaluesTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename JacobiEigenvaluesTypeSelector<TypeParam>::Vector4Type Vector4Type;
	constexpr auto epsilon = JacobiEigenvaluesTypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix =
	{{
		{4, -30, 60, -35},
		{-30, 300, -675, 420},
		{60, -675, 1620, -1050},
		{-35, 420, -1050, 700}
	}};

	Matrix44Type eigenvectors;
	Vector4Type eigenvalues;
	ASSERT_TRUE(dsJacobiEigenvaluesCyclic(eigenvectors, eigenvalues, matrix, 8));
	dsSortEigenvalues(eigenvectors, eigenvalues);

	Matrix44Type eigenvectorsTrans;
	dsMatrix44_transpose(eigenvectorsTrans, eigenvectors);
	Matrix44Type eigenvaluesDiag =
	{{
		{eigenvalues.x, 0, 0, 0},
		{0, eigenvalues.y, 0, 0},
		{0, 0, eigenvalues.z, 0},
		{0, 0, 0, eigenvalues.w}
	}};

	Matrix44Type temp, restored;
	dsMatrix44_mul(temp, eigenvaluesDiag, eigenvectorsTrans);
	dsMatrix44_mul(restored, eigenvectors, temp);
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
			EXPECT_NEAR(matrix.values[i][j], restored.values[i][j], epsilon) << i << ", " << j;
	}

	// Only compare the eigenvalues as the eigenvectors may be flipped.
	EXPECT_NEAR(2585.25381092892231, eigenvalues.x, epsilon);
	EXPECT_NEAR(37.1014913651276582, eigenvalues.y, epsilon);
	EXPECT_NEAR(1.4780548447781369, eigenvalues.z, epsilon);
	EXPECT_NEAR(0.1666428611718905, eigenvalues.w, epsilon);
}
