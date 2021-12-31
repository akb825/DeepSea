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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating deferred light resolves.
 */

/**
 * @brief The deferred light resolve type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsDeferredLightResolve_typeName;

/**
 * @brief Gets the type of a deferred light resolve.
 * @return The type of a deferred light resolve.
 */
DS_SCENELIGHTING_EXPORT dsSceneItemListType dsDeferredLightResolve_type(void);

/**
 * @brief Creates a deferred light resolve.
 *
 * The following vertex elements are used based on the light type:
 * - Ambient:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - color: vec3 ambient color.
 * - Directional:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - normal: vec3 normalized direction to the light.
 *     - color: vec3 light color.
 * - Point:
 *     - position0: vec3 world-space vertex position.
 *     - position1: vec3 world-space light position.
 *     - color: vec3 light color.
 *     - texcoord0: vec2 for linear and quadratic falloff.
 * - Spot:
 *     - position0: vec3 world-space vertex position.
 *     - position1: vec3 world-space light position.
 *     - normal: vec3 normalized direction to the light.
 *     - color: vec3 light color.
 *     - texcoord0: vec4 for linear and quadratic falloff, and inner and outer cos spot angle.
 *
 * @remark Any shader may be NULL to avoid drawing that type of light. For example, this can be used
 *     to draw specific light types in different render passes.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the defferred light resolve with. This must support
 *     freeing memory.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL this will use
 *     the deferred light resolve allocator.
 * @param name The name of the deferred light resolve. This will be copied.
 * @param lightSet The light set to draw the lgihts from.
 * @param shadowManager The shadow manager to get shadows from when drawing shadowed lights.
 * @param ambientInfo The draw info for ambient lights. If this or any members are NULL, the ambient
 *     light won't be drawn.
 * @param lightInfos The draw info for non-shadowed lights. This may be NULL to not draw any
 *     non-shadowed lights or an array of length dsSceneLightType_Count. Each light type is indexed
 *     by the dsSceneLightType enum values. Any array elements that contain NULL elements will
 *     ignore that light type.
 * @param shadowLightInfos The draw info for shadowed lights. This may be NULL to not draw any
 *     shadowed lights or an array of length dsSceneLightType_Count. Each light type is indexed
 *     by the dsSceneLightType enum values. Any array elements that contain NULL elements will
 *     ignore that light type.
 * @param intensityThreshold The threshold below which the light is considered out of view. This
 *     must be > 0. Use DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD for the default value.
 * @return The deferred light resolve or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsDeferredLightResolve* dsDeferredLightResolve_create(
	dsAllocator* allocator, dsAllocator* resourceAllocator, const char* name,
	const dsSceneLightSet* lightSet, const dsSceneShadowManager* shadowManager,
	const dsDeferredLightDrawInfo* ambientInfo, const dsDeferredLightDrawInfo* lightInfos,
	const dsDeferredShadowLightDrawInfo* shadowLightInfos, float intensityThreshold);

/**
 * @brief Gets the ambient shader.
 * @param resolve The deferred light resolve.
 * @return The shader or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsDeferredLightResolve_getAmbientShader(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the ambient shader.
 * @remark This may only be called if an ambient shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param shader The ambient shader. The vertex elements for the shader are:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - color: vec3 ambient color.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setAmbientShader(
	dsDeferredLightResolve* resolve, dsShader* shader);

/**
 * @brief Gets the ambient material.
 * @param resolve The deferred light resolve.
 * @return The material or NULL if unset.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsDeferredLightResolve_getAmbientMaterial(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the ambient material.
 * @remark This may only be called if an ambient shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param material The ambient material.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setAmbientMaterial(
	dsDeferredLightResolve* resolve, dsMaterial* material);

/**
 * @brief Gets a non-shadowed light shader.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @return The shader or NULL if unset.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsDeferredLightResolve_getLightShader(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType);

/**
 * @brief Sets a non-shadowed light shader.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param shader The directional light shader. The vertex elements for the shader are:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - normal: vec3 normalized direction.
 *     - color: vec3 light color.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setLightShader(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, dsShader* shader);

/**
 * @brief Gets a non-shadowed light material.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @return The material or NULL if unset.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsDeferredLightResolve_getLightMaterial(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType);

/**
 * @brief Sets a non-shadowed light material.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param material The directional light material.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setLightMaterial(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, dsMaterial* material);

/**
 * @brief Gets a shadowed light shader.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @return The shader or NULL if unset.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsDeferredLightResolve_getShadowLightShader(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType);

/**
 * @brief Sets a shadowed light shader.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param shader The directional light shader. The vertex elements for the shader are:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - normal: vec3 normalized direction.
 *     - color: vec3 light color.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setShadowLightShader(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, dsShader* shader);

/**
 * @brief Gets a shadowed light material.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @return The material or NULL if unset.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsDeferredLightResolve_getShadowLightMaterial(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType);

/**
 * @brief Sets a shadowed light material.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param material The directional light material.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setShadowLightMaterial(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, dsMaterial* material);

/**
 * @brief Gets a shadowed light transform group.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @return The transform group ID or 0 if unset.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsDeferredLightResolve_getShadowLightTransformGroupID(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType);

/**
 * @brief Sets a shadowed light transform group by ID.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param groupID The ID for the transform group.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setShadowLightTransformGroupID(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, uint32_t groupID);

/**
 * @brief Sets a shadowed light transform group by name.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param groupName The name for the transform group.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setShadowLightTransformGroupName(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, const char* groupName);

/**
 * @brief Gets a shadowed light texture.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @return The shadow light texture ID or 0 if unset.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsDeferredLightResolve_getShadowLightTextureID(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType);

/**
 * @brief Sets a shadowed light texture by ID.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param textureID The ID for the texture.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setShadowLightTextureID(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, uint32_t textureID);

/**
 * @brief Sets a shadowed light texture by name.
 * @remark This may only be called if the light type's shader was previously set in the constructor.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param lightType The type of the light.
 * @param textureName The name for the texture.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setShadowLightTextureName(
	dsDeferredLightResolve* resolve, dsSceneLightType lightType, const char* textureName);

/**
 * @brief Gets the intensity threshold.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @return The intensity threshold or 0 if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT float dsDeferredLightResolve_getIntensityThreshold(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the intensity threshold.
 * @remark errno will be set on failure.
 * @param resolve The deferred light resolve.
 * @param intensityThreshold The threshold below which the light is considered out of view. This
 *     must be > 0. Use DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD for the default value.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setIntensityThreshold(
	dsDeferredLightResolve* resolve, float intensityThreshold);

/**
 * @brief Destroys a deferred light resolve.
 * @param resolve The deferred light resolve to destroy.
 */
DS_SCENELIGHTING_EXPORT void dsDeferredLightResolve_destroy(dsDeferredLightResolve* resolve);

#ifdef __cplusplus
}
#endif


