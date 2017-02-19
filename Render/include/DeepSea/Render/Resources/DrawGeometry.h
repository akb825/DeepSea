/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and interacting with draw geometry.
 * @see dsDrawGeometry
 */

/**
 * @brief Creates a draw geometry.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the draw geometry from.
 * @param allocator The allocator to create the draw geometry buffer with. If NULL, it will use the
 *     same allocator as the resource manager.
 * @param vertexBuffers The vertex buffers to be used. NULL vertex buffers are ignored.
 * @param indexBuffer The index buffer to be used. This may be NULL if no index buffer is needed.
 * @return The created draw geometry, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsDrawGeometry* dsDrawGeometry_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsIndexBuffer* indexBuffer);

/**
 * @brief Destroys a draw geometry.
 * @remark errno will be set on failure.
 * @param geometry The draw geometry to destroy.
 * @return False if the draw geometry couldn't be destroyed. errno will be set to an appropriate
 *     value on failure.
 */
DS_RENDER_EXPORT bool dsDrawGeometry_destroy(dsDrawGeometry* geometry);

#ifdef __cplusplus
}
#endif
