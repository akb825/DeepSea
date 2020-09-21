#include <DeepSea/SceneLighting/SceneLight.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Math/Color.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <float.h>
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

static float getLightRadius(const dsSceneLight* light, float intensityThreshold)
{
	float intensity = dsColor3f_grayscale(&light->color)*light->intensity;

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
		return -1;

	// Guaranteed that a factor is > 0, so only need to check "+" and not "-" of quadratic formula.
	// (the "-" factor will always be < 0)
	float root = sqrtf(innerRoot);
	return (-b + root)/(2.0f*a);
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
	if (!light || !position)
		return 0;

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
	if (!light || !position)
		return 0.0f;

	return dsSceneLight_getFalloff(light, position)*dsColor3f_grayscale(&light->color)*
		light->intensity;
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
		outBounds->min.x = -FLT_MAX;
		outBounds->min.y = -FLT_MAX;
		outBounds->min.z = -FLT_MAX;
		outBounds->max.x = FLT_MAX;
		outBounds->max.y = FLT_MAX;
		outBounds->max.z = FLT_MAX;
		return true;
	}

	float radius = getLightRadius(light, intensityThreshold);
	if (radius < 0)
	{
		dsAlignedBox3f_makeInvalid(outBounds);
		return true;
	}

	if (light->type == dsSceneLightType_Point)
	{
		dsVector3f sizeOffset = {{radius, radius, radius}};
		dsVector3_sub(outBounds->min, light->position, sizeOffset);
		dsVector3_add(outBounds->min, light->position, sizeOffset);
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

	// sin of the angle multiplied by the distance is how far to extend by the X and Y perpendicular
	// axes for the spot light.
	float outerSinAngle = sqrtf(1.0f - dsPow2(light->outerSpotCosAngle));
	float spotEndDist = outerSinAngle*radius;
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
