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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Math/Export.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to compute eigenvalues for a symmetric matrix using Jacobi iteration.
 */

/**
 * @brief Computes the eigenvalues for a symmetric matrix using Jacobi iteration.
 *
 * This is the "classic" version of the algorithm, which will pivot along the largest values. It
 * will typically have fewer rotations than the cyclic method, but may be slower for larger values
 * of n due to the extra cost to search.
 *
 * @param[out] outEigenvectors The resulting eigenvectors, or basis for the eigenvalues. This must
 *     contain n x n elements.
 * @param[out] outEigenvalues The resulting eigenvalues, or diagonal scales applied to the
 *     eigenvalues. This must contain n elements.
 * @param matrix The symmetric matrix to compute the eigenvalues for. This must contain n x n
 *     elements.
 * @param n The size of the matrix. This should typically be < 10, and will not accept a value >
 *     100.
 * @param maxSweeps The maximum number of sweeps to perform. Each sweep will have n*(n - 1)/2
 *     iterations.
 * @return False if the factors couldn't be found within the maximum number of sweeps.
 */
DS_MATH_EXPORT bool dsJacobiEigenvaluesClassicf(float* outEigenvectors, float* outEigenvalues,
	const float* matrix, unsigned int n, unsigned int maxSweeps);

/** @copydoc dsJacobiEigenvaluesClassicf() */
DS_MATH_EXPORT bool dsJacobiEigenvaluesClassicd(double* outEigenvectors, double* outEigenvalues,
	const double* matrix, unsigned int n, unsigned int maxSweeps);

/**
 * @brief Computes the eigenvalues for a symmetric matrix using Jacobi iteration.
 *
 * This is the "cyclic" version of the algorithm, which will pivot along all non-diagonals in order.
 * This will often have more rotations thant he "classic" version, but may still be faster for larger
 * values of n due to not needing to search for the maximum pivot value.
 *
 * @param[out] outEigenvectors The resulting eigenvectors, or basis for the eigenvalues. This must
 *     contain n x n elements.
 * @param[out] outEigenvalues The resulting eigenvalues, or diagonal scales applied to the
 *     eigenvalues. This must contain n elements.
 * @param matrix The symmetric matrix to compute the eigenvalues for. This must contain n x n
 *     elements.
 * @param n The size of the matrix. This should typically be < 10, and will not accept a value >
 *     100.
 * @param maxSweeps The maximum number of sweeps to perform. Each sweep will have n*(n - 1)/2
 *     iterations.
 * @return False if the factors couldn't be found within the maximum number of sweeps.
 */
DS_MATH_EXPORT bool dsJacobiEigenvaluesCyclicf(float* outEigenvectors, float* outEigenvalues,
	const float* matrix, unsigned int n, unsigned int maxSweeps);

/** @copydoc dsJacobiEigenvaluesCyclicf() */
DS_MATH_EXPORT bool dsJacobiEigenvaluesCyclicd(double* outEigenvectors, double* outEigenvalues,
	const double* matrix, unsigned int n, unsigned int maxSweeps);

/**
 * @brief Sorts the eigenvalues from largest to smallest.
 *
 * The order of eigenvalues may be unstable depending on how many iterations it takes. Sorting the
 * values may give more deterministic results while still preserving the properties of restoring
 * the original matrix by multiplying the eigenvectors and eigenvalues.
 *
 * @param[inout] eigenvectors The eigenvectors. This must contain n x n elements.
 * @param[inout] eigenvalues The eigenvalues. This must contain n elements.
 * @param n The number of eigenvalues.
 */
DS_MATH_EXPORT void dsSortEigenvaluesf(float* eigenvectors, float* eigenvalues, unsigned int n);

/** @copydoc dsSortEigenvaluesf() */
DS_MATH_EXPORT void dsSortEigenvaluesd(double* eigenvectors, double* eigenvalues, unsigned int n);

#ifdef __cplusplus
}
#endif

