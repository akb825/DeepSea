/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Math/SIMD/Matrix33xSIMD.h>

#if DS_HAS_SIMD

void dsMatrix33xf_mulSIMD(dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);
void dsMatrix33xf_affineMulSIMD(dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);
void dsMatrix33xf_transformSIMD(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);
void dsMatrix33xf_transformTransposedSIMD(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);
void dsMatrix33xf_transposeSIMD(dsMatrix33xf* result, const dsMatrix33xf* a);
float dsMatrix33xf_determinantSIMD(const dsMatrix33xf* a);
void dsMatrix33xf_fastInvertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xf_affineInvertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xf_invertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xf_inverseTransposeSIMD(dsMatrix22f* result, const dsMatrix33xf* a);

#if !DS_DETERMINISTIC_MATH
void dsMatrix33xf_mulFMA(dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);
void dsMatrix33xf_affineMulFMA(dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);
void dsMatrix33xf_transformFMA(
		dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);
void dsMatrix33xf_transformTransposedFMA(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);
float dsMatrix33xf_determinantFMA(const dsMatrix33xf* a);
void dsMatrix33xf_fastInvertFMA(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xf_affineInvertFMA(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xf_invertFMA(dsMatrix33xf* result, const dsMatrix33xf* a);
#endif // !DS_DETERMINISTIC_MATH

void dsMatrix33xd_mulSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);
void dsMatrix33xd_affineMulSIMD2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);
void dsMatrix33xd_transformSIMD2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);
void dsMatrix33xd_transformTransposedSIMD2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);
void dsMatrix33xd_transposeSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a);
double dsMatrix33xd_determinantSIMD2(const dsMatrix33xd* a);
void dsMatrix33xd_fastInvertSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a);
void dsMatrix33xd_affineInvertSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a);
void dsMatrix33xd_invertSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a);
void dsMatrix33xd_inverseTransposeSIMD2(dsMatrix22d* result, const dsMatrix33xd* a);

#if !DS_DETERMINISTIC_MATH
void dsMatrix33xd_mulFMA2(dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);
void dsMatrix33xd_affineMulFMA2(dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);
void dsMatrix33xd_transformFMA2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);
void dsMatrix33xd_transformTransposedFMA2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);
double dsMatrix33xd_determinantFMA2(const dsMatrix33xd* a);
void dsMatrix33xd_fastInvertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a);
void dsMatrix33xd_affineInvertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a);
void dsMatrix33xd_invertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a);
#endif // !DS_DETERMINISTIC_MATH

void dsMatrix33xd_mulSIMD4(dsMatrix33xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) a, const dsMatrix33xd* DS_ALIGN_PARAM(32) b);
void dsMatrix33xd_affineMulSIMD4(dsMatrix33xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) a, const dsMatrix33xd* DS_ALIGN_PARAM(32) b);
void dsMatrix33xd_transformSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) mat, const dsVector3xd* DS_ALIGN_PARAM(32) vec);
void dsMatrix33xd_transformTransposedSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) mat, const dsVector3xd* DS_ALIGN_PARAM(32) vec);
void dsMatrix33xd_transposeSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);
double dsMatrix33xd_determinantSIMD4(const dsMatrix33xd* DS_ALIGN_PARAM(32) a);
void dsMatrix33xd_fastInvertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);
void dsMatrix33xd_affineInvertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);
void dsMatrix33xd_invertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);
void dsMatrix33xd_inverseTransposeSIMD4(
	dsMatrix22d* result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);

#endif
