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
 * @brief Functions for creating and manipulating scene vector item lists.
 * @see dsSceneVectorItemList
 */

/**
 * @brief The scene vector item list type name.
 */
DS_VECTORDRAWSCENE_EXPORT extern const char* const dsSceneVectorItemList_typeName;

/**
 * @brief Gets the default vertex format for text.
 *
 * This has the following elements:
 * - Position: 2D position as a 2-element float.
 * - Color0: text color as an RGBA8
 * - Color1: outine color as an RGB8
 * - TexCoord0: texture coordinate and LOD index as 3-element float.
 * - TexCoord1: style parameters (embolden, outline position, outline thickness, anti-alias) as
 *   4-element float.
 *
 * @remark errno will be set on failure.
 * @param[out] outFormat The vertex format to opulate.
 * @return False if outFormat is NULL.
 */
DS_VECTORDRAWSCENE_EXPORT bool dsSceneVectorItemList_defaultTextVertexFormat(
	dsVertexFormat* outFormat);

/**
 * @brief Gets the default vertex format for text when used with tessellation shaders.
 *
 * This has the following elements:
 * - Position0: 2D position, mip level, and anti-alias value as a 4-element float.
 * - Position1: 2D bounding box for the glyph as a 4-element float.
 * - Color0: text color as an RGBA8.
 * - Color1: outine color as an RGB8.
 * - TexCoord0: bounding box of the texture coordinates of the glyph as a 4-element float.
 * - TexCoord1: style parameters (embolden, outline position, outline thickness, anti-alias) as
 *   4-element float.
 *
 * @remark errno will be set on failure.
 * @param[out] outFormat The vertex format to opulate.
 * @return False if outFormat is NULL.
 */
DS_VECTORDRAWSCENE_EXPORT bool dsSceneVectorItemList_defaultTessTextVertexFormat(
	dsVertexFormat* outFormat);

/**
 * @brief Default glyph data function.
 * @param userData The user data for the function.
 * @param layout The text layout that will be added.
 * @param layoutUserData The user data provided with the layout.
 * @param glyphIndex The index of the glyph to add.
 * @param vertexData The vertex data to write to. You should write vertexCount vertices to this
 *     array depending on if it's 4 vertices for a quad or 1 for a tessellation shader. When writing
 *     4 vertices for a quad, it will typically be a clockwise loop. (since Y points down, the
 *     shader will typically flip it to become counter-clockwise)
 * @param format The vertex format.
 * @param vertexCount The number of vertices to write. This will either be 4 vertices for a quad,
 *     which should follow winding order, or 1 vertex when using the tessellation shader.
 */
DS_VECTORDRAWSCENE_EXPORT void dsSceneVectorItemList_defaultGlyphDataFunc(void* userData,
	const dsTextLayout* layout, void* layoutUserData, uint32_t glyphIndex, void* vertexData,
	const dsVertexFormat* format, uint32_t vertexCount);

/**
 * @brief Default glyph data function for tessellated text.
 * @param userData The user data for the function.
 * @param layout The text layout that will be added.
 * @param layoutUserData The user data provided with the layout.
 * @param glyphIndex The index of the glyph to add.
 * @param vertexData The vertex data to write to. You should write vertexCount vertices to this
 *     array depending on if it's 4 vertices for a quad or 1 for a tessellation shader. When writing
 *     4 vertices for a quad, it will typically be a clockwise loop. (since Y points down, the
 *     shader will typically flip it to become counter-clockwise)
 * @param format The vertex format.
 * @param vertexCount The number of vertices to write. This will either be 4 vertices for a quad,
 *     which should follow winding order, or 1 vertex when using the tessellation shader.
 */
DS_VECTORDRAWSCENE_EXPORT void dsSceneVectorItemList_defaultTessGlyphDataFunc(void* userData,
	const dsTextLayout* layout, void* layoutUserData, uint32_t glyphIndex, void* vertexData,
	const dsVertexFormat* format, uint32_t vertexCount);

/**
 * @brief Creates a scene vector item list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the vector item list. This will be copied.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param instanceData The list of instance datas to use. The array will be copied, and this will
 *     take ownership of each instance data. The instances will be destroyed if an error occurrs.
 * @param instanceDataCount The number of instance datas.
 * @param textRenderBufferInfo The info for creating a dsSceneTextRenderBuffer. If NULL, this won't
 *     support drawing text outside of dsVectorImage.
 * @param renderStates The render states to use, or NULL if no special render states are needed.
 * @return The model list or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsSceneVectorItemList* dsSceneVectorItemList_create(
	dsAllocator* allocator, const char* name, dsResourceManager* resourceManager,
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount,
	const dsSceneTextRenderBufferInfo* textRenderBufferInfo,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Gets the render states for a vector item list.
 * @param vectorList The vector item list.
 * @return The render states or NULL if no special render states are used.
 */
DS_VECTORDRAWSCENE_EXPORT const dsDynamicRenderStates* dsSceneVectorItemList_getRenderStates(
	const dsSceneVectorItemList* vectorList);

/**
 * @brief Sets the render states for a vector item list.
 * @param vectorList The vector item list.
 * @param renderStates The render states or NULL if no special render states are needed.
 */
DS_VECTORDRAWSCENE_EXPORT void dsdsSceneVectorItemList_setRenderStates(
	dsSceneVectorItemList* vectorList, const dsDynamicRenderStates* renderStates);

/**
 * @brief Destroys the vector item list.
 * @param vectorList The vector item list.
 */
DS_VECTORDRAWSCENE_EXPORT void dsSceneVectorItemList_destroy(dsSceneVectorItemList* vectorList);

#ifdef __cplusplus
}
#endif
