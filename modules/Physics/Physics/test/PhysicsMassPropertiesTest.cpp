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

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/OrientedBox3.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Math/Vector4.h>

#include <DeepSea/Physics/PhysicsMassProperties.h>

#include <gtest/gtest.h>

// Use as close to original reference math as possible when verifying computation of inertia to
// ensure the optimized versions don't have mistakes.

constexpr float massEpsilon = 1e-4f;
constexpr float inertiaEpsilon = 1e-3f;

constexpr uint32_t boxIndexCount = 36;
static const uint16_t boxIndices[boxIndexCount] =
{
	// Front face
	dsBox3Corner_xyz, dsBox3Corner_Xyz, dsBox3Corner_XyZ,
	dsBox3Corner_XyZ, dsBox3Corner_xyZ, dsBox3Corner_xyz,

	// Right face
	dsBox3Corner_Xyz, dsBox3Corner_XYz, dsBox3Corner_XYZ,
	dsBox3Corner_XYZ, dsBox3Corner_XyZ, dsBox3Corner_Xyz,

	// Back face
	dsBox3Corner_XYz, dsBox3Corner_xYz, dsBox3Corner_xYZ,
	dsBox3Corner_xYZ, dsBox3Corner_XYZ, dsBox3Corner_XYz,

	// Left face
	dsBox3Corner_xYz, dsBox3Corner_xyz, dsBox3Corner_xyZ,
	dsBox3Corner_xyZ, dsBox3Corner_xYZ, dsBox3Corner_xYz,

	// Bottom face
	dsBox3Corner_xyz, dsBox3Corner_xYz, dsBox3Corner_XYz,
	dsBox3Corner_XYz, dsBox3Corner_Xyz, dsBox3Corner_xyz,

	// Top face
	dsBox3Corner_xyZ, dsBox3Corner_XyZ, dsBox3Corner_XYZ,
	dsBox3Corner_XYZ, dsBox3Corner_xYZ, dsBox3Corner_xyZ
};

TEST(PhysicsMassPropertiesTest, InitializeBox)
{
	float width = 2.0f;
	float height = 3.0f;
	float depth = 4.0f;
	float density = 2.5f;

	dsVector3f halfExtents = {{width/2, height/2, depth/2}};
	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&massProperties, &halfExtents, density));

	float volume = width*height*depth;
	float mass = volume*density;
	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);
	EXPECT_EQ(0, massProperties.inertiaRotate.i);
	EXPECT_EQ(0, massProperties.inertiaRotate.j);
	EXPECT_EQ(0, massProperties.inertiaRotate.k);
	EXPECT_EQ(1, massProperties.inertiaRotate.r);

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(1.0f/12.0f*mass*(dsPow2(height) + dsPow2(depth)), inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(1.0f/12.0f*mass*(dsPow2(width) + dsPow2(depth)), inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(1.0f/12.0f*mass*(dsPow2(width) + dsPow2(height)), inertia.values[2][2]);
}

TEST(PhysicsMassPropertiesTest, InitializeSphere)
{
	float radius = 1.5f;
	float density = 2.5f;

	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeSphere(&massProperties, radius, density));

	float volume = 4.0f/3.0f*(float)M_PI*dsPow3(radius);
	float mass = volume*density;
	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);
	EXPECT_EQ(0, massProperties.inertiaRotate.i);
	EXPECT_EQ(0, massProperties.inertiaRotate.j);
	EXPECT_EQ(0, massProperties.inertiaRotate.k);
	EXPECT_EQ(1, massProperties.inertiaRotate.r);

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(2.0f/5.0f*mass*dsPow2(radius), inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(2.0f/5.0f*mass*dsPow2(radius), inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(2.0f/5.0f*mass*dsPow2(radius), inertia.values[2][2]);
}

TEST(PhysicsMassPropertiesTest, InitializeCylinder)
{
	float height = 3.5f;
	float radius = 1.5f;
	float density = 2.5f;

	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeCylinder(&massProperties, height/2, radius,
		dsPhysicsAxis_X, density));

	float volume = (float)M_PI*dsPow2(radius)*height;
	float mass = volume*density;
	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);
	EXPECT_EQ(0, massProperties.inertiaRotate.i);
	EXPECT_EQ(0, massProperties.inertiaRotate.j);
	EXPECT_EQ(0, massProperties.inertiaRotate.k);
	EXPECT_EQ(1, massProperties.inertiaRotate.r);

	float heightInertia = 1/2.0f*mass*dsPow2(radius);
	float radiusInertia = 1/12.0f*mass*(3*dsPow2(radius) + dsPow2(height));

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(heightInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCylinder(&massProperties, height/2, radius,
		dsPhysicsAxis_Y, density));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCylinder(&massProperties, height/2, radius,
		dsPhysicsAxis_Z, density));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[2][2]);
}

TEST(PhysicsMassPropertiesTest, InitializeCapsule)
{
	float height = 3.5f;
	float radius = 1.5f;
	float density = 2.5f;

	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeCapsule(&massProperties, height/2, radius,
		dsPhysicsAxis_X, density));

	float hemisphereVolume = 4.0f/3.0f*(float)M_PI*dsPow3(radius)/2;
	float hemisphereMass = hemisphereVolume*density;
	float cylinderVolume = (float)M_PI*dsPow2(radius)*height;
	float cylinderMass = cylinderVolume*density;
	float mass = 2*hemisphereMass + cylinderMass;
	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);
	EXPECT_EQ(0, massProperties.inertiaRotate.i);
	EXPECT_EQ(0, massProperties.inertiaRotate.j);
	EXPECT_EQ(0, massProperties.inertiaRotate.k);
	EXPECT_EQ(1, massProperties.inertiaRotate.r);

	// https://www.gamedev.net/resources/_/technical/math-and-physics/capsule-inertia-tensor-r3856
	// NOTE: The final equation has an error showing H^2/2 rather than H^2/4 (or (H/2)^2)

	float heightInertia =
		1/2.0f*cylinderMass*dsPow2(radius) + 2*hemisphereMass*2/5.0f*dsPow2(radius);
	float radiusInertia = cylinderMass*(dsPow2(height)/12.0f + dsPow2(radius)/4.0f) +
		2*hemisphereMass*(2/5.0f*dsPow2(radius) + dsPow2(height)/4.0f + 3/8.0f*height*radius);

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(heightInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCapsule(&massProperties, height/2, radius,
		dsPhysicsAxis_Y, density));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCapsule(&massProperties, height/2, radius,
		dsPhysicsAxis_Z, density));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[2][2]);
}

TEST(PhysicsMassPropertiesTest, InitializeCone)
{
	float height = 3.5f;
	float radius = 1.5f;
	float density = 2.5f;

	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeCone(&massProperties, height, radius,
		dsPhysicsAxis_X, density));

	float volume = (float)M_PI*dsPow2(radius)*height/3.0f;
	float mass = volume*density;
	float centerOfMass = 3/4.0f*height;
	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);
	EXPECT_EQ(0, massProperties.inertiaRotate.i);
	EXPECT_EQ(0, massProperties.inertiaRotate.j);
	EXPECT_EQ(0, massProperties.inertiaRotate.k);
	EXPECT_EQ(1, massProperties.inertiaRotate.r);

	float heightInertia = 3/10.0f*mass*dsPow2(radius);
	float radiusInertia = mass*(3/20.0f*dsPow2(radius) + 3/80.0f*dsPow2(height));

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(heightInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCone(&massProperties, height, radius,
		dsPhysicsAxis_Y, density));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCone(&massProperties, height, radius,
		dsPhysicsAxis_Z, density));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.inertiaTranslate.z);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[2][2]);
}

TEST(PhysicsMassPropertiesTest, InitializeMesh)
{
	dsAlignedBox3f box =
	{
		{{-1.5f, 3.5f, 6.0f}},
		{{3.0f, 9.0f, 12.5f}}
	};
	float density = 2.5f;

	dsVector3f corners[DS_BOX3_CORNER_COUNT];
	dsAlignedBox3_corners(corners, box);

	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeMesh(&massProperties, corners,
		DS_BOX3_CORNER_COUNT, sizeof(dsVector3f), boxIndices, boxIndexCount,
		sizeof(*boxIndices), density));

	dsVector3f halfExtents, center;
	dsAlignedBox3_extents(halfExtents, box);
	dsVector3_scale(halfExtents, halfExtents, 0.5f);
	dsAlignedBox3_center(center, box);
	dsPhysicsMassProperties boxMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&boxMassProperties, &halfExtents, density));

	EXPECT_FLOAT_EQ(boxMassProperties.mass, massProperties.mass);
	EXPECT_FLOAT_EQ(center.x, massProperties.centerOfMass.x);
	EXPECT_FLOAT_EQ(center.y, massProperties.centerOfMass.y);
	EXPECT_FLOAT_EQ(center.z, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaRotate.i);
	EXPECT_EQ(0, massProperties.inertiaRotate.j);
	EXPECT_EQ(0, massProperties.inertiaRotate.k);
	EXPECT_EQ(1, massProperties.inertiaRotate.r);

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(boxMassProperties.centeredInertia.values[i][j], inertia.values[i][j],
				inertiaEpsilon) << i << ", " << j;
		}
	}
}

TEST(PhysicsMassPropertiesTest, InitializeCapsuleMesh)
{
	// Mesh approximation of a capsule. More of a test that the capsule inertia is correct, since
	// the reference has a mistake for the final equation and there aren't many other sources to
	// cross-reference.
	float height = 3.5f;
	float radius = 1.5f;
	float density = 2.5f;

	constexpr uint32_t circleSteps = 64;
	constexpr uint32_t hemisphereRows = 16;
	constexpr uint32_t hemisphereVertexCount = circleSteps*hemisphereRows + 1;
	constexpr uint32_t vertexCount = hemisphereVertexCount*2;
	constexpr uint32_t hemisphereTriangleCount = circleSteps*(hemisphereRows - 1)*2 + circleSteps;
	constexpr uint32_t cylinderTriangleCount = circleSteps*2;
	constexpr uint32_t triangleCount = hemisphereTriangleCount*2 + cylinderTriangleCount;

	dsVector3f vertices[vertexCount];
	uint32_t indices[triangleCount*3];
	uint32_t curBotVertex = 0, curTopVertex = hemisphereVertexCount, curBotIndex = 0,
		curTopIndex = hemisphereTriangleCount*3, curCylinderIndex = hemisphereTriangleCount*6;
	for (uint32_t i = 0; i < hemisphereRows; ++i)
	{
		float phi = (float)M_PI_2*(float)i/(float)hemisphereRows;
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);
		uint32_t botRowStartVertex = curBotVertex;
		uint32_t topRowStartVertex = curTopVertex;
		bool lastRow = i == hemisphereRows - 1;
		uint32_t stepTriangles = lastRow ? 3 : 6;
		for (uint32_t j = 0; j < circleSteps; ++j)
		{
			float theta = 2*(float)M_PI*(float)j/(float)(circleSteps - 1);
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			float x = cosTheta*cosPhi*radius;
			float y = sinTheta*cosPhi*radius;
			float z = sinPhi*radius;

			ASSERT_GT(vertexCount, curBotVertex);
			vertices[curBotVertex].x = x;
			vertices[curBotVertex].y = y;
			vertices[curBotVertex].z = -z - height/2;
			++curBotVertex;

			ASSERT_GT(vertexCount, curTopVertex);
			vertices[curTopVertex].x = x;
			vertices[curTopVertex].y = y;
			vertices[curTopVertex].z = z + height/2;
			++curTopVertex;

			uint32_t nextStep = (j + 1)%circleSteps;
			ASSERT_GE(triangleCount*3, curBotIndex + stepTriangles);
			indices[curBotIndex++] = botRowStartVertex + nextStep;
			indices[curBotIndex++] = botRowStartVertex + j;
			if (lastRow)
				indices[curBotIndex++] = botRowStartVertex + circleSteps;
			else
			{
				indices[curBotIndex++] = botRowStartVertex + circleSteps + j;
				indices[curBotIndex++] = botRowStartVertex + circleSteps + j;
				indices[curBotIndex++] = botRowStartVertex + circleSteps + nextStep;
				indices[curBotIndex++] = botRowStartVertex + nextStep;
			}

			ASSERT_GE(triangleCount*3, curTopIndex + stepTriangles);
			indices[curTopIndex++] = topRowStartVertex + j;
			indices[curTopIndex++] = topRowStartVertex + nextStep;
			if (lastRow)
				indices[curTopIndex++] = topRowStartVertex + circleSteps;
			else
			{
				indices[curTopIndex++] = topRowStartVertex + circleSteps + nextStep;
				indices[curTopIndex++] = topRowStartVertex + circleSteps + nextStep;
				indices[curTopIndex++] = topRowStartVertex + circleSteps + j;
				indices[curTopIndex++] = topRowStartVertex + j;
			}
		}
	}

	// Cylinder indices. First vertices of each hemisphere are the first row.
	for (uint32_t i = 0; i < circleSteps; ++i)
	{
		uint32_t nextStep = (i + 1)%circleSteps;
		ASSERT_GE(triangleCount*3, curCylinderIndex + 6);
		indices[curCylinderIndex++] = i;
		indices[curCylinderIndex++] = nextStep;
		indices[curCylinderIndex++] = hemisphereVertexCount + nextStep;
		indices[curCylinderIndex++] = hemisphereVertexCount + nextStep;
		indices[curCylinderIndex++] = hemisphereVertexCount + i;
		indices[curCylinderIndex++] = i;
	}

	// End point for each sphere.
	ASSERT_GT(vertexCount, curBotVertex);
	vertices[curBotVertex].x = 0.0f;
	vertices[curBotVertex].y = 0.0f;
	vertices[curBotVertex].z = -height/2 - radius;
	++curBotVertex;

	ASSERT_GT(vertexCount, curTopVertex);
	vertices[curTopVertex].x = 0.0f;
	vertices[curTopVertex].y = 0.0f;
	vertices[curTopVertex].z = height/2 + radius;
	++curTopVertex;

	ASSERT_EQ(hemisphereVertexCount, curBotVertex);
	ASSERT_EQ(hemisphereVertexCount*2, curTopVertex);
	ASSERT_EQ(hemisphereTriangleCount*3, curBotIndex);
	ASSERT_EQ(hemisphereTriangleCount*6, curTopIndex);
	ASSERT_EQ(triangleCount*3, curCylinderIndex);

	dsPhysicsMassProperties meshMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeMesh(&meshMassProperties, vertices, vertexCount,
		sizeof(dsVector3f), indices, triangleCount*3, sizeof(*indices), density));

	dsPhysicsMassProperties capsuleMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeCapsule(&capsuleMassProperties, height/2, radius,
		dsPhysicsAxis_Z, density));

	// Mesh will be slightly smaller. Expect it to be within 1% the mass.
	float epsilon = capsuleMassProperties.mass*0.01f;
	EXPECT_GT(capsuleMassProperties.mass, meshMassProperties.mass);
	EXPECT_NEAR(capsuleMassProperties.mass, meshMassProperties.mass, epsilon);

	EXPECT_NEAR(0, meshMassProperties.centerOfMass.x, massEpsilon);
	EXPECT_NEAR(0, meshMassProperties.centerOfMass.y, massEpsilon);
	EXPECT_NEAR(0, meshMassProperties.centerOfMass.z, massEpsilon);
	EXPECT_NEAR(0, meshMassProperties.inertiaTranslate.x, massEpsilon);
	EXPECT_NEAR(0, meshMassProperties.inertiaTranslate.y, massEpsilon);
	EXPECT_NEAR(0, meshMassProperties.inertiaTranslate.z, massEpsilon);

	// Slightly looser check for the inertia.
	float inertiaEpsilon = epsilon*1.5f;
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(capsuleMassProperties.centeredInertia.values[i][j],
				meshMassProperties.centeredInertia.values[i][j], inertiaEpsilon) << i << ", " << j;
		}
	}
}

TEST(PhysicsMassPropertiesTest, InitializeCombined)
{
	float width = 2.0f;
	float height = 3.0f;
	float depth = 4.0f;
	float density = 2.5f;

	dsQuaternion4f orientation;
	dsQuaternion4f_fromEulerAngles(&orientation, dsDegreesToRadiansf(-5.0f),
		dsDegreesToRadiansf(45.0f), dsDegreesToRadiansf(-65.0f));

	dsOrientedBox3f box;
	dsQuaternion4f_toMatrix33(&box.orientation, &orientation);
	box.center.x = 5.0f;
	box.center.y = -10.0f;
	box.center.z = 15.0f;
	box.halfExtents.x = width/2;
	box.halfExtents.y = height/2;
	box.halfExtents.z = depth/2;

	dsVector3f corners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3f_corners(corners, &box);

	dsPhysicsMassProperties meshMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeMesh(&meshMassProperties, corners,
		DS_BOX3_CORNER_COUNT, sizeof(dsVector3f), boxIndices, boxIndexCount,
		sizeof(*boxIndices), density));
	EXPECT_FLOAT_EQ(box.center.x, meshMassProperties.inertiaTranslate.x);
	EXPECT_FLOAT_EQ(box.center.y, meshMassProperties.inertiaTranslate.y);
	EXPECT_FLOAT_EQ(box.center.z, meshMassProperties.inertiaTranslate.z);

	// Combine two half boxes into one.
	dsVector3f halfBoxHalfExtents = {{width/4, height/2, depth/2}};
	dsPhysicsMassProperties halfBoxMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&halfBoxMassProperties, &halfBoxHalfExtents,
		density));

	// Right box.
	dsVector3f halfBoxOffset = {{width/4, 0, 0}};
	dsPhysicsMassProperties rightBoxMassProperties = halfBoxMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_transform(
		&rightBoxMassProperties, &halfBoxOffset, NULL, NULL));
	ASSERT_TRUE(dsPhysicsMassProperties_transform(
		&rightBoxMassProperties, &box.center, &orientation, NULL));

	// Left box.
	halfBoxOffset.x = -halfBoxOffset.x;
	dsPhysicsMassProperties leftBoxMassProperties = halfBoxMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_transform(
		&leftBoxMassProperties, &halfBoxOffset, NULL, NULL));
	ASSERT_TRUE(dsPhysicsMassProperties_transform(
		&leftBoxMassProperties, &box.center, &orientation, NULL));

	// Combine into one.
	const dsPhysicsMassProperties* componentProperties[] =
		{&rightBoxMassProperties, &leftBoxMassProperties};
	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeCombined(
		&massProperties, componentProperties, 2));

	EXPECT_NEAR(meshMassProperties.mass, massProperties.mass, massEpsilon);
	EXPECT_FLOAT_EQ(meshMassProperties.centerOfMass.x, massProperties.centerOfMass.x);
	EXPECT_FLOAT_EQ(meshMassProperties.centerOfMass.y, massProperties.centerOfMass.y);
	EXPECT_FLOAT_EQ(meshMassProperties.centerOfMass.z, massProperties.centerOfMass.z);
	EXPECT_FLOAT_EQ(meshMassProperties.inertiaTranslate.x, massProperties.inertiaTranslate.x);
	EXPECT_FLOAT_EQ(meshMassProperties.inertiaTranslate.y, massProperties.inertiaTranslate.y);
	EXPECT_FLOAT_EQ(meshMassProperties.inertiaTranslate.z, massProperties.inertiaTranslate.z);
	EXPECT_FLOAT_EQ(meshMassProperties.inertiaRotate.i, massProperties.inertiaRotate.i);
	EXPECT_FLOAT_EQ(meshMassProperties.inertiaRotate.j, massProperties.inertiaRotate.j);
	EXPECT_FLOAT_EQ(meshMassProperties.inertiaRotate.k, massProperties.inertiaRotate.k);
	EXPECT_FLOAT_EQ(meshMassProperties.inertiaRotate.r, massProperties.inertiaRotate.r);

	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(meshMassProperties.centeredInertia.values[i][j],
				massProperties.centeredInertia.values[i][j], inertiaEpsilon) << i << ", " << j;
		}
	}
}

TEST(PhysicsMassPropertiesTest, SetMass)
{
	float width = 2.0f;
	float height = 3.0f;
	float depth = 4.0f;
	float density = 2.5f;

	dsVector3f halfExtents = {{width/2, height/2, depth/2}};
	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&massProperties, &halfExtents, density));
	ASSERT_TRUE(dsPhysicsMassProperties_setMass(&massProperties, massProperties.mass*3.0f));

	dsPhysicsMassProperties scaledMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&scaledMassProperties, &halfExtents,
		density*3.0f));

	EXPECT_FLOAT_EQ(scaledMassProperties.mass, massProperties.mass);
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_FLOAT_EQ(scaledMassProperties.centeredInertia.values[i][j],
				massProperties.centeredInertia.values[i][j]) << i << ", " << j;
		}
	}
}

TEST(PhysicsMassPropertiesTest, Transform)
{
	float width = 2.0f;
	float height = 3.0f;
	float depth = 4.0f;
	float density = 2.5f;

	dsVector3f halfExtents = {{width/2, height/2, depth/2}};
	dsPhysicsMassProperties boxMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&boxMassProperties, &halfExtents, density));

	dsQuaternion4f rotateA, rotateB;
	dsQuaternion4f_fromEulerAngles(&rotateA, dsDegreesToRadiansf(30.0f),
		dsDegreesToRadiansf(-20.0f), dsDegreesToRadiansf(40.0f));
	dsQuaternion4f_fromEulerAngles(&rotateB, dsDegreesToRadiansf(-5.0f),
		dsDegreesToRadiansf(45.0f), dsDegreesToRadiansf(-65.0f));

	dsVector3f translateA = {{5.0f, -10.0f, 15.0f}};
	dsVector3f translateB = {{-20.0f, 25.0f, -30.0f}};

	halfExtents.x = halfExtents.y = halfExtents.z = 0.5f;
	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&massProperties, &halfExtents, density));

	dsVector3f scale = {{width/2, height/2, depth/2}};
	ASSERT_TRUE(dsPhysicsMassProperties_transform(&massProperties, &translateA, &rotateA, &scale));

	// Can't apply a non-uniform scale once rotated.
	EXPECT_FALSE(dsPhysicsMassProperties_transform(&massProperties, &translateB, &rotateB, &scale));

	// Can apply a uniform scale.
	scale.x = scale.y = scale.z = 2.0f;
	ASSERT_TRUE(dsPhysicsMassProperties_transform(&massProperties, &translateB, &rotateB, &scale));

	EXPECT_FLOAT_EQ(boxMassProperties.mass, massProperties.mass);
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_FLOAT_EQ(boxMassProperties.centeredInertia.values[i][j],
				massProperties.centeredInertia.values[i][j]) << i << ", " << j;
		}
	}

	dsMatrix44f transformA;
	dsQuaternion4f_toMatrix44(&transformA, &rotateA);
	*reinterpret_cast<dsVector3f*>(transformA.columns + 3) = translateA;

	dsMatrix44f transformB;
	dsQuaternion4f_toMatrix44(&transformB, &rotateB);
	dsVector4f_scale(&transformB.columns[0], &transformB.columns[0], 2.0f);
	dsVector4f_scale(&transformB.columns[1], &transformB.columns[1], 2.0f);
	dsVector4f_scale(&transformB.columns[2], &transformB.columns[2], 2.0f);
	*reinterpret_cast<dsVector3f*>(transformB.columns + 3) = translateB;

	dsMatrix44f finalTransform;
	dsMatrix44f_mul(&finalTransform, &transformB, &transformA);

	dsMatrix44f normalizedFinalTransform;
	dsVector4f_normalize(normalizedFinalTransform.columns, finalTransform.columns);
	dsVector4f_normalize(normalizedFinalTransform.columns + 1, finalTransform.columns + 1);
	dsVector4f_normalize(normalizedFinalTransform.columns + 2, finalTransform.columns + 2);
	normalizedFinalTransform.columns[3] = finalTransform.columns[3];

	dsQuaternion4f finalRotate;
	dsQuaternion4f_fromMatrix44(&finalRotate, &normalizedFinalTransform);

	const float rotateEpsilon = 1e-6f;
	EXPECT_FLOAT_EQ(finalTransform.columns[3].x, massProperties.centerOfMass.x);
	EXPECT_FLOAT_EQ(finalTransform.columns[3].y, massProperties.centerOfMass.y);
	EXPECT_FLOAT_EQ(finalTransform.columns[3].z, massProperties.centerOfMass.z);
	EXPECT_FLOAT_EQ(finalTransform.columns[3].x, massProperties.inertiaTranslate.x);
	EXPECT_FLOAT_EQ(finalTransform.columns[3].y, massProperties.inertiaTranslate.y);
	EXPECT_FLOAT_EQ(finalTransform.columns[3].z, massProperties.inertiaTranslate.z);
	EXPECT_NEAR(finalRotate.i, massProperties.inertiaRotate.i, rotateEpsilon);
	EXPECT_NEAR(finalRotate.j, massProperties.inertiaRotate.j, rotateEpsilon);
	EXPECT_NEAR(finalRotate.k, massProperties.inertiaRotate.k, rotateEpsilon);
	EXPECT_NEAR(finalRotate.r, massProperties.inertiaRotate.r, rotateEpsilon);
}

TEST(PhysicsMassPropertiesTest, ShiftTranslate)
{
	// Cone center of mass is offset from the shape origin, which is at the tip. Translate back to
	// the tip and compare to the provided formula for the moment of inertia at the tip.
	float height = 3.5f;
	float radius = 1.5f;
	float density = 2.5f;

	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeCone(&massProperties, height, radius,
		dsPhysicsAxis_X, density));

	dsVector3f offset;
	dsVector3_neg(offset, massProperties.inertiaTranslate);
	ASSERT_TRUE(dsPhysicsMassProperties_shift(&massProperties, &offset, NULL));

	float volume = (float)M_PI*dsPow2(radius)*height/3.0f;
	float mass = volume*density;
	float centerOfMass = 3/4.0f*height;
	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);

	float heightInertia = 3/10.0f*mass*dsPow2(radius);
	float radiusInertia = mass*(3/20.0f*dsPow2(radius) + 3/5.0f*dsPow2(height));

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(heightInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	// Move back to undo the shift.
	dsVector3_neg(offset, massProperties.inertiaTranslate);
	ASSERT_TRUE(dsPhysicsMassProperties_shift(&massProperties, &offset, NULL));

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCone(&massProperties, height, radius,
		dsPhysicsAxis_Y, density));
	dsVector3_neg(offset, massProperties.inertiaTranslate);
	ASSERT_TRUE(dsPhysicsMassProperties_shift(&massProperties, &offset, NULL));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.centerOfMass.y);
	EXPECT_EQ(0, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[2][2]);

	ASSERT_TRUE(dsPhysicsMassProperties_initializeCone(&massProperties, height, radius,
		dsPhysicsAxis_Z, density));
	dsVector3_neg(offset, massProperties.inertiaTranslate);
	ASSERT_TRUE(dsPhysicsMassProperties_shift(&massProperties, &offset, NULL));

	EXPECT_FLOAT_EQ(mass, massProperties.mass);
	EXPECT_EQ(0, massProperties.centerOfMass.x);
	EXPECT_EQ(0, massProperties.centerOfMass.y);
	EXPECT_FLOAT_EQ(centerOfMass, massProperties.centerOfMass.z);
	EXPECT_EQ(0, massProperties.inertiaTranslate.x);
	EXPECT_EQ(0, massProperties.inertiaTranslate.y);
	EXPECT_EQ(0, massProperties.inertiaTranslate.z);

	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[0][0]);
	EXPECT_EQ(0, inertia.values[0][1]);
	EXPECT_EQ(0, inertia.values[0][2]);

	EXPECT_EQ(0, inertia.values[1][0]);
	EXPECT_FLOAT_EQ(radiusInertia, inertia.values[1][1]);
	EXPECT_EQ(0, inertia.values[1][2]);

	EXPECT_EQ(0, inertia.values[2][0]);
	EXPECT_EQ(0, inertia.values[2][1]);
	EXPECT_FLOAT_EQ(heightInertia, inertia.values[2][2]);
}

TEST(PhysicsMassPropertiesTest, ShiftRotate)
{
	float width = 2.0f;
	float height = 3.0f;
	float depth = 4.0f;
	float density = 2.5f;

	dsQuaternion4f orientation;
	dsQuaternion4f_fromEulerAngles(&orientation, dsDegreesToRadiansf(-5.0f),
		dsDegreesToRadiansf(45.0f), dsDegreesToRadiansf(-65.0f));

	dsOrientedBox3f box;
	dsQuaternion4f_toMatrix33(&box.orientation, &orientation);
	box.center.x = 5.0f;
	box.center.y = -10.0f;
	box.center.z = 15.0f;
	box.halfExtents.x = width/2;
	box.halfExtents.y = height/2;
	box.halfExtents.z = depth/2;

	dsVector3f corners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3f_corners(corners, &box);

	dsPhysicsMassProperties meshMassProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeMesh(&meshMassProperties, corners,
		DS_BOX3_CORNER_COUNT, sizeof(dsVector3f), boxIndices, boxIndexCount,
		sizeof(*boxIndices), density));
	EXPECT_FLOAT_EQ(box.center.x, meshMassProperties.inertiaTranslate.x);
	EXPECT_FLOAT_EQ(box.center.y, meshMassProperties.inertiaTranslate.y);
	EXPECT_FLOAT_EQ(box.center.z, meshMassProperties.inertiaTranslate.z);

	// Translate first.
	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&massProperties, &box.halfExtents, density));

	dsQuaternion4f orientationInv;
	dsQuaternion4f_conjugate(&orientationInv, &orientation);
	dsVector3f orientationSpaceCenter;
	dsQuaternion4f_rotate(&orientationSpaceCenter, &orientation, &box.center);

	ASSERT_TRUE(dsPhysicsMassProperties_transform(
		&massProperties, &orientationSpaceCenter, NULL, NULL));
	ASSERT_TRUE(dsPhysicsMassProperties_shift(&massProperties, NULL, &orientationInv));

	EXPECT_NEAR(meshMassProperties.mass, massProperties.mass, massEpsilon);
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(meshMassProperties.centeredInertia.values[i][j],
				massProperties.centeredInertia.values[i][j], inertiaEpsilon) << i << ", " << j;
		}
	}
	EXPECT_FLOAT_EQ(box.center.x, massProperties.centerOfMass.x);
	EXPECT_FLOAT_EQ(box.center.y, massProperties.centerOfMass.y);
	EXPECT_FLOAT_EQ(box.center.z, massProperties.centerOfMass.z);
	EXPECT_FLOAT_EQ(box.center.x, massProperties.inertiaTranslate.x);
	EXPECT_FLOAT_EQ(box.center.y, massProperties.inertiaTranslate.y);
	EXPECT_FLOAT_EQ(box.center.z, massProperties.inertiaTranslate.z);
	EXPECT_FLOAT_EQ(orientationInv.i, massProperties.inertiaRotate.i);
	EXPECT_FLOAT_EQ(orientationInv.j, massProperties.inertiaRotate.j);
	EXPECT_FLOAT_EQ(orientationInv.k, massProperties.inertiaRotate.k);
	EXPECT_FLOAT_EQ(orientationInv.r, massProperties.inertiaRotate.r);

	// Rotate first.
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&massProperties, &box.halfExtents, density));
	ASSERT_TRUE(dsPhysicsMassProperties_shift(&massProperties, NULL, &orientationInv));
	ASSERT_TRUE(dsPhysicsMassProperties_transform(
		&massProperties, &box.center, NULL, NULL));

	EXPECT_NEAR(meshMassProperties.mass, massProperties.mass, massEpsilon);
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NEAR(meshMassProperties.centeredInertia.values[i][j],
				massProperties.centeredInertia.values[i][j], inertiaEpsilon) << i << ", " << j;
		}
	}
	EXPECT_FLOAT_EQ(box.center.x, massProperties.centerOfMass.x);
	EXPECT_FLOAT_EQ(box.center.y, massProperties.centerOfMass.y);
	EXPECT_FLOAT_EQ(box.center.z, massProperties.centerOfMass.z);
	EXPECT_FLOAT_EQ(box.center.x, massProperties.inertiaTranslate.x);
	EXPECT_FLOAT_EQ(box.center.y, massProperties.inertiaTranslate.y);
	EXPECT_FLOAT_EQ(box.center.z, massProperties.inertiaTranslate.z);
	EXPECT_FLOAT_EQ(orientationInv.i, massProperties.inertiaRotate.i);
	EXPECT_FLOAT_EQ(orientationInv.j, massProperties.inertiaRotate.j);
	EXPECT_FLOAT_EQ(orientationInv.k, massProperties.inertiaRotate.k);
	EXPECT_FLOAT_EQ(orientationInv.r, massProperties.inertiaRotate.r);
}

TEST(PhysicsMassPropertiesTest, DecomposeInertia)
{
	float width = 2.0f;
	float height = 3.0f;
	float depth = 4.0f;
	float density = 2.5f;

	dsQuaternion4f orientation;
	dsQuaternion4f_fromEulerAngles(&orientation, dsDegreesToRadiansf(-5.0f),
		dsDegreesToRadiansf(45.0f), dsDegreesToRadiansf(-65.0f));
	dsVector3f translate = {{5.0f, -10.0f, 15.0f}};

	dsVector3f halfExtents = {{width/2, height/2, depth/2}};
	dsPhysicsMassProperties massProperties;
	ASSERT_TRUE(dsPhysicsMassProperties_initializeBox(&massProperties, &halfExtents, density));
	ASSERT_TRUE(dsPhysicsMassProperties_shift(&massProperties, &translate, &orientation));

	dsMatrix33f rotate;
	dsVector3f diagonal;
	EXPECT_TRUE(dsPhysicsMassProperties_getDecomposedInertia(&rotate, &diagonal, &massProperties));
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			EXPECT_NE(1, rotate.values[i][j]);
			EXPECT_NE(0, rotate.values[i][j]);
		}
	}

	dsMatrix33f rotateTrans, diagonalMat;
	dsMatrix33_transpose(rotateTrans, rotate);
	dsMatrix33f_makeScale3D(&diagonalMat, diagonal.x, diagonal.y, diagonal.z);

	dsMatrix33f temp, restoredInertia;
	dsMatrix33_mul(temp, rotate, diagonalMat);
	dsMatrix33_mul(restoredInertia, temp, rotateTrans);

	dsMatrix33f inertia;
	ASSERT_TRUE(dsPhysicsMassProperties_getInertia(&inertia, &massProperties));

	const float decomposeEpsilon = 4e-3f;
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
			EXPECT_NEAR(inertia.values[i][j], restoredInertia.values[i][j], decomposeEpsilon);
	}
}
