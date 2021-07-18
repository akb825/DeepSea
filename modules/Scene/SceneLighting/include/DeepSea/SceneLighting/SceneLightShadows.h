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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>
#include <DeepSea/Render/Resources/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate shadow managers for the scene.
 * @see dsSceneShadowManager
 */

/**
 * @brief Creates scene light shadows to manage shadows for a single light.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create this with. This must support freeing memory.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param lightSet The light set to retrieve lights from.
 * @param lightType The type of the light this will cast shadows for.
 * @param lightName The name of the light to get shadows for. This may be NULL if it will be set
 *     later.
 * @param matrixGroupDesc The shader variable group that contains the matrices. This must have a
 *     elements based on lightType:
 *     - Directional:
 *         - mat44 array for the shadow projection of size 4, or non-array element if not cascaded.
 *         - vec4 of floats for cascade split distances, or omitted if not cascaded.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 *     - Point:
 *         - mat44 array of 6 elements for the shadow projection.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 *     - Spot:
 *         - mat44 for the shadow projection.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 * @param shadowParams Parameters controlling the shadow behavior.
 * @return The light shadows or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightShadows* dsSceneLightShadows_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsSceneLightSet* lightSet, dsSceneLightType lightType,
	const char* lightName, const dsShaderVariableGroupDesc* matrixGroupDesc,
	const dsSceneShadowParams* shadowParams);

/**
 * @brief Prepares the scene light shadows for the next frame.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows to prepare.
 * @param view The view to prepare for.
 * @return False if an error occurred.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_prepare(dsSceneLightShadows* shadows,
	const dsView* view);

/**
 * @brief Destroys a scene light shadows instance.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows to destroy.
 * @return False if the graphics resources couldn't be deleted.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_destroy(dsSceneLightShadows* shadows);

#ifdef __cplusplus
}
#endif

