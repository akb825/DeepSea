/*
 * Copyright 2024 Aaron Barany
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

#include <DeepSea/Physics/PhysicsMassProperties.h>

#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Math/Quaternion.h>

#include <string.h>

// See https://en.wikipedia.org/wiki/List_of_moments_of_inertia for most of the formula for shape
// initialization.

inline static void mulBTransposed(dsMatrix33f* result, const dsMatrix33f* a, const dsMatrix33f* b)
{
	for (unsigned int i = 0; i < 3; ++i)
	{
		float b0i = b->values[0][i];
		float b1i = b->values[1][i];
		float b2i = b->values[2][i];
		for (unsigned int j = 0; j < 3; ++j)
			result->values[i][j] = a->values[0][j]*b0i + a->values[1][j]*b1i + a->values[2][j]*b2i;
	}
}

inline static bool getTriangle(dsVector3f* outA, dsVector3f* outB, dsVector3f* outC,
	const void* vertices, uint32_t vertexCount, size_t vertexStride, const void* indices,
	size_t indexSize, uint32_t triangle)
{
	uint32_t vA, vB, vC;
	switch (indexSize)
	{
		case sizeof(uint16_t):
		{
			const uint16_t* indices16 = (const uint16_t*)indices;
			vA = indices16[triangle*3];
			vB = indices16[triangle*3 + 1];
			vC = indices16[triangle*3 + 2];
			break;
		}
		case sizeof(uint32_t):
		{
			const uint32_t* indices32 = (const uint32_t*)indices;
			vA = indices32[triangle*3];
			vB = indices32[triangle*3 + 1];
			vC = indices32[triangle*3 + 2];
			break;
		}
		default:
			DS_ASSERT(false);
			return false;
	}

	if (vA >= vertexCount || vB >= vertexCount || vC >= vertexCount)
	{
		errno = EINDEX;
		return false;
	}

	const uint8_t* vertexBytes = (const uint8_t*)vertices;
	*outA = *(const dsVector3f*)(vertexBytes + vertexStride*vA);
	*outB = *(const dsVector3f*)(vertexBytes + vertexStride*vB);
	*outC = *(const dsVector3f*)(vertexBytes + vertexStride*vC);
	return true;
}

static bool computeCenterOfMassAndVolume(dsVector3f* outCenterOfMass, float* outVolume,
	const void* vertices, uint32_t vertexCount, size_t vertexStride, const void* indices,
	uint32_t indexCount, size_t indexSize)
{
	const float minVolume = 1e-6f;

	dsVector3f a, b, c;
	memset(outCenterOfMass, 0, sizeof(dsVector3f));
	*outVolume = 0.0f;
	memset(&a, 0, sizeof(dsVector3f));
	memset(&b, 0, sizeof(dsVector3f));
	memset(&c, 0, sizeof(dsVector3f));

	// Average centroid of each triangle for a reference point in the middle of the volume.
	float centroidBaryCoord = 1/3.0f;
	dsVector3f averageCentroid = {{0.0f, 0.0f, 0.0f}};
	uint32_t triangleCount = indexCount/3;
	for (uint32_t i = 0; i < triangleCount; ++i)
	{
		if (!getTriangle(&a, &b, &c, vertices, vertexCount, vertexStride, indices, indexSize, i))
			return false;

		dsVector3f centroid, temp;
		dsVector3_scale(centroid, a, centroidBaryCoord);
		dsVector3_scale(temp, b, centroidBaryCoord);
		dsVector3_add(centroid, centroid, temp);
		dsVector3_scale(temp, c, centroidBaryCoord);
		dsVector3_add(averageCentroid, centroid, temp);
	}
	dsVector3_scale(averageCentroid, averageCentroid, 1/(float)triangleCount);

	// Take the volume of each tetrahedron formed by each triangle and the average centroid.
	for (uint32_t i = 0; i < triangleCount; ++i)
	{
		DS_VERIFY(getTriangle(
			&a, &b, &c, vertices, vertexCount, vertexStride, indices, indexSize, i));

		// NOTE: Will need to divide by 4 at the end.
		dsVector3f tetraCenterOfMass = averageCentroid;
		dsVector3_add(tetraCenterOfMass, tetraCenterOfMass, a);
		dsVector3_add(tetraCenterOfMass, tetraCenterOfMass, b);
		dsVector3_add(tetraCenterOfMass, tetraCenterOfMass, c);

		dsVector3f ad, bd, cd;
		dsVector3_sub(ad, a, averageCentroid);
		dsVector3_sub(bd, b, averageCentroid);
		dsVector3_sub(cd, c, averageCentroid);

		dsVector3f bdCrossCD;
		dsVector3_cross(bdCrossCD, bd, cd);
		// NOTE: Don't bother dividing by 6 until the end.
		float baseVolume = dsVector3_dot(ad, bdCrossCD);

		*outVolume += baseVolume;
		// Weighted average based on volume.
		dsVector3_scale(tetraCenterOfMass, tetraCenterOfMass, baseVolume);
		dsVector3_add(*outCenterOfMass, *outCenterOfMass, tetraCenterOfMass);
	}

	if (*outVolume < minVolume)
	{
		// Don't print as this is cached for dsPhysicsConvexHull.
		// DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Can't compute the volume of a flat mesh.");
		errno = EPERM;
		return false;
	}

	// Need to normalize for the volume weight and divide by 4 from adding 4 points each
	// centroid.
	float scale = 1/(*outVolume*4.0f);
	dsVector3_scale(*outCenterOfMass, *outCenterOfMass, scale);

	// Saved the divide of 6 to the end since it's common across all factors.
	*outVolume /= 6.0f;
	return true;
}

inline static void translateInertia(dsMatrix33f* result, const dsMatrix33f* inertia, float mass,
	const dsVector3f* translate)
{
	// https://en.wikipedia.org/wiki/Parallel_axis_theorem
	// Add to the inertia tensor m*(scaleMat(translate^2) - outer(translate, translate))
	float translate2 = dsVector3_dot(*translate, *translate);

	dsVector3f translateCol;
	translateCol.x = mass*(translate2 - translate->x*translate->x);
	translateCol.y = -mass*translate->x*translate->y;
	translateCol.z = -mass*translate->x*translate->z;
	dsVector3_add(result->columns[0], inertia->columns[0], translateCol);

	translateCol.x = -mass*translate->y*translate->x;
	translateCol.y = mass*(translate2 - translate->y*translate->y);
	translateCol.z = -mass*translate->y*translate->z;
	dsVector3_add(result->columns[1], inertia->columns[1], translateCol);

	translateCol.x = -mass*translate->z*translate->x;
	translateCol.y = -mass*translate->z*translate->y;
	translateCol.z = mass*(translate2 - translate->z*translate->z);
	dsVector3_add(result->columns[2], inertia->columns[2], translateCol);
}

inline static void rotateInertia(dsMatrix33f* result, const dsMatrix33f* inertia,
	const dsQuaternion4f* rotate)
{
	dsMatrix33f rotateMat;
	dsQuaternion4f_toMatrix33(&rotateMat, rotate);

	dsMatrix33f temp;
	mulBTransposed(&temp, inertia, &rotateMat);
	dsMatrix33_mul(*result, rotateMat, temp);
}

inline static float scaleInertia(dsMatrix33f* result, const dsMatrix33f* inertia, float mass,
	const dsVector3f* scale)
{
	/*
	 * https://en.wikipedia.org/wiki/Moment_of_inertia#Inertia_tensor
	 *
	 * Need to apply scale across the three axes and the mass. Mass scale is trivial, for the axis
	 * scale we need to extract the axes from the original inertia.
	 *
	 * The diagonal factors are sum(mass*(a^2 + b^2)), where a and b are the perpendicular axes.
	 * (e.g. y and z for the x axis) If we sum the diagonal factors, it will be
	 * 2*m*(x^2 + y^2 + z^2), so if we halve that and subtract by each diagonal value, we can get
	 * get the vector [m*x^2, m*y^2, m*z^2], which we can use to scale and re-compute the inertia
	 * tensor factors.
	 */
	dsVector3f diagonals = {{inertia->values[0][0], inertia->values[1][1], inertia->values[2][2]}};
	float halfDiagonalSum = (diagonals.x + diagonals.y + diagonals.z)*0.5f;
	dsVector3f massAxis2 = {{halfDiagonalSum - diagonals.x, halfDiagonalSum - diagonals.y,
		halfDiagonalSum - diagonals.z}};

	// Axes are squared, so we need to square the scale as well.
	dsVector3_mul(massAxis2, massAxis2, *scale);
	dsVector3_mul(massAxis2, massAxis2, *scale);

	// Apply the mass scale as well.
	float massScale = scale->x*scale->y*scale->z;
	dsVector3_scale(massAxis2, massAxis2, massScale);
	float newMass = mass*massScale;

	// Diagonal factors: sum(mass*(a^2 + b^2)), where a and b are the perpendicular axes.
	result->values[0][0] = massAxis2.y + massAxis2.z;
	result->values[1][1] = massAxis2.x + massAxis2.z;
	result->values[2][2] = massAxis2.x + massAxis2.y;

	// Off-diagonals are -sum(mass*a^2*b^2), where a is the axis corresponding to the first index
	// and b is the axis corresponding to the second index. As a result, we can scale the existing
	// factors by the axis scales and mass scale.
	result->values[0][1] = result->values[1][0] =
		inertia->values[0][1]*scale->x*scale->y*massScale;
	result->values[0][2] = result->values[2][0] =
		inertia->values[0][2]*scale->x*scale->z*massScale;
	result->values[1][2] = result->values[2][1] =
		inertia->values[1][2]*scale->y*scale->z*massScale;

	return newMass;
}

bool dsPhysicsMassProperties_initializeEmpty(dsPhysicsMassProperties* massProperties)
{
	if (!massProperties)
	{
		errno = EINVAL;
		return false;
	}

	memset(&massProperties->centeredInertia, 0, sizeof(dsMatrix33f));
	memset(&massProperties->centerOfMass, 0, sizeof(dsVector3f));
	memset(&massProperties->inertiaTranslate, 0, sizeof(dsVector3f));
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	massProperties->mass = 0.0f;
	return true;
}

bool dsPhysicsMassProperties_initializeBox(dsPhysicsMassProperties* massProperties,
	const dsVector3f* halfExtents, float density)
{
	if (!massProperties || !halfExtents || halfExtents->x <= 0 || halfExtents->y <= 0 ||
		halfExtents->z <= 0 || density <= 0)
	{
		errno = EINVAL;
		return false;
	}

	float x2 = dsPow2(halfExtents->x);
	float y2 = dsPow2(halfExtents->y);
	float z2 = dsPow2(halfExtents->z);
	float volume = 8*halfExtents->x*halfExtents->y*halfExtents->z;

	massProperties->mass = density*volume;
	float inertiaScale = massProperties->mass/3.0f;
	dsMatrix33f_makeScale3D(&massProperties->centeredInertia, (y2 + z2)*inertiaScale,
		(x2 + z2)*inertiaScale, (x2 + y2)*inertiaScale);

	memset(&massProperties->centerOfMass, 0, sizeof(dsVector3f));
	memset(&massProperties->inertiaTranslate, 0, sizeof(dsVector3f));
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	return true;
}

bool dsPhysicsMassProperties_initializeSphere(dsPhysicsMassProperties* massProperties, float radius,
	float density)
{
	if (!massProperties || radius <= 0 || density <= 0)
	{
		errno = EINVAL;
		return false;
	}

	float radius2 = dsPow2(radius);
	float volume = 4.0f/3.0f*(float)M_PI*radius*radius2;

	massProperties->mass = density*volume;
	float inertiaScale = 0.4f*radius2*massProperties->mass;
	dsMatrix33f_makeScale3D(&massProperties->centeredInertia, inertiaScale, inertiaScale,
		inertiaScale);

	memset(&massProperties->centerOfMass, 0, sizeof(dsVector3f));
	memset(&massProperties->inertiaTranslate, 0, sizeof(dsVector3f));
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	return true;
}

bool dsPhysicsMassProperties_initializeCylinder(dsPhysicsMassProperties* massProperties,
	float halfHeight, float radius, dsPhysicsAxis axis, float density)
{
	if (!massProperties || halfHeight <= 0 || radius <= 0 || density <= 0 ||
		axis < dsPhysicsAxis_X || axis > dsPhysicsAxis_Z)
	{
		errno = EINVAL;
		return false;
	}

	float radius2 = dsPow2(radius);
	float volume = (float)M_PI*radius2*2.0f*halfHeight;

	massProperties->mass = density*volume;
	float heightInertia = 0.5f*radius2*massProperties->mass;
	float radiusInertia = 0.5f*heightInertia + dsPow2(halfHeight)*massProperties->mass/3.0f;
	switch (axis)
	{
		case dsPhysicsAxis_X:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, heightInertia, radiusInertia,
				radiusInertia);
			break;
		case dsPhysicsAxis_Y:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, radiusInertia, heightInertia,
				radiusInertia);
			break;
		case dsPhysicsAxis_Z:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, radiusInertia, radiusInertia,
				heightInertia);
			break;
	}

	memset(&massProperties->centerOfMass, 0, sizeof(dsVector3f));
	memset(&massProperties->inertiaTranslate, 0, sizeof(dsVector3f));
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	return true;
}

bool dsPhysicsMassProperties_initializeCapsule(dsPhysicsMassProperties* massProperties,
	float halfHeight, float radius, dsPhysicsAxis axis, float density)
{
	if (!massProperties || halfHeight <= 0 || radius <= 0 || density <= 0 ||
		axis < dsPhysicsAxis_X || axis > dsPhysicsAxis_Z)
	{
		errno = EINVAL;
		return false;
	}

	// https://www.gamedev.net/resources/_/technical/math-and-physics/capsule-inertia-tensor-r3856
	// NOTE: The final equation has an error showing H^2/2 rather than H^2/4 (or (H/2)^2)

	float radius2 = dsPow2(radius);
	float halfHeight2 = dsPow2(halfHeight);
	float circleArea = (float)M_PI*radius2;
	float cylinderVolume = circleArea*2.0f*halfHeight;
	float sphereVolume = 4.0f/3.0f*circleArea*radius;

	float cylinderMass = density*cylinderVolume;
	float sphereMass = density*sphereVolume;
	massProperties->mass = cylinderMass + sphereMass;

	// Cylinder portion.
	float heightInertia = 0.5f*radius2*cylinderMass;
	float radiusInertia = 0.5f*heightInertia + halfHeight2*cylinderMass/3.0f;

	// Hemisphere portion.
	float sphereInertiaScale = 0.4f*radius2*sphereMass;
	heightInertia += sphereInertiaScale;
	radiusInertia += sphereInertiaScale + (halfHeight2 + 0.75f*halfHeight*radius)*sphereMass;

	switch (axis)
	{
		case dsPhysicsAxis_X:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, heightInertia, radiusInertia,
				radiusInertia);
			break;
		case dsPhysicsAxis_Y:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, radiusInertia, heightInertia,
				radiusInertia);
			break;
		case dsPhysicsAxis_Z:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, radiusInertia, radiusInertia,
				heightInertia);
			break;
	}

	memset(&massProperties->centerOfMass, 0, sizeof(dsVector3f));
	memset(&massProperties->inertiaTranslate, 0, sizeof(dsVector3f));
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	return true;
}

bool dsPhysicsMassProperties_initializeCone(dsPhysicsMassProperties* massProperties, float height,
	float radius, dsPhysicsAxis axis, float density)
{
	if (!massProperties || height <= 0 || radius <= 0 || density <= 0 || axis < dsPhysicsAxis_X ||
		axis > dsPhysicsAxis_Z)
	{
		errno = EINVAL;
		return false;
	}

	float radius2 = dsPow2(radius);
	float volume = (float)M_PI*radius2*height/3.0f;

	// Inertia through center of mass, which unlike most shapes is offset from the origin.
	massProperties->mass = density*volume;
	float heightInertia = 0.3f*radius2*massProperties->mass;
	float radiusInertia = 0.5f*heightInertia + 0.0375f*dsPow2(height)*massProperties->mass;
	switch (axis)
	{
		case dsPhysicsAxis_X:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, heightInertia, radiusInertia,
				radiusInertia);
			massProperties->centerOfMass.x = 0.75f*height;
			massProperties->centerOfMass.y = 0.0f;
			massProperties->centerOfMass.z = 0.0f;
			break;
		case dsPhysicsAxis_Y:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, radiusInertia, heightInertia,
				radiusInertia);
			massProperties->centerOfMass.x = 0.0f;
			massProperties->centerOfMass.y = 0.75f*height;
			massProperties->centerOfMass.z = 0.0f;
			break;
		case dsPhysicsAxis_Z:
			dsMatrix33f_makeScale3D(&massProperties->centeredInertia, radiusInertia, radiusInertia,
				heightInertia);
			massProperties->centerOfMass.x = 0.0f;
			massProperties->centerOfMass.y = 0.0f;
			massProperties->centerOfMass.z = 0.75f*height;
			break;
	}

	massProperties->inertiaTranslate = massProperties->centerOfMass;
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	return true;
}

bool dsPhysicsMassProperties_initializeMesh(dsPhysicsMassProperties* massProperties,
	const void* vertices, uint32_t vertexCount, size_t vertexStride, const void* indices,
	uint32_t indexCount, size_t indexSize, float density)
{
	if (!massProperties || !vertices || vertexCount < 3 || vertexStride < sizeof(dsVector3f) ||
		!indices || indexCount < 3 || indexCount % 3 != 0 ||
		(indexSize != sizeof(uint16_t) && indexSize != sizeof(uint32_t)) || density <= 0)
	{
		errno = EINVAL;
		return false;
	}

	// First need to compute the center of mass and volume.
	float volume;
	if (!computeCenterOfMassAndVolume(&massProperties->centerOfMass, &volume, vertices,
			vertexCount, vertexStride, indices, indexCount, indexSize))
	{
		return false;
	}

	massProperties->mass = volume*density;

	// See "How to find the inertia tensor (or other mass properties) of a 3D solid body represented
	// by a triangle mesh"  http://number-none.com/blow/inertia/bb_inertia.doc
	dsMatrix33f canonicalTetraCovariance =
	{{
		{1/60.0f, 1/120.0f, 1/120.0f},
		{1/120.0f, 1/60.0f, 1/120.0f},
		{1/120.0f, 1/120.0f, 1/60.0f}
	}};

	dsMatrix33f totalCovariance;
	dsVector3f a, b, c;
	memset(&totalCovariance, 0, sizeof(dsMatrix33f));
	memset(&a, 0, sizeof(dsVector3f));
	memset(&b, 0, sizeof(dsVector3f));
	memset(&c, 0, sizeof(dsVector3f));

	uint32_t triangleCount = indexCount/3;
	for (uint32_t i = 0; i < triangleCount; ++i)
	{
		DS_VERIFY(getTriangle(
			&a, &b, &c, vertices, vertexCount, vertexStride, indices, indexSize, i));

		// Mapping for tetrahedron to the canonical.
		dsMatrix33f tetraMap;
		dsVector3_sub(tetraMap.columns[0], a, massProperties->centerOfMass);
		dsVector3_sub(tetraMap.columns[1], b, massProperties->centerOfMass);
		dsVector3_sub(tetraMap.columns[2], c, massProperties->centerOfMass);

		dsMatrix33f covariance, temp;
		mulBTransposed(&temp, &canonicalTetraCovariance, &tetraMap);
		dsMatrix33_mul(covariance, tetraMap, temp);

		float detTetraMap = dsMatrix33_determinant(tetraMap);
		for (unsigned int j = 0; j < 3; ++j)
		{
			dsVector3f scaledCol;
			dsVector3_scale(scaledCol, covariance.columns[j], detTetraMap);
			dsVector3_add(totalCovariance.columns[j], totalCovariance.columns[j], scaledCol);
		}
	}

	// Moment of inertia based on the total covariance.
	float traceCovariance = totalCovariance.values[0][0] + totalCovariance.values[1][1] +
		totalCovariance.values[2][2];

	massProperties->centeredInertia.values[0][0] =
		(traceCovariance - totalCovariance.values[0][0])*density;
	massProperties->centeredInertia.values[0][1] = -totalCovariance.values[0][1]*density;
	massProperties->centeredInertia.values[0][2] = -totalCovariance.values[0][2]*density;

	massProperties->centeredInertia.values[1][0] = -totalCovariance.values[1][0]*density;
	massProperties->centeredInertia.values[1][1] =
		(traceCovariance - totalCovariance.values[1][1])*density;
	massProperties->centeredInertia.values[1][2] = -totalCovariance.values[1][2]*density;

	massProperties->centeredInertia.values[2][0] = -totalCovariance.values[2][0]*density;
	massProperties->centeredInertia.values[2][1] = -totalCovariance.values[2][1]*density;
	massProperties->centeredInertia.values[2][2] =
		(traceCovariance - totalCovariance.values[2][2])*density;

	massProperties->inertiaTranslate = massProperties->centerOfMass;
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	return true;
}

bool dsPhysicsMassProperties_initializeCombined(
	dsPhysicsMassProperties* massProperties,
	const dsPhysicsMassProperties* const* componentMassProperties,
	uint32_t componentMassPropertiesCount)
{
	if (!massProperties || (!componentMassProperties && componentMassPropertiesCount > 0))
	{
		errno = EINVAL;
		return false;
	}

	float totalMass = 0.0f;
	dsVector3f scaledCenterOfMass = {{0.0f, 0.0f, 0.0f}};
	for (uint32_t i = 0; i < componentMassPropertiesCount; ++i)
	{
		const dsPhysicsMassProperties* curMassProperties = componentMassProperties[i];
		if (!curMassProperties)
		{
			errno = EINVAL;
			return false;
		}

		dsVector3f scaledCenter;
		dsVector3_scale(scaledCenter, curMassProperties->centerOfMass, curMassProperties->mass);
		dsVector3_add(scaledCenterOfMass, scaledCenterOfMass, scaledCenter);
		totalMass += curMassProperties->mass;
	}

	memset(&massProperties->centeredInertia, 0, sizeof(dsMatrix33f));
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);
	massProperties->mass = totalMass;
	if (totalMass == 0.0f)
	{
		// Empty, but keep transform information.
		memset(&massProperties->centerOfMass, 0, sizeof(dsVector3f));
		memset(&massProperties->inertiaTranslate, 0, sizeof(dsVector3f));
		return true;
	}

	float invTotalMass = 1/totalMass;
	dsVector3_scale(massProperties->centerOfMass, scaledCenterOfMass, invTotalMass);
	massProperties->inertiaTranslate = massProperties->centerOfMass;
	dsQuaternion4_identityRotation(massProperties->inertiaRotate);

	for (uint32_t i = 0; i < componentMassPropertiesCount; ++i)
	{
		const dsPhysicsMassProperties* curMassProperties = componentMassProperties[i];

		// Shift the current mass properties' inertia into the current reference frame. Shift will
		// be the inverse of the transform to go to the same reference frame in order to move the
		// inertia back to its original position and orientation.
		dsVector3f shiftTranslate;
		dsVector3_sub(shiftTranslate, curMassProperties->centerOfMass,
			massProperties->centerOfMass);

		dsMatrix33f shiftedInertia;
		rotateInertia(&shiftedInertia, &curMassProperties->centeredInertia,
			&curMassProperties->inertiaRotate);
		translateInertia(&shiftedInertia, &shiftedInertia, curMassProperties->mass,
			&shiftTranslate);

		for (unsigned int i = 0; i < 3; ++i)
		{
			dsVector3_add(massProperties->centeredInertia.columns[i],
				massProperties->centeredInertia.columns[i], shiftedInertia.columns[i]);
		}
	}
	return true;
}

bool dsPhysicsMassProperties_setMass(dsPhysicsMassProperties* massProperties, float mass)
{
	if (!massProperties || mass <= 0)
	{
		errno = EINVAL;
		return false;
	}

	float scale = mass/massProperties->mass;
	dsVector3_scale(massProperties->centeredInertia.columns[0],
		massProperties->centeredInertia.columns[0], scale);
	dsVector3_scale(massProperties->centeredInertia.columns[1],
		massProperties->centeredInertia.columns[1], scale);
	dsVector3_scale(massProperties->centeredInertia.columns[2],
		massProperties->centeredInertia.columns[2], scale);
	massProperties->mass = mass;
	return true;
}

bool dsPhysicsMassProperties_transform(dsPhysicsMassProperties* massProperties,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale)
{
	if (!massProperties)
	{
		errno = EINVAL;
		return false;
	}

	if (scale && (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f))
	{
		if (scale->x == 0.0f || scale->y == 0.0f || scale->z == 0.0f)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Can't scale a mass properties by zero.");
			errno = EPERM;
			return false;
		}

		bool hasRotate = massProperties->inertiaRotate.i != 0.0f ||
			massProperties->inertiaRotate.j != 0.0f || massProperties->inertiaRotate.k != 0.0f ||
			massProperties->inertiaRotate.r != 1.0f;
		if (hasRotate && (scale->x != scale->y || scale->x != scale->z))
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Can't apply a non-uniform scale to a previously rotated mass properties.");
			errno = EPERM;
			return false;
		}

		massProperties->mass = scaleInertia(&massProperties->centeredInertia,
			&massProperties->centeredInertia, massProperties->mass, scale);
		dsVector3_mul(massProperties->centerOfMass, massProperties->centerOfMass, *scale);
		dsVector3_mul(massProperties->inertiaTranslate, massProperties->inertiaTranslate, *scale);
	}

	if (rotate)
	{
		dsQuaternion4f_rotate(&massProperties->centerOfMass, rotate, &massProperties->centerOfMass);
		dsQuaternion4f_rotate(&massProperties->inertiaTranslate, rotate,
			&massProperties->inertiaTranslate);
		dsQuaternion4f curRotate = massProperties->inertiaRotate;
		dsQuaternion4f_mul(&massProperties->inertiaRotate, rotate, &curRotate);
	}

	if (translate)
	{
		dsVector3_add(massProperties->centerOfMass, massProperties->centerOfMass, *translate);
		dsVector3_add(massProperties->inertiaTranslate, massProperties->inertiaTranslate,
			*translate);
	}

	return true;
}

bool dsPhysicsMassProperties_shift(dsPhysicsMassProperties* massProperties,
	const dsVector3f* translate, const dsQuaternion4f* rotate)
{
	if (!massProperties)
	{
		errno = EINVAL;
		return false;
	}

	if (rotate)
	{
		// Rotate by the inverse to move back to the original location.
		dsQuaternion4f rotateInv;
		dsQuaternion4f_conjugate(&rotateInv, rotate);
		rotateInertia(&massProperties->centeredInertia, &massProperties->centeredInertia,
			&rotateInv);

		dsQuaternion4f curRotate = massProperties->inertiaRotate;
		dsQuaternion4f_mul(&massProperties->inertiaRotate, rotate, &curRotate);
		dsQuaternion4f_rotate(&massProperties->centerOfMass, rotate, &massProperties->centerOfMass);
		dsQuaternion4f_rotate(&massProperties->inertiaTranslate, rotate,
			&massProperties->inertiaTranslate);
	}

	if (translate)
	{
		// Move translateInertia but not center of mass. The final shift will be done later when
		// querying the final non-centered inertia.
		dsVector3_add(massProperties->inertiaTranslate, massProperties->inertiaTranslate,
			*translate);
	}

	return true;
}

bool dsPhysicsMassProperties_getInertia(dsMatrix33f* outInertia,
	const dsPhysicsMassProperties* massProperties)
{
	if (!outInertia || !massProperties)
	{
		errno = EINVAL;
		return false;
	}

	if (dsVector3_equal(massProperties->centerOfMass, massProperties->inertiaTranslate))
		*outInertia = massProperties->centeredInertia;
	else
	{
		dsVector3f shiftTranslate;
		dsVector3_sub(shiftTranslate, massProperties->inertiaTranslate,
			massProperties->centerOfMass);
		translateInertia(outInertia, &massProperties->centeredInertia, massProperties->mass,
			&shiftTranslate);
	}
	return true;
}

bool dsPhysicsMassProperties_getDecomposedInertia(dsMatrix33f* outInertiaRotate,
	dsVector3f* outInertiaDiagonal, const dsPhysicsMassProperties* massProperties)
{
	if (!outInertiaRotate || !outInertiaDiagonal || !massProperties)
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix33f inertia;
	const dsMatrix33f* inertiaPtr;
	if (dsVector3_equal(massProperties->centerOfMass, massProperties->inertiaTranslate))
		inertiaPtr = &massProperties->centeredInertia;
	else
	{
		dsVector3f shiftTranslate;
		dsVector3_sub(shiftTranslate, massProperties->inertiaTranslate,
			massProperties->centerOfMass);
		translateInertia(&inertia, &massProperties->centeredInertia, massProperties->mass,
			&shiftTranslate);
		inertiaPtr = &inertia;
	}

	if (!dsMatrix33f_jacobiEigenvalues(outInertiaRotate, outInertiaDiagonal, inertiaPtr))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Couldn't decompose mass properties inertia tensor.");
		return false;
	}

	// May have created a non right-handed rotation.
	dsVector3f expectedDir;
	dsVector3_cross(expectedDir, outInertiaRotate->columns[0], outInertiaRotate->columns[1]);
	if (dsVector3_dot(expectedDir, outInertiaRotate->columns[2]) < 0.0f)
		dsVector3_neg(outInertiaRotate->columns[2], outInertiaRotate->columns[2]);
	return true;
}
