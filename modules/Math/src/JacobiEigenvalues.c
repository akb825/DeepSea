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

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/Core.h>

#include <string.h>

#define MAX_N 100

// See https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm for reference.
// The basic math for creating and applying the rotation is pretty much the same, though the
// surrounding logic has been improved. Both a classical version (as done by the reference)
// and cyclic version (which rotates all off-diagonals in order) are provided for the different
// performance tradeoffs.

inline static void rotatef(float* matrix, unsigned int n, float cosRot, float sinRot,
	unsigned int xCol, unsigned int xRow, unsigned int yCol, unsigned int yRow)
{
	unsigned int xIndex = xCol*n + xRow;
	unsigned int yIndex = yCol*n + yRow;
	float x = matrix[xIndex];
	float y = matrix[yIndex];
	matrix[xIndex] = cosRot*x - sinRot*y;
	matrix[yIndex] = sinRot*x + cosRot*y;
}

inline static void rotated(double* matrix, unsigned int n, double cosRot, double sinRot,
	unsigned int xCol, unsigned int xRow, unsigned int yCol, unsigned int yRow)
{
	unsigned int xIndex = xCol*n + xRow;
	unsigned int yIndex = yCol*n + yRow;
	double x = matrix[xIndex];
	double y = matrix[yIndex];
	matrix[xIndex] = cosRot*x - sinRot*y;
	matrix[yIndex] = sinRot*x + cosRot*y;
}

inline static bool pivotf(float* outEigenvectors, float* outEigenvalues, float* matrix,
	unsigned int n, unsigned int pivotCol, unsigned int pivotRow, float pivotValue)
{
	// Always clear out the pivot value first in case it's *almost* zero and fails one of the
	// below checks.
	matrix[pivotCol*n + pivotRow] = 0.0f;

	float pivotValue2 = dsPow2(pivotValue);
	// Already pivoted. Check square since it would underflow.
	if (pivotValue2 == 0.0f)
		return false;

	DS_ASSERT(pivotCol < pivotRow);

	// Compute the rotation.
	// When using the definitions of sin (opposite/hypotenuse) and cos (adjacent/hypotenuse),
	// the pivot value is the "opposite" value.
	float eigenDiff = (outEigenvalues[pivotRow] - outEigenvalues[pivotCol])/2;
	float adjacent = fabsf(eigenDiff) + sqrtf(pivotValue2 + dsPow2(eigenDiff));
	float eigenOffset = pivotValue2/adjacent;
	float hypotenuse = sqrtf(pivotValue2 + dsPow2(adjacent));
	// These values should be a valid non-zero value, otherwise the floating point math broke
	// down for a basically non-rotated row.
	if (!isnormal(eigenOffset) || !isnormal(hypotenuse))
		return false;

	float cosRot = adjacent/hypotenuse;
	float sinRot = pivotValue/hypotenuse;
	if (eigenDiff < 0.0f)
	{
		sinRot = -sinRot;
		eigenOffset = -eigenOffset;
	}

	// Offset the eigenvalues and clear out the current largest off-diagonal.
	outEigenvalues[pivotCol] -= eigenOffset;
	outEigenvalues[pivotRow] += eigenOffset;

	// Rotate curMatrix across the pivot axis.
	for (unsigned int i = 0; i < pivotCol; ++i)
		rotatef(matrix, n, cosRot, sinRot, i, pivotCol, i, pivotRow);
	for (unsigned int i = pivotCol + 1; i < pivotRow; ++i)
		rotatef(matrix, n, cosRot, sinRot, pivotCol, i, i, pivotRow);
	for (unsigned int i = pivotRow + 1; i < n; ++i)
		rotatef(matrix, n, cosRot, sinRot, pivotCol, i, pivotRow, i);

	// Rotate the eigenvectors.
	for (unsigned int i = 0; i < n; ++i)
		rotatef(outEigenvectors, n, cosRot, sinRot, pivotCol, i, pivotRow, i);

	return true;
}

inline static bool pivotd(double* outEigenvectors, double* outEigenvalues, double* matrix,
	unsigned int n, unsigned int pivotCol, unsigned int pivotRow, double pivotValue)
{
	// Always clear out the pivot value first in case it's *almost* zero and fails one of the
	// below checks.
	matrix[pivotCol*n + pivotRow] = 0.0;

	double pivotValue2 = dsPow2(pivotValue);
	// Already pivoted. Check square since it would underflow.
	if (pivotValue2 == 0.0)
		return false;

	DS_ASSERT(pivotCol < pivotRow);

	// Compute the rotation.
	// When using the definitions of sin (opposite/hypotenuse) and cos (adjacent/hypotenuse),
	// the pivot value is the "opposite" value.
	double eigenDiff = (outEigenvalues[pivotRow] - outEigenvalues[pivotCol])/2;
	double adjacent = fabs(eigenDiff) + sqrt(pivotValue2 + dsPow2(eigenDiff));
	double eigenOffset = pivotValue2/adjacent;
	double hypotenuse = sqrt(pivotValue2 + dsPow2(adjacent));
	// These values should be a valid non-zero value, otherwise the floating point math broke
	// down for a basically non-rotated row.
	if (!isnormal(eigenOffset) || !isnormal(hypotenuse))
		return false;

	double cosRot = adjacent/hypotenuse;
	double sinRot = pivotValue/hypotenuse;
	if (eigenDiff < 0.0f)
	{
		sinRot = -sinRot;
		eigenOffset = -eigenOffset;
	}

	// Offset the eigenvalues and clear out the current largest off-diagonal.
	outEigenvalues[pivotCol] -= eigenOffset;
	outEigenvalues[pivotRow] += eigenOffset;

	// Rotate curMatrix across the pivot axis.
	for (unsigned int i = 0; i < pivotCol; ++i)
		rotated(matrix, n, cosRot, sinRot, i, pivotCol, i, pivotRow);
	for (unsigned int i = pivotCol + 1; i < pivotRow; ++i)
		rotated(matrix, n, cosRot, sinRot, pivotCol, i, i, pivotRow);
	for (unsigned int i = pivotRow + 1; i < n; ++i)
		rotated(matrix, n, cosRot, sinRot, pivotCol, i, pivotRow, i);

	// Rotate the eigenvectors.
	for (unsigned int i = 0; i < n; ++i)
		rotated(outEigenvectors, n, cosRot, sinRot, pivotCol, i, pivotRow, i);

	return true;
}

bool dsJacobiEigenvaluesClassicf(float* outEigenvectors, float* outEigenvalues, const float* matrix,
	unsigned int n, unsigned int maxSweeps)
{
	DS_ASSERT(outEigenvectors);
	DS_ASSERT(outEigenvalues);
	DS_ASSERT(matrix);
	DS_ASSERT(n > 1);
	DS_ASSERT(maxSweeps > 0);
	DS_ASSERT(n <= MAX_N);

	// Initialize the output values assuming this is a diagonal matrix.
	float* curMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(float, n*n);
	memcpy(curMatrix, matrix, sizeof(float)*n*n);
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = curMatrix[i*n + i];
		float* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (float)(i == j);
	}

	// Each sweep is a rotation around each non-diagonal, or n*(n - 1)/2 for a symmetric matrix.
	unsigned int maxIterations = n*(n - 1)/2*maxSweeps;
	for (unsigned int iter = 0; iter < maxIterations; ++iter)
	{
		// Find the maximum index to pivot on.
		unsigned int pivotCol = 0, pivotRow = 0;
		float pivotValue = 0.0f;
		for (unsigned int i = 0; i < n - 1; ++i)
		{
			const float* column = curMatrix + i*n;
			for (unsigned int j = i + 1; j < n; ++j)
			{
				float thisPivotValue = column[j];
				if (fabsf(thisPivotValue) > fabsf(pivotValue))
				{
					pivotCol = i;
					pivotRow = j;
					pivotValue = thisPivotValue;
				}
			}
		}

		if (!pivotf(outEigenvectors, outEigenvalues, curMatrix, n, pivotCol, pivotRow, pivotValue))
		{
			// If we couldn't pivot along the maximum value, we've converged.
			return true;
		}
	}

	// Couldn't converge within the maximum number of sweeps.
	return false;
}

bool dsJacobiEigenvaluesClassicd(double* outEigenvectors, double* outEigenvalues,
	const double* matrix, unsigned int n, unsigned int maxSweeps)
{
	DS_ASSERT(outEigenvectors);
	DS_ASSERT(outEigenvalues);
	DS_ASSERT(matrix);
	DS_ASSERT(n > 1);
	DS_ASSERT(maxSweeps > 0);
	DS_ASSERT(n <= MAX_N);

	// Initialize the output values assuming this is a diagonal matrix.
	double* curMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(double, n*n);
	memcpy(curMatrix, matrix, sizeof(double)*n*n);
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = curMatrix[i*n + i];
		double* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (double)(i == j);
	}

	// Each sweep is a rotation around each non-diagonal, or n*(n - 1)/2 for a symmetric matrix.
	unsigned int maxIterations = n*(n - 1)/2*maxSweeps;
	for (unsigned int iter = 0; iter < maxIterations; ++iter)
	{
		// Find the maximum index to pivot on.
		unsigned int pivotCol = 0, pivotRow = 0;
		double pivotValue = 0.0;
		for (unsigned int i = 0; i < n - 1; ++i)
		{
			const double* column = curMatrix + i*n;
			for (unsigned int j = i + 1; j < n; ++j)
			{
				double thisPivotValue = column[j];
				if (fabs(thisPivotValue) > fabs(pivotValue))
				{
					pivotCol = i;
					pivotRow = j;
					pivotValue = thisPivotValue;
				}
			}
		}

		if (!pivotd(outEigenvectors, outEigenvalues, curMatrix, n, pivotCol, pivotRow, pivotValue))
		{
			// If we couldn't pivot along the maximum value, we've converged.
			return true;
		}
	}

	// Couldn't converge within the maximum number of sweeps.
	return false;
}

bool dsJacobiEigenvaluesCyclicf(float* outEigenvectors, float* outEigenvalues, const float* matrix,
	unsigned int n, unsigned int maxSweeps)
{
	DS_ASSERT(outEigenvectors);
	DS_ASSERT(outEigenvalues);
	DS_ASSERT(matrix);
	DS_ASSERT(n > 1);
	DS_ASSERT(maxSweeps > 0);
	DS_ASSERT(n <= MAX_N);

	// Initialize the output values assuming this is a diagonal matrix.
	float* curMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(float, n*n);
	memcpy(curMatrix, matrix, sizeof(float)*n*n);
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = curMatrix[i*n + i];
		float* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (float)(i == j);
	}

	for (unsigned int sweep = 0; sweep < maxSweeps; ++sweep)
	{
		// Check if we're done based on all non-diagonals being zero.
		float nonDiagonalSum = 0.0f;
		for (unsigned int i = 0; i < n - 1; ++i)
		{
			const float* column = curMatrix + i*n;
			for (unsigned int j = i + 1; j < n; ++j)
				nonDiagonalSum += fabsf(column[j]);
		}
		if (nonDiagonalSum == 0.0f)
			return true;

		// Pivot each non-diagonal sequentially for the sweep.
		for (unsigned int i = 0; i < n - 1; ++i)
		{
			const float* column = curMatrix + i*n;
			for (unsigned int j = i + 1; j < n; ++j)
				pivotf(outEigenvectors, outEigenvalues, curMatrix, n, i, j, column[j]);
		}
	}

	// Couldn't converge within the maximum number of sweeps.
	return false;
}

bool dsJacobiEigenvaluesCyclicd(double* outEigenvectors, double* outEigenvalues,
	const double* matrix, unsigned int n, unsigned int maxSweeps)
{
	DS_ASSERT(outEigenvectors);
	DS_ASSERT(outEigenvalues);
	DS_ASSERT(matrix);
	DS_ASSERT(n > 1);
	DS_ASSERT(maxSweeps > 0);
	DS_ASSERT(n <= MAX_N);

	// Initialize the output values assuming this is a diagonal matrix.
	double* curMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(double, n*n);
	memcpy(curMatrix, matrix, sizeof(double)*n*n);
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = curMatrix[i*n + i];
		double* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (double)(i == j);
	}

	for (unsigned int sweep = 0; sweep < maxSweeps; ++sweep)
	{
		// Check if we're done based on all non-diagonals being zero.
		double nonDiagonalSum = 0.0;
		for (unsigned int i = 0; i < n - 1; ++i)
		{
			const double* column = curMatrix + i*n;
			for (unsigned int j = i + 1; j < n; ++j)
				nonDiagonalSum += fabs(column[j]);
		}
		if (nonDiagonalSum == 0.0)
			return true;

		// Pivot each non-diagonal sequentially for the sweep.
		for (unsigned int i = 0; i < n - 1; ++i)
		{
			const double* column = curMatrix + i*n;
			for (unsigned int j = i + 1; j < n; ++j)
				pivotd(outEigenvectors, outEigenvalues, curMatrix, n, i, j, column[j]);
		}
	}

	// Couldn't converge within the maximum number of sweeps.
	return false;
}

void dsSortEigenvaluesf(float* eigenvectors, float* eigenvalues, unsigned int n)
{
	DS_ASSERT(eigenvectors);
	DS_ASSERT(eigenvalues);
	DS_ASSERT(n <= MAX_N);

	const size_t vectorSize = sizeof(float)*n;
	float* tempVector = DS_ALLOCATE_STACK_OBJECT_ARRAY(float, n);
	for (unsigned int i = 0; i < n; ++i)
	{
		float largestValue = eigenvalues[i];
		unsigned int largestIndex = i;
		for (unsigned int j = i + 1; j < n; ++j)
		{
			float curValue = eigenvalues[j];
			if (curValue > largestValue)
			{
				largestValue = curValue;
				largestIndex = j;
			}
		}

		if (largestIndex > i)
		{
			eigenvalues[largestIndex] = eigenvalues[i];
			eigenvalues[i] = largestValue;

			memcpy(tempVector, eigenvectors + largestIndex*n, vectorSize);
			memcpy(eigenvectors + largestIndex*n, eigenvectors + i*n, vectorSize);
			memcpy(eigenvectors + i*n, tempVector, vectorSize);
		}
	}
}

void dsSortEigenvaluesd(double* eigenvectors, double* eigenvalues, unsigned int n)
{
	DS_ASSERT(eigenvectors);
	DS_ASSERT(eigenvalues);
	DS_ASSERT(n <= MAX_N);

	const size_t vectorSize = sizeof(double)*n;
	double* tempVector = DS_ALLOCATE_STACK_OBJECT_ARRAY(double, n);
	for (unsigned int i = 0; i < n; ++i)
	{
		double largestValue = eigenvalues[i];
		unsigned int largestIndex = i;
		for (unsigned int j = i + 1; j < n; ++j)
		{
			double curValue = eigenvalues[j];
			if (curValue > largestValue)
			{
				largestValue = curValue;
				largestIndex = j;
			}
		}

		if (largestIndex > i)
		{
			eigenvalues[largestIndex] = eigenvalues[i];
			eigenvalues[i] = largestValue;

			memcpy(tempVector, eigenvectors + largestIndex*n, vectorSize);
			memcpy(eigenvectors + largestIndex*n, eigenvectors + i*n, vectorSize);
			memcpy(eigenvectors + i*n, tempVector, vectorSize);
		}
	}
}
