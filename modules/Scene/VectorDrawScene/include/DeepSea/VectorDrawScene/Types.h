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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/VectorDrawScene library.
 */

/**
 * @brief Log tag used by the vector draw scene library.
 */
#define DS_VECTOR_DRAW_SCENE_LOG_TAG "vectordraw-scene"

/**
 * @brief Struct describing text used within a scene.
 * @see SceneText.h
 */
typedef struct dsSceneText
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The text that has gone through initial processing, but not yet formatted.
	 */
	dsText* text;

	/**
	 * @brief User data to use with the text.
	 */
	void* userData;

	/**
	 * @brief The styles for the text.
	 */
	dsTextStyle* styles;

	/**
	 * @brief The number of styles for the text.
	 */
	uint32_t styleCount;
} dsSceneText;

/**
 * @brief Struct describing a node with vector drawing.
 * @see SceneVectorNode.h
 */
typedef struct dsSceneVectorNode
{
	/**
	 * @brief The node as a base class.
	 */
	dsSceneNode node;

	/**
	 * @brief The resources to keep a reference to.
	 *
	 * This will ensure that any resources used within this node are kept alive.
	 */
	dsSceneResources** resources;

	/**
	 * @brief The number of resources.
	 */
	uint32_t resourceCount;

	/**
	 * @brief The z level for the image used for sorting.
	 */
	int32_t z;
} dsSceneVectorNode;

/**
 * @brief Struct for a vector node subclass to display a vector image.
 * @see SceneVectorImageNode.h
 */
typedef struct dsSceneVectorImageNode
{
	/**
	 * @brief The node as a base class.
	 */
	dsSceneVectorNode node;

	/**
	 * @brief The vector image to draw.
	 */
	const dsVectorImage* vectorImage;

	/**
	 * @brief The vector shaders to draw with.
	 */
	const dsVectorShaders* shaders;

	/**
	 * @brief The material to draw with.
	 */
	dsMaterial* material;
} dsSceneVectorImageNode;

/**
 * @brief Struct for a vector node subclass to display text.
 * @see SceneTextNode.h
 */
typedef struct dsSceneTextNode
{
	/**
	 * @brief The node as a base class.
	 */
	dsSceneVectorNode node;

	/**
	 * @brief The text to display.
	 *
	 * This should not be re-assigned.
	 */
	const dsText* text;

	/**
	 * @brief User data to pass with the text.
	 */
	void* textUserData;

	/**
	 * @brief The shader to draw with.
	 */
	dsShader* shader;

	/**
	 * @brief The material to draw with.
	 */
	dsMaterial* material;

	/**
	 * @brief The styles to apply to the text.
	 */
	dsTextStyle* styles;

	/**
	 * @brief The number of styles.
	 */
	uint32_t styleCount;

	/**
	 * @brief The element index for the font texture in the material.
	 */
	uint32_t fontTextureElement;

	/**
	 * @brief The alignment of the text.
	 */
	dsTextAlign alignment;

	/**
	 * @brief The maximum width of the text when aligning.
	 */
	float maxWidth;

	/**
	 * @brief The scale to apply to the distance between each line.
	 *
	 * Set to 1 to use the base font height directly.
	 */
	float lineScale;

	/**
	 * @brief The first character to display.
	 */
	uint32_t firstChar;

	/**
	 * @brief The number of characters to display.
	 */
	uint32_t charCount;

	/**
	 * @brief Version number to determine when the layout needs to be be re-calculated.
	 */
	uint32_t layoutVersion;
} dsSceneTextNode;

/**
 * @brief Text with info required to create a dsTextRenderBuffer with a dsSceneVectorItemList.
 * @see SceneVectorItemList.h
 */
typedef struct dsSceneTextRenderBufferInfo
{
	/**
	 * @brief The vertex format used for text.
	 */
	const dsVertexFormat* vertexFormat;

	/**
	 * @brief The function to populate glyph data.
	 */
	dsGlyphDataFunction glyphDataFunc;

	/**
	 * @brief User data to provide with the glyph function.
	 */
	void* userData;

	/**
	 * @brief The maximum number of glyphs that can be drawn at once.
	 */
	uint32_t maxGlyphs;

	/**
	 * @brief Whether or not a tessellation shader is used.
	 */
	bool tessellationShader;
} dsSceneTextRenderBufferInfo;

/**
 * @brief Scene item list implementation for drawing vector images and text.
 *
 * This will hold information from dsSceneVectorNode node types.
 *
 * @see SceneVectorItemList.h
 */
typedef struct dsSceneVectorItemList dsSceneVectorItemList;

#ifdef __cplusplus
}
#endif
