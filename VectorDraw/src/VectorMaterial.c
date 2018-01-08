/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/VectorDraw/VectorMaterial.h>

#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/VectorDraw/Gradient.h>

bool dsVectorMaterial_setColor(dsVectorMaterial* material, dsColor color)
{
	if (!material)
	{
		errno = EINVAL;
		return false;
	}

	material->materialType = dsVectorMaterialType_Color;
	material->color = color;
	return true;
}

bool dsVectorMaterial_setLinearGradient(dsVectorMaterial* material, const dsGradient* gradient,
	const dsVector2f* start, const dsVector2f* end, dsGradientEdge edge,
	dsVectorElementSpace coordinateSpace, const dsMatrix33f* transform)
{
	if (!material || !dsGradient_isValid(gradient) || !start || !end)
	{
		errno = EINVAL;
		return false;
	}

	if (start->x == end->x && start->y == end->y)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
			"Start and end point cannot be the same for a linear gradient.");
		return false;
	}

	material->materialType = dsVectorMaterialType_LinearGradient;
	material->linearGradient.gradient = gradient;
	material->linearGradient.start = *start;
	material->linearGradient.end = *end;
	material->linearGradient.edge = edge;
	material->linearGradient.coordinateSpace = coordinateSpace;
	if (transform)
		material->linearGradient.transform = *transform;
	else
		dsMatrix33_identity(material->linearGradient.transform);
	return true;
}

bool dsVectorMaterial_setRadialGradient(dsVectorMaterial* material, const dsGradient* gradient,
	const dsVector2f* center, float radius, const dsVector2f* focus, float focusRadius,
	dsGradientEdge edge, dsVectorElementSpace coordinateSpace, const dsMatrix33f* transform)
{
	if (!material || !dsGradient_isValid(gradient) || !center || radius <= 0.0f || !focus ||
		focusRadius < 0.0f || focusRadius > 1.0f)
	{
		errno = EINVAL;
		return false;
	}

	material->materialType = dsVectorMaterialType_RadialGradient;
	material->radialGradient.gradient = gradient;
	material->radialGradient.center = *center;
	material->radialGradient.radius = radius;
	material->radialGradient.focus = *focus;
	material->radialGradient.focusRadius = focusRadius;
	material->radialGradient.edge = edge;
	material->radialGradient.coordinateSpace = coordinateSpace;
	if (transform)
		material->radialGradient.transform = *transform;
	else
		dsMatrix33_identity(material->radialGradient.transform);
	return true;
}

const dsGradient* dsVectorMaterial_getGradient(const dsVectorMaterial* material)
{
	if (!material)
		return NULL;

	switch (material->materialType)
	{
		case dsVectorMaterialType_Color:
			return NULL;
		case dsVectorMaterialType_LinearGradient:
			return material->linearGradient.gradient;
		case dsVectorMaterialType_RadialGradient:
			return material->radialGradient.gradient;
		default:
			return NULL;
	}
}

bool dsVectorMaterial_setGradient(dsVectorMaterial* material, const dsGradient* gradient)
{
	if (!material || dsGradient_isValid(gradient))
	{
		errno = EINVAL;
		return false;
	}

	switch (material->materialType)
	{
		case dsVectorMaterialType_LinearGradient:
			material->linearGradient.gradient = gradient;
			return true;
		case dsVectorMaterialType_RadialGradient:
			material->radialGradient.gradient = gradient;
			return true;
		default:
			errno = EPERM;
			return false;
	}
}
