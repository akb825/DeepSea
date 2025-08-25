/*
 * Copyright 2020-2025 Aaron Barany
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

#include <DeepSea/SceneVectorDraw/SceneVectorItemList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneNodeItemData.h>

#include <DeepSea/SceneVectorDraw/SceneVectorImageNode.h>
#include <DeepSea/SceneVectorDraw/SceneTextNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorNode.h>

#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>

#include <DeepSea/VectorDraw/VectorImage.h>

#include <stdlib.h>
#include <string.h>

typedef enum DrawType
{
	DrawType_Text,
	DrawType_Image
} DrawType;

typedef struct Entry
{
	const dsSceneVectorNode* node;
	const dsSceneTreeNode* treeNode;
	const dsSceneNodeItemData* itemData;
	uint64_t nodeID;
} Entry;

typedef struct TextInfo
{
	dsShader* shader;
	const dsTextLayout* layout;
	dsTextRenderBuffer* renderBuffer;
	void* textUserData;
	const dsTextStyle* styles;
	uint32_t styleCount;
	uint32_t fontTextureID;
	uint32_t firstChar;
	uint32_t charCount;
} TextInfo;

typedef struct ImageInfo
{
	const dsVectorShaders* shaders;
	const dsVectorImage* image;
	dsVector2f size;
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

	uint64_t* removeEntries;
	uint32_t removeEntryCount;
	uint32_t maxRemoveEntries;

	const dsSceneTreeNode** instances;
	DrawItem* drawItems;
	uint32_t maxInstances;
	uint32_t maxDrawItems;
};

static void getGlyphRange(uint32_t* outFirstGlyph, uint32_t* outGlyphCount,
	const dsTextLayout* layout, uint32_t firstChar, uint32_t charCount)
{
	*outFirstGlyph = 0;
	uint32_t maxChar = firstChar + charCount;
	uint32_t glyphIndex = 0;
	for (uint32_t i = 0; i < maxChar; ++i)
	{
		if (i == firstChar)
			*outFirstGlyph = glyphIndex;

		const dsCharMapping* charMapping = layout->text->charMappings + i;
		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
		{
			const dsGlyphLayout* glyph = layout->glyphs + charMapping->firstGlyph + j;

			// Skip empty glyphs.
			if (glyph->geometry.min.x == glyph->geometry.max.x ||
				glyph->geometry.min.y == glyph->geometry.max.y)
			{
				continue;
			}

			++glyphIndex;
		}
	}
	*outGlyphCount = glyphIndex - *outFirstGlyph;
}

static bool addInstances(dsSceneItemList* itemList)
{
	DS_PROFILE_FUNC_START();

	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;

	uint32_t dummyCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, vectorList->drawItems, dummyCount,
			vectorList->maxDrawItems, vectorList->entryCount))
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	dummyCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, vectorList->instances, dummyCount,
			vectorList->maxInstances, vectorList->entryCount))
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
			drawItem->material = node->material;
			drawItem->text.shader = node->shader;
			drawItem->text.layout = node->layout;
			drawItem->text.renderBuffer = node->renderBuffer;
			drawItem->text.textUserData = node->textUserData;
			drawItem->text.styles = node->styles;
			drawItem->text.styleCount = node->styleCount;
			drawItem->text.fontTextureID = node->fontTextureID;
			drawItem->text.firstChar = node->firstChar;
			drawItem->text.charCount = node->charCount;
		}
		else
		{
			DS_ASSERT(dsSceneNode_isOfType((const dsSceneNode*)entry->node, vectorImageType));
			const dsSceneVectorImageNode* node = (const dsSceneVectorImageNode*)entry->node;
			drawItem->type = DrawType_Image;
			drawItem->material = node->material;
			drawItem->image.image = node->vectorImage;
			drawItem->image.shaders = node->shaders;
			drawItem->image.size = node->size;
			drawItem->material = node->material;
		}

		vectorList->instances[i] = entry->treeNode;
	}

	DS_PROFILE_FUNC_RETURN(true);
}

static void setupInstances(dsSceneVectorItemList* vectorList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	for (uint32_t i = 0; i < vectorList->instanceDataCount; ++i)
	{
		DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG, dsSceneInstanceData_populateData(
			vectorList->instanceData[i], view, commandBuffer, vectorList->instances,
			vectorList->entryCount));
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
		return DS_CMP(leftZ, rightZ);

	// NOTE: First element of both text and image structs is the shaders, so compare for shader
	// should be valid whether it's image or text despite using image struct of the union.
	int shaderCmp = DS_CMP(leftInfo->image.shaders, rightInfo->image.shaders);
	int materialCmp = DS_CMP(leftInfo->material, rightInfo->material);
	// Small enough that subtract should be safe.
	int instanceCmp = leftInfo->instance - rightInfo->instance;

	int result = dsCombineCmp(shaderCmp, materialCmp);
	return dsCombineCmp(result, instanceCmp);
}

static void sortItems(dsSceneVectorItemList* vectorList)
{
	DS_PROFILE_FUNC_START();

	qsort(vectorList->drawItems, vectorList->entryCount, sizeof(DrawItem), &compareNodes);

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void drawItems(dsSceneVectorItemList* vectorList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	dsShader* lastTextShader = NULL;
	dsMaterial* lastTextMaterial = NULL;
	dsDynamicRenderStates* renderStates =
		vectorList->hasRenderStates ? &vectorList->renderStates : NULL;

	for (uint32_t i = 0; i < vectorList->entryCount; ++i)
	{
		const DrawItem* drawItem = vectorList->drawItems + i;
		for (uint32_t j = 0; j < vectorList->instanceDataCount; ++j)
		{
			DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG, dsSceneInstanceData_bindInstance(
				vectorList->instanceData[j], drawItem->instance, vectorList->instanceValues));
		}

		switch (drawItem->type)
		{
			case DrawType_Text:
			{
				const dsTextLayout* layout = drawItem->text.layout;
				dsTextRenderBuffer* renderBuffer = drawItem->text.renderBuffer;
				const dsText* text = layout->text;
				dsFont* font = text->font;
				DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					dsSharedMaterialValues_setTextureID(vectorList->instanceValues,
						drawItem->text.fontTextureID, dsFont_getTexture(font)));
				if (lastTextShader != drawItem->text.shader ||
					lastTextMaterial != drawItem->material)
				{
					if (lastTextShader)
					{
						DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
							dsShader_unbind(lastTextShader, commandBuffer));
					}

					DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
						dsShader_bind(drawItem->text.shader, commandBuffer, drawItem->material,
							view->globalValues, renderStates));
					lastTextShader = drawItem->text.shader;
					lastTextMaterial = drawItem->material;
				}

				DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					dsShader_updateInstanceValues(lastTextShader, commandBuffer,
						vectorList->instanceValues));

				// Try to add to the current buffer. If this fails, flush it and try again.
				uint32_t firstChar = drawItem->text.firstChar;
				uint32_t charCount = drawItem->text.charCount;
				if (firstChar < text->characterCount && charCount > 0)
				{
					uint32_t maxCharCount = text->characterCount - firstChar;
					charCount = dsMin(charCount, maxCharCount);
					uint32_t firstGlyph, glyphCount;
					getGlyphRange(&firstGlyph, &glyphCount, layout, firstChar, charCount);
					DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
						dsTextRenderBuffer_drawRange(renderBuffer, commandBuffer, firstGlyph,
							glyphCount));
				}
				break;
			}
			case DrawType_Image:
			{
				if (lastTextShader)
				{
					dsShader_unbind(lastTextShader, commandBuffer);
					lastTextShader = NULL;
					lastTextMaterial = NULL;
				}

				dsVector2f imageSize;
				DS_VERIFY(dsVectorImage_getSize(&imageSize, drawItem->image.image));

				dsMatrix44f scale;
				dsMatrix44f_makeScale(&scale, drawItem->image.size.x/imageSize.x,
					drawItem->image.size.y/imageSize.y, 1.0f);

				const dsMatrix44f* nodeTransform =
					&vectorList->instances[drawItem->instance]->transform;
				dsMatrix44f transform;
				dsMatrix44f_affineMul(&transform, nodeTransform, &scale);

				dsMatrix44f modelViewProjection;
				dsMatrix44f_mul(&modelViewProjection, &view->viewProjectionMatrix, &transform);

				DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					dsVectorImage_draw(drawItem->image.image, commandBuffer,
						drawItem->image.shaders, drawItem->material, &modelViewProjection,
						view->globalValues, renderStates));
				break;
			}
		}
	}

	if (lastTextShader)
		dsShader_unbind(lastTextShader, commandBuffer);

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

static uint64_t dsSceneVectorItemList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_ASSERT(itemList);
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneVectorNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneVectorItemList* modelList = (dsSceneVectorItemList*)itemList;
	uint32_t index = modelList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->entries, modelList->entryCount,
			modelList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = modelList->entries + index;
	entry->node = (const dsSceneVectorNode*)node;
	entry->treeNode = treeNode;
	entry->itemData = itemData;
	entry->nodeID = modelList->nextNodeID++;

	return entry->nodeID;
}

static void dsSceneVectorItemList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_ASSERT(itemList);
	DS_UNUSED(treeNode);
	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;

	uint32_t index = vectorList->removeEntryCount;
	if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, vectorList->removeEntries,
			vectorList->removeEntryCount, vectorList->maxRemoveEntries, 1))
	{
		vectorList->removeEntries[index] = nodeID;
	}
	else
	{
		dsSceneItemListEntries_removeSingle(vectorList->entries, &vectorList->entryCount,
			sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	}
}

static void dsSceneVectorItemList_preRenderPass(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(itemList);
	DS_ASSERT(!itemList->skipPreRenderPass);
	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);

	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(vectorList->entries, &vectorList->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), vectorList->removeEntries,
		vectorList->removeEntryCount);
	vectorList->removeEntryCount = 0;

	addInstances(itemList);
	setupInstances(vectorList, view, commandBuffer);

	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
}

static void dsSceneVectorItemList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(itemList);
	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);

	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;
	if (itemList->skipPreRenderPass)
	{
		// Lazily remove entries.
		dsSceneItemListEntries_removeMulti(vectorList->entries, &vectorList->entryCount,
			sizeof(Entry), offsetof(Entry, nodeID), vectorList->removeEntries,
			vectorList->removeEntryCount);
		vectorList->removeEntryCount = 0;

		addInstances(itemList);
		setupInstances(vectorList, view, NULL);
	}
	sortItems(vectorList);
	drawItems(vectorList, view, commandBuffer);
	cleanup(vectorList);

	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
}

static uint32_t dsSceneVectorItemList_hash(const dsSceneItemList* itemList, uint32_t commonHash)
{
	DS_ASSERT(itemList);
	const dsSceneVectorItemList* vectorList = (const dsSceneVectorItemList*)itemList;
	uint32_t hash = commonHash;
	if (vectorList->hasRenderStates)
		hash = dsHashCombineBytes(hash, &vectorList->renderStates, sizeof(dsDynamicRenderStates));
	for (uint32_t i = 0; i < vectorList->instanceDataCount; ++i)
		hash = dsSceneInstanceData_hash(vectorList->instanceData[i], hash);
	return hash;
}

static bool dsSceneVectorItemList_equal(const dsSceneItemList* left, const dsSceneItemList* right)
{
	DS_ASSERT(left);
	DS_ASSERT(left->type == dsSceneVectorItemList_type());
	DS_ASSERT(right);
	DS_ASSERT(right->type == dsSceneVectorItemList_type());

	const dsSceneVectorItemList* leftVectorItemList = (const dsSceneVectorItemList*)left;
	const dsSceneVectorItemList* rightVectorItemList = (const dsSceneVectorItemList*)right;

	if (leftVectorItemList->hasRenderStates != rightVectorItemList->hasRenderStates ||
		(leftVectorItemList->hasRenderStates && memcmp(&leftVectorItemList->renderStates,
			&rightVectorItemList->renderStates, sizeof(dsDynamicRenderStates)) != 0) ||
		leftVectorItemList->instanceDataCount != rightVectorItemList->instanceDataCount)
	{
		return false;
	}

	for (uint32_t i = 0; i < leftVectorItemList->instanceDataCount; ++i)
	{
		if (!dsSceneInstanceData_equal(leftVectorItemList->instanceData[i],
				rightVectorItemList->instanceData[i]))
		{
			return false;
		}
	}
	return true;
}

static void dsSceneVectorItemList_destroy(dsSceneItemList* itemList)
{
	DS_ASSERT(itemList);
	dsSceneVectorItemList* vectorList = (dsSceneVectorItemList*)itemList;
	destroyInstanceData(vectorList->instanceData, vectorList->instanceDataCount);
	dsSharedMaterialValues_destroy(vectorList->instanceValues);
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList->removeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, (void*)vectorList->instances));
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList->drawItems));
	DS_VERIFY(dsAllocator_free(itemList->allocator, vectorList));
}

const char* const dsSceneVectorItemList_typeName = "VectorItemList";

static dsSceneItemListType itemListType =
{
	.addNodeFunc = &dsSceneVectorItemList_addNode,
	.removeNodeFunc = &dsSceneVectorItemList_removeNode,
	.preRenderPassFunc = dsSceneVectorItemList_preRenderPass,
	.commitFunc = &dsSceneVectorItemList_commit,
	.hashFunc = &dsSceneVectorItemList_hash,
	.equalFunc = &dsSceneVectorItemList_equal,
	.destroyFunc = &dsSceneVectorItemList_destroy
};

const dsSceneItemListType* dsSceneVectorItemList_type(void)
{
	return &itemListType;
}

dsSceneVectorItemList* dsSceneVectorItemList_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, dsSceneInstanceData* const* instanceData,
	uint32_t instanceDataCount, const dsDynamicRenderStates* renderStates)
{
	if (!allocator || !name || !resourceManager || (!instanceData && instanceDataCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG,
			"Vector item list allocator must support freeing memory.");
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	uint32_t valueCount = 1;
	bool skipPreRenderPass = true;
	for (uint32_t i = 0; i < instanceDataCount; ++i)
	{
		if (!instanceData[i])
		{
			errno = EINVAL;
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}
		valueCount += instanceData[i]->valueCount;
		if (instanceData[i]->needsCommandBuffer)
			skipPreRenderPass = false;

	}

	size_t nameLen = strlen(name);
	size_t globalDataSize = dsSharedMaterialValues_fullAllocSize(valueCount);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneVectorItemList)) +
		DS_ALIGNED_SIZE(nameLen + 1) +
		DS_ALIGNED_SIZE(sizeof(dsSceneInstanceData*)*instanceDataCount) + globalDataSize;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneVectorItemList* vectorList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneVectorItemList);
	DS_ASSERT(vectorList);

	dsSceneItemList* itemList = (dsSceneItemList*)vectorList;
	itemList->allocator = allocator;
	itemList->type = dsSceneVectorItemList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->skipPreRenderPass = skipPreRenderPass;

	if (renderStates)
	{
		vectorList->renderStates = *renderStates;
		vectorList->hasRenderStates = true;
	}
	else
		vectorList->hasRenderStates = false;

	vectorList->instanceValues = dsSharedMaterialValues_create((dsAllocator*)&bufferAlloc,
		valueCount);
	DS_ASSERT(vectorList->instanceValues);

	if (instanceDataCount > 0)
	{
		vectorList->instanceData = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneInstanceData*,
			instanceDataCount);
		DS_ASSERT(vectorList->instanceData);
		memcpy(vectorList->instanceData, instanceData,
			sizeof(dsSceneInstanceData*)*instanceDataCount);
	}
	else
		vectorList->instanceData = NULL;
	vectorList->instanceDataCount = instanceDataCount;

	vectorList->entries = NULL;
	vectorList->entryCount = 0;
	vectorList->maxEntries = 0;
	vectorList->nextNodeID = 0;

	vectorList->removeEntries = NULL;
	vectorList->removeEntryCount = 0;
	vectorList->maxRemoveEntries = 0;

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
