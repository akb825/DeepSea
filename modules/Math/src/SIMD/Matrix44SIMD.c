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
void dsMatrix44f_affineMulSIMD(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44f_affineMulFMA(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44f_transformSIMD(dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);
void dsMatrix44f_transformFMA(dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);
void dsMatrix44f_transformTransposedSIMD(dsVector4f* result, const dsMatrix44f* mat,
	const dsVector4f* vec);
void dsMatrix44f_transformTransposedFMA(dsVector4f* result, const dsMatrix44f* mat,
	const dsVector4f* vec);
void dsMatrix44f_transposeSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_fastInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_fastInvertFMA(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_affineInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_affineInvertFMA(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_affineInvert33SIMD(dsVector4f* result, const dsMatrix44f* a);
void dsMatrix44f_affineInvert33FMA(dsVector4f* result, const dsMatrix44f* a);
void dsMatrix44f_invertSIMD(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44f_invertFMA(dsMatrix44f* result, const dsMatrix44f* a);
#endif
