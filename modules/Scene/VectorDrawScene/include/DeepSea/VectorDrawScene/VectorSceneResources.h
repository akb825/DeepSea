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
 * @brief Function for registering dsVectorResources with dsSceneResources.
 */

/**
 * @brief Gets the type for the dsVectorResources custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_VECTORDRAWSCENE_EXPORT const dsCustomSceneResourceType* dsVectorSceneResources_getType(void);

/**
 * @brief Creates a custom resource to wrap a dsVectorResource.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param resources The vector resources to wrap.
 * @return The custom resource or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsCustomSceneResource* dsVectorSceneResources_create(
	dsAllocator* allocator, dsVectorResources* resources);

#ifdef __cplusplus
}
#endif
