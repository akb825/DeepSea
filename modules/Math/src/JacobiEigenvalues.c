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

/*
 * See https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm for reference.
 *
 * The basic math for creating and applying the rotation is pretty much the same, though the
 * surrounding logic has been improved. Both a classical version (as done by the reference)
 * and cyclic version (which rotates all off-diagonals in order) are provided for the different
 * performance tradeoffs.
 *
 * Possible future improvements if larger values of n than the standard matrix classes are used:
 * - Option to use sparse upper matrix for input, where the size is n*(n + 1)/2 and flattened index
 *   for (col, row) is col*n - (col*(col - 1)/2) + row - col.
 * - Option to skip eigenvectors.
 */

inline static unsigned int flattenedOffDiagonalIndex(unsigned int col, unsigned int row,
	unsigned int n)
{
	DS_ASSERT(col < row);
	unsigned int nextCol = col + 1;
	return col*n - col*nextCol/2 + row - nextCol;
}

inline static void rotateOffDiagonalf(float* offDiagonalMatrix, unsigned int n, float cosRot,
	float sinRot, unsigned int xCol, unsigned int xRow, unsigned int yCol, unsigned int yRow)
{
	unsigned int xIndex = flattenedOffDiagonalIndex(xCol, xRow, n);
	unsigned int yIndex = flattenedOffDiagonalIndex(yCol, yRow, n);
	float x = offDiagonalMatrix[xIndex];
	float y = offDiagonalMatrix[yIndex];
	offDiagonalMatrix[xIndex] = cosRot*x - sinRot*y;
	offDiagonalMatrix[yIndex] = sinRot*x + cosRot*y;
}

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

inline static void rotateOffDiagonald(double* offDiagonalMatrix, unsigned int n, double cosRot,
	double sinRot, unsigned int xCol, unsigned int xRow, unsigned int yCol, unsigned int yRow)
{
	unsigned int xIndex = flattenedOffDiagonalIndex(xCol, xRow, n);
	unsigned int yIndex = flattenedOffDiagonalIndex(yCol, yRow, n);
	double x = offDiagonalMatrix[xIndex];
	double y = offDiagonalMatrix[yIndex];
	offDiagonalMatrix[xIndex] = cosRot*x - sinRot*y;
	offDiagonalMatrix[yIndex] = sinRot*x + cosRot*y;
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

inline static bool pivotf(float* outEigenvectors, float* outEigenvalues, float* offDiagonalMatrix,
	unsigned int n, unsigned int pivotCol, unsigned int pivotRow, unsigned int pivotFlat,
	float pivotValue)
{
	DS_ASSERT(pivotCol < pivotRow);

	// Always clear out the pivot value first in case it's *almost* zero and fails one of the
	// below checks.
	offDiagonalMatrix[pivotFlat] = 0.0f;

	float pivotValue2 = dsPow2(pivotValue);
	// Already pivoted. Check square since it would underflow.
	if (pivotValue2 == 0.0f)
		return false;

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

	// Offset the eigenvalues.
	outEigenvalues[pivotCol] -= eigenOffset;
	outEigenvalues[pivotRow] += eigenOffset;

	// Rotate curMatrix across the pivot axis.
	for (unsigned int i = 0; i < pivotCol; ++i)
		rotateOffDiagonalf(offDiagonalMatrix, n, cosRot, sinRot, i, pivotCol, i, pivotRow);
	for (unsigned int i = pivotCol + 1; i < pivotRow; ++i)
		rotateOffDiagonalf(offDiagonalMatrix, n, cosRot, sinRot, pivotCol, i, i, pivotRow);
	for (unsigned int i = pivotRow + 1; i < n; ++i)
		rotateOffDiagonalf(offDiagonalMatrix, n, cosRot, sinRot, pivotCol, i, pivotRow, i);

	// Rotate the eigenvectors.
	for (unsigned int i = 0; i < n; ++i)
		rotatef(outEigenvectors, n, cosRot, sinRot, pivotCol, i, pivotRow, i);

	return true;
}

inline static bool pivotd(double* outEigenvectors, double* outEigenvalues,
	double* offDiagonalMatrix, unsigned int n, unsigned int pivotCol, unsigned int pivotRow,
	unsigned int pivotFlat, double pivotValue)
{
	// Always clear out the pivot value first in case it's *almost* zero and fails one of the
	// below checks.
	offDiagonalMatrix[pivotFlat] = 0.0;

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

	// Offset the eigenvalues.
	outEigenvalues[pivotCol] -= eigenOffset;
	outEigenvalues[pivotRow] += eigenOffset;

	// Rotate curMatrix across the pivot axis.
	for (unsigned int i = 0; i < pivotCol; ++i)
		rotateOffDiagonald(offDiagonalMatrix, n, cosRot, sinRot, i, pivotCol, i, pivotRow);
	for (unsigned int i = pivotCol + 1; i < pivotRow; ++i)
		rotateOffDiagonald(offDiagonalMatrix, n, cosRot, sinRot, pivotCol, i, i, pivotRow);
	for (unsigned int i = pivotRow + 1; i < n; ++i)
		rotateOffDiagonald(offDiagonalMatrix, n, cosRot, sinRot, pivotCol, i, pivotRow, i);

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

	unsigned int offDiagonalCount = n*(n - 1)/2;

	// Initialize the output values assuming this is a diagonal matrix.
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = matrix[i*n + i];
		float* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (float)(i == j);
	}

	// Copy the off diagonal values in a flattened array so we can modify them during the
	// iterations.
	float* offDiagonalMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(float, offDiagonalCount);
	for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
	{
		const float* column = matrix + i*n;
		for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			offDiagonalMatrix[flatIndex] = column[j];
	}
	bool* unchangedEigenvalues = DS_ALLOCATE_STACK_OBJECT_ARRAY(bool, n);
	memset(unchangedEigenvalues, 0, sizeof(bool)*n);
	unsigned int unchangedCount = 0;

	// Each sweep is a rotation around each non-diagonal, or n*(n - 1)/2 for a symmetric matrix.
	unsigned int maxIterations = offDiagonalCount*maxSweeps;
	for (unsigned int iter = 0; iter < maxIterations; ++iter)
	{
		// Find the maximum index to pivot on.
		unsigned int pivotCol = 0, pivotRow = 0, pivotFlat = 0;
		float pivotValue = 0.0f;
		for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
		{
			for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			{
				float thisPivotValue = offDiagonalMatrix[flatIndex];
				if (fabsf(thisPivotValue) > fabsf(pivotValue))
				{
					pivotCol = i;
					pivotRow = j;
					pivotFlat = flatIndex;
					pivotValue = thisPivotValue;
				}
			}
		}

		float prevRowEigenvalue = outEigenvalues[pivotRow];
		float prevColEigenvalue = outEigenvalues[pivotCol];
		if (!pivotf(outEigenvectors, outEigenvalues, offDiagonalMatrix, n, pivotCol, pivotRow,
				pivotFlat, pivotValue))
		{
			// If we couldn't pivot along the maximum value, we've converged.
			return true;
		}

		// Keep track of which eigenvalues have been set but onchanged. Once the last update for
		// each eigenvalue resulted in no change, we've converged.
		bool rowEigenvalueUnchanged = prevRowEigenvalue == outEigenvalues[pivotRow];
		if (rowEigenvalueUnchanged != unchangedEigenvalues[pivotRow])
		{
			unchangedEigenvalues[pivotRow] = rowEigenvalueUnchanged;
			unchangedCount += rowEigenvalueUnchanged ? 1 : -1;
		}

		bool colEigenvalueUnchanged = prevColEigenvalue == outEigenvalues[pivotCol];
		if (colEigenvalueUnchanged != unchangedEigenvalues[pivotCol])
		{
			unchangedEigenvalues[pivotCol] = colEigenvalueUnchanged;
			unchangedCount += colEigenvalueUnchanged ? 1 : -1;
		}

		if (unchangedCount == n)
			return true;
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

	unsigned int offDiagonalCount = n*(n - 1)/2;

	// Initialize the output values assuming this is a diagonal matrix.
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = matrix[i*n + i];
		double* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (double)(i == j);
	}

	// Copy the off diagonal values in a flattened array so we can modify them during the
	// iterations.
	double* offDiagonalMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(double, offDiagonalCount);
	for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
	{
		const double* column = matrix + i*n;
		for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			offDiagonalMatrix[flatIndex] = column[j];
	}
	bool* unchangedEigenvalues = DS_ALLOCATE_STACK_OBJECT_ARRAY(bool, n);
	memset(unchangedEigenvalues, 0, sizeof(bool)*n);
	unsigned int unchangedCount = 0;

	// Each sweep is a rotation around each non-diagonal, or n*(n - 1)/2 for a symmetric matrix.
	unsigned int maxIterations = offDiagonalCount*maxSweeps;
	for (unsigned int iter = 0; iter < maxIterations; ++iter)
	{
		// Find the maximum index to pivot on.
		unsigned int pivotCol = 0, pivotRow = 0, pivotFlat = 0;
		double pivotValue = 0.0;
		for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
		{
			for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			{
				double thisPivotValue = offDiagonalMatrix[flatIndex];
				if (fabs(thisPivotValue) > fabs(pivotValue))
				{
					pivotCol = i;
					pivotRow = j;
					pivotFlat = flatIndex;
					pivotValue = thisPivotValue;
				}
			}
		}

		double prevRowEigenvalue = outEigenvalues[pivotRow];
		double prevColEigenvalue = outEigenvalues[pivotCol];
		if (!pivotd(outEigenvectors, outEigenvalues, offDiagonalMatrix, n, pivotCol, pivotRow,
				pivotFlat, pivotValue))
		{
			// If we couldn't pivot along the maximum value, we've converged.
			return true;
		}

		// Keep track of which eigenvalues have been set but onchanged. Once the last update for
		// each eigenvalue resulted in no change, we've converged.
		bool rowEigenvalueUnchanged = prevRowEigenvalue == outEigenvalues[pivotRow];
		if (rowEigenvalueUnchanged != unchangedEigenvalues[pivotRow])
		{
			unchangedEigenvalues[pivotRow] = rowEigenvalueUnchanged;
			unchangedCount += rowEigenvalueUnchanged ? 1 : -1;
		}

		bool colEigenvalueUnchanged = prevColEigenvalue == outEigenvalues[pivotCol];
		if (colEigenvalueUnchanged != unchangedEigenvalues[pivotCol])
		{
			unchangedEigenvalues[pivotCol] = colEigenvalueUnchanged;
			unchangedCount += colEigenvalueUnchanged ? 1 : -1;
		}

		if (unchangedCount == n)
			return true;
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

	unsigned int offDiagonalCount = n*(n - 1)/2;

	// Initialize the output values assuming this is a diagonal matrix.
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = matrix[i*n + i];
		float* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (float)(i == j);
	}

	// Copy the off diagonal values in a flattened array so we can modify them during the
	// iterations.
	float* offDiagonalMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(float, offDiagonalCount);
	for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
	{
		const float* column = matrix + i*n;
		for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			offDiagonalMatrix[flatIndex] = column[j];
	}

	float* prevEigenvalues = DS_ALLOCATE_STACK_OBJECT_ARRAY(float, n);
	for (unsigned int sweep = 0; sweep < maxSweeps; ++sweep)
	{
		// Check if we're done based on all non-diagonals being zero. Use square as this will
		// be where the pivoting converges.
		float offDiagonalSum = 0.0f;
		for (unsigned int i = 0; i < offDiagonalCount; ++i)
			offDiagonalSum += dsPow2(offDiagonalMatrix[i]);
		if (offDiagonalSum == 0.0f)
			return true;

		// Pivot each non-diagonal sequentially for the sweep.
		memcpy(prevEigenvalues, outEigenvalues, sizeof(float)*n);
		for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
		{
			for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			{
				pivotf(outEigenvectors, outEigenvalues, offDiagonalMatrix, n, i, j, flatIndex,
					offDiagonalMatrix[flatIndex]);
			}
		}

		// If unchanged we've converged.
		if (memcmp(prevEigenvalues, outEigenvalues, sizeof(float)*n) == 0)
			return true;
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

	unsigned int offDiagonalCount = n*(n - 1)/2;

	// Initialize the output values assuming this is a diagonal matrix.
	for (unsigned int i = 0; i < n; ++i)
	{
		outEigenvalues[i] = matrix[i*n + i];
		double* eigenvectorColumn = outEigenvectors + i*n;
		for (unsigned int j = 0; j < n; ++j)
			eigenvectorColumn[j] = (double)(i == j);
	}

	// Copy the off diagonal values in a flattened array so we can modify them during the
	// iterations.
	double* offDiagonalMatrix = DS_ALLOCATE_STACK_OBJECT_ARRAY(double, offDiagonalCount);
	for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
	{
		const double* column = matrix + i*n;
		for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			offDiagonalMatrix[flatIndex] = column[j];
	}

	double* prevEigenvalues = DS_ALLOCATE_STACK_OBJECT_ARRAY(double, n);
	for (unsigned int sweep = 0; sweep < maxSweeps; ++sweep)
	{
		// Check if we're done based on all non-diagonals being zero.
		double offDiagonalSum = 0.0;
		for (unsigned int i = 0; i < offDiagonalCount; ++i)
			offDiagonalSum += dsPow2(offDiagonalMatrix[i]);
		if (offDiagonalSum == 0.0)
			return true;

		// Pivot each non-diagonal sequentially for the sweep.
		memcpy(prevEigenvalues, outEigenvalues, sizeof(double)*n);
		for (unsigned int i = 0, flatIndex = 0; i < n - 1; ++i)
		{
			for (unsigned int j = i + 1; j < n; ++j, ++flatIndex)
			{
				pivotd(outEigenvectors, outEigenvalues, offDiagonalMatrix, n, i, j, flatIndex,
					offDiagonalMatrix[flatIndex]);
			}
		}

		// If unchanged we've converged.
		if (memcmp(prevEigenvalues, outEigenvalues, sizeof(double)*n) == 0)
			return true;
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
