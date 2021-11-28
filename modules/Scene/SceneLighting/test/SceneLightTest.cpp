/*
 * Copyright 2020-2021 Aaron Barany
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

#include "FixtureBase.h"

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Packing.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/SceneLighting/SceneLight.h>

#include <gtest/gtest.h>

#include <cstring>
#include <limits>

class SceneLightTest : public FixtureBase
{
};

static dsVector3f zero;

static void computeNormal(dsVector3f& outNormal, const dsVector3f& p0, const dsVector3f& p1,
	const dsVector3f& p2)
{
	dsVector3f a, b;
	dsVector3_sub(a, p1, p0);
	dsVector3_sub(b, p2, p0);
	dsVector3_cross(outNormal, a, b);
}

TEST_F(SceneLightTest, GetAmbientLightVertexFormat)
{
	EXPECT_FALSE(dsSceneLight_getAmbientLightVertexFormat(nullptr));

	dsVertexFormat format;
	dsSceneLight_getAmbientLightVertexFormat(&format);
	EXPECT_EQ(sizeof(dsAmbientLightVertex), format.size);

	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Position));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Color));

	EXPECT_EQ(offsetof(dsAmbientLightVertex, position),
		format.elements[dsVertexAttrib_Position].offset);
	EXPECT_EQ(offsetof(dsAmbientLightVertex, color), format.elements[dsVertexAttrib_Color].offset);
}

TEST_F(SceneLightTest, GetDirectionalLightVertexFormat)
{
	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertexFormat(nullptr));

	dsVertexFormat format;
	dsSceneLight_getDirectionalLightVertexFormat(&format);
	EXPECT_EQ(sizeof(dsDirectionalLightVertex), format.size);

	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Position));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Normal));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Color));

	EXPECT_EQ(offsetof(dsDirectionalLightVertex, position),
		format.elements[dsVertexAttrib_Position].offset);
	EXPECT_EQ(offsetof(dsDirectionalLightVertex, direction),
		format.elements[dsVertexAttrib_Normal].offset);
	EXPECT_EQ(offsetof(dsDirectionalLightVertex, color),
		format.elements[dsVertexAttrib_Color].offset);
}

TEST_F(SceneLightTest, GetPointLightVertexFormat)
{
	EXPECT_FALSE(dsSceneLight_getPointLightVertexFormat(nullptr));

	dsVertexFormat format;
	dsSceneLight_getPointLightVertexFormat(&format);
	EXPECT_EQ(sizeof(dsPointLightVertex), format.size);

	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Position0));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Position1));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Color));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_TexCoord0));

	EXPECT_EQ(offsetof(dsPointLightVertex, vertexPosition),
		format.elements[dsVertexAttrib_Position0].offset);
	EXPECT_EQ(offsetof(dsPointLightVertex, lightPosition),
		format.elements[dsVertexAttrib_Position1].offset);
	EXPECT_EQ(offsetof(dsPointLightVertex, color), format.elements[dsVertexAttrib_Color].offset);
	EXPECT_EQ(offsetof(dsPointLightVertex, falloff),
		format.elements[dsVertexAttrib_TexCoord0].offset);
}

TEST_F(SceneLightTest, GetSpotLightVertexFormat)
{
	EXPECT_FALSE(dsSceneLight_getSpotLightVertexFormat(nullptr));

	dsVertexFormat format;
	dsSceneLight_getSpotLightVertexFormat(&format);
	EXPECT_EQ(sizeof(dsSpotLightVertex), format.size);

	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Position0));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Position1));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Normal));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_Color));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&format, dsVertexAttrib_TexCoord0));

	EXPECT_EQ(offsetof(dsSpotLightVertex, vertexPosition),
		format.elements[dsVertexAttrib_Position0].offset);
	EXPECT_EQ(offsetof(dsSpotLightVertex, lightPosition),
		format.elements[dsVertexAttrib_Position1].offset);
	EXPECT_EQ(offsetof(dsSpotLightVertex, direction),
		format.elements[dsVertexAttrib_Normal].offset);
	EXPECT_EQ(offsetof(dsSpotLightVertex, color), format.elements[dsVertexAttrib_Color].offset);
	EXPECT_EQ(offsetof(dsSpotLightVertex, falloffAndSpotAngles),
		format.elements[dsVertexAttrib_TexCoord0].offset);
}

TEST_F(SceneLightTest, MakeDirectional)
{
	dsSceneLight light;
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;

	EXPECT_FALSE(dsSceneLight_makeDirectional(nullptr, &direction, &color, intensity));
	EXPECT_FALSE(dsSceneLight_makeDirectional(&light, nullptr, &color, intensity));
	EXPECT_FALSE(dsSceneLight_makeDirectional(&light, &direction, nullptr, intensity));

	EXPECT_TRUE(dsSceneLight_makeDirectional(&light, &direction, &color, intensity));
	EXPECT_EQ(dsSceneLightType_Directional, light.type);
	EXPECT_TRUE(dsVector3_equal(zero, light.position));
	EXPECT_TRUE(dsVector3_equal(direction, light.direction));
	EXPECT_TRUE(dsVector3_equal(color, light.color));
	EXPECT_EQ(intensity, light.intensity);
	EXPECT_EQ(0.0f, light.linearFalloff);
	EXPECT_EQ(0.0f, light.quadraticFalloff);
	EXPECT_EQ(0.0f, light.innerSpotCosAngle);
	EXPECT_EQ(0.0f, light.outerSpotCosAngle);
}

TEST_F(SceneLightTest, MakePoint)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;

	EXPECT_FALSE(dsSceneLight_makePoint(nullptr, &position, &color, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_FALSE(dsSceneLight_makePoint(&light, nullptr, &color, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_FALSE(dsSceneLight_makePoint(&light, &position, nullptr, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_FALSE(dsSceneLight_makePoint(&light, &position, &color, intensity, -1,
		quadraticFalloff));
	EXPECT_FALSE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff, -1));

	EXPECT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_EQ(dsSceneLightType_Point, light.type);
	EXPECT_TRUE(dsVector3_equal(position, light.position));
	EXPECT_TRUE(dsVector3_equal(zero, light.direction));
	EXPECT_TRUE(dsVector3_equal(color, light.color));
	EXPECT_EQ(intensity, light.intensity);
	EXPECT_EQ(linearFalloff, light.linearFalloff);
	EXPECT_EQ(quadraticFalloff, light.quadraticFalloff);
	EXPECT_EQ(0.0f, light.innerSpotCosAngle);
	EXPECT_EQ(0.0f, light.outerSpotCosAngle);
}

TEST_F(SceneLightTest, MakeSpot)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	EXPECT_FALSE(dsSceneLight_makeSpot(nullptr, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_makeSpot(&light, nullptr, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_makeSpot(&light, &position, nullptr, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_makeSpot(&light, &position, &direction, nullptr, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity, -1,
		quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, -1, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, outerSpotCosAngle, innerSpotCosAngle));

	EXPECT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_EQ(dsSceneLightType_Spot, light.type);
	EXPECT_TRUE(dsVector3_equal(position, light.position));
	EXPECT_TRUE(dsVector3_equal(direction, light.direction));
	EXPECT_TRUE(dsVector3_equal(color, light.color));
	EXPECT_EQ(intensity, light.intensity);
	EXPECT_EQ(linearFalloff, light.linearFalloff);
	EXPECT_EQ(quadraticFalloff, light.quadraticFalloff);
	EXPECT_EQ(innerSpotCosAngle, light.innerSpotCosAngle);
	EXPECT_EQ(outerSpotCosAngle, light.outerSpotCosAngle);
}

TEST_F(SceneLightTest, GetFalloff)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	dsVector3f objectPos = {{4.0f, 1.0f, 0.0f}};
	float distance = dsVector3f_dist(&position, &objectPos);

	EXPECT_TRUE(dsSceneLight_makeDirectional(&light, &direction, &color, intensity));
	EXPECT_EQ(1.0f, dsSceneLight_getFalloff(&light, &objectPos));

	EXPECT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));

	float expectedFalloff = 1.0f/(1.0f + linearFalloff*distance + quadraticFalloff*dsPow2(distance));
	EXPECT_NEAR(expectedFalloff, dsSceneLight_getFalloff(&light, &objectPos), 1e-6f);

	EXPECT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));

	dsVector3f posDir;
	dsVector3_sub(posDir, objectPos, position);
	dsVector3f_normalize(&posDir, &posDir);
	float cosAngle = dsVector3_dot(posDir, direction);
	float spotFalloff = (cosAngle - outerSpotCosAngle)/(innerSpotCosAngle - outerSpotCosAngle);
	EXPECT_NEAR(expectedFalloff*spotFalloff, dsSceneLight_getFalloff(&light, &objectPos), 1e-6f);

	objectPos.x = 1.0f;
	EXPECT_EQ(0.0f, dsSceneLight_getFalloff(&light, &objectPos));
}

TEST_F(SceneLightTest, ComputeBounds)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	dsAlignedBox3f bounds;
	EXPECT_TRUE(dsSceneLight_makeDirectional(&light, &direction, &color, intensity));
	EXPECT_TRUE(dsSceneLight_computeBounds(&bounds, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	EXPECT_EQ(std::numeric_limits<float>::lowest(), bounds.min.x);
	EXPECT_EQ(std::numeric_limits<float>::lowest(), bounds.min.y);
	EXPECT_EQ(std::numeric_limits<float>::lowest(), bounds.min.z);
	EXPECT_EQ(std::numeric_limits<float>::max(), bounds.max.x);
	EXPECT_EQ(std::numeric_limits<float>::max(), bounds.max.y);
	EXPECT_EQ(std::numeric_limits<float>::max(), bounds.max.z);

	EXPECT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_TRUE(dsSceneLight_computeBounds(&bounds, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	dsVector3f corners[DS_BOX3_CORNER_COUNT];
	dsAlignedBox3_corners(corners, bounds);
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		EXPECT_GE(DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD,
			dsSceneLight_getIntensity(&light, corners + i));
	}

	EXPECT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_TRUE(dsSceneLight_computeBounds(&bounds, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	dsAlignedBox3_corners(corners, bounds);
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		EXPECT_GE(DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD,
			dsSceneLight_getIntensity(&light, corners + i));
	}
}

TEST_F(SceneLightTest, IsInFrustum)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	// NOTE: Z is inverted for ortho matrices.
	dsMatrix44f matrix;
	dsMatrix44f_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	dsFrustum3f frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);
	dsFrustum3f_normalize(&frustum);

	EXPECT_TRUE(dsSceneLight_makeDirectional(&light, &direction, &color, intensity));
	EXPECT_TRUE(dsSceneLight_isInFrustum(&light, &frustum,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	EXPECT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_TRUE(dsSceneLight_isInFrustum(&light, &frustum,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));
	light.position.x = -4.0f;
	EXPECT_FALSE(dsSceneLight_isInFrustum(&light, &frustum,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));
	light.position.x = 3.1f;
	EXPECT_TRUE(dsSceneLight_isInFrustum(&light, &frustum,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	EXPECT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_TRUE(dsSceneLight_isInFrustum(&light, &frustum,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));
	light.position.x = -4.0f;
	EXPECT_FALSE(dsSceneLight_isInFrustum(&light, &frustum,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));
	light.position.x = 3.1f;
	EXPECT_FALSE(dsSceneLight_isInFrustum(&light, &frustum,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));
}

TEST_F(SceneLightTest, GetPointLightProjection)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;

	ASSERT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));

	dsMatrix44f transform;
	dsMatrix44f projection;
	EXPECT_FALSE(dsSceneLight_getSpotLightTransform(&transform, &light));
	EXPECT_FALSE(dsSceneLight_getSpotLightProjection(&projection, &light, renderer,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	dsFrustum3f frustums[6];
	for (int i = 0; i < 6; ++i)
	{
		ASSERT_TRUE(dsSceneLight_getPointLightTransform(&transform, &light, (dsCubeFace)i));
		ASSERT_TRUE(dsSceneLight_getPointLightProjection(&projection, &light, renderer,
			DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

		dsMatrix44f pointProjection;
		dsMatrix44_mul(pointProjection, projection, transform);
		ASSERT_TRUE(dsRenderer_frustumFromMatrix(frustums + i, renderer, &pointProjection));
	}

	dsVector3f testPos = {{2.0f, 2.0f, 3.0f}};
	for (int i = 0; i < 6; ++i)
	{
		dsIntersectResult result = i == dsCubeFace_PosX ? dsIntersectResult_Inside :
			dsIntersectResult_Outside;
		EXPECT_EQ(result, dsFrustum3f_intersectSphere(frustums + i, &testPos, 0.1f));
	}

	testPos.x = 0.0f;
	for (int i = 0; i < 6; ++i)
	{
		dsIntersectResult result = i == dsCubeFace_NegX ? dsIntersectResult_Inside :
			dsIntersectResult_Outside;
		EXPECT_EQ(result, dsFrustum3f_intersectSphere(frustums + i, &testPos, 0.1f));
	}

	testPos.x = 1.0f;
	testPos.y = 3.0f;
	for (int i = 0; i < 6; ++i)
	{
		dsIntersectResult result = i == dsCubeFace_PosY ? dsIntersectResult_Inside :
			dsIntersectResult_Outside;
		EXPECT_EQ(result, dsFrustum3f_intersectSphere(frustums + i, &testPos, 0.1f));
	}

	testPos.y = 1.0f;
	for (int i = 0; i < 6; ++i)
	{
		dsIntersectResult result = i == dsCubeFace_NegY ? dsIntersectResult_Inside :
			dsIntersectResult_Outside;
		EXPECT_EQ(result, dsFrustum3f_intersectSphere(frustums + i, &testPos, 0.1f));
	}

	testPos.y = 2.0f;
	testPos.z = 4.0f;
	for (int i = 0; i < 6; ++i)
	{
		dsIntersectResult result = i == dsCubeFace_PosZ ? dsIntersectResult_Inside :
			dsIntersectResult_Outside;
		EXPECT_EQ(result, dsFrustum3f_intersectSphere(frustums + i, &testPos, 0.1f));
	}

	testPos.z = 2.0f;
	for (int i = 0; i < 6; ++i)
	{
		dsIntersectResult result = i == dsCubeFace_NegZ ? dsIntersectResult_Inside :
			dsIntersectResult_Outside;
		EXPECT_EQ(result, dsFrustum3f_intersectSphere(frustums + i, &testPos, 0.1f));
	}

	testPos.x = 2.0f;
	testPos.y = 3.0f;
	testPos.z = 4.0f;
	for (int i = 0; i < 6; ++i)
	{
		dsIntersectResult result =
			i == dsCubeFace_PosX || i == dsCubeFace_PosY || i == dsCubeFace_PosZ ?
				dsIntersectResult_Intersects : dsIntersectResult_Outside;
		EXPECT_EQ(result, dsFrustum3f_intersectSphere(frustums + i, &testPos, 0.1f));
	}
}

TEST_F(SceneLightTest, GetSpotLightProjection)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	ASSERT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));

	dsMatrix44f transform;
	EXPECT_FALSE(dsSceneLight_getPointLightTransform(&transform, &light,
		dsCubeFace_PosX));
	ASSERT_TRUE(dsSceneLight_getSpotLightTransform(&transform, &light));

	dsMatrix44f projection;
	EXPECT_FALSE(dsSceneLight_getPointLightProjection(&projection, &light, renderer,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));
	ASSERT_TRUE(dsSceneLight_getSpotLightProjection(&projection, &light, renderer,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	dsMatrix44f spotProjection;
	dsMatrix44_mul(spotProjection, projection, transform);
	dsFrustum3f frustum;
	ASSERT_TRUE(dsRenderer_frustumFromMatrix(&frustum, renderer, &spotProjection));

	dsVector3f testPos = {{2.0f, 2.0f, 3.0f}};
	EXPECT_EQ(dsIntersectResult_Inside, dsFrustum3f_intersectSphere(&frustum, &testPos, 0.1f));

	testPos.x = 10.0f;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3f_intersectSphere(&frustum, &testPos, 0.1f));

	testPos.x = 2.0f;
	testPos.y = 2.3f;
	EXPECT_EQ(dsIntersectResult_Inside, dsFrustum3f_intersectSphere(&frustum, &testPos, 0.1f));

	testPos.y = 4.0f;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3f_intersectSphere(&frustum, &testPos, 0.1f));
}

TEST_F(SceneLightTest, GetAmbientLightVertices)
{
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};

	dsAmbientLightVertex lightVertices[DS_AMBIENT_LIGHT_VERTEX_COUNT];
	uint16_t lightIndices[DS_AMBIENT_LIGHT_INDEX_COUNT];

	EXPECT_FALSE(dsSceneLight_getAmbientLightVertices(nullptr,
		DS_AMBIENT_LIGHT_VERTEX_COUNT, lightIndices, DS_AMBIENT_LIGHT_INDEX_COUNT, &color, 0));
	EXPECT_FALSE(dsSceneLight_getAmbientLightVertices(lightVertices,
		DS_AMBIENT_LIGHT_VERTEX_COUNT - 1, lightIndices, DS_AMBIENT_LIGHT_INDEX_COUNT, &color, 0));
	EXPECT_FALSE(dsSceneLight_getAmbientLightVertices(lightVertices,
		DS_AMBIENT_LIGHT_VERTEX_COUNT, nullptr, DS_AMBIENT_LIGHT_INDEX_COUNT, &color, 0));
	EXPECT_FALSE(dsSceneLight_getAmbientLightVertices(lightVertices,
		DS_AMBIENT_LIGHT_VERTEX_COUNT, lightIndices, DS_AMBIENT_LIGHT_INDEX_COUNT - 1, &color, 0));
	EXPECT_FALSE(dsSceneLight_getAmbientLightVertices(lightVertices,
		DS_AMBIENT_LIGHT_VERTEX_COUNT, lightIndices, DS_AMBIENT_LIGHT_INDEX_COUNT, nullptr, 0));

	EXPECT_TRUE(dsSceneLight_getAmbientLightVertices(lightVertices,
		DS_AMBIENT_LIGHT_VERTEX_COUNT, lightIndices, DS_AMBIENT_LIGHT_INDEX_COUNT, &color, 0));

	dsHalfFloat expectedPackedColor[4] = {dsPackHalfFloat(color.r), dsPackHalfFloat(color.g),
		dsPackHalfFloat(color.b), {0}};
	for (unsigned int i = 0; i < DS_DIRECTIONAL_LIGHT_VERTEX_COUNT; ++i)
	{
		EXPECT_EQ(0, std::memcmp(expectedPackedColor, lightVertices[i].color,
			sizeof(expectedPackedColor)));
	}

	EXPECT_EQ(dsPackInt16(-1), lightVertices[0].position[0]);
	EXPECT_EQ(dsPackInt16(-1), lightVertices[0].position[1]);

	EXPECT_EQ(dsPackInt16(1), lightVertices[1].position[0]);
	EXPECT_EQ(dsPackInt16(-1), lightVertices[1].position[1]);

	EXPECT_EQ(dsPackInt16(1), lightVertices[2].position[0]);
	EXPECT_EQ(dsPackInt16(1), lightVertices[2].position[1]);

	EXPECT_EQ(dsPackInt16(-1), lightVertices[3].position[0]);
	EXPECT_EQ(dsPackInt16(1), lightVertices[3].position[1]);

	EXPECT_EQ(0U, lightIndices[0]);
	EXPECT_EQ(1U, lightIndices[1]);
	EXPECT_EQ(2U, lightIndices[2]);

	EXPECT_EQ(0U, lightIndices[3]);
	EXPECT_EQ(2U, lightIndices[4]);
	EXPECT_EQ(3U, lightIndices[5]);
}

TEST_F(SceneLightTest, GetDirectionalLightVertices)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	dsDirectionalLightVertex lightVertices[DS_DIRECTIONAL_LIGHT_VERTEX_COUNT];
	uint16_t lightIndices[DS_DIRECTIONAL_LIGHT_INDEX_COUNT];

	ASSERT_TRUE(dsSceneLight_makeDirectional(&light, &direction, &color, intensity));

	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertices(nullptr,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, lightIndices, DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		&light, 0));
	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertices(lightVertices,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT - 1, lightIndices, DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		&light, 0));
	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertices(lightVertices,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, nullptr, DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		&light, 0));
	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertices(lightVertices,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, lightIndices, DS_DIRECTIONAL_LIGHT_INDEX_COUNT - 1,
		&light, 0));
	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertices(lightVertices,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, lightIndices, DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		nullptr, 0));

	EXPECT_TRUE(dsSceneLight_getDirectionalLightVertices(lightVertices,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, lightIndices, DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		&light, 0));

	int16_t expectedPackedDirection[4] = {dsPackInt16(-direction.x), dsPackInt16(-direction.y),
		dsPackInt16(-direction.z), 0};
	dsHalfFloat expectedPackedColor[4] = {dsPackHalfFloat(color.r*intensity),
		dsPackHalfFloat(color.g*intensity), dsPackHalfFloat(color.b*intensity), {0}};
	for (unsigned int i = 0; i < DS_DIRECTIONAL_LIGHT_VERTEX_COUNT; ++i)
	{
		EXPECT_EQ(0, std::memcmp(expectedPackedDirection, lightVertices[i].direction,
			sizeof(expectedPackedDirection)));
		EXPECT_EQ(0, std::memcmp(expectedPackedColor, lightVertices[i].color,
			sizeof(expectedPackedColor)));
	}

	EXPECT_EQ(dsPackInt16(-1), lightVertices[0].position[0]);
	EXPECT_EQ(dsPackInt16(-1), lightVertices[0].position[1]);

	EXPECT_EQ(dsPackInt16(1), lightVertices[1].position[0]);
	EXPECT_EQ(dsPackInt16(-1), lightVertices[1].position[1]);

	EXPECT_EQ(dsPackInt16(1), lightVertices[2].position[0]);
	EXPECT_EQ(dsPackInt16(1), lightVertices[2].position[1]);

	EXPECT_EQ(dsPackInt16(-1), lightVertices[3].position[0]);
	EXPECT_EQ(dsPackInt16(1), lightVertices[3].position[1]);

	EXPECT_EQ(0U, lightIndices[0]);
	EXPECT_EQ(1U, lightIndices[1]);
	EXPECT_EQ(2U, lightIndices[2]);

	EXPECT_EQ(0U, lightIndices[3]);
	EXPECT_EQ(2U, lightIndices[4]);
	EXPECT_EQ(3U, lightIndices[5]);

	EXPECT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertices(lightVertices,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, lightIndices, DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		&light, 0));

	EXPECT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_getDirectionalLightVertices(lightVertices,
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, lightIndices, DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		&light, 0));
}

TEST_F(SceneLightTest, GetPointLightVertices)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	dsPointLightVertex lightVertices[DS_POINT_LIGHT_VERTEX_COUNT];
	uint16_t lightIndices[DS_POINT_LIGHT_INDEX_COUNT];

	EXPECT_TRUE(dsSceneLight_makeDirectional(&light, &direction, &color, intensity));
	EXPECT_FALSE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));

	EXPECT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));

	EXPECT_FALSE(dsSceneLight_getPointLightVertices(nullptr, DS_POINT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT - 1,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT,
		nullptr, DS_POINT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT - 1, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT, nullptr,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT, &light, 0.0f, 0));

	ASSERT_TRUE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));

	dsHalfFloat expectedPackedColor[4] = {dsPackHalfFloat(color.r*intensity),
		dsPackHalfFloat(color.g*intensity), dsPackHalfFloat(color.b*intensity), {0}};
	dsHalfFloat expectedFalloff[2] = {dsPackHalfFloat(linearFalloff),
		dsPackHalfFloat(quadraticFalloff)};
	for (unsigned int i = 0; i < DS_POINT_LIGHT_VERTEX_COUNT; ++i)
	{
		EXPECT_EQ(0, std::memcmp(expectedPackedColor, lightVertices[i].color,
			sizeof(expectedPackedColor)));
		EXPECT_EQ(0, std::memcmp(expectedFalloff, lightVertices[i].falloff,
			sizeof(expectedFalloff)));
	}

	const float epsilon = 1e-6f;
	dsVector3f normal;

	// Make sure that the box triangels face inward.
	// front
	computeNormal(normal, lightVertices[lightIndices[0]].vertexPosition,
		lightVertices[lightIndices[1]].vertexPosition,
		lightVertices[lightIndices[2]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_GT(0.0f, normal.z);

	computeNormal(normal, lightVertices[lightIndices[3]].vertexPosition,
		lightVertices[lightIndices[4]].vertexPosition,
		lightVertices[lightIndices[5]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_GT(0.0f, normal.z);

	// right
	computeNormal(normal, lightVertices[lightIndices[6]].vertexPosition,
		lightVertices[lightIndices[7]].vertexPosition,
		lightVertices[lightIndices[8]].vertexPosition);
	EXPECT_GT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	computeNormal(normal, lightVertices[lightIndices[9]].vertexPosition,
		lightVertices[lightIndices[10]].vertexPosition,
		lightVertices[lightIndices[11]].vertexPosition);
	EXPECT_GT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	// back
	computeNormal(normal, lightVertices[lightIndices[12]].vertexPosition,
		lightVertices[lightIndices[13]].vertexPosition,
		lightVertices[lightIndices[14]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_LT(0.0f, normal.z);

	computeNormal(normal, lightVertices[lightIndices[15]].vertexPosition,
		lightVertices[lightIndices[16]].vertexPosition,
		lightVertices[lightIndices[17]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_LT(0.0f, normal.z);

	// left
	computeNormal(normal, lightVertices[lightIndices[18]].vertexPosition,
		lightVertices[lightIndices[19]].vertexPosition,
		lightVertices[lightIndices[20]].vertexPosition);
	EXPECT_LT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	computeNormal(normal, lightVertices[lightIndices[21]].vertexPosition,
		lightVertices[lightIndices[22]].vertexPosition,
		lightVertices[lightIndices[23]].vertexPosition);
	EXPECT_LT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	// bottom
	computeNormal(normal, lightVertices[lightIndices[24]].vertexPosition,
		lightVertices[lightIndices[25]].vertexPosition,
		lightVertices[lightIndices[26]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_LT(0.0f, normal.y);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	computeNormal(normal, lightVertices[lightIndices[27]].vertexPosition,
		lightVertices[lightIndices[28]].vertexPosition,
		lightVertices[lightIndices[29]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_LT(0.0f, normal.y);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	// top
	computeNormal(normal, lightVertices[lightIndices[30]].vertexPosition,
		lightVertices[lightIndices[31]].vertexPosition,
		lightVertices[lightIndices[32]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_GT(0.0f, normal.y);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	computeNormal(normal, lightVertices[lightIndices[33]].vertexPosition,
		lightVertices[lightIndices[34]].vertexPosition,
		lightVertices[lightIndices[35]].vertexPosition);
	EXPECT_NEAR(0.0f, normal.x, epsilon);
	EXPECT_GT(0.0f, normal.y);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	EXPECT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));
	EXPECT_FALSE(dsSceneLight_getPointLightVertices(lightVertices, DS_POINT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_POINT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
}

TEST_F(SceneLightTest, GetSpotLightVertices)
{
	dsSceneLight light;
	dsVector3f position = {{1.0f, 2.0f, 3.0f}};
	dsVector3f direction = {{1.0f, 0.0f, 0.0f}};
	dsColor3f color = {{0.1f, 0.2f, 0.3f}};
	float intensity = 3.5f;
	float linearFalloff = 1.0f;
	float quadraticFalloff = 2.0f;
	float innerSpotCosAngle = 0.75f;
	float outerSpotCosAngle = 0.5f;

	dsSpotLightVertex lightVertices[DS_SPOT_LIGHT_VERTEX_COUNT];
	uint16_t lightIndices[DS_SPOT_LIGHT_INDEX_COUNT];

	EXPECT_TRUE(dsSceneLight_makeDirectional(&light, &direction, &color, intensity));
	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));

	EXPECT_TRUE(dsSceneLight_makePoint(&light, &position, &color, intensity, linearFalloff,
		quadraticFalloff));
	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));

	EXPECT_TRUE(dsSceneLight_makeSpot(&light, &position, &direction, &color, intensity,
		linearFalloff, quadraticFalloff, innerSpotCosAngle, outerSpotCosAngle));

	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(nullptr, DS_SPOT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT - 1,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT,
		nullptr, DS_SPOT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT - 1, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT, nullptr,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));
	EXPECT_FALSE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT, &light, 0.0f, 0));

	ASSERT_TRUE(dsSceneLight_getSpotLightVertices(lightVertices, DS_SPOT_LIGHT_VERTEX_COUNT,
		lightIndices, DS_SPOT_LIGHT_INDEX_COUNT, &light,
		DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD, 0));

	int16_t expectedPackedDirection[4] = {dsPackInt16(-direction.x), dsPackInt16(-direction.y),
		dsPackInt16(-direction.z), 0};
	dsHalfFloat expectedPackedColor[4] = {dsPackHalfFloat(color.r*intensity),
		dsPackHalfFloat(color.g*intensity), dsPackHalfFloat(color.b*intensity), {0}};
	dsHalfFloat expectedFalloffAndSpotAngles[4] = {dsPackHalfFloat(linearFalloff),
		dsPackHalfFloat(quadraticFalloff), dsPackHalfFloat(innerSpotCosAngle),
		dsPackHalfFloat(outerSpotCosAngle)};
	for (unsigned int i = 0; i < DS_SPOT_LIGHT_VERTEX_COUNT; ++i)
	{
		EXPECT_EQ(0, std::memcmp(expectedPackedDirection, lightVertices[i].direction,
			sizeof(expectedPackedDirection)));
		EXPECT_EQ(0, std::memcmp(expectedPackedColor, lightVertices[i].color,
			sizeof(expectedPackedColor)));
		EXPECT_EQ(0, std::memcmp(expectedFalloffAndSpotAngles,
			lightVertices[i].falloffAndSpotAngles, sizeof(expectedFalloffAndSpotAngles)));
	}

	const float epsilon = 1e-6f;
	dsVector3f normal;

	// Make sure that the box triangels face inward.
	// left
	computeNormal(normal, lightVertices[lightIndices[0]].vertexPosition,
		lightVertices[lightIndices[1]].vertexPosition,
		lightVertices[lightIndices[2]].vertexPosition);
	EXPECT_LT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_LT(0.0f, normal.z);

	// bottom
	computeNormal(normal, lightVertices[lightIndices[3]].vertexPosition,
		lightVertices[lightIndices[4]].vertexPosition,
		lightVertices[lightIndices[5]].vertexPosition);
	EXPECT_LT(0.0f, normal.x);
	EXPECT_LT(0.0f, normal.y);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	// right
	computeNormal(normal, lightVertices[lightIndices[6]].vertexPosition,
		lightVertices[lightIndices[7]].vertexPosition,
		lightVertices[lightIndices[8]].vertexPosition);
	EXPECT_LT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_GT(0.0f, normal.z);

	// left
	computeNormal(normal, lightVertices[lightIndices[9]].vertexPosition,
		lightVertices[lightIndices[10]].vertexPosition,
		lightVertices[lightIndices[11]].vertexPosition);
	EXPECT_LT(0.0f, normal.x);
	EXPECT_GT(0.0f, normal.y);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	// back
	computeNormal(normal, lightVertices[lightIndices[12]].vertexPosition,
		lightVertices[lightIndices[13]].vertexPosition,
		lightVertices[lightIndices[14]].vertexPosition);
	EXPECT_GT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_NEAR(0.0f, normal.z, epsilon);

	computeNormal(normal, lightVertices[lightIndices[15]].vertexPosition,
		lightVertices[lightIndices[16]].vertexPosition,
		lightVertices[lightIndices[17]].vertexPosition);
	EXPECT_GT(0.0f, normal.x);
	EXPECT_NEAR(0.0f, normal.y, epsilon);
	EXPECT_NEAR(0.0f, normal.z, epsilon);
}
