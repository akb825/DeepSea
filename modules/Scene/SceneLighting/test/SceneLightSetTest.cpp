#include "FixtureBase.h"

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Scene/CustomSceneResource.h>
#include <DeepSea/SceneLighting/SceneLight.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <gtest/gtest.h>

class SceneLightSetTest : public FixtureBase
{
};

static bool hasLight(const dsSceneLight* const* lights, uint32_t lightCount,
	const dsSceneLight* light)
{
	for (uint32_t i = 0; i < lightCount; ++i)
	{
		if (lights[i] == light)
			return true;
	}

	return false;
}

static bool hasLight(const std::vector<const dsSceneLight*>& lights, const dsSceneLight* light)
{
	return hasLight(lights.data(), static_cast<uint32_t>(lights.size()), light);
}

static bool visitLight(void* userData, const dsSceneLightSet*, const dsSceneLight* light)
{
	auto& lights = *reinterpret_cast<std::vector<const dsSceneLight*>*>(userData);
	lights.push_back(light);
	return true;
}

TEST_F(SceneLightSetTest, Create)
{
	dsColor3f ambientColor = {{1.0f, 1.0f, 1.0f}};
	EXPECT_FALSE(dsSceneLightSet_create(nullptr, 100, &ambientColor, 0.1f));
	EXPECT_FALSE(dsSceneLightSet_create((dsAllocator*)&allocator, 0, &ambientColor, 0.1f));
	EXPECT_FALSE(dsSceneLightSet_create((dsAllocator*)&allocator, 100, nullptr, 0.1f));

	dsSceneLightSet* lightSet =
		dsSceneLightSet_create((dsAllocator*)&allocator, 100, &ambientColor, 0.1f);
	EXPECT_TRUE(lightSet);
	dsSceneLightSet_destroy(lightSet);
}

TEST_F(SceneLightSetTest, CreateResource)
{
	dsColor3f ambientColor = {{1.0f, 1.0f, 1.0f}};
	dsSceneLightSet* lightSet =
		dsSceneLightSet_create((dsAllocator*)&allocator, 100, &ambientColor, 0.1f);
	ASSERT_TRUE(lightSet);

	EXPECT_FALSE(dsSceneLightSet_createResource(nullptr, lightSet));
	EXPECT_FALSE(dsSceneLightSet_createResource((dsAllocator*)&allocator, nullptr));

	dsCustomSceneResource* resource =
		dsSceneLightSet_createResource((dsAllocator*)&allocator, lightSet);
	ASSERT_TRUE(resource);

	EXPECT_EQ(dsSceneLightSet_type(), resource->type);
	EXPECT_EQ(lightSet, resource->resource);
	EXPECT_TRUE(dsCustomSceneResource_destroy(resource));
}

TEST_F(SceneLightSetTest, AddRemoveLights)
{
	dsColor3f ambientColor = {{1.0f, 1.0f, 1.0f}};
	dsSceneLightSet* lightSet =
		dsSceneLightSet_create((dsAllocator*)&allocator, 3, &ambientColor, 0.1f);
	ASSERT_TRUE(lightSet);

	dsSceneLight* light1 = dsSceneLightSet_addLightName(lightSet, "first");
	ASSERT_TRUE(light1);
	EXPECT_EQ(2U, dsSceneLightSet_getRemainingLights(lightSet));
	EXPECT_FALSE(dsSceneLightSet_addLightName(lightSet, "first"));

	dsSceneLight* light2 = dsSceneLightSet_addLightName(lightSet, "second");
	ASSERT_TRUE(light2);
	EXPECT_EQ(1U, dsSceneLightSet_getRemainingLights(lightSet));

	dsSceneLight* light3 = dsSceneLightSet_addLightName(lightSet, "third");
	ASSERT_TRUE(light3);
	EXPECT_EQ(0U, dsSceneLightSet_getRemainingLights(lightSet));

	EXPECT_FALSE(dsSceneLightSet_addLightName(lightSet, "fourth"));

	EXPECT_EQ(light1, dsSceneLightSet_findLightName(lightSet, "first"));
	EXPECT_EQ(light2, dsSceneLightSet_findLightName(lightSet, "second"));
	EXPECT_EQ(light3, dsSceneLightSet_findLightName(lightSet, "third"));
	EXPECT_FALSE(dsSceneLightSet_findLightName(lightSet, "fourth"));

	EXPECT_TRUE(dsSceneLightSet_removeLightName(lightSet, "first"));
	EXPECT_EQ(1U, dsSceneLightSet_getRemainingLights(lightSet));
	EXPECT_FALSE(dsSceneLightSet_removeLightName(lightSet, "first"));

	EXPECT_FALSE(dsSceneLightSet_findLightName(lightSet, "first"));

	dsSceneLight* light4 = dsSceneLightSet_addLightName(lightSet, "fourth");
	ASSERT_TRUE(light4);
	EXPECT_EQ(0U, dsSceneLightSet_getRemainingLights(lightSet));
	EXPECT_EQ(light4, dsSceneLightSet_findLightName(lightSet, "fourth"));

	EXPECT_TRUE(dsSceneLightSet_clearLights(lightSet));

	dsSceneLightSet_destroy(lightSet);
}

TEST_F(SceneLightSetTest, GetSetAmbient)
{
	dsColor3f ambientColor = {{1.0f, 1.0f, 1.0f}};
	dsSceneLightSet* lightSet =
		dsSceneLightSet_create((dsAllocator*)&allocator, 199, &ambientColor, 0.1f);
	ASSERT_TRUE(lightSet);

	ASSERT_TRUE(dsSceneLightSet_getAmbientColor(lightSet));
	EXPECT_TRUE(dsVector3_equal(ambientColor, *dsSceneLightSet_getAmbientColor(lightSet)));
	EXPECT_EQ(0.1f, dsSceneLightSet_getAmbientIntensity(lightSet));

	ambientColor.r = 0.0f;
	EXPECT_TRUE(dsSceneLightSet_setAmbientColor(lightSet, &ambientColor));
	EXPECT_TRUE(dsVector3_equal(ambientColor, *dsSceneLightSet_getAmbientColor(lightSet)));

	EXPECT_TRUE(dsSceneLightSet_setAmbientIntensity(lightSet, 0.2f));
	EXPECT_EQ(0.2f, dsSceneLightSet_getAmbientIntensity(lightSet));

	ambientColor.g = 0.0f;
	EXPECT_TRUE(dsSceneLightSet_setAmbient(lightSet, &ambientColor, 0.3f));
	EXPECT_TRUE(dsVector3_equal(ambientColor, *dsSceneLightSet_getAmbientColor(lightSet)));
	EXPECT_EQ(0.3f, dsSceneLightSet_getAmbientIntensity(lightSet));

	dsSceneLightSet_destroy(lightSet);
}

TEST_F(SceneLightSetTest, FindBrightestLights)
{
	dsColor3f color = {{1.0f, 1.0f, 1.0f}};
	dsSceneLightSet* lightSet = dsSceneLightSet_create((dsAllocator*)&allocator, 4, &color, 0.1f);
	ASSERT_TRUE(lightSet);

	dsVector3f direction = {{0.0f, 0.0f, -1.0f}};
	dsSceneLight* light1 = dsSceneLightSet_addLightName(lightSet, "first");
	ASSERT_TRUE(dsSceneLight_makeDirectional(light1, &direction, &color, 1.0f));

	dsSceneLight* light2 = dsSceneLightSet_addLightName(lightSet, "second");
	ASSERT_TRUE(dsSceneLight_makeDirectional(light2, &direction, &color, 0.05f));

	dsVector3f position = {{-1.0f, 0.0f, 0.0f}};
	dsSceneLight* light3 = dsSceneLightSet_addLightName(lightSet, "third");
	ASSERT_TRUE(dsSceneLight_makePoint(light3, &position, &color, 1.0f, 1.0f, 1.0f));

	position.x = 1.0f;
	direction.x = -1.0f;
	direction.z = 0.0f;
	dsSceneLight* light4 = dsSceneLightSet_addLightName(lightSet, "fourth");
	ASSERT_TRUE(dsSceneLight_makeSpot(light4, &position, &direction, &color, 1.0f, 1.0f, 1.0f,
		0.5f, 0.5f));

	EXPECT_TRUE(dsSceneLightSet_prepare(lightSet, DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	const dsSceneLight* brightestLights[4];

	bool hasMain = false;
	position.x = 0.0f;
	uint32_t lightCount = 4;
	ASSERT_EQ(3U, dsSceneLightSet_findBrightestLights(brightestLights, lightCount, &hasMain,
		lightSet, &position));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light1));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light3));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light4));

	position.x = -0.5f;
	lightCount = 2;
	ASSERT_EQ(2U, dsSceneLightSet_findBrightestLights(brightestLights, lightCount, &hasMain,
		lightSet, &position));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light1));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light3));

	position.x = 0.5f;
	ASSERT_EQ(2U, dsSceneLightSet_findBrightestLights(brightestLights, lightCount, &hasMain,
		lightSet, &position));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light1));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light4));

	position.x = -1.0f;
	position.z = 2.0f;
	lightCount = 4;
	ASSERT_EQ(2U, dsSceneLightSet_findBrightestLights(brightestLights, lightCount, &hasMain,
		lightSet, &position));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light1));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light3));

	position.x = 0.0f;
	position.z = 0.0f;
	lightCount = 2;
	ASSERT_TRUE(dsSceneLightSet_setMainLightName(lightSet, "second"));
	ASSERT_EQ(2U, dsSceneLightSet_findBrightestLights(brightestLights, lightCount, &hasMain,
		lightSet, &position));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light1));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light4));
	EXPECT_FALSE(hasMain);

	light2->intensity = 0.2f;
	ASSERT_EQ(2U, dsSceneLightSet_findBrightestLights(brightestLights, lightCount, &hasMain,
		lightSet, &position));
	EXPECT_EQ(light2, brightestLights[0]);
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light1));
	EXPECT_TRUE(hasLight(brightestLights, lightCount, light2));

	dsSceneLightSet_destroy(lightSet);
}

TEST_F(SceneLightSetTest, ForEachLightInFrustum)
{
	dsColor3f color = {{1.0f, 1.0f, 1.0f}};
	dsSceneLightSet* lightSet = dsSceneLightSet_create((dsAllocator*)&allocator, 4, &color, 0.1f);
	ASSERT_TRUE(lightSet);

	dsVector3f direction = {{0.0f, 0.0f, -1.0f}};
	dsSceneLight* light1 = dsSceneLightSet_addLightName(lightSet, "first");
	ASSERT_TRUE(dsSceneLight_makeDirectional(light1, &direction, &color, 1.0f));

	dsSceneLight* light2 = dsSceneLightSet_addLightName(lightSet, "second");
	ASSERT_TRUE(dsSceneLight_makeDirectional(light2, &direction, &color, 0.05f));

	dsVector3f position = {{-1.0f, 0.0f, 0.0f}};
	dsSceneLight* light3 = dsSceneLightSet_addLightName(lightSet, "third");
	ASSERT_TRUE(dsSceneLight_makePoint(light3, &position, &color, 1.0f, 1.0f, 1.0f));

	position.x = 1.0f;
	direction.x = 1.0f;
	direction.z = 0.0f;
	dsSceneLight* light4 = dsSceneLightSet_addLightName(lightSet, "fourth");
	ASSERT_TRUE(dsSceneLight_makeSpot(light4, &position, &direction, &color, 1.0f, 1.0f, 1.0f,
		0.5f, 0.5f));

	EXPECT_TRUE(dsSceneLightSet_prepare(lightSet, DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD));

	dsMatrix44f projection;
	dsFrustum3f frustum;
	dsMatrix44f_makeOrtho(&projection, -2.0f, 0.0f, -1.0f, 0.0f, -1.0f, 1.0f, false, false);
	dsFrustum3_fromMatrix(frustum, projection, false, false);

	std::vector<const dsSceneLight*> lights;
	EXPECT_EQ(2U, dsSceneLightSet_forEachLightInFrustum(lightSet, &frustum, &visitLight, &lights));
	EXPECT_TRUE(hasLight(lights, light1));
	EXPECT_TRUE(hasLight(lights, light3));

	dsMatrix44f_makeOrtho(&projection, 0.0f, 2.0f, -1.0f, 0.0f, -1.0f, 1.0f, false, false);
	dsFrustum3_fromMatrix(frustum, projection, false, false);

	lights.clear();
	EXPECT_EQ(3U, dsSceneLightSet_forEachLightInFrustum(lightSet, &frustum, &visitLight, &lights));
	EXPECT_TRUE(hasLight(lights, light1));
	EXPECT_TRUE(hasLight(lights, light3));
	EXPECT_TRUE(hasLight(lights, light4));

	dsMatrix44f_makeOrtho(&projection, 2.0f, 4.0f, -1.0f, 0.0f, -1.0f, 1.0f, false, false);
	dsFrustum3_fromMatrix(frustum, projection, false, false);
	EXPECT_EQ(2U, dsSceneLightSet_forEachLightInFrustum(lightSet, &frustum, &visitLight, &lights));
	EXPECT_TRUE(hasLight(lights, light1));
	EXPECT_TRUE(hasLight(lights, light4));

	dsSceneLightSet_destroy(lightSet);
}
