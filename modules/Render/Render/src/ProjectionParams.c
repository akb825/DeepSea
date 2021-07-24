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

#include <DeepSea/Render/ProjectionParams.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Render/Renderer.h>

bool dsProjectionParams_makeOrtho(dsProjectionParams* params, float left, float right, float bottom,
	float top, float near, float far)
{
	if (!params || left == right || bottom == top || near == far)
	{
		errno = EINVAL;
		return false;
	}

	params->type = dsProjectionType_Ortho;
	params->projectionPlanes.left = left;
	params->projectionPlanes.right = right;
	params->projectionPlanes.bottom = bottom;
	params->projectionPlanes.top = top;
	params->near = near;
	params->far = far;
	return true;
}

bool dsProjectionParams_makeFrustum(dsProjectionParams* params, float left, float right,
	float bottom, float top, float near, float far)
{
	if (!params || left == right || bottom == top || near == far)
	{
		errno = EINVAL;
		return false;
	}

	params->type = dsProjectionType_Frustum;
	params->projectionPlanes.left = left;
	params->projectionPlanes.right = right;
	params->projectionPlanes.bottom = bottom;
	params->projectionPlanes.top = top;
	params->near = near;
	params->far = far;
	return true;
}

bool dsProjectionParams_makePerspective(dsProjectionParams* params, float fovy, float aspect,
	float near, float far)
{
	if (!params || fovy == 0 || aspect == 0|| near == far)
	{
		errno = EINVAL;
		return false;
	}

	params->type = dsProjectionType_Perspective;
	params->perspectiveParams.fovy = fovy;
	params->perspectiveParams.aspect = aspect;
	params->near = near;
	params->far = far;
	return true;
}

bool dsProjectionParams_createMatrix(dsMatrix44f* result, const dsProjectionParams* params,
	const dsRenderer* renderer)
{
	if (!result || !params || !renderer)
	{
		errno = EINVAL;
		return false;
	}

	switch (params->type)
	{
		case dsProjectionType_Ortho:
			return dsRenderer_makeOrtho(result, renderer, params->projectionPlanes.left,
				params->projectionPlanes.right, params->projectionPlanes.bottom,
				params->projectionPlanes.top, params->near, params->far);
		case dsProjectionType_Frustum:
			return dsRenderer_makeFrustum(result, renderer, params->projectionPlanes.left,
				params->projectionPlanes.right, params->projectionPlanes.bottom,
				params->projectionPlanes.top, params->near, params->far);
		case dsProjectionType_Perspective:
			return dsRenderer_makePerspective(result, renderer, params->perspectiveParams.fovy,
				params->perspectiveParams.aspect, params->near, params->far);
	}

	errno = EINVAL;
	return false;
}
