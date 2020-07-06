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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/VectorDrawScene/Export.h>
#include <DeepSea/VectorDrawScene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for registering the VectorDrawScene types with a dsSceneLoadContext.
 */

/**
 * @brief Registers the vector draw scene types for loading.
 * @remark errno will be set on failure.
 * @param loadContext The load context to register the types with.
 * @param allocator The allocator to use for copying extra data passed into this function.
 * @param commandBuffer Command buffer to place graphics commands when loading vector images. If
 *     NULL, vector images will not be loadable.
 * @param qualityRemap Array to remap text qualities, or NULL to use values as-is. If not NULL,
 *     it must be of size DS_TEXT_QUALITY_REMAP_SIZE.
 * @param substitutionTable The text substitution table used when creating dsSceneText instances.
 *     This may be NULL if no text substitution is performed.
 * @param textRenderInfo Info for how to render text in vector item lists.
 * @param pixelSize The size of a pixel in the local coordinate space for the vector images and
 *     text. This will be used to determine tessellation quality and amount to smooth text for
 *     ant-aliasing.
 * @return False if not all of the types could be registered.
 */
DS_VECTORDRAWSCENE_EXPORT bool dsVectorSceneLoadConext_registerTypes(
	dsSceneLoadContext* loadContext, dsAllocator* allocator, dsCommandBuffer* commandBuffer,
	const dsTextQuality* qualityRemap, const dsTextSubstitutionTable* substitutionTable,
	const dsSceneTextRenderBufferInfo* textRenderInfo, float pixelSize);

#ifdef __cplusplus
}
#endif
