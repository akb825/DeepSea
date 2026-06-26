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

#include <DeepSea/Math/RigidTransform3.h>

void dsRigidTransform3f_initialize(dsRigidTransform3f* result,
	const dsVector3xf* position, const dsQuaternion4f* orientation, const dsVector3xf* scale);
void dsRigidTransform3d_initialize(dsRigidTransform3d* result,
	const dsVector3xd* position, const dsQuaternion4d* orientation, const dsVector3xd* scale);

void dsRigidTransform3f_fromMatrix(dsRigidTransform3f* result, const dsMatrix44f* matrix);
void dsRigidTransform3d_fromMatrix(dsRigidTransform3d* result, const dsMatrix44d* matrix);

void dsRigidTransform3f_toMatrix(dsMatrix44f* result, const dsRigidTransform3f* transform);
void dsRigidTransform3d_toMatrix(dsMatrix44d* result, const dsRigidTransform3d* transform);

void dsRigidTransform3f_makeOrientationConsistent(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation);
void dsRigidTransform3d_makeOrientationConsistent(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation);

bool dsRigidTransform3f_isMulValid(const dsRigidTransform3f* a, const dsRigidTransform3f* b);
bool dsRigidTransform3d_isMulValid(const dsRigidTransform3d* a, const dsRigidTransform3d* b);

void dsRigidTransform3f_mul(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b);
void dsRigidTransform3d_mul(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b);

void dsRigidTransform3f_transform(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point);
void dsRigidTransform3d_transform(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point);

void dsRigidTransform3f_lerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);
void dsRigidTransform3d_lerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

void dsRigidTransform3f_nearLerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);
void dsRigidTransform3d_nearLerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

bool dsRigidTransform3f_equal(const dsRigidTransform3f* a, const dsRigidTransform3f* b);
bool dsRigidTransform3d_equal(const dsRigidTransform3d* a, const dsRigidTransform3d* b);

#if DS_HAS_SIMD

void dsRigidTransform3f_fromMatrixSIMD(dsRigidTransform3f* result, const dsMatrix44f* matrix);
void dsRigidTransform3f_toMatrixSIMD(dsMatrix44f* result, const dsRigidTransform3f* transform);
void dsRigidTransform3f_makeOrientationConsistentSIMD(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation);
void dsRigidTransform3f_mulSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b);
void dsRigidTransform3f_transformSIMD(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point);
void dsRigidTransform3f_lerpSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);
void dsRigidTransform3f_nearLerpSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);
bool dsRigidTransform3f_equalSIMD(const dsRigidTransform3f* a, const dsRigidTransform3f* b);

#if !DS_DETERMINISTIC_MATH
void dsRigidTransform3f_toMatrixFMA(dsMatrix44f* result, const dsRigidTransform3f* transform);
void dsRigidTransform3f_makeOrientationConsistentFMA(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation);
void dsRigidTransform3f_mulFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b);
void dsRigidTransform3f_transformFMA(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point);
void dsRigidTransform3f_lerpFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);
void dsRigidTransform3f_nearLerpFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);
#endif // !DS_DETERMINISTIC_MATH

void dsRigidTransform3d_fromMatrixSIMD2(dsRigidTransform3d* result, const dsMatrix44d* matrix);
void dsRigidTransform3d_toMatrixSIMD2(dsMatrix44d* result, const dsRigidTransform3d* transform);
void dsRigidTransform3d_makeOrientationConsistentSIMD2(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation);
void dsRigidTransform3d_mulSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b);
void dsRigidTransform3d_transformSIMD2(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point);
void dsRigidTransform3d_lerpSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);
void dsRigidTransform3d_nearLerpSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);
bool dsRigidTransform3d_equalSIMD2(const dsRigidTransform3d* a, const dsRigidTransform3d* b);

#if !DS_DETERMINISTIC_MATH
void dsRigidTransform3d_toMatrixFMA2(dsMatrix44d* result, const dsRigidTransform3d* transform);
void dsRigidTransform3d_makeOrientationConsistentFMA2(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation);
void dsRigidTransform3d_mulFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b);
void dsRigidTransform3d_transformFMA2(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point);
void dsRigidTransform3d_lerpFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);
void dsRigidTransform3d_nearLerpFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);
#endif // !DS_DETERMINISTIC_MATH

void dsRigidTransform3d_fromMatrixSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix);
void dsRigidTransform3d_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsRigidTransform3d* DS_ALIGN_PARAM(32) transform);
void dsRigidTransform3d_makeOrientationConsistentSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) transform,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) otherOrientation);
void dsRigidTransform3d_mulSIMD4(dsRigidTransform3d* DS_ALIGN_PARAM(32) result,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b);
void dsRigidTransform3d_transformSIMD4(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point);
void dsRigidTransform3d_lerpSIMD4(dsRigidTransform3d* DS_ALIGN_PARAM(32) result,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b,
	double t);
void dsRigidTransform3d_nearLerpSIMD4(dsRigidTransform3d* DS_ALIGN_PARAM(32) result,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b,
	double t);
bool dsRigidTransform3d_equalSIMD4(
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b);

#endif // DS_HAS_SIMD
