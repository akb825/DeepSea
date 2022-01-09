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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Resources/Types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/SceneLighting library.
 */

/**
 * @brief Log tag used by the scene lighting library.
 */
#define DS_SCENE_LIGHTING_LOG_TAG "scene-lighting"

/**
 * @brief The default intensity threshold below which a light is considered out of view.
 */
#define DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD 0.1f

/**
 * @brief The maximum number of samples that can be made to compute SSAO.
 */
#define DS_MAX_SCENE_SSAO_SAMPLES 128U

/**
 * @brief The size fo the random rotation texture.
 */
#define DS_SCENE_SSAO_ROTATION_SIZE 4U

/**
 * @brief Enum for the type of a light.
 */
typedef enum dsSceneLightType
{
	dsSceneLightType_Directional, ///< Infinite light shining in a direction.
	dsSceneLightType_Point,       ///< Light shining in all directions from a point.
	dsSceneLightType_Spot,        ///< Light shining in a direction from a point.
	dsSceneLightType_Count        ///< The number of light types, useful when declaring arrays.
} dsSceneLightType;

/**
 * @brief Struct defining a light within a scene.
 *
 * Depending on the parameters, this can represent a directional, point, or spot light.
 *
 * @see SceneLight.h
 */
typedef struct dsSceneLight
{
	/**
	 * @brief The type of the light.
	 */
	dsSceneLightType type;

	/**
	 * @brief The position of the light.
	 */
	dsVector3f position;

	/**
	 * @brief The direction of the light.
	 */
	dsVector3f direction;

	/**
	 * @brief The color of the light.
	 *
	 * When doing sRGB correct rendering, this light should be in linear space.
	 */
	dsColor3f color;

	/**
	 * @brief The intensity of the light.
	 *
	 * This will multiply with the color for the final color to draw with. This allows the
	 * brightness and color to be adjusted independently more easily.
	 */
	float intensity;

	/**
	 * @brief Linear falloff factor for point and spot lights.
	 */
	float linearFalloff;

	/**
	 * @brief Quadratic falloff factor for point and spot lights.
	 */
	float quadraticFalloff;

	/**
	 * @brief The cosine of the inner angle for a spot.
	 *
	 * This is the angle when a spot light starts to fade out.
	 */
	float innerSpotCosAngle;

	/**
	 * @brief The cosine of the outer angle for a spot.
	 *
	 * This is the angle when a spot light finishes fading out.
	 */
	float outerSpotCosAngle;

	/**
	 * @brief The ID of the light name.
	 *
	 * This will be populated by dsSceneLightSet.
	 */
	uint32_t nameID;
} dsSceneLight;

/**
 * @brief Struct defining the vertex elements when drawing an ambient light.
 * @see SceneLight.h
 */
typedef struct dsAmbientLightVertex
{
	/**
	 * @brief The screen position of the light as a -1 or 1 value.
	 */
	int16_t position[2];

	/**
	 * @brief The color of the light as half floats.
	 */
	dsHalfFloat color[4];
} dsAmbientLightVertex;

/**
 * @brief Struct defining the vertex elements when drawing a directional light.
 * @see SceneLight.h
 */
typedef struct dsDirectionalLightVertex
{
	/**
	 * @brief The screen position of the light as a -1 or 1 value.
	 */
	int16_t position[2];

	/**
	 * @brief The direction as normalized integer values.
	 */
	int16_t direction[4];

	/**
	 * @brief The color of the light as half floats.
	 */
	dsHalfFloat color[4];
} dsDirectionalLightVertex;

/**
 * @brief Struct defining the vertex elements when drawing a point light.
 * @see SceneLight.h
 */
typedef struct dsPointLightVertex
{
	/**
	 * @brief The position of the vertex.
	 */
	dsVector3f vertexPosition;

	/**
	 * @brief The position of the light.
	 */
	dsVector3f lightPosition;

	/**
	 * @brief The color of the light.
	 */
	dsHalfFloat color[4];

	/**
	 * @brief The linear and quadratic falloff factors.
	 */
	dsHalfFloat falloff[2];
} dsPointLightVertex;

/**
 * @brief Struct defining the vertex elements when drawing a spot light.
 * @see SceneLight.h
 */
typedef struct dsSpotLightVertex
{
	/**
	 * @brief The position of the vertex.
	 */
	dsVector3f vertexPosition;

	/**
	 * @brief The position of the light.
	 */
	dsVector3f lightPosition;

	/**
	 * @brief The direction as normalized integer values.
	 */
	int16_t direction[4];

	/**
	 * @brief The color of the light.
	 */
	dsHalfFloat color[4];

	/**
	 * @brief The linear and quadratic falloff factors and cosine of the inner and outer spot
	 *     angles.
	 */
	dsHalfFloat falloffAndSpotAngles[4];
} dsSpotLightVertex;

/**
 * @brief Struct describing parameters for shadows in a scene.
 * @see SceneLightShadows.h
 */
typedef struct dsSceneShadowParams
{
	/**
	 * @brief The maximum number of cascades.
	 *
	 * This is only used with directional lights that use an array for the matrices. Must be in the
	 * range [1, 4].
	 */
	uint32_t maxCascades;

	/**
	 * @brief The maximum distance for the first split for cascaded shadows.
	 */
	float maxFirstSplitDistance;

	/**
	 * @brief Exponential factor for cascaded shadows in the range [0, 1], where 0 uses linear
	 * distances between the splits and 1 is fully exponential.
	 */
	float cascadeExpFactor;

	/**
	 * @brief The distance to start fading out shadows.
	 */
	float fadeStartDistance;

	/**
	 * @brief The maximum to display shadows.
	 */
	float maxDistance;
} dsSceneShadowParams;

/**
 * @brief Struct containing the info required to draw a deferred light.
 */
typedef struct dsDeferredLightDrawInfo
{
	/**
	 * @brief The shader to draw the light with.
	 */
	dsShader* shader;

	/**
	 * @brief The material to bind with the shader.
	 */
	dsMaterial* material;
} dsDeferredLightDrawInfo;

/**
 * @brief Struct containing the info required to draw a deferred light with shadows.
 *
 * When a light has a dsSceneLightShadows instance registered with a dsSceneShadowManager, the
 * shadow light will be used. The shadow transform will be bound to transformGroupName.
 *
 * The texture is expected to be registered with the dsView instance using same name as the the
 * dsSceneLightShadows. This texture is bound to shadowTextureName.
 */
typedef struct dsDeferredShadowLightDrawInfo
{
	/**
	 * @brief The shader to draw the light with.
	 */
	dsShader* shader;

	/**
	 * @brief The material to bind with the shader.
	 */
	dsMaterial* material;

	/**
	 * @brief The name of the shadow transform group in the shader.
	 *
	 * This should be created with instance binding in the material.
	 */
	const char* transformGroupName;

	/**
	 * @brief The name of the shadow texture in the shader.
	 *
	 * This should be created with instance binding in the material.
	 */
	const char* shadowTextureName;
} dsDeferredShadowLightDrawInfo;

/**
 * @brief Struct defining the set of lights used in a scene.
 *
 * This is typically stored as a scene resource to provide the full set of lights for a scene. This
 * can be used to search for the brightest lights at a point for standard forward lighting or get
 * the lights in view for deferred lighting.
 *
 * @see SceneLightSet.h
 */
typedef struct dsSceneLightSet dsSceneLightSet;

/**
 * @brief Function for visiting a light within a light set.
 * @param userData User data forwarded for the function.
 * @param lightSet The light set the light is from.
 * @param light The light being visited.
 * @return True to continue visiting lights, false to stop.
 */
typedef bool (*dsSceneLightVisitFunction)(void* userData, const dsSceneLightSet* lightSet,
	const dsSceneLight* light);

/**
 * @brief Struct defining a scene light set prepare.
 *
 * This will prepare provided scene light sets for use in the scene. This a scene global data object
 * fit into the scene layout.
 *
 * @see SceneLightSetPrepare.h
 */
typedef struct dsSceneLightSetPrepare dsSceneLightSetPrepare;

/**
 * @brief Struct defining a deferred light resolve.
 *
 * This will draw the lights to the screen in order to light the scene based on the gbuffers. This
 * is an item list type in order to fit into the scene, but doesn't interact with any nodes in the
 * scene graph.
 *
 * @see DeferredLightResolve.h
 */
typedef struct dsDeferredLightResolve dsDeferredLightResolve;

/**
 * @brief Struct that manages shadows within a scene.
 *
 * This contains multiple dsSceneLightShadows instances, managed with their own shadow name as well
 * as light they are currently assigned to. This provides a central location to query and manage
 * shadow instances.
 */
typedef struct dsSceneShadowManager dsSceneShadowManager;

/**
 * @brief Struct that manages shadows for a single light in the scene.
 *
 * The shadow manager keeps track of the light that casts shadows and parameters for getting the
 * cull volumes and projection matrices. This will be referenced by the cull item lists and set the
 * variables for the projection matrices consumed by the shaders.
 *
 * This is generally stored with a dsSceneShadowManager instance.
 *
 * @see SceneShadowManager.h
 */
typedef struct dsSceneLightShadows dsSceneLightShadows;

/**
 * @brief Struct defining a scene screen-space ambient occlusion.
 *
 * This will compute ambient occlusion in screen-space based on the gbuffers. This is an item list
 * type in order to fit into the scene, but doesn't interact with any nodes in the scene graph.
 *
 * The geometry drawn will be vec2 values in the range [-1, 1], with (-1, -1) being the lower-left
 * corner and (1, 1) being the upper-right corner.
 *
 * @see SceneSSAO.h
 */
typedef struct dsSceneSSAO dsSceneSSAO;

/**
 * @brief Struct defining a scene compute screen-space ambient occlusion.
 *
 * This will compute ambient occlusion in screen-space based on the gbuffers. This is an item list
 * type in order to fit into the scene, but doesn't interact with any nodes in the scene graph.
 *
 * This is largely identical to dsSceneSSAO, except it uses a compute shader to calculate the SSAO
 * rather than a traditional full-screen resolve.
 *
 * @see SceneComputeSSAO.h
 */
typedef struct dsSceneComputeSSAO dsSceneComputeSSAO;


#ifdef __cplusplus
}
#endif
