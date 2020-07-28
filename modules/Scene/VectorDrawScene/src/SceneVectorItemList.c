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

#include <DeepSea/VectorDrawScene/SceneVectorItemList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneNodeItemData.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>
#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDrawScene/SceneVectorImageNode.h>
#include <DeepSea/VectorDrawScene/SceneTextNode.h>
#include <DeepSea/VectorDrawScene/SceneVectorNode.h>

#include <stdlib.h>
#include <string.h>

typedef struct TextVertex
{
	dsVector2f position;
	dsColor textColor;
	dsColor outlineColor;
	dsVector3f texCoords;
	dsVector4f style;
} TextVertex;

typedef struct TessTextVertexx
{
	dsVector4f position;
	dsAlignedBox2f geometry;
	dsColor textColor;
	dsColor outlineColor;
	dsAlignedBox2f texCoords;
	dsVector4f style;
} TessTextVertex;

typedef enum DrawType
{
	DrawType_Text,
	DrawType_Image
} DrawType;

typedef struct Entry
{
	const dsSceneVectorNode* node;
	const dsMatrix44f* transform;
	dsSceneNodeItemData* itemData;
	dsTextLayout* layout;
	uint32_t layoutVersion;
	uint64_t nodeID;
} Entry;

typedef struct TextInfo
{
	dsShader* shader;
	const dsTextLayout* text;
	void* textUserData;
	const dsTextStyle* styles;
	uint32_t styleCount;
	uint32_t fontTextureElement;
	uint32_t firstChar;
	uint32_t charCount;
} TextInfo;

typedef struct ImageInfo
{
	const dsVectorShaders* shaders;
	const dsVectorImage* image;
} ImageInfo;

typedef struct DrawItem
{
	int32_t z;
	uint32_t instance;
	DrawType type;
	union
	{
		TextInfo text;
		ImageInfo image;
	};
	dsMaterial* material;
} DrawItem;

struct dsSceneVectorItemList
{
	dsSceneItemList itemList;

	dsDynamicRenderStates renderStates;
	bool hasRenderStates;

	dsSharedMaterialValues* instanceValues;
	dsSceneInstanceData** instanceData;
	uint32_t instanceDataCount;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;

	dsSceneInstanceInfo* instances;
	DrawItem* drawItems;
	uint32_t maxInstances;
	uint32_t maxDrawItems;

	dsTextRenderBuffer* textRenderBuffer;
};

static void glyphPosition(dsVector2f* outPos, const dsVector2f* basePos,
	const dsVector2f* geometryPos, float slant)
{
	dsVector2_add(*outPos, *basePos, *geometryPos);
	outPos->x -= geometryPos->y*slant;
}

static bool addInstances(dsSceneItemList* itemList, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;

	uint32_t dummyCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, vectorList->drawItems, dummyCount,
			vectorList->entryCount, vectorList->maxDrawItems))
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	dummyCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, vectorList->instances, dummyCount,
			vectorList->entryCount, vectorList->maxInstances))
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	const dsSceneNodeType* textType = dsSceneTextNode_type();
	const dsSceneNodeType* vectorImageType = dsSceneVectorImageNode_type();
	DS_UNUSED(vectorImageType);
	for (uint32_t i = 0; i < vectorList->entryCount; ++i)
	{
		Entry* entry = vectorList->entries + i;

		DrawItem* drawItem = vectorList->drawItems + i;
		drawItem->z = entry->node->z;
		drawItem->instance = i;
		if (dsSceneNode_isOfType((const dsSceneNode*)entry->node, textType))
		{
			const dsSceneTextNode* node = (const dsSceneTextNode*)entry->node;
			drawItem->type = DrawType_Text;
			if (entry->layoutVersion == node->layoutVersion)
			{
				DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
					dsTextLayout_refresh(entry->layout, commandBuffer));
			}
			else
			{
				DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
					dsTextLayout_layout(entry->layout, commandBuffer, node->alignment,
						node->maxWidth, node->lineScale));
				entry->layoutVersion = node->layoutVersion;
			}

			drawItem->text.shader = node->shader;
			drawItem->text.text = entry->layout;
			drawItem->text.textUserData = node->textUserData;
			drawItem->text.styles = node->styles;
			drawItem->text.styleCount = node->styleCount;
			drawItem->text.fontTextureElement = node->fontTextureElement;
			drawItem->text.firstChar = node->firstChar;
			drawItem->text.charCount = node->charCount;
		}
		else
		{
			DS_ASSERT(dsSceneNode_isOfType((const dsSceneNode*)entry->node, vectorImageType));
			const dsSceneVectorImageNode* node = (const dsSceneVectorImageNode*)entry->node;
			drawItem->type = DrawType_Image;
			drawItem->image.image = node->vectorImage;
			drawItem->image.shaders = node->shaders;
			drawItem->material = node->material;
		}

		dsSceneInstanceInfo* instance = vectorList->instances + i;
		instance->node = (const dsSceneNode*)entry->node;
		instance->transform = *entry->transform;
	}

	DS_PROFILE_FUNC_RETURN(true);
}

static void setupInstances(dsSceneVectorItemList* vectorList, const dsView* view,
	uint32_t instanceCount)
{
	DS_PROFILE_FUNC_START();

	for (uint32_t i = 0; i < vectorList->instanceDataCount; ++i)
	{
		dsSceneInstanceData_populateData(vectorList->instanceData[i], view, vectorList->instances,
			instanceCount);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static int compareNodes(const void* left, const void* right)
{
	const DrawItem* leftInfo = (const DrawItem*)left;
	const DrawItem* rightInfo = (const DrawItem*)right;
	int32_t leftZ = leftInfo->z;
	int32_t rightZ = rightInfo->z;
	if (leftZ != rightZ)
		return leftZ - rightZ;

	// NOTE: First element of both text and image structs is the shaders, so compare for shader
	// should be valid whether it's image or text despite using image struct of the union.
	if (leftInfo->image.shaders != rightInfo->image.shaders)
		return leftInfo->image.shaders < rightInfo->image.shaders ? -1 : 1;
	if (leftInfo->material != rightInfo->material)
		return leftInfo->material < rightInfo->material ? -1 : 1;
	return leftInfo->instance - rightInfo->instance;
}

static void sortItems(dsSceneVectorItemList* vectorList)
{
	DS_PROFILE_FUNC_START();

	qsort(vectorList->drawItems, vectorList->entryCount, sizeof(DrawItem), &compareNodes);

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void drawItems(dsSceneVectorItemList* vectorList, uint32_t drawItemCount, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	dsShader* lastTextShader = NULL;
	dsMaterial* lastTextMaterial = NULL;
	dsDynamicRenderStates* renderStates =
		vectorList->hasRenderStates ? &vectorList->renderStates : NULL;
	dsTextRenderBuffer* textRenderBuffer = vectorList->textRenderBuffer;

	for (uint32_t i = 0; i < drawItemCount; ++i)
	{
		const DrawItem* drawItem = vectorList->drawItems + i;
		for (uint32_t j = 0; j < vectorList->instanceDataCount; ++j)
		{
			DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG, dsSceneInstanceData_bindInstance(
				vectorList->instanceData[j], drawItem->instance, vectorList->instanceValues));
		}

		switch (drawItem->type)
		{
			case DrawType_Text:
			{
				const dsText* text = drawItem->text.text->text;
				dsFont* font = text->font;
				DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
					dsSharedMaterialValues_setTextureID(vectorList->instanceValues,
						drawItem->text.fontTextureElement, dsFont_getTexture(font)));
				if (lastTextShader != drawItem->text.shader ||
					lastTextMaterial != drawItem->material)
				{
					if (lastTextShader)
					{
						DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
							dsTextRenderBuffer_draw(textRenderBuffer, commandBuffer));
						DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
							dsShader_unbind(lastTextShader, commandBuffer));
					}

					DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
						dsShader_bind(drawItem->text.shader, commandBuffer, drawItem->material,
							vectorList->instanceValues, renderStates));
					lastTextShader = drawItem->text.shader;
					lastTextMaterial = drawItem->material;
				}
				else
				{
					DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
						dsShader_updateInstanceValues(lastTextShader, commandBuffer,
							vectorList->instanceValues));
				}

				// Try to add to the current buffer. If this fails, flush it and try again.
				uint32_t firstChar = drawItem->text.firstChar;
				uint32_t charCount = drawItem->text.charCount;
				if (firstChar < text->characterCount && charCount > 0)
				{
					uint32_t maxCharCount = text->characterCount - firstChar;
					charCount = dsMin(charCount, maxCharCount);
					if (!dsTextRenderBuffer_addTextRange(textRenderBuffer, drawItem->text.text,
							drawItem->text.textUserData, firstChar, charCount))
					{
						DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
							dsTextRenderBuffer_draw(textRenderBuffer, commandBuffer));
						DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
							dsTextRenderBuffer_addTextRange(textRenderBuffer, drawItem->text.text,
								drawItem->text.textUserData, firstChar, charCount));
					}
				}
			}
			case DrawType_Image:
			{
				if (lastTextShader)
				{
					DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
						dsTextRenderBuffer_draw(textRenderBuffer, commandBuffer));
					dsShader_unbind(lastTextShader, commandBuffer);
					lastTextShader = NULL;
					lastTextMaterial = NULL;
				}

				dsMatrix44f modelViewProjection;
				dsMatrix44_mul(modelViewProjection, view->viewProjectionMatrix,
					vectorList->instances[drawItem->instance].transform);
				DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
					dsVectorImage_draw(drawItem->image.image, commandBuffer,
						drawItem->image.shaders, drawItem->material, &modelViewProjection,
						vectorList->instanceValues, renderStates));
				break;
			}
		}
	}

	if (lastTextShader)
	{
		DS_CHECK(DS_VECTOR_DRAW_SCENE_LOG_TAG,
			dsTextRenderBuffer_draw(textRenderBuffer, commandBuffer));
		dsShader_unbind(lastTextShader, commandBuffer);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void cleanup(dsSceneVectorItemList* vectorList)
{
	for (uint32_t i = 0; i < vectorList->instanceDataCount; ++i)
		dsSceneInstanceData_finish(vectorList->instanceData[i]);
}

static void destroyInstanceData(dsSceneInstanceData* const* instanceData,
	uint32_t instanceDataCount)
{
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
}

const char* const dsSceneVectorItemList_typeName = "VectorItemList";

bool dsSceneVectorItemList_defaultVertexFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Color0].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_Color1].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_TexCoord0].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_TexCoord1].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));
	return true;
}

bool dsSceneVectorItemList_defaultTessVertexFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position0].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Position1].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Color0].format = dsGfxFormat_decorate(
		dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_Color1].format = dsGfxFormat_decorate(
		dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_TexCoord0].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_TexCoord1].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));
	return true;
}

void dsSceneVectorItemList_defaultGlyphDataFunc(void* userData, const dsTextLayout* layout,
	void* layoutUserData, uint32_t glyphIndex, void* vertexData, const dsVertexFormat* format,
	uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(layoutUserData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
	DS_ASSERT(format->elements[dsVertexAttrib_Position].offset == offsetof(TextVertex, position));
	DS_ASSERT(format->elements[dsVertexAttrib_Color0].offset == offsetof(TextVertex, textColor));
	DS_ASSERT(format->elements[dsVertexAttrib_Color1].offset == offsetof(TextVertex, outlineColor));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord0].offset == offsetof(TextVertex, texCoords));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord1].offset == offsetof(TextVertex, style));
	DS_ASSERT(format->size == sizeof(TextVertex));
	DS_ASSERT(vertexCount == 4);

	const dsTextStyle* style = layout->styles + layout->glyphs[glyphIndex].styleIndex;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;
	dsVector2f position = layout->glyphs[glyphIndex].position;

	dsVector2f geometryPos;
	TextVertex* vertices = (TextVertex*)vertexData;
	geometryPos.x = glyph->geometry.min.x;
	geometryPos.y = glyph->geometry.min.y;
	glyphPosition(&vertices[0].position, &position, &geometryPos, style->slant);
	vertices[0].textColor = style->color;
	vertices[0].outlineColor = style->outlineColor;
	vertices[0].texCoords.x = glyph->texCoords.min.x;
	vertices[0].texCoords.y = glyph->texCoords.min.y;
	vertices[0].texCoords.z = (float)glyph->mipLevel;
	vertices[0].style.x = style->embolden;
	vertices[0].style.y = style->outlinePosition;
	vertices[0].style.z = style->outlineThickness;
	vertices[0].style.w = style->antiAlias;

	geometryPos.x = glyph->geometry.min.x;
	geometryPos.y = glyph->geometry.max.y;
	glyphPosition(&vertices[1].position, &position, &geometryPos, style->slant);
	vertices[1].textColor = style->color;
	vertices[1].outlineColor = style->outlineColor;
	vertices[1].texCoords.x = glyph->texCoords.min.x;
	vertices[1].texCoords.y = glyph->texCoords.max.y;
	vertices[1].texCoords.z = (float)glyph->mipLevel;
	vertices[1].style.x = style->embolden;
	vertices[1].style.y = style->outlinePosition;
	vertices[1].style.z = style->outlineThickness;
	vertices[1].style.w = style->antiAlias;

	geometryPos.x = glyph->geometry.max.x;
	geometryPos.y = glyph->geometry.max.y;
	glyphPosition(&vertices[2].position, &position, &geometryPos, style->slant);
	vertices[2].textColor = style->color;
	vertices[2].outlineColor = style->outlineColor;
	vertices[2].texCoords.x = glyph->texCoords.max.x;
	vertices[2].texCoords.y = glyph->texCoords.max.y;
	vertices[2].texCoords.z = (float)glyph->mipLevel;
	vertices[2].style.x = style->embolden;
	vertices[2].style.y = style->outlinePosition;
	vertices[2].style.z = style->outlineThickness;
	vertices[2].style.w = style->antiAlias;

	geometryPos.x = glyph->geometry.max.x;
	geometryPos.y = glyph->geometry.min.y;
	glyphPosition(&vertices[3].position, &position, &geometryPos, style->slant);
	vertices[3].textColor = style->color;
	vertices[3].outlineColor = style->outlineColor;
	vertices[3].texCoords.x = glyph->texCoords.max.x;
	vertices[3].texCoords.y = glyph->texCoords.min.y;
	vertices[3].texCoords.z = (float)glyph->mipLevel;
	vertices[3].style.x = style->embolden;
	vertices[3].style.y = style->outlinePosition;
	vertices[3].style.z = style->outlineThickness;
	vertices[3].style.w = style->antiAlias;
}

void dsSceneVectorItemList_defaultTessGlyphDataFunc(void* userData,
	const dsTextLayout* layout, void* layoutUserData, uint32_t glyphIndex, void* vertexData,
	const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(layoutUserData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
	DS_ASSERT(format->elements[dsVertexAttrib_Position0].offset ==
		offsetof(TessTextVertex, position));
	DS_ASSERT(format->elements[dsVertexAttrib_Position1].offset ==
		offsetof(TessTextVertex, geometry));
	DS_ASSERT(format->elements[dsVertexAttrib_Color0].offset ==
		offsetof(TessTextVertex, textColor));
	DS_ASSERT(format->elements[dsVertexAttrib_Color1].offset ==
		offsetof(TessTextVertex, outlineColor));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(TessTextVertex, texCoords));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord1].offset == offsetof(TessTextVertex, style));
	DS_ASSERT(format->size == sizeof(TessTextVertex));
	DS_ASSERT(vertexCount == 1);

	TessTextVertex* vertex = (TessTextVertex*)vertexData;
	const dsTextStyle* style = layout->styles + layout->glyphs[glyphIndex].styleIndex;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;
	vertex->position.x = layout->glyphs[glyphIndex].position.x;
	vertex->position.y = layout->glyphs[glyphIndex].position.y;
	vertex->position.z = (float)layout->glyphs[glyphIndex].mipLevel;
	vertex->position.w = style->antiAlias;
	vertex->geometry = glyph->geometry;
	vertex->texCoords = glyph->texCoords;
	vertex->textColor = style->color;
	vertex->outlineColor = style->outlineColor;
	vertex->style.x = style->slant;
	vertex->style.y = style->embolden;
	vertex->style.z = style->outlinePosition;
	vertex->style.w = style->outlineThickness;
}

uint64_t dsSceneVectorItemList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform, dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneVectorNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneVectorItemList* modelList = (dsSceneVectorItemList*)itemList;
	bool isText = dsSceneNode_isOfType(node, dsSceneTextNode_type());
	if (!modelList->textRenderBuffer && isText)
	{
		DS_LOG_WARNING(DS_VECTOR_DRAW_LOG_TAG,
			"Trying to add a text node to a vector item list that doesn't support text rendering.");
		return DS_NO_SCENE_NODE;
	}

	uint32_t index = modelList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->entries, modelList->entryCount,
			modelList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = modelList->entries + index;
	entry->node = (dsSceneVectorNode*)node;
	entry->transform = transform;
	entry->itemData = itemData;
	entry->nodeID = modelList->nextNodeID++;

	if (isText)
	{
		dsSceneTextNode* textNode = (dsSceneTextNode*)node;
		entry->layout = dsTextLayout_create(itemList->allocator, textNode->text, textNode->styles,
			textNode->styleCount);
		if (!entry->layout)
		{
			--modelList->entryCount;
			return DS_NO_SCENE_NODE;
		}

		// Force a re-layout the first time.
		entry->layoutVersion = textNode->layoutVersion - 1;
	}
	else
	{
		entry->layout = NULL;
		entry->layoutVersion = 0;
	}

	return entry->nodeID;
}

void dsSceneVectorItemList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;
	for (uint32_t i = 0; i < vectorList->entryCount; ++i)
	{
		if (vectorList->entries[i].nodeID != nodeID)
			continue;

		dsTextLayout_destroy(vectorList->entries[i].layout);

		// Order shouldn't matter, so use constant-time removal.
		vectorList->entries[i] = vectorList->entries[vectorList->entryCount - 1];
		--vectorList->entryCount;
		break;
	}
}

void dsSceneVectorItemList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);

	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;
	uint32_t instanceCount = 0;
	uint32_t drawItemCount = 0;
	addInstances(itemList, commandBuffer);
	setupInstances(vectorList, view, instanceCount);
	sortItems(vectorList);
	drawItems(vectorList, drawItemCount, view, commandBuffer);
	cleanup(vectorList);

	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
	DS_PROFILE_SCOPE_END();
}

dsSceneVectorItemList* dsSceneVectorItemList_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, dsSceneInstanceData* const* instanceData,
	uint32_t instanceDataCount, const dsSceneTextRenderBufferInfo* textRenderBufferInfo,
	const dsDynamicRenderStates* renderStates)
{
	if (!allocator || !name || !resourceManager || (!instanceData && instanceDataCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene model list allocator must support freeing memory.");
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	uint32_t valueCount = 0;
	for (uint32_t i = 0; i < instanceDataCount; ++i)
	{
		if (!instanceData[i])
		{
			errno = EINVAL;
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}
		valueCount += instanceData[i]->valueCount;
	}

	dsTextRenderBuffer* textRenderBuffer = NULL;
	if (textRenderBufferInfo)
	{
		textRenderBuffer = dsTextRenderBuffer_create(allocator, resourceManager,
			textRenderBufferInfo->maxGlyphs, textRenderBufferInfo->vertexFormat,
			textRenderBufferInfo->tessellationShader, textRenderBufferInfo->glyphDataFunc,
			textRenderBufferInfo->userData);
		if (!textRenderBuffer)
		{
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}
	}

	size_t nameLen = strlen(name);
	size_t globalDataSize = instanceDataCount > 0 ?
		dsSharedMaterialValues_fullAllocSize(instanceDataCount) : 0;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneVectorItemList)) +
		DS_ALIGNED_SIZE(nameLen + 1) +
		DS_ALIGNED_SIZE(sizeof(dsSceneInstanceData*)*instanceDataCount) + globalDataSize;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyInstanceData(instanceData, instanceDataCount);
		dsTextRenderBuffer_destroy(textRenderBuffer);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneVectorItemList* vectorList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneVectorItemList);
	DS_ASSERT(vectorList);

	dsSceneItemList* itemList = (dsSceneItemList*)vectorList;
	itemList->allocator = allocator;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsHashString(name);
	itemList->addNodeFunc = &dsSceneVectorItemList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneVectorItemList_removeNode;
	itemList->commitFunc = &dsSceneVectorItemList_commit;
	itemList->destroyFunc = (dsDestroySceneItemListFunction)&dsSceneVectorItemList_destroy;

	if (renderStates)
	{
		vectorList->renderStates = *renderStates;
		vectorList->hasRenderStates = true;
	}
	else
		vectorList->hasRenderStates = false;

	if (instanceDataCount > 0)
	{
		if (valueCount > 0)
		{
			vectorList->instanceValues = dsSharedMaterialValues_create((dsAllocator*)&bufferAlloc,
				instanceDataCount);
			DS_ASSERT(vectorList->instanceValues);
		}
		else
			vectorList->instanceValues = NULL;
		vectorList->instanceData = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneInstanceData*,
			instanceDataCount);
		DS_ASSERT(vectorList->instanceData);
		memcpy(vectorList->instanceData, instanceData,
			sizeof(dsSceneInstanceData*)*instanceDataCount);
	}
	else
	{
		vectorList->instanceValues = NULL;
		vectorList->instanceData = NULL;
	}
	vectorList->instanceDataCount = instanceDataCount;

	vectorList->entries = NULL;
	vectorList->entryCount = 0;
	vectorList->maxEntries = 0;
	vectorList->nextNodeID = 0;
	vectorList->instances = NULL;
	vectorList->drawItems = NULL;
	vectorList->maxInstances = 0;
	vectorList->maxDrawItems = 0;
	return vectorList;
}

const dsDynamicRenderStates* dsSceneVectorItemList_getRenderStates(
	const dsSceneVectorItemList* vectorList)
{
	return vectorList && vectorList->hasRenderStates ? &vectorList->renderStates : NULL;
}

void dsSceneVectorItemList_setRenderStates(dsSceneVectorItemList* vectorList,
	const dsDynamicRenderStates* renderStates)
{
	if (!vectorList)
		return;

	if (renderStates)
	{
		vectorList->hasRenderStates = true;
		vectorList->renderStates = *renderStates;
	}
	else
		vectorList->hasRenderStates = false;
}

void dsSceneVectorItemList_destroy(dsSceneVectorItemList* vectorList)
{
	if (!vectorList)
		return;

	for (uint32_t i = 0; i < vectorList->entryCount; ++i)
		dsTextLayout_destroy(vectorList->entries[i].layout);

	dsSceneItemList* itemList = (dsSceneItemList*)vectorList;
	destroyInstanceData(vectorList->instanceData, vectorList->instanceDataCount);
	dsSharedMaterialValues_destroy(vectorList->instanceValues);
	DS_VERIFY(dsTextRenderBuffer_destroy(vectorList->textRenderBuffer));
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList->instances));
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList->drawItems));
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList));
}
