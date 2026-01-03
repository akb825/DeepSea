/*
 * Copyright 2020-2026 Aaron Barany
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
#include <DeepSea/SceneVectorDraw/Export.h>
#include <DeepSea/SceneVectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate text nodes.
 * @see dsSceneTextNode
 */

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
DS_SCENEVECTORDRAW_EXPORT bool dsSceneTextNode_defaultTextVertexFormat(dsVertexFormat* outFormat);

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
DS_SCENEVECTORDRAW_EXPORT bool dsSceneTextNode_defaultTessTextVertexFormat(
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
DS_SCENEVECTORDRAW_EXPORT void dsSceneTextNode_defaultGlyphDataFunc(void* userData,
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
DS_SCENEVECTORDRAW_EXPORT void dsSceneTextNode_defaultTessGlyphDataFunc(void* userData,
	const dsTextLayout* layout, void* layoutUserData, uint32_t glyphIndex, void* vertexData,
	const dsVertexFormat* format, uint32_t vertexCount);

/**
 * @brief The type name for a text node.
 */
DS_SCENEVECTORDRAW_EXPORT extern const char* const dsSceneTextNode_typeName;

/**
 * @brief Gets the type of a text node.
 * @return The type of a text node.
 */
DS_SCENEVECTORDRAW_EXPORT const dsSceneNodeType* dsSceneTextNode_type(void);

/**
 * @brief Sets up the parent type for a node type subclassing from dsSceneTextNode.
 * @param type The subclass type for dsSceneTextNode.
 * @return The type parameter or the type for dsSceneTextNode if type is NULL.
 */
DS_SCENEVECTORDRAW_EXPORT const dsSceneNodeType* dsSceneTextNode_setupParentType(
	dsSceneNodeType* type);

/**
 * @brief Creates a text node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the text node with.
 * @param text The text to draw.
 * @param textUserData User data to provide with the text.
 * @param styles The styles to apply to the text. The array will be copied.
 * @param styleCount The number of styles.
 * @param alignment The alignmnet of the text.
 * @param maxWidth The maximum width of the text when laying out.
 * @param lineScale The scale to apply to the distance between each line. Set to 1 to use the base
 *     font height directly.
 * @param z The Z value used for sorting vector nodes.
 * @param firstChar The first character to draw.
 * @param charCount The number of characters to draw.
 * @param shader The shader to draw with.
 * @param textRenderBufferInfo The info for creating a dsSceneTextRenderBuffer.
 * @param itemLists List of item list names to add the node to. The array will be copied.
 * @param itemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @return The vector text node or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneTextNode* dsSceneTextNode_create(
	dsAllocator* allocator, const dsText* text, void* textUserData, const dsTextStyle* styles,
	uint32_t styleCount, dsTextAlign alignment, float maxWidth, float lineScale, int32_t z,
	uint32_t firstChar, uint32_t charCount, dsShader* shader,
	const dsSceneTextRenderBufferInfo* textRenderBufferInfo, const char* const* itemLists,
	uint32_t itemListCount, dsSceneResources** resources, uint32_t resourceCount);

/**
 * @brief Creates a text node as a base class of another node type.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param structSize The size of the struct.
 * @param text The text to draw.
 * @param textUserData User data to provide with the text.
 * @param styles The styles to apply to the text. The array will be copied.
 * @param styleCount The number of styles.
 * @param alignment The alignmnet of the text.
 * @param maxWidth The maximum width of the text when laying out.
 * @param lineScale The scale to apply to the distance between each line. Set to 1 to use the base
 *     font height directly.
 * @param z The Z value used for sorting vector nodes.
 * @param firstChar The first character to draw.
 * @param charCount The number of characters to draw.
 * @param shader The shader to draw with.
 * @param textRenderBufferInfo The info for creating a dsSceneTextRenderBuffer.
 * @param itemLists List of item list names to add the node to. The array will be copied.
 * @param itemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @return The vector text node or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneTextNode* dsSceneTextNode_createBase(
	dsAllocator* allocator, size_t structSize, const dsText* text, void* textUserData,
	const dsTextStyle* styles, uint32_t styleCount, dsTextAlign alignment, float maxWidth,
	float lineScale, int32_t z, uint32_t firstChar, uint32_t charCount, dsShader* shader,
	const dsSceneTextRenderBufferInfo* textRenderBufferInfo, const char* const* itemLists,
	uint32_t itemListCount, dsSceneResources** resources, uint32_t resourceCount);

/**
 * @brief Triggers an update for layout when it's next updated.
 *
 * This should be called when the following changes:
 * - contents of the styles
 * - alignment
 * - maxWidth
 * - lineScale
 *
 * @param node The node to update.
 */
DS_SCENEVECTORDRAW_EXPORT void dsSceneTextNode_updateLayout(dsSceneTextNode* node);

/**
 * @brief Destroys a text node.
 * @remark This should only be called as part of a subclass' destroy function, never to explicitly
 *     a model node instance since nodes are reference counted.
 * @param node The node to destroy.
 */
DS_SCENEVECTORDRAW_EXPORT void dsSceneTextNode_destroy(dsSceneNode* node);

#ifdef __cplusplus
}
#endif
