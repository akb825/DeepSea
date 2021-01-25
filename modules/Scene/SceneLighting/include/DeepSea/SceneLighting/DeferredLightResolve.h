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
 * @brief Creates a deferred light resolve.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the defferred light resolve with.This must support
 *     freeing memory.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL this will use
 *     the deferred light resolve allocator.
 * @param name The name of the deferred light resolve. This will be copied.
 * @param lightSet The light set to draw the lgihts from.
 * @param ambientShader The shader used for the ambient light. The vertex elements for the shader
 *     are:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - color: vec3 ambient color.
 * @param ambientMaterial The material used for the ambient light.
 * @param directionalShader The shader used for directional lights. The vertex elements for the
 *     shader are:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - normal: vec3 normalized direction to the light.
 *     - color: vec3 light color.
 * @param directionalMaterial The material used for directional lights.
 * @param pointShader The shader used for point lights. The vertex elements for the shader are:
 *     - position0: vec3 world-space vertex position.
 *     - position1: vec3 world-space light position.
 *     - color: vec3 light color.
 *     - texcoord0: vec2 for linear and quadratic falloff.
 * @param pointMaterial The material used for point lights.
 * @param spotShader The shader used for spot lights. The vertex elements for the shader are:
 *     - position0: vec3 world-space vertex position.
 *     - position1: vec3 world-space light position.
 *     - normal: vec3 normalized direction to the light.
 *     - color: vec3 light color.
 *     - texcoord0: vec4 for linear and quadratic falloff, and inner and outer cos spot angle.
 * @param spotMaterial The material used for spot lights.
 * @param intensityThreshold The threshold below which the light is considered out of view. This
 *     must be > 0. Use DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD for the default value.
 * @return The deferred light resolve or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsDeferredLightResolve* dsDeferredLightResolve_create(
	dsAllocator* allocator, dsAllocator* resourceAllocator, const char* name,
	const dsSceneLightSet* lightSet, dsShader* ambientShader, dsMaterial* ambientMaterial,
	dsShader* directionalShader, dsMaterial* directionalMaterial, dsShader* pointShader,
	dsMaterial* pointMaterial, dsShader* spotShader, dsMaterial* spotMaterial,
	float intensityThreshold);

/**
 * @brief Gets the ambient shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The shader or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsDeferredLightResolve_getAmbientShader(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the ambient shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param shader The ambient shader. The vertex elements for the shader are:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - color: vec3 ambient color.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setAmbientShader(
	dsDeferredLightResolve* resolve, dsShader* shader);

/**
 * @brief Gets the ambient material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The material or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsDeferredLightResolve_getAmbientMaterial(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the ambient material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param material The ambient material.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setAmbientMaterial(
	dsDeferredLightResolve* resolve, dsMaterial* material);

/**
 * @brief Gets the directional light shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The shader or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsDeferredLightResolve_getDirectionalShader(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the directional light shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param shader The directional light shader. The vertex elements for the shader are:
 *     - position: vec2 clip-space [-1, 1] values.
 *     - normal: vec3 normalized direction.
 *     - color: vec3 light color.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setDirectionalShader(
	dsDeferredLightResolve* resolve, dsShader* shader);

/**
 * @brief Gets the directional light material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The material or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsDeferredLightResolve_getDirectionalMaterial(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the directional light material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param material The directional light material.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setDirectionalMaterial(
	dsDeferredLightResolve* resolve, dsMaterial* material);

/**
 * @brief Gets the point light shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The shader or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsDeferredLightResolve_getPointShader(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the point light shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param shader The point light shader. The vertex elements for the shader are:
 *     - position0: vec3 world-space vertex position.
 *     - position1: vec3 world-space light position.
 *     - color: vec3 light color.
 *     - texcoord0: vec2 for linear and quadratic falloff.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setPointShader(dsDeferredLightResolve* resolve,
	dsShader* shader);

/**
 * @brief Gets the point light material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The material or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsDeferredLightResolve_getPointMaterial(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the point light material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param material The point light material.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setPointMaterial(
	dsDeferredLightResolve* resolve, dsMaterial* material);

/**
 * @brief Gets the spot light shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The shader or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsDeferredLightResolve_getSpotShader(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the spot light shader.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param shader The spot light shader.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setSpotShader(dsDeferredLightResolve* resolve,
	dsShader* shader);

/**
 * @brief Gets the spot light material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The material or NULL if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsDeferredLightResolve_getSpotMaterial(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the spot light material.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @param material The spot light material.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsDeferredLightResolve_setSpotMaterial(dsDeferredLightResolve* resolve,
	dsMaterial* material);

/**
 * @brief Gets the intensity threshold.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
 * @return The intensity threshold or 0 if resolve is NULL.
 */
DS_SCENELIGHTING_EXPORT float dsDeferredLightResolve_getIntensityThreshold(
	const dsDeferredLightResolve* resolve);

/**
 * @brief Sets the intensity threshold.
 * @remark errno will be set on failure.
 * @param resolve The deferred lighting resolve.
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


