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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/SceneVectorDraw library.
 */

/**
 * @brief Log tag used by the scene vector draw library.
 */
#define DS_SCENE_VECTOR_DRAW_LOG_TAG "scene=vectordraw"

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
	 * @brief The font for the text.
	 */
	dsFont* font;

	/**
	 * @brief The original string of the text to use for re-substitution.
	 *
	 * This will be NULL if substitution wasn't required.
	 */
	const char* originalString;

	/**
	 * @brief The original styles of the text to use for re-substitution.
	 *
	 * This will be NULL if substitution wasn't required.
	 */
	const dsTextStyle* originalStyles;

	/**
	 * @brief The text that has gone through initial processing, but not yet formatted.
	 */
	dsText* text;

	/**
	 * @brief The styles for the text.
	 */
	dsTextStyle* styles;

	/**
	 * @brief The number of styles for the text.
	 */
	uint32_t styleCount;

	/**
	 * @brief Version number to determine when the text has been updated.
	 */
	uint32_t textVersion;
} dsSceneText;

/**
 * @brief Struct describing a vector image used within a scene.
 * @see SceneVectorImage.h
 */
typedef struct dsSceneVectorImage
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The vector image to draw.
	 */
	dsVectorImage* image;

	/**
	 * @brief The shaders to draw the vector image with.
	 */
	const dsVectorShaders* shaders;
} dsSceneVectorImage;

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
	dsVectorImage* vectorImage;

	/**
	 * @brief The size to draw the image as.
	 */
	dsVector2f size;

	/**
	 * @brief The vector shaders to draw with.
	 */
	const dsVectorShaders* shaders;
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
	 * @brief The text to draw.
	 */
	const dsSceneText* text;

	/**
	 * @brief The text layout to draw.
	 */
	dsTextLayout* layout;

	/**
	 * @brief Render buffer to draw the text.
	 *
	 * This will be populated whenever the layout is updated.
	 */
	dsTextRenderBuffer* renderBuffer;

	/**
	 * @brief The shader to draw with.
	 */
	dsShader* shader;

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

	/**
	 * @brief Version number of the text for the last time the text was created.
	 */
	uint32_t textVersion;
} dsSceneTextNode;

/**
 * @brief Struct for a node that discards pixels outside of a bounding box in child nodes.
 * @see SceneDiscardBoundsNode.h
 */
typedef struct dsSceneDiscardBoundsNode
{
	/**
	 * @brief The node as a base class.
	 */
	dsSceneNode node;

	/**
	 * @brief The bounds outside of which shader fragments are discarded.
	 *
	 * The bounds will follow the transform for the current node, and the transform of child nodes
	 * will be respected relative to this.
	 */
	dsAlignedBox2f discardBounds;
} dsSceneDiscardBoundsNode;

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
} dsSceneTextRenderBufferInfo;

/**
 * @brief Scene item list implementation for drawing vector images and text.
 *
 * This will handle drawing for dsSceneVectorNode node types.
 *
 * @see SceneVectorItemList.h
 */
typedef struct dsSceneVectorItemList dsSceneVectorItemList;

/**
 * @brief Struct providing the parameters for loading vector draw items.
 * @see SceneVectorDrawLoadContext.h
 */
typedef struct dsSceneVectorDrawLoadContext
{
	/**
	 * @brief The allocator the load context was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Scratch data for each thread that is loaded with.
	 */
	dsThreadObjectStorage* scratchData;

	/**
	 * @brief Mapping from original dsTextQuality enum to the actual quality to use.
	 */
	dsTextQuality textQualityRemap[DS_TEXT_QUALITY_REMAP_SIZE];

	/**
	 * @brief Text substitution table to apply when loading text.
	 */
	const dsTextSubstitutionTable* substitutionTable;

	/**
	 * @brief The size of a pixel.
	 */
	float pixelSize;
} dsSceneVectorDrawLoadContext;

#ifdef __cplusplus
}
#endif
