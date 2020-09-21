/*
 * Copyright 2020 Aaron Barany
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
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/SceneLighting/SceneLight.h>

#include <float.h>
#include <gtest/gtest.h>

static dsVector3f zero;

TEST(SceneLightTest, MakeDirectional)
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

TEST(SceneLightTest, MakePoint)
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

TEST(SceneLightTest, MakeSpot)
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

TEST(SceneLightTest, GetFalloff)
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

TEST(SceneLightTest, ComputeBounds)
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

	EXPECT_EQ(-FLT_MAX, bounds.min.x);
	EXPECT_EQ(-FLT_MAX, bounds.min.y);
	EXPECT_EQ(-FLT_MAX, bounds.min.z);
	EXPECT_EQ(FLT_MAX, bounds.max.x);
	EXPECT_EQ(FLT_MAX, bounds.max.y);
	EXPECT_EQ(FLT_MAX, bounds.max.z);

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
