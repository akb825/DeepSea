/*
 * Copyright 2021 Aaron Barany
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

#include <DeepSea/Render/ShadowProjection.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>

#define DS_PARALLEL_THRESHOLD 0.001f

// https://www.cg.tuwien.ac.at/research/vr/lispsm/shadows_egsr2004_revised.pdf

static void makeShadowOrtho(dsMatrix44f* result, float left, float right, float bottom,
	float top, float near, float far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	// Rotate the frustum so top is actually near, and near is actually bottom. Half depth
	// influences top/bottom rather than near/far.
	float yMult = invertY ? -1.0f : 1.0f;

	result->values[0][0] = 2/(right - left);
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 0;
	if (halfDepth)
		result->values[1][2] = -1/(top - bottom);
	else
		result->values[1][2] = -2/(top - bottom);
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 2/(near - far)*yMult;
	result->values[2][2] = 0;
	result->values[2][3] = 0;

	result->values[3][0] = (left + right)/(left - right);
	result->values[3][1] = (near + far)/(near - far)*yMult;
	if (halfDepth)
		result->values[3][2] = -top/(bottom - top);
	else
		result->values[3][2] = -(bottom + top)/(bottom - top);
	result->values[3][3] = 1;
}

static void makeShadowFrustum(dsMatrix44f* result, float left, float right, float bottom,
	float top, float near, float far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	// Rotate the frustum so top is actually near, and near is actually bottom. Half depth
	// influences top/bottom rather than near/far.
	float yMult = invertY ? -1.0f : 1.0f;

	result->values[0][0] = 2*near/(right - left);
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 0;
	if (halfDepth)
		result->values[1][2] = -near/(top - bottom);
	else
		result->values[1][2] = -2*near/(top - bottom);
	result->values[1][3] = 0;

	result->values[2][0] = (right + left)/(right - left);
	result->values[2][1] = (near + far)/(near - far)*yMult;
	if (halfDepth)
		result->values[2][2] = -top/(top - bottom);
	else
		result->values[2][2] = -(top + bottom)/(top - bottom);
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 2*near*far/(near - far)*yMult;
	result->values[3][2] = 0;
	result->values[3][3] = 0;
}

bool dsShadowProjection_initialize(dsShadowProjection* shadowProj, const dsRenderer* renderer,
	const dsMatrix44f* camera, const dsVector3f* toLight, const dsMatrix44f* lightProjection,
	bool uniform)
{
	if (!shadowProj || !renderer || !camera || !toLight)
	{
		errno = EINVAL;
		return false;
	}

	dsAlignedBox3f_makeInvalid(&shadowProj->pointBounds);

	dsVector3f viewDir;
	dsVector3f viewDown;
	dsVector3f viewPos;
	dsVector3f lightDir;
	if (lightProjection)
	{
		// When a light projection is provided, perform the computations in projected space.
		shadowProj->hasLightProjection = true;

		dsVector4f proj;
		DS_ASSERT(camera->columns[2].w == 0);
		dsMatrix44_transform(proj, *lightProjection, camera->columns[2]);
		dsVector3f_normalize(&viewDir, (const dsVector3f*)&proj);

		DS_ASSERT(camera->columns[1].w == 0);
		dsMatrix44_transform(proj, *lightProjection, camera->columns[1]);
		dsVector3f_normalize(&viewDown, (const dsVector3f*)&proj);
		dsVector3_neg(viewDown, viewDown);

		DS_ASSERT(camera->columns[3].w == 1);
		dsMatrix44_transform(proj, *lightProjection, camera->columns[2]);
		float invW = 1/proj.w;
		dsVector3_scale(viewPos, proj, invW);

		dsVector4f temp = {{toLight->x, toLight->y, toLight->z, 0.0f}};
		dsMatrix44_transform(proj, *lightProjection, temp);
		dsVector3f_normalize(&lightDir, (const dsVector3f*)&proj);
	}
	else
	{
		shadowProj->hasLightProjection = false;
		viewDir = *(const dsVector3f*)(camera->columns + 2);
		dsVector3_neg(viewDown, camera->columns[1]);
		viewPos = *(const dsVector3f*)(camera->columns + 3);
		lightDir = *toLight;
	}

	// Define the shadow space based on the view position and direction and light.
	// Up direction will always be the light direction.
	shadowProj->shadowSpace.columns[1].x = lightDir.x;
	shadowProj->shadowSpace.columns[1].y = lightDir.y;
	shadowProj->shadowSpace.columns[1].z = lightDir.z;
	shadowProj->shadowSpace.columns[1].w = 0;

	dsVector3f viewCrossLight;
	dsVector3_cross(viewCrossLight, viewDir, lightDir);
	shadowProj->sinViewLight = dsVector3f_len(&viewCrossLight);
	if (shadowProj->sinViewLight <= DS_PARALLEL_THRESHOLD)
	{
		// If the view is looking directlyat the light, use the down direction and fall back to
		// uniform shadows.
		dsVector3_cross(shadowProj->shadowSpace.columns[0], shadowProj->shadowSpace.columns[1],
			viewDown);
		shadowProj->uniform = true;
	}
	else
	{
		dsVector3_cross(shadowProj->shadowSpace.columns[0], shadowProj->shadowSpace.columns[1],
			viewDir);
		shadowProj->uniform = uniform;
	}
	dsVector3f_normalize((dsVector3f*)shadowProj->shadowSpace.columns,
		(const dsVector3f*)shadowProj->shadowSpace.columns);
	shadowProj->shadowSpace.columns[0].w = 0;

	dsVector3_cross(shadowProj->shadowSpace.columns[2], shadowProj->shadowSpace.columns[0],
		shadowProj->shadowSpace.columns[1]);
	dsVector3f_normalize((dsVector3f*)(shadowProj->shadowSpace.columns + 2),
		(const dsVector3f*)(shadowProj->shadowSpace.columns + 2));
	shadowProj->shadowSpace.columns[2].w = 0;

	shadowProj->shadowSpace.columns[3].x = viewPos.x;
	shadowProj->shadowSpace.columns[3].y = viewPos.y;
	shadowProj->shadowSpace.columns[3].z = viewPos.z;
	shadowProj->shadowSpace.columns[3].w = 1;

	// World to shadow space is the inverse. When a light projection matrix is provided, first
	// convert to projected light space.
	if (lightProjection)
	{
		dsMatrix44f shadowSpaceInv;
		dsMatrix44_fastInvert(shadowSpaceInv, shadowProj->shadowSpace);
		dsMatrix44_mul(shadowProj->worldToShadowSpace, shadowSpaceInv, *lightProjection);
	}
	else
		dsMatrix44_fastInvert(shadowProj->worldToShadowSpace, shadowProj->shadowSpace);

	shadowProj->clipHalfDepth = renderer->clipHalfDepth;
	shadowProj->clipInvertY = renderer->clipInvertY;
	return true;
}

bool dsShadowProjection_reset(dsShadowProjection* shadowProj)
{
	if (!shadowProj)
	{
		errno = EINVAL;
		return false;
	}

	dsAlignedBox3f_makeInvalid(&shadowProj->pointBounds);
	return true;
}

bool dsShadowProjection_addPoints(dsShadowProjection* shadowProj, const dsVector3f* points,
	uint32_t pointCount)
{
	if (!shadowProj || (!points && pointCount > 0))
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < pointCount; ++i)
	{
		dsVector4f point = {{points[i].x, points[i].y, points[i].z, 1.0f}};
		dsVector4f proj;
		dsMatrix44_transform(proj, shadowProj->worldToShadowSpace, point);

		float invW = 1/proj.w;
		dsVector3f worldPoint;
		dsVector3_scale(worldPoint, proj, invW);
		dsAlignedBox3_addPoint(shadowProj->pointBounds, worldPoint);
	}

	return true;
}

bool dsShadowProjection_computeMatrix(dsMatrix44f* outMatrix, const dsShadowProjection* shadowProj)
{
	if (!outMatrix || !shadowProj || !dsAlignedBox3_isValid(shadowProj->pointBounds))
		return false;

	const dsAlignedBox3f* bounds = &shadowProj->pointBounds;
	// Frustum looks along negative Z axis, so need to invert Z values.
	float near = -bounds->max.z;
	float far = -bounds->min.z;
	dsMatrix44f projection;
	if (shadowProj->uniform)
	{
		makeShadowOrtho(&projection, bounds->min.x, bounds->max.x, bounds->min.y, bounds->max.y,
			near, far, shadowProj->clipHalfDepth, shadowProj->clipInvertY);
	}
	else
	{
		DS_ASSERT(shadowProj->sinViewLight > DS_PARALLEL_THRESHOLD);

		// Hard-coded near plane to ensure a well-formed frustum.
		// Need to invert
		const float targetNear = 1;
		float zOffset = near;
		float yOffset = -0.5f*(bounds->min.y + bounds->max.y);
		float farDist = far - near;
		float targetFar = targetNear + farDist;

		float n = (targetNear + sqrtf(targetNear*targetFar))/shadowProj->sinViewLight;

		// Take original view point X in shadow space, center Y coordinate. Offset Z to get the desired
		// near plane. Take into account the fact that the frustum is along negative Z.
		dsMatrix44f translate;
		dsMatrix44f_makeTranslate(&translate, 0.0f, yOffset, -n + zOffset);

		float yExtent = bounds->max.y + yOffset;
		float left = bounds->min.x;
		float right = bounds->max.x;
		float top = yExtent;
		float bottom = -top;
		dsMatrix44f frustum;
		makeShadowFrustum(&frustum, left, right, bottom, top, n, n + farDist,
			shadowProj->clipHalfDepth, shadowProj->clipInvertY);
		dsMatrix44_mul(projection, frustum, translate);
	}

	dsMatrix44_mul(*outMatrix, projection, shadowProj->worldToShadowSpace);
	return true;
}
