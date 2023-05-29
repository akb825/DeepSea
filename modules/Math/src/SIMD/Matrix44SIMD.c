/*
 * Copyright 2022-2023 Aaron Barany
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

#include <DeepSea/Math/SIMD/Matrix44SIMD.h>

#if DS_HAS_SIMD
void dsMatrix44f_mulSIMD(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44f_mulFMA(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44d_mulSIMD2(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);
void dsMatrix44d_mulFMA2(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);
void dsMatrix44d_mulFMA4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b);
void dsMatrix44f_affineMulSIMD(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44f_affineMulFMA(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44d_affineMulSIMD2(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);
void dsMatrix44d_affineMulFMA2(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);
void dsMatrix44d_affineMulFMA4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b);
void dsMatrix44f_transformSIMD(dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);
void dsMatrix44f_transformFMA(dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);
void dsMatrix44d_transformSIMD2(dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec);
void dsMatrix44d_transformFMA2(dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec);
void dsMatrix44d_transformFMA4(dsVector4d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) mat, const dsVector4d* DS_ALIGN_PARAM(32) vec);
void dsMatrix44f_transformTransposedSIMD(dsVector4f* result, const dsMatrix44f* mat,
	const dsVector4f* vec);
void dsMatrix44f_transformTransposedFMA(dsVector4f* result, const dsMatrix44f* mat,
	const dsVector4f* vec);
void dsMatrix44d_transformTransposedSIMD2(dsVector4d* result, const dsMatrix44d* mat,
	const dsVector4d* vec);
void dsMatrix44d_transformTransposedFMA2(dsVector4d* result, const dsMatrix44d* mat,
	const dsVector4d* vec);
void dsMatrix44d_transformTransposedFMA4(dsVector4d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) mat, const dsVector4d* DS_ALIGN_PARAM(32) vec);
void dsMatrix44f_transposeSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_transposeSIMD2(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44d_transposeSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a);
float dsMatrix44f_determinantSIMD(const dsMatrix44f* a);
float dsMatrix44f_determinantFMA(const dsMatrix44f* a);
double dsMatrix44d_determinantSIMD2(const dsMatrix44d* a);
double dsMatrix44d_determinantFMA2(const dsMatrix44d* a);
void dsMatrix44f_fastInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_fastInvertFMA(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_fastInvertSIMD2(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44d_fastInvertFMA2(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44d_fastInvertFMA4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a);
void dsMatrix44f_affineInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_affineInvertFMA(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_affineInvertSIMD2(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44d_affineInvertFMA2(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44d_affineInvertFMA4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a);
void dsMatrix44f_affineInvert33SIMD(dsVector4f result[3], const dsMatrix44f* a);
void dsMatrix44f_affineInvert33FMA(dsVector4f result[3], const dsMatrix44f* a);
void dsMatrix44d_affineInvert33SIMD2(dsVector4d result[3], const dsMatrix44d* a);
void dsMatrix44d_affineInvert33FMA2(dsVector4d result[3], const dsMatrix44d* a);
void dsMatrix44d_affineInvert33FMA4(dsVector4d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a);
void dsMatrix44f_invertSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_invertFMA(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_invertSIMD2(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44d_invertFMA2(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44f_inverseTransposeSIMD(dsVector4f result[3], const dsMatrix44f* a);
void dsMatrix44f_inverseTransposeFMA(dsVector4f result[3], const dsMatrix44f* a);
void dsMatrix44d_inverseTransposeSIMD2(dsVector4d result[3], const dsMatrix44d* a);
void dsMatrix44d_inverseTransposeFMA2(dsVector4d result[3], const dsMatrix44d* a);
void dsMatrix44d_inverseTransposeFMA4(dsVector4d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a);
#endif
