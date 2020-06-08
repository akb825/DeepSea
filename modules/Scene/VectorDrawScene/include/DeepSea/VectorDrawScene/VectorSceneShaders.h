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

#include <DeepSea/Core/Config.h>
#include <DeepSea/VectorDrawScene/Export.h>
#include <DeepSea/VectorDrawScene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for registering dsVectorSceneShaderSet with dsSceneResources.
 */

/**
 * @brief The type name for vector scene shader bundle.
 */
DS_VECTORDRAWSCENE_EXPORT extern const char* const dsVectorSceneShaders_typeName;

/**
 * @brief Gets the type for the dsVectorShaders custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_VECTORDRAWSCENE_EXPORT const dsCustomSceneResourceType* dsVectorSceneShaders_type(void);

/**
 * @brief Creates a custom resource to wrap a dsVectorShaders.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param shaders The vector shaders. This will take ownership of both the shaders and its shader
 *     module, and both will destroyed even if the creation failed. It must have been created with
 *     shaderModule.
 * @return The custom resource or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsCustomSceneResource* dsVectorSceneShaders_create(dsAllocator* allocator,
	dsVectorShaders* shaders);

#ifdef __cplusplus
}
#endif


