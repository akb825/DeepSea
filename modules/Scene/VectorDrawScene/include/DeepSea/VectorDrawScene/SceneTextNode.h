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
 * @brief Functions to create and manipulate text nodes.
 * @see dsSceneTextNode
 */

/**
 * @brief The type name for a text node.
 */
DS_VECTORDRAWSCENE_EXPORT extern const char* const dsSceneTextNode_typeName;

/**
 * @brief Gets the type of a text node.
 * @return The type of a text node.
 */
DS_VECTORDRAWSCENE_EXPORT const dsSceneNodeType* dsSceneTextNode_type(void);

/**
 * @brief Sets up the parent type for a node type subclassing from dsSceneTextNode.
 * @param type The subclass type for dsSceneTextNode.
 * @return The type parameter or the type for dsSceneTextNode if type is NULL.
 */
DS_VECTORDRAWSCENE_EXPORT const dsSceneNodeType* dsSceneTextNode_setupParentType(
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
 * @param firstChar The first character to draw.
 * @param charCount The number of characters to draw.
 * @param shader The shader to draw with.
 * @param material The material to draw with.
 * @param itemLists List of item list names to add the node to. The array will be copied.
 * @param itemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @return The vector text node or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsSceneTextNode* dsSceneTextNode_create(
	dsAllocator* allocator, const dsText* text, void* textUserData, const dsTextStyle* styles,
	uint32_t styleCount, dsTextAlign alignment, float maxWidth, float lineScale, uint32_t firstChar,
	uint32_t charCount, dsShader* shader, dsMaterial* material, const char** itemLists,
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
 * @param firstChar The first character to draw.
 * @param charCount The number of characters to draw.
 * @param shader The shader to draw with.
 * @param material The material to draw with.
 * @param itemLists List of item list names to add the node to. The array will be copied.
 * @param itemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @return The vector text node or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsSceneTextNode* dsSceneTextNode_createBase(
	dsAllocator* allocator, size_t structSize, const dsText* text, void* textUserData,
	const dsTextStyle* styles, uint32_t styleCount, dsTextAlign alignment, float maxWidth,
	float lineScale, uint32_t firstChar, uint32_t charCount, dsShader* shader, dsMaterial* material,
	const char** itemLists, uint32_t itemListCount, dsSceneResources** resources,
	uint32_t resourceCount);

/**
 * @brief Triggers an update for layout when next drawn.
 *
 * This should be called when the following changes:
 * - contents of the styles
 * - alignment
 * - maxWidth
 * - lineScale
 *
 * @param node The node to update.
 */
DS_VECTORDRAWSCENE_EXPORT void dsSceneTextNode_updateLayout(dsSceneTextNode* node);

#ifdef __cplusplus
}
#endif
