/*
 * Copyright 2020-2024 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneLight.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Geometry/OrientedBox3.h>

#include <DeepSea/Math/SIMD/SIMD.h>
#include <DeepSea/Math/Color.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Packing.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <float.h>
#include <limits.h>
#include <string.h>

static void spotPerpAxes(dsVector3f* outX, dsVector3f* outY, const dsSceneLight* light)
{
	const float epsilon = 1e-6f;
	dsVector3f z;
	dsVector3_neg(z, light->direction);
	if (dsEpsilonEqualsZerof(z.x, epsilon) && dsEpsilonEqualsZerof(z.z, epsilon))
	{
		outY->x = 1.0;
		outY->y = 0.0;
		outY->z = 0.0;
	}
	else
	{
		outY->x = 0.0;
		outY->y = 1.0;
		outY->z = 0.0;
	}

	dsVector3_cross(*outX, *outY, z);
	dsVector3f_normalize(outX, outX);

	dsVector3_cross(*outY, z, *outX);
	dsVector3f_normalize(outY, outY);
}

static inline float getLightIntensity(const dsSceneLight* light)
{
	float maxRG = dsMax(light->color.r, light->color.g);
	return light->intensity*dsMax(maxRG, light->color.b);
}

static float getLightRadius(const dsSceneLight* light, float intensityThreshold)
{
	float intensity = getLightIntensity(light);
	if (intensity < intensityThreshold)
		return 0.0f;

	/*
	 * target = intensity/(1 + linear*distance + quadratic*distance^2)
	 * 1 + linear*distance + quadratic*distance^2 = intenisty/target
	 * quadratic*distance^2 + linear*distance + (1 - intenisty/target) = 0;
	 */
	float a = light->quadraticFalloff;
	float b = light->linearFalloff;
	float c = 1.0f - intensity/intensityThreshold;

	float innerRoot = dsPow2(b) - 4.0f*a*c;
	if (innerRoot < 0)
		return 0.0f;

	// Guaranteed that a factor is > 0, so only need to check "+" and not "-" of quadratic formula.
	// (the "-" factor will always be < 0)
	float root = sqrtf(innerRoot);
	return (-b + root)/(2.0f*a);
}

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_HALF_FLOAT)
static inline void packAmbientSIMD(dsHalfFloat* result, const dsVector3f* ambient)
{
	dsSIMD4hf_store4(result,
		dsSIMD4hf_fromFloat(dsSIMD4f_set4(ambient->x, ambient->y, ambient->z, 0.0f)));
}

static inline void packLightColorSIMD(dsHalfFloat* result, const dsSceneLight* light)
{
	dsSIMD4f color = dsSIMD4f_set4(light->color.x, light->color.y, light->color.z, 0.0f);
	dsSIMD4hf_store4(result,
		dsSIMD4hf_fromFloat(dsSIMD4f_mul(color, dsSIMD4f_set1(light->intensity))));
}

static inline void packLightSphereFalloffSIMD(dsHalfFloat* result, const dsSceneLight* light)
{
	dsSIMD4hf_store2(result, dsSIMD4hf_fromFloat(
		dsSIMD4f_set4(light->linearFalloff, light->quadraticFalloff, 0.0f, 0.0f)));
}

static inline void packLightSpotFalloffSIMD(dsHalfFloat* result, const dsSceneLight* light)
{
	dsSIMD4hf_store4(result, dsSIMD4hf_fromFloat(dsSIMD4f_set4(light->linearFalloff,
		light->quadraticFalloff, light->innerSpotCosAngle, light->outerSpotCosAngle)));
}
DS_SIMD_END()
#endif // DS_HAS_SIMD

static inline void packLightColor(dsHalfFloat* result, const dsSceneLight* light)
{
	result[0] = dsPackHalfFloat(light->color.r*light->intensity);
	result[1] = dsPackHalfFloat(light->color.g*light->intensity);
	result[2] = dsPackHalfFloat(light->color.b*light->intensity);
	result[3].data = 0;
}

bool dsSceneLight_getAmbientLightVertexFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_SNorm);
	outFormat->elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));

	return true;
}

bool dsSceneLight_getDirectionalLightVertexFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_SNorm);
	outFormat->elements[dsVertexAttrib_Normal].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SNorm);
	outFormat->elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Normal, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));

	return true;
}

bool dsSceneLight_getPointLightVertexFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position0].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Position1].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));

	return true;
}

bool dsSceneLight_getSpotLightVertexFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position0].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Position1].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Normal].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SNorm);
	outFormat->elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Normal, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));

	return true;
}

bool dsSceneLight_makeDirectional(dsSceneLight* outLight, const dsVector3f* direction,
	const dsColor3f* color, float intensity)
{
	if (!outLight || !direction || !color)
	{
		errno = EINVAL;
		return false;
	}

	outLight->type = dsSceneLightType_Directional;
	memset(&outLight->position, 0, sizeof(outLight->position));
	outLight->direction = *direction;
	outLight->color = *color;
	outLight->intensity = intensity;
	outLight->linearFalloff = 0;
	outLight->quadraticFalloff = 0;
	outLight->innerSpotCosAngle = 0;
	outLight->outerSpotCosAngle = 0;
	return true;
}

bool dsSceneLight_makePoint(dsSceneLight* outLight, const dsVector3f* position,
	const dsColor3f* color, float intensity, float linearFalloff, float quadraticFalloff)
{
	if (!outLight || !position || !color || linearFalloff < 0 || quadraticFalloff < 0)
	{
		errno = EINVAL;
		return false;
	}

	outLight->type = dsSceneLightType_Point;
	outLight->position = *position;
	memset(&outLight->direction, 0, sizeof(outLight->direction));
	outLight->color = *color;
	outLight->intensity = intensity;
	outLight->linearFalloff = linearFalloff;
	outLight->quadraticFalloff = quadraticFalloff;
	outLight->innerSpotCosAngle = 0;
	outLight->outerSpotCosAngle = 0;
	return true;
}

bool dsSceneLight_makeSpot(dsSceneLight* outLight, const dsVector3f* position,
	const dsVector3f* direction, const dsColor3f* color, float intensity, float linearFalloff,
	float quadraticFalloff, float innerSpotCosAngle, float outerSpotCosAngle)
{
	if (!outLight || !position || !direction || !color || innerSpotCosAngle < outerSpotCosAngle ||
		linearFalloff < 0 || quadraticFalloff < 0)
	{
		errno = EINVAL;
		return false;
	}

	outLight->type = dsSceneLightType_Spot;
	outLight->position = *position;
	outLight->direction = *direction;
	outLight->color = *color;
	outLight->intensity = intensity;
	outLight->linearFalloff = linearFalloff;
	outLight->quadraticFalloff = quadraticFalloff;
	outLight->innerSpotCosAngle = innerSpotCosAngle;
	outLight->outerSpotCosAngle = outerSpotCosAngle;
	return true;
}

float dsSceneLight_getFalloff(const dsSceneLight* light, const dsVector3f* position)
{
	DS_ASSERT(light);
	DS_ASSERT(position);

	if (light->type == dsSceneLightType_Directional)
		return 1.0f;

	dsVector3f direction;
	dsVector3_sub(direction, *position, light->position);
	float distance = dsVector3f_len(&direction);
	float distanceFalloff =
		1/(1.0f + light->linearFalloff*distance + light->quadraticFalloff*dsPow2(distance));
	if (light->type == dsSceneLightType_Point)
		return distanceFalloff;

	DS_ASSERT(light->type == dsSceneLightType_Spot);
	const float epsilon = 1e-6f;
	if (distance < epsilon)
		return distanceFalloff;

	dsVector3_scale(direction, direction, 1.0f/distance);
	float cosAngle = dsVector3_dot(direction, light->direction);

	// Inner cos angle should be larger than the cos outer angle.
	if (cosAngle >= light->innerSpotCosAngle)
		return distanceFalloff;
	else if (cosAngle < light->outerSpotCosAngle)
		return 0.0f;

	float spotFalloff =
		(cosAngle - light->outerSpotCosAngle)/(light->innerSpotCosAngle - light->outerSpotCosAngle);
	return spotFalloff*distanceFalloff;
}

float dsSceneLight_getIntensity(const dsSceneLight* light, const dsVector3f* position)
{
	DS_ASSERT(light);
	DS_ASSERT(position);
	return dsSceneLight_getFalloff(light, position)*getLightIntensity(light);
}

bool dsSceneLight_computeBounds(dsAlignedBox3f* outBounds, const dsSceneLight* light,
	float intensityThreshold)
{
	if (!outBounds || !light || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	if (light->type == dsSceneLightType_Directional)
	{
		if (getLightIntensity(light) >= intensityThreshold)
		{
			outBounds->min.x = -FLT_MAX;
			outBounds->min.y = -FLT_MAX;
			outBounds->min.z = -FLT_MAX;
			outBounds->max.x = FLT_MAX;
			outBounds->max.y = FLT_MAX;
			outBounds->max.z = FLT_MAX;
		}
		else
			dsAlignedBox3f_makeInvalid(outBounds);
		return true;
	}

	float radius = getLightRadius(light, intensityThreshold);
	if (radius <= 0)
	{
		dsAlignedBox3f_makeInvalid(outBounds);
		return true;
	}

	if (light->type == dsSceneLightType_Point)
	{
		dsVector3f sizeOffset = {{radius, radius, radius}};
		dsVector3_sub(outBounds->min, light->position, sizeOffset);
		dsVector3_add(outBounds->max, light->position, sizeOffset);
		return true;
	}

	DS_ASSERT(light->type == dsSceneLightType_Spot);

	// Create an orthonormal basis around the spotlight.
	dsVector3f spotX, spotY;
	spotPerpAxes(&spotX, &spotY, light);

	outBounds->min = outBounds->max = light->position;

	// Middle of the spotlight where it ends.
	dsVector3f middlePos;
	dsVector3_scale(middlePos, light->direction, radius);
	dsVector3_add(middlePos, middlePos, light->position);

	// cos of the angle multiplied by the distance is how far to extend by the X and Y perpendicular
	// axes for the spot light.
	float spotEndDist = light->outerSpotCosAngle*radius;
	dsVector3_scale(spotX, spotX, spotEndDist);
	dsVector3_scale(spotY, spotY, spotEndDist);

	dsVector3f extremeX, extremePos;
	dsVector3_sub(extremeX, middlePos, spotX);
	dsVector3_sub(extremePos, extremeX, spotY);
	dsAlignedBox3_addPoint(*outBounds, extremePos);

	dsVector3_add(extremePos, extremeX, spotY);
	dsAlignedBox3_addPoint(*outBounds, extremePos);

	dsVector3_add(extremeX, middlePos, spotX);
	dsVector3_sub(extremePos, extremeX, spotY);
	dsAlignedBox3_addPoint(*outBounds, extremePos);

	dsVector3_add(extremePos, extremeX, spotY);
	dsAlignedBox3_addPoint(*outBounds, extremePos);
	return true;
}

bool dsSceneLight_isInFrustum(const dsSceneLight* light, const dsFrustum3f* frustum,
	float intensityThreshold)
{
	if (!light || !frustum)
		return false;

	if (intensityThreshold <= 0)
		return true;

	switch (light->type)
	{
		case dsSceneLightType_Directional:
			return getLightIntensity(light) >= intensityThreshold;
		case dsSceneLightType_Point:
		{
			float radius = getLightRadius(light, intensityThreshold);
			if (radius <= 0)
				return false;

			return dsFrustum3f_intersectSphere(frustum, &light->position, radius) !=
				dsIntersectResult_Outside;
		}
		case dsSceneLightType_Spot:
		{
			// Use an oriented box as the simplest approximation.
			dsOrientedBox3f bounds;
			float radius = getLightRadius(light, intensityThreshold);
			if (radius <= 0)
				return false;

			spotPerpAxes(bounds.orientation.columns, bounds.orientation.columns + 1, light);
			dsVector3_neg(bounds.orientation.columns[2], light->direction);

			bounds.halfExtents.z = radius*0.5f;
			dsVector3_scale(bounds.center, light->direction, bounds.halfExtents.z);
			dsVector3_add(bounds.center, bounds.center, light->position);

			// sin of the angle multiplied by the distance is how far to extend by the X and Y
			// perpendicular axes for the spot light.
			float outerSinAngle = sqrtf(1.0f - dsPow2(light->outerSpotCosAngle));
			bounds.halfExtents.x = bounds.halfExtents.y = radius*outerSinAngle;

			return dsFrustum3f_intersectOrientedBox(frustum, &bounds) !=  dsIntersectResult_Outside;
		}
		default:
			DS_ASSERT(false);
			return false;
	}
}

bool dsSceneLight_getPointLightTransform(dsMatrix44f* result, const dsSceneLight* light,
	dsCubeFace cubeFace)
{
	if (!result || !light || light->type != dsSceneLightType_Point || cubeFace < dsCubeFace_PosX ||
		cubeFace > dsCubeFace_NegZ)
	{
		errno = EINVAL;
		return false;
	}

	dsVector4f lightWorldPos = {{light->position.x, light->position.y, light->position.z, 1.0f}};

	dsMatrix44f lightWorld;
	DS_VERIFY(dsTexture_cubeOrientation(&lightWorld, cubeFace));
	lightWorld.columns[3] = lightWorldPos;

	dsMatrix44f_fastInvert(result, &lightWorld);
	return true;
}

bool dsSceneLight_getPointLightProjection(dsMatrix44f* result,
	const dsSceneLight* light, const dsRenderer* renderer, float intensityThreshold)
{
	if (!result || !light || light->type != dsSceneLightType_Point || !renderer ||
		intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	float distance = getLightRadius(light, intensityThreshold);
	float near = 0.1f;
	if (near >= distance)
		near = distance*0.5f;

	DS_VERIFY(dsRenderer_makePerspective(result, renderer, M_PI_2f, 1.0f, near, distance));
	return true;
}

bool dsSceneLight_getSpotLightTransform(dsMatrix44f* result, const dsSceneLight* light)
{
	if (!result || !light || light->type != dsSceneLightType_Spot)
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix44f lightWorld;
	spotPerpAxes((dsVector3f*)lightWorld.columns, (dsVector3f*)(lightWorld.columns + 1), light);
	dsVector3_neg(lightWorld.columns[2], light->direction);
	*(dsVector3f*)(lightWorld.values + 3) = light->position;

	lightWorld.values[0][3] = 0.0f;
	lightWorld.values[1][3] = 0.0f;
	lightWorld.values[2][3] = 0.0f;
	lightWorld.values[3][3] = 1.0f;

	dsMatrix44f_fastInvert(result, &lightWorld);
	return true;
}

bool dsSceneLight_getSpotLightProjection(dsMatrix44f* result, const dsSceneLight* light,
	const dsRenderer* renderer, float intensityThreshold)
{
	if (!result || !light || light->type != dsSceneLightType_Spot || !renderer ||
		intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	float distance = getLightRadius(light, intensityThreshold);
	float near = 0.1f;
	if (near >= distance)
		near = distance*0.5f;

	float outerSpotAngle = acosf(light->outerSpotCosAngle)*2;
	DS_VERIFY(dsRenderer_makePerspective(result, renderer, outerSpotAngle, 1.0f, near, distance));
	return true;
}

bool dsSceneLight_getAmbientLightVertices(dsAmbientLightVertex* outVertices,
	uint32_t vertexCount, uint16_t* outIndices, uint32_t indexCount, const dsColor3f* ambient,
	uint16_t firstIndex)
{
	if (!outVertices || !outIndices || !ambient)
	{
		errno = EINVAL;
		return false;
	}

	if (vertexCount < DS_AMBIENT_LIGHT_VERTEX_COUNT ||
		indexCount < DS_AMBIENT_LIGHT_INDEX_COUNT)
	{
		errno = EINDEX;
		return false;
	}

	if (firstIndex + DS_AMBIENT_LIGHT_VERTEX_COUNT > UINT16_MAX)
	{
		errno = ERANGE;
		return false;
	}

	dsHalfFloat color[4];
	_Static_assert(sizeof(color) == sizeof(outVertices->color), "Unexpected color size.");
#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_HALF_FLOAT || (dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat))
		packAmbientSIMD(color, ambient);
	else
#endif
	{
		color[0] = dsPackHalfFloat(ambient->r);
		color[1] = dsPackHalfFloat(ambient->g);
		color[2] = dsPackHalfFloat(ambient->b);
		color[3].data = 0;
	}

	outVertices[0].position[0] = (int16_t)0x8001;
	outVertices[0].position[1] = (int16_t)0x8001;
	memcpy(outVertices[0].color, color, sizeof(color));

	outVertices[1].position[0] = (int16_t)0x7FFF;
	outVertices[1].position[1] = (int16_t)0x8001;
	memcpy(outVertices[1].color, color, sizeof(color));

	outVertices[2].position[0] = (int16_t)0x7FFF;
	outVertices[2].position[1] = (int16_t)0x7FFF;
	memcpy(outVertices[2].color, color, sizeof(color));

	outVertices[3].position[0] = (int16_t)0x8001;
	outVertices[3].position[1] = (int16_t)0x7FFF;
	memcpy(outVertices[3].color, color, sizeof(color));

	outIndices[0] = (uint16_t)(firstIndex + 0);
	outIndices[1] = (uint16_t)(firstIndex + 1);
	outIndices[2] = (uint16_t)(firstIndex + 2);

	outIndices[3] = (uint16_t)(firstIndex + 0);
	outIndices[4] = (uint16_t)(firstIndex + 2);
	outIndices[5] = (uint16_t)(firstIndex + 3);

	return true;
}

bool dsSceneLight_getDirectionalLightVertices(dsDirectionalLightVertex* outVertices,
	uint32_t vertexCount, uint16_t* outIndices, uint32_t indexCount, const dsSceneLight* light,
	uint16_t firstIndex)
{
	if (!outVertices || !outIndices || !light || light->type != dsSceneLightType_Directional)
	{
		errno = EINVAL;
		return false;
	}

	if (vertexCount < DS_DIRECTIONAL_LIGHT_VERTEX_COUNT ||
		indexCount < DS_DIRECTIONAL_LIGHT_INDEX_COUNT)
	{
		errno = EINDEX;
		return false;
	}

	if (firstIndex + DS_DIRECTIONAL_LIGHT_VERTEX_COUNT > UINT16_MAX)
	{
		errno = ERANGE;
		return false;
	}

	dsDirectionalLightVertex commonData;
	commonData.direction[0] = dsPackInt16(-light->direction.x);
	commonData.direction[1] = dsPackInt16(-light->direction.y);
	commonData.direction[2] = dsPackInt16(-light->direction.z);
	commonData.direction[3] = 0;

#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_HALF_FLOAT || (dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat))
		packLightColorSIMD(commonData.color, light);
	else
#endif
		packLightColor(commonData.color, light);

	const size_t commonDataSize = sizeof(dsDirectionalLightVertex) -
		offsetof(dsDirectionalLightVertex, direction);

	outVertices[0].position[0] = (int16_t)0x8001;
	outVertices[0].position[1] = (int16_t)0x8001;
	memcpy(outVertices[0].direction, commonData.direction, commonDataSize);

	outVertices[1].position[0] = (int16_t)0x7FFF;
	outVertices[1].position[1] = (int16_t)0x8001;
	memcpy(outVertices[1].direction, commonData.direction, commonDataSize);

	outVertices[2].position[0] = (int16_t)0x7FFF;
	outVertices[2].position[1] = (int16_t)0x7FFF;
	memcpy(outVertices[2].direction, commonData.direction, commonDataSize);

	outVertices[3].position[0] = (int16_t)0x8001;
	outVertices[3].position[1] = (int16_t)0x7FFF;
	memcpy(outVertices[3].direction, commonData.direction, commonDataSize);

	outIndices[0] = (uint16_t)(firstIndex + 0);
	outIndices[1] = (uint16_t)(firstIndex + 1);
	outIndices[2] = (uint16_t)(firstIndex + 2);

	outIndices[3] = (uint16_t)(firstIndex + 0);
	outIndices[4] = (uint16_t)(firstIndex + 2);
	outIndices[5] = (uint16_t)(firstIndex + 3);

	return true;
}

bool dsSceneLight_getPointLightVertices(
	dsPointLightVertex* outVertices, uint32_t vertexCount, uint16_t* outIndices,
	uint32_t indexCount, const dsSceneLight* light, float intensityThreshold, uint16_t firstIndex)
{
	if (!outVertices || !outIndices || !light || light->type != dsSceneLightType_Point ||
		intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	if (vertexCount < DS_POINT_LIGHT_VERTEX_COUNT || indexCount < DS_POINT_LIGHT_INDEX_COUNT)
	{
		errno = EINDEX;
		return false;
	}

	if (firstIndex + DS_POINT_LIGHT_VERTEX_COUNT > UINT16_MAX)
	{
		errno = ERANGE;
		return false;
	}

	float radius = getLightRadius(light, intensityThreshold);
	if (radius <= 0)
	{
		errno = EINVAL;
		return false;
	}

	dsAlignedBox3f bounds;
	dsVector3f sizeOffset = {{radius, radius, radius}};
	dsVector3_sub(bounds.min, light->position, sizeOffset);
	dsVector3_add(bounds.max, light->position, sizeOffset);

	dsPointLightVertex commonData;
	commonData.lightPosition = light->position;

#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_HALF_FLOAT || (dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat))
	{
		packLightColorSIMD(commonData.color, light);
		packLightSphereFalloffSIMD(commonData.falloff, light);
	}
	else
#endif
	{
		packLightColor(commonData.color, light);
		commonData.falloff[0] = dsPackHalfFloat(light->linearFalloff);
		commonData.falloff[1] = dsPackHalfFloat(light->quadraticFalloff);
	}

	const size_t commonDataSize = sizeof(dsPointLightVertex) -
		offsetof(dsPointLightVertex, lightPosition);

	outVertices[0].vertexPosition.x = bounds.min.x;
	outVertices[0].vertexPosition.y = bounds.min.y;
	outVertices[0].vertexPosition.z = bounds.min.z;
	memcpy(&outVertices[0].lightPosition, &commonData.lightPosition, commonDataSize);

	outVertices[1].vertexPosition.x = bounds.min.x;
	outVertices[1].vertexPosition.y = bounds.min.y;
	outVertices[1].vertexPosition.z = bounds.max.z;
	memcpy(&outVertices[1].lightPosition, &commonData.lightPosition, commonDataSize);

	outVertices[2].vertexPosition.x = bounds.min.x;
	outVertices[2].vertexPosition.y = bounds.max.y;
	outVertices[2].vertexPosition.z = bounds.min.z;
	memcpy(&outVertices[2].lightPosition, &commonData.lightPosition, commonDataSize);

	outVertices[3].vertexPosition.x = bounds.min.x;
	outVertices[3].vertexPosition.y = bounds.max.y;
	outVertices[3].vertexPosition.z = bounds.max.z;
	memcpy(&outVertices[3].lightPosition, &commonData.lightPosition, commonDataSize);

	outVertices[4].vertexPosition.x = bounds.max.x;
	outVertices[4].vertexPosition.y = bounds.min.y;
	outVertices[4].vertexPosition.z = bounds.min.z;
	memcpy(&outVertices[4].lightPosition, &commonData.lightPosition, commonDataSize);

	outVertices[5].vertexPosition.x = bounds.max.x;
	outVertices[5].vertexPosition.y = bounds.min.y;
	outVertices[5].vertexPosition.z = bounds.max.z;
	memcpy(&outVertices[5].lightPosition, &commonData.lightPosition, commonDataSize);

	outVertices[6].vertexPosition.x = bounds.max.x;
	outVertices[6].vertexPosition.y = bounds.max.y;
	outVertices[6].vertexPosition.z = bounds.min.z;
	memcpy(&outVertices[6].lightPosition, &commonData.lightPosition, commonDataSize);

	outVertices[7].vertexPosition.x = bounds.max.x;
	outVertices[7].vertexPosition.y = bounds.max.y;
	outVertices[7].vertexPosition.z = bounds.max.z;
	memcpy(&outVertices[7].lightPosition, &commonData.lightPosition, commonDataSize);

	// front
	outIndices[0] = (uint16_t)(firstIndex + 5);
	outIndices[1] = (uint16_t)(firstIndex + 1);
	outIndices[2] = (uint16_t)(firstIndex + 3);

	outIndices[3] = (uint16_t)(firstIndex + 5);
	outIndices[4] = (uint16_t)(firstIndex + 3);
	outIndices[5] = (uint16_t)(firstIndex + 7);

	// right
	outIndices[6] = (uint16_t)(firstIndex + 4);
	outIndices[7] = (uint16_t)(firstIndex + 5);
	outIndices[8] = (uint16_t)(firstIndex + 7);

	outIndices[9] = (uint16_t)(firstIndex + 4);
	outIndices[10] = (uint16_t)(firstIndex + 7);
	outIndices[11] = (uint16_t)(firstIndex + 6);

	// back
	outIndices[12] = (uint16_t)(firstIndex + 0);
	outIndices[13] = (uint16_t)(firstIndex + 4);
	outIndices[14] = (uint16_t)(firstIndex + 6);

	outIndices[15] = (uint16_t)(firstIndex + 0);
	outIndices[16] = (uint16_t)(firstIndex + 6);
	outIndices[17] = (uint16_t)(firstIndex + 2);

	// left
	outIndices[18] = (uint16_t)(firstIndex + 0);
	outIndices[19] = (uint16_t)(firstIndex + 2);
	outIndices[20] = (uint16_t)(firstIndex + 3);

	outIndices[21] = (uint16_t)(firstIndex + 0);
	outIndices[22] = (uint16_t)(firstIndex + 3);
	outIndices[23] = (uint16_t)(firstIndex + 1);

	// bottom
	outIndices[24] = (uint16_t)(firstIndex + 0);
	outIndices[25] = (uint16_t)(firstIndex + 1);
	outIndices[26] = (uint16_t)(firstIndex + 5);

	outIndices[27] = (uint16_t)(firstIndex + 0);
	outIndices[28] = (uint16_t)(firstIndex + 5);
	outIndices[29] = (uint16_t)(firstIndex + 4);

	// top
	outIndices[30] = (uint16_t)(firstIndex + 2);
	outIndices[31] = (uint16_t)(firstIndex + 6);
	outIndices[32] = (uint16_t)(firstIndex + 7);

	outIndices[33] = (uint16_t)(firstIndex + 2);
	outIndices[34] = (uint16_t)(firstIndex + 7);
	outIndices[35] = (uint16_t)(firstIndex + 3);

	return true;
}

bool dsSceneLight_getSpotLightVertices(dsSpotLightVertex* outVertices, uint32_t vertexCount,
	uint16_t* outIndices, uint32_t indexCount, const dsSceneLight* light, float intensityThreshold,
	uint16_t firstIndex)
{
	if (!outVertices || !outIndices || !light || light->type != dsSceneLightType_Spot ||
		intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	if (vertexCount < DS_SPOT_LIGHT_VERTEX_COUNT || indexCount < DS_SPOT_LIGHT_INDEX_COUNT)
	{
		errno = EINDEX;
		return false;
	}

	if (firstIndex + DS_SPOT_LIGHT_VERTEX_COUNT > UINT16_MAX)
	{
		errno = ERANGE;
		return false;
	}

	float radius = getLightRadius(light, intensityThreshold);
	if (radius <= 0)
	{
		errno = EINVAL;
		return false;
	}

	dsSpotLightVertex commonData;
	commonData.lightPosition = light->position;
	commonData.direction[0] = dsPackInt16(-light->direction.x);
	commonData.direction[1] = dsPackInt16(-light->direction.y);
	commonData.direction[2] = dsPackInt16(-light->direction.z);
	commonData.direction[3] = 0;

#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_HALF_FLOAT || (dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat))
	{
		packLightColorSIMD(commonData.color, light);
		packLightSpotFalloffSIMD(commonData.falloffAndSpotAngles, light);
	}
	else
#endif
	{
		packLightColor(commonData.color, light);
		commonData.falloffAndSpotAngles[0] = dsPackHalfFloat(light->linearFalloff);
		commonData.falloffAndSpotAngles[1] = dsPackHalfFloat(light->quadraticFalloff);
		commonData.falloffAndSpotAngles[2] = dsPackHalfFloat(light->innerSpotCosAngle);
		commonData.falloffAndSpotAngles[3] = dsPackHalfFloat(light->outerSpotCosAngle);
	}

	const size_t commonDataSize = sizeof(dsSpotLightVertex) -
		offsetof(dsSpotLightVertex, lightPosition);

	outVertices[0].vertexPosition = light->position;
	memcpy(&outVertices[0].lightPosition, &commonData.lightPosition, commonDataSize);

	// Create an orthonormal basis around the spotlight.
	dsVector3f spotX, spotY;
	spotPerpAxes(&spotX, &spotY, light);

	// Middle of the spotlight where it ends.
	dsVector3f middlePos;
	dsVector3_scale(middlePos, light->direction, radius);
	dsVector3_add(middlePos, middlePos, light->position);

	// cos of the angle multiplied by the distance is how far to extend by the X and Y perpendicular
	// axes for the spot light.
	float spotEndDist = light->outerSpotCosAngle*radius;
	dsVector3_scale(spotX, spotX, spotEndDist);
	dsVector3_scale(spotY, spotY, spotEndDist);

	dsVector3f extremeX;
	dsVector3_sub(extremeX, middlePos, spotX);
	dsVector3_sub(outVertices[1].vertexPosition, extremeX, spotY);
	memcpy(&outVertices[1].lightPosition, &commonData.lightPosition, commonDataSize);

	dsVector3_add(outVertices[2].vertexPosition, extremeX, spotY);
	memcpy(&outVertices[2].lightPosition, &commonData.lightPosition, commonDataSize);

	dsVector3_add(extremeX, middlePos, spotX);
	dsVector3_sub(outVertices[3].vertexPosition, extremeX, spotY);
	memcpy(&outVertices[3].lightPosition, &commonData.lightPosition, commonDataSize);

	dsVector3_add(outVertices[4].vertexPosition, extremeX, spotY);
	memcpy(&outVertices[4].lightPosition, &commonData.lightPosition, commonDataSize);

	// left
	outIndices[0] = (uint16_t)(firstIndex + 0);
	outIndices[1] = (uint16_t)(firstIndex + 1);
	outIndices[2] = (uint16_t)(firstIndex + 2);

	// bottom
	outIndices[3] = (uint16_t)(firstIndex + 0);
	outIndices[4] = (uint16_t)(firstIndex + 3);
	outIndices[5] = (uint16_t)(firstIndex + 1);

	// right
	outIndices[6] = (uint16_t)(firstIndex + 0);
	outIndices[7] = (uint16_t)(firstIndex + 4);
	outIndices[8] = (uint16_t)(firstIndex + 3);

	// top
	outIndices[9] = (uint16_t)(firstIndex + 0);
	outIndices[10] = (uint16_t)(firstIndex + 2);
	outIndices[11] = (uint16_t)(firstIndex + 4);

	// back
	outIndices[12] = (uint16_t)(firstIndex + 1);
	outIndices[13] = (uint16_t)(firstIndex + 3);
	outIndices[14] = (uint16_t)(firstIndex + 4);

	outIndices[15] = (uint16_t)(firstIndex + 1);
	outIndices[16] = (uint16_t)(firstIndex + 4);
	outIndices[17] = (uint16_t)(firstIndex + 2);

	return true;
}
