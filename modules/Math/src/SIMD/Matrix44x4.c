/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/Math/SIMD/Matrix44x4.h>

#if DS_HAS_SIMD
void dsMatrix44x4f_load(dsMatrix44x4f* result, const dsMatrix44f* a, const dsMatrix44f* b,
	const dsMatrix44f* c, const dsMatrix44f* d);
void dsMatrix44x4f_store(dsMatrix44f* outA, dsMatrix44f* outB, dsMatrix44f* outC, dsMatrix44f* outD,
	const dsMatrix44x4f* matrices);
void dsMatrix44x4f_store33(dsVector4f* outA, dsVector4f* outB, dsVector4f* outC, dsVector4f* outD,
	const dsMatrix44x4f* matrices);
void dsMatrix44x4f_mul(dsMatrix44x4f* result, const dsMatrix44x4f* a, const dsMatrix44x4f* b);
void dsMatrix44x4f_mulFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a, const dsMatrix44x4f* b);
void dsMatrix44x4f_affineMul(dsMatrix44x4f* result, const dsMatrix44x4f* a, const dsMatrix44x4f* b);
void dsMatrix44x4f_affineMulFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b);
void dsMatrix44x4f_transpose(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_fastInvert(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_fastInvertFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_affineInvert(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_affineInvertFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_invert(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_invertFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_inverseTranspose(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_inverseTransposeFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_invert33(dsMatrix44x4f* result, const dsMatrix44x4f* a);
void dsMatrix44x4f_invert33FMA(dsMatrix44x4f* result, const dsMatrix44x4f* a);
#endif
