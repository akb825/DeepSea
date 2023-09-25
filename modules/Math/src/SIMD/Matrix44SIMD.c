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
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector4.h>

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4);
void dsMatrix44f_affineLerpSIMD(dsMatrix44f* result, const dsMatrix44f* a,
	const dsMatrix44f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsVector4f dot;
	dot.simd = dsSIMD4f_mul(a->columns[0].simd, a->columns[0].simd);
	float len2x = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(a->columns[1].simd, a->columns[1].simd);
	float len2y = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(a->columns[2].simd, a->columns[2].simd);
	float len2z = dot.x + dot.y + dot.z;

	dsSIMD4f one = dsSIMD4f_set1(1.0f);
	dsSIMD4f scaleA = dsSIMD4f_sqrt(dsSIMD4f_set4(len2x, len2y, len2z, 0.0f));
	dsVector4f invScaleA;
	invScaleA.simd = dsSIMD4f_div(one, scaleA);

	dsMatrix44f rotateMatA;
	rotateMatA.columns[0].simd = dsSIMD4f_mul(a->columns[0].simd, dsSIMD4f_set1(invScaleA.x));
	rotateMatA.columns[1].simd = dsSIMD4f_mul(a->columns[1].simd, dsSIMD4f_set1(invScaleA.y));
	rotateMatA.columns[2].simd = dsSIMD4f_mul(a->columns[2].simd, dsSIMD4f_set1(invScaleA.z));
	rotateMatA.columns[3].simd = dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f);

	dsQuaternion4f quatA;
	dsQuaternion4f_fromMatrix44(&quatA, &rotateMatA);

	dot.simd = dsSIMD4f_mul(b->columns[0].simd, b->columns[0].simd);
	len2x = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(b->columns[1].simd, b->columns[1].simd);
	len2y = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(b->columns[2].simd, b->columns[2].simd);
	len2z = dot.x + dot.y + dot.z;

	dsSIMD4f scaleB = dsSIMD4f_sqrt(dsSIMD4f_set4(len2x, len2y, len2z, 0.0f));
	dsVector4f invScaleB;
	invScaleB.simd = dsSIMD4f_div(one, scaleB);

	dsMatrix44f rotateMatB;
	rotateMatB.columns[0].simd = dsSIMD4f_mul(b->columns[0].simd, dsSIMD4f_set1(invScaleB.x));
	rotateMatB.columns[1].simd = dsSIMD4f_mul(b->columns[1].simd, dsSIMD4f_set1(invScaleB.y));
	rotateMatB.columns[2].simd = dsSIMD4f_mul(b->columns[2].simd, dsSIMD4f_set1(invScaleB.z));
	rotateMatB.columns[3].simd = dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f);

	dsQuaternion4f quatB;
	dsQuaternion4f_fromMatrix44(&quatB, &rotateMatB);

	dsQuaternion4f quatInterp;
	dsQuaternion4f_slerp(&quatInterp, &quatA, &quatB, t);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	dsVector4f scaleInterp;
	scaleInterp.simd = dsSIMD4f_add(scaleA, dsSIMD4f_mul(dsSIMD4f_sub(scaleB, scaleA), t4));

	dsMatrix44f scaleMatInterp;
	dsMatrix44f_makeScale(&scaleMatInterp, scaleInterp.x, scaleInterp.y, scaleInterp.z);

	dsMatrix44f rotateMatInterp;
	dsQuaternion4f_toMatrix44(&rotateMatInterp, &quatInterp);

	dsMatrix44f_affineMulSIMD(result, &rotateMatInterp, &scaleMatInterp);
	result->columns[3].simd = dsSIMD4f_add(a->columns[3].simd,
		dsSIMD4f_mul(dsSIMD4f_sub(b->columns[3].simd, a->columns[3].simd), t4));
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA);
void dsMatrix44f_affineLerpFMA(dsMatrix44f* result, const dsMatrix44f* a,
	const dsMatrix44f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsVector4f dot;
	dot.simd = dsSIMD4f_mul(a->columns[0].simd, a->columns[0].simd);
	float len2x = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(a->columns[1].simd, a->columns[1].simd);
	float len2y = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(a->columns[2].simd, a->columns[2].simd);
	float len2z = dot.x + dot.y + dot.z;

	dsSIMD4f one = dsSIMD4f_set1(1.0f);
	dsSIMD4f scaleA = dsSIMD4f_sqrt(dsSIMD4f_set4(len2x, len2y, len2z, 0.0f));
	dsVector4f invScaleA;
	invScaleA.simd = dsSIMD4f_div(one, scaleA);

	dsMatrix44f rotateMatA;
	rotateMatA.columns[0].simd = dsSIMD4f_mul(a->columns[0].simd, dsSIMD4f_set1(invScaleA.x));
	rotateMatA.columns[1].simd = dsSIMD4f_mul(a->columns[1].simd, dsSIMD4f_set1(invScaleA.y));
	rotateMatA.columns[2].simd = dsSIMD4f_mul(a->columns[2].simd, dsSIMD4f_set1(invScaleA.z));
	rotateMatA.columns[3].simd = dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f);

	dsQuaternion4f quatA;
	dsQuaternion4f_fromMatrix44(&quatA, &rotateMatA);

	dot.simd = dsSIMD4f_mul(b->columns[0].simd, b->columns[0].simd);
	len2x = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(b->columns[1].simd, b->columns[1].simd);
	len2y = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(b->columns[2].simd, b->columns[2].simd);
	len2z = dot.x + dot.y + dot.z;

	dsSIMD4f scaleB = dsSIMD4f_sqrt(dsSIMD4f_set4(len2x, len2y, len2z, 0.0f));
	dsVector4f invScaleB;
	invScaleB.simd = dsSIMD4f_div(one, scaleB);

	dsMatrix44f rotateMatB;
	rotateMatB.columns[0].simd = dsSIMD4f_mul(b->columns[0].simd, dsSIMD4f_set1(invScaleB.x));
	rotateMatB.columns[1].simd = dsSIMD4f_mul(b->columns[1].simd, dsSIMD4f_set1(invScaleB.y));
	rotateMatB.columns[2].simd = dsSIMD4f_mul(b->columns[2].simd, dsSIMD4f_set1(invScaleB.z));
	rotateMatB.columns[3].simd = dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f);

	dsQuaternion4f quatB;
	dsQuaternion4f_fromMatrix44(&quatB, &rotateMatB);

	dsQuaternion4f quatInterp;
	dsQuaternion4f_slerp(&quatInterp, &quatA, &quatB, t);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	dsVector4f scaleInterp;
	scaleInterp.simd = dsSIMD4f_fmadd(dsSIMD4f_sub(scaleB, scaleA), t4, scaleA);

	dsMatrix44f scaleMatInterp;
	dsMatrix44f_makeScale(&scaleMatInterp, scaleInterp.x, scaleInterp.y, scaleInterp.z);

	dsMatrix44f rotateMatInterp;
	dsQuaternion4f_toMatrix44(&rotateMatInterp, &quatInterp);

	dsMatrix44f_affineMulFMA(result, &rotateMatInterp, &scaleMatInterp);
	result->columns[3].simd = dsSIMD4f_fmadd(dsSIMD4f_sub(b->columns[3].simd, a->columns[3].simd),
		t4, a->columns[3].simd);
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_DOUBLE2);
void dsMatrix44d_affineLerpSIMD2(dsMatrix44d* result, const dsMatrix44d* a,
	const dsMatrix44d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsVector2d dotXY;
	dotXY.simd = dsSIMD2d_mul(a->columns[0].simd2[0], a->columns[0].simd2[0]);
	double len2x = dotXY.x + dotXY.y + a->columns[0].z*a->columns[0].z;
	dotXY.simd = dsSIMD2d_mul(a->columns[1].simd2[0], a->columns[1].simd2[0]);
	double len2y = dotXY.x + dotXY.y + a->columns[1].z*a->columns[1].z;
	dotXY.simd = dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0]);
	double len2z = dotXY.x + dotXY.y + a->columns[2].z*a->columns[2].z;

	dsSIMD2d one = dsSIMD2d_set1(1.0);
	dsVector4d scaleA, invScaleA;
	scaleA.simd2[0] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2x, len2y));
	invScaleA.simd2[0] = dsSIMD2d_div(one, scaleA.simd2[0]);
	scaleA.simd2[1] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2z, 0.0));
	invScaleA.simd2[1] = dsSIMD2d_div(one, scaleA.simd2[1]);

	dsMatrix44d rotateMatA;
	dsSIMD2d invScale = dsSIMD2d_set1(invScaleA.x);
	rotateMatA.columns[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale);
	rotateMatA.columns[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleA.y);
	rotateMatA.columns[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale);
	rotateMatA.columns[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleA.z);
	rotateMatA.columns[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale);
	rotateMatA.columns[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale);
	rotateMatA.columns[3].simd2[0] = dsSIMD2d_set2(0.0, 0.0);
	rotateMatA.columns[3].simd2[1] = dsSIMD2d_set2(0.0, 1.0);

	dsQuaternion4d quatA;
	dsQuaternion4d_fromMatrix44(&quatA, &rotateMatA);

	dotXY.simd = dsSIMD2d_mul(b->columns[0].simd2[0], b->columns[0].simd2[0]);
	len2x = dotXY.x + dotXY.y + b->columns[0].z*b->columns[0].z;
	dotXY.simd = dsSIMD2d_mul(b->columns[1].simd2[0], b->columns[1].simd2[0]);
	len2y = dotXY.x + dotXY.y + b->columns[1].z*b->columns[1].z;
	dotXY.simd = dsSIMD2d_mul(b->columns[2].simd2[0], b->columns[2].simd2[0]);
	len2z = dotXY.x + dotXY.y + b->columns[2].z*b->columns[2].z;

	dsVector4d scaleB, invScaleB;
	scaleB.simd2[0] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2x, len2y));
	invScaleB.simd2[0] = dsSIMD2d_div(one, scaleB.simd2[0]);
	scaleB.simd2[1] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2z, 0.0));
	invScaleB.simd2[1] = dsSIMD2d_div(one, scaleB.simd2[1]);

	dsMatrix44d rotateMatB;
	invScale = dsSIMD2d_set1(invScaleB.x);
	rotateMatB.columns[0].simd2[0] = dsSIMD2d_mul(b->columns[0].simd2[0], invScale);
	rotateMatB.columns[0].simd2[1] = dsSIMD2d_mul(b->columns[0].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleB.y);
	rotateMatB.columns[1].simd2[0] = dsSIMD2d_mul(b->columns[1].simd2[0], invScale);
	rotateMatB.columns[1].simd2[1] = dsSIMD2d_mul(b->columns[1].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleB.z);
	rotateMatB.columns[2].simd2[0] = dsSIMD2d_mul(b->columns[2].simd2[0], invScale);
	rotateMatB.columns[2].simd2[1] = dsSIMD2d_mul(b->columns[2].simd2[1], invScale);
	rotateMatB.columns[3].simd2[0] = dsSIMD2d_set2(0.0, 0.0);
	rotateMatB.columns[3].simd2[1] = dsSIMD2d_set2(0.0, 1.0);

	dsQuaternion4d quatB;
	dsQuaternion4d_fromMatrix44(&quatB, &rotateMatB);

	dsQuaternion4d quatInterp;
	dsQuaternion4d_slerp(&quatInterp, &quatA, &quatB, t);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	dsVector4d scaleInterp;
	scaleInterp.simd2[0] = dsSIMD2d_add(scaleA.simd2[0],
		dsSIMD2d_mul(dsSIMD2d_sub(scaleB.simd2[0], scaleA.simd2[0]), t2));
	scaleInterp.simd2[1] = dsSIMD2d_add(scaleA.simd2[1],
		dsSIMD2d_mul(dsSIMD2d_sub(scaleB.simd2[1], scaleA.simd2[1]), t2));

	dsMatrix44d scaleMatInterp;
	dsMatrix44d_makeScale(&scaleMatInterp, scaleInterp.x, scaleInterp.y, scaleInterp.z);

	dsMatrix44d rotateMatInterp;
	dsQuaternion4d_toMatrix44(&rotateMatInterp, &quatInterp);

	dsMatrix44d_affineMulSIMD2(result, &rotateMatInterp, &scaleMatInterp);
	result->columns[3].simd2[0] = dsSIMD2d_add(a->columns[3].simd2[0],
		dsSIMD2d_mul(dsSIMD2d_sub(b->columns[3].simd2[0], a->columns[3].simd2[0]), t2));
	result->columns[3].simd2[1] = dsSIMD2d_add(a->columns[3].simd2[1],
		dsSIMD2d_mul(dsSIMD2d_sub(b->columns[3].simd2[1], a->columns[3].simd2[1]), t2));
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA);
void dsMatrix44d_affineLerpFMA2(dsMatrix44d* result, const dsMatrix44d* a,
	const dsMatrix44d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsVector2d dotXY;
	dotXY.simd = dsSIMD2d_mul(a->columns[0].simd2[0], a->columns[0].simd2[0]);
	double len2x = dotXY.x + dotXY.y + a->columns[0].z*a->columns[0].z;
	dotXY.simd = dsSIMD2d_mul(a->columns[1].simd2[0], a->columns[1].simd2[0]);
	double len2y = dotXY.x + dotXY.y + a->columns[1].z*a->columns[1].z;
	dotXY.simd = dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0]);
	double len2z = dotXY.x + dotXY.y + a->columns[2].z*a->columns[2].z;

	dsSIMD2d one = dsSIMD2d_set1(1.0);
	dsVector4d scaleA, invScaleA;
	scaleA.simd2[0] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2x, len2y));
	invScaleA.simd2[0] = dsSIMD2d_div(one, scaleA.simd2[0]);
	scaleA.simd2[1] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2z, 0.0));
	invScaleA.simd2[1] = dsSIMD2d_div(one, scaleA.simd2[1]);

	dsMatrix44d rotateMatA;
	dsSIMD2d invScale = dsSIMD2d_set1(invScaleA.x);
	rotateMatA.columns[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale);
	rotateMatA.columns[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleA.y);
	rotateMatA.columns[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale);
	rotateMatA.columns[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleA.z);
	rotateMatA.columns[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale);
	rotateMatA.columns[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale);
	rotateMatA.columns[3].simd2[0] = dsSIMD2d_set2(0.0, 0.0);
	rotateMatA.columns[3].simd2[1] = dsSIMD2d_set2(0.0, 1.0);

	dsQuaternion4d quatA;
	dsQuaternion4d_fromMatrix44(&quatA, &rotateMatA);

	dotXY.simd = dsSIMD2d_mul(b->columns[0].simd2[0], b->columns[0].simd2[0]);
	len2x = dotXY.x + dotXY.y + b->columns[0].z*b->columns[0].z;
	dotXY.simd = dsSIMD2d_mul(b->columns[1].simd2[0], b->columns[1].simd2[0]);
	len2y = dotXY.x + dotXY.y + b->columns[1].z*b->columns[1].z;
	dotXY.simd = dsSIMD2d_mul(b->columns[2].simd2[0], b->columns[2].simd2[0]);
	len2z = dotXY.x + dotXY.y + b->columns[2].z*b->columns[2].z;

	dsVector4d scaleB, invScaleB;
	scaleB.simd2[0] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2x, len2y));
	invScaleB.simd2[0] = dsSIMD2d_div(one, scaleB.simd2[0]);
	scaleB.simd2[1] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2z, 0.0));
	invScaleB.simd2[1] = dsSIMD2d_div(one, scaleB.simd2[1]);

	dsMatrix44d rotateMatB;
	invScale = dsSIMD2d_set1(invScaleB.x);
	rotateMatB.columns[0].simd2[0] = dsSIMD2d_mul(b->columns[0].simd2[0], invScale);
	rotateMatB.columns[0].simd2[1] = dsSIMD2d_mul(b->columns[0].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleB.y);
	rotateMatB.columns[1].simd2[0] = dsSIMD2d_mul(b->columns[1].simd2[0], invScale);
	rotateMatB.columns[1].simd2[1] = dsSIMD2d_mul(b->columns[1].simd2[1], invScale);
	invScale = dsSIMD2d_set1(invScaleB.z);
	rotateMatB.columns[2].simd2[0] = dsSIMD2d_mul(b->columns[2].simd2[0], invScale);
	rotateMatB.columns[2].simd2[1] = dsSIMD2d_mul(b->columns[2].simd2[1], invScale);
	rotateMatB.columns[3].simd2[0] = dsSIMD2d_set2(0.0, 0.0);
	rotateMatB.columns[3].simd2[1] = dsSIMD2d_set2(0.0, 1.0);

	dsQuaternion4d quatB;
	dsQuaternion4d_fromMatrix44(&quatB, &rotateMatB);

	dsQuaternion4d quatInterp;
	dsQuaternion4d_slerp(&quatInterp, &quatA, &quatB, t);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	dsVector4d scaleInterp;
	scaleInterp.simd2[0] = dsSIMD2d_fmadd(dsSIMD2d_sub(scaleB.simd2[0], scaleA.simd2[0]), t2,
		scaleA.simd2[0]);
	scaleInterp.simd2[1] = dsSIMD2d_fmadd(dsSIMD2d_sub(scaleB.simd2[1], scaleA.simd2[1]), t2,
		scaleA.simd2[1]);

	dsMatrix44d scaleMatInterp;
	dsMatrix44d_makeScale(&scaleMatInterp, scaleInterp.x, scaleInterp.y, scaleInterp.z);

	dsMatrix44d rotateMatInterp;
	dsQuaternion4d_toMatrix44(&rotateMatInterp, &quatInterp);

	dsMatrix44d_affineMulFMA2(result, &rotateMatInterp, &scaleMatInterp);
	result->columns[3].simd2[0] = dsSIMD2d_fmadd(
		dsSIMD2d_sub(b->columns[3].simd2[0], a->columns[3].simd2[0]), t2, a->columns[3].simd2[0]);
	result->columns[3].simd2[1] = dsSIMD2d_fmadd(
		dsSIMD2d_sub(b->columns[3].simd2[1], a->columns[3].simd2[1]), t2, a->columns[3].simd2[1]);
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_FMA);
void dsMatrix44d_affineLerpFMA4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4d aCol0 = dsSIMD4d_load(a->columns);
	dsSIMD4d aCol1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d aCol2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d aCol3 = dsSIMD4d_load(a->columns + 3);

	DS_ALIGN(32) dsVector4d dot;
	dsSIMD4d_store(&dot, dsSIMD4d_mul(aCol0, aCol0));
	double len2x = dot.x + dot.y + dot.z;
	dsSIMD4d_store(&dot, dsSIMD4d_mul(aCol1, aCol1));
	double len2y = dot.x + dot.y + dot.z;
	dsSIMD4d_store(&dot, dsSIMD4d_mul(aCol2, aCol2));
	double len2z = dot.x + dot.y + dot.z;

	dsSIMD4d one = dsSIMD4d_set1(1.0);
	dsSIMD4d scaleA = dsSIMD4d_sqrt(dsSIMD4d_set4(len2x, len2y, len2z, 0.0));
	DS_ALIGN(32) dsVector4d invScaleA;
	dsSIMD4d_store(&invScaleA, dsSIMD4d_div(one, scaleA));

	DS_ALIGN(32) dsMatrix44d rotateMatA;
	dsSIMD4d_store(rotateMatA.columns, dsSIMD4d_mul(aCol0, dsSIMD4d_set1(invScaleA.x)));
	dsSIMD4d_store(rotateMatA.columns + 1, dsSIMD4d_mul(aCol1, dsSIMD4d_set1(invScaleA.y)));
	dsSIMD4d_store(rotateMatA.columns + 2, dsSIMD4d_mul(aCol2, dsSIMD4d_set1(invScaleA.z)));
	dsSIMD4d_store(rotateMatA.columns + 3, dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0));

	dsQuaternion4d quatA;
	dsQuaternion4d_fromMatrix44(&quatA, &rotateMatA);

	dsSIMD4d bCol0 = dsSIMD4d_load(b->columns);
	dsSIMD4d bCol1 = dsSIMD4d_load(b->columns + 1);
	dsSIMD4d bCol2 = dsSIMD4d_load(b->columns + 2);
	dsSIMD4d bCol3 = dsSIMD4d_load(b->columns + 3);

	dsSIMD4d_store(&dot, dsSIMD4d_mul(bCol0, bCol0));
	len2x = dot.x + dot.y + dot.z;
	dsSIMD4d_store(&dot, dsSIMD4d_mul(bCol1, bCol1));
	len2y = dot.x + dot.y + dot.z;
	dsSIMD4d_store(&dot, dsSIMD4d_mul(bCol2, bCol2));
	len2z = dot.x + dot.y + dot.z;

	dsSIMD4d scaleB = dsSIMD4d_sqrt(dsSIMD4d_set4(len2x, len2y, len2z, 0.0));
	DS_ALIGN(32) dsVector4d invScaleB;
	dsSIMD4d_store(&invScaleB, dsSIMD4d_div(one, scaleB));

	DS_ALIGN(32) dsMatrix44d rotateMatB;
	dsSIMD4d_store(rotateMatB.columns, dsSIMD4d_mul(bCol0, dsSIMD4d_set1(invScaleB.x)));
	dsSIMD4d_store(rotateMatB.columns + 1, dsSIMD4d_mul(bCol1, dsSIMD4d_set1(invScaleB.y)));
	dsSIMD4d_store(rotateMatB.columns + 2, dsSIMD4d_mul(bCol2, dsSIMD4d_set1(invScaleB.z)));
	dsSIMD4d_store(rotateMatB.columns + 3, dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0));

	dsQuaternion4d quatB;
	dsQuaternion4d_fromMatrix44(&quatB, &rotateMatB);

	dsQuaternion4d quatInterp;
	dsQuaternion4d_slerp(&quatInterp, &quatA, &quatB, t);

	dsSIMD4d t4 = dsSIMD4d_set1(t);
	DS_ALIGN(32) dsVector4d scaleInterp;
	dsSIMD4d_store(&scaleInterp, dsSIMD4d_fmadd(dsSIMD4d_sub(scaleB, scaleA), t4, scaleA));

	DS_ALIGN(32) dsMatrix44d scaleMatInterp;
	dsMatrix44d_makeScale(&scaleMatInterp, scaleInterp.x, scaleInterp.y, scaleInterp.z);

	DS_ALIGN(32) dsMatrix44d rotateMatInterp;
	dsQuaternion4d_toMatrix44(&rotateMatInterp, &quatInterp);

	dsMatrix44d_affineMulFMA4(result, &rotateMatInterp, &scaleMatInterp);
	dsSIMD4d_store(result->columns + 3, dsSIMD4d_fmadd(dsSIMD4d_sub(bCol3, aCol3), t4, aCol3));
}
DS_SIMD_END()

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
