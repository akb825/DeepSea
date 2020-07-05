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

#include <DeepSea/VectorDrawScene/SceneVectorTextNode.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/VectorDrawScene/SceneVectorNode.h>

#include <string.h>

const char* const dsSceneVectorText_typeName = "TextNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneVectorTextNode_type(void)
{
	return &nodeType;
}

const dsSceneNodeType* dsSceneVectorTextNode_setupParentType(dsSceneNodeType* type)
{
	dsSceneNode_setupParentType(&nodeType, dsSceneVectorTextNode_type());
	return dsSceneNode_setupParentType(type, &nodeType);
}

dsSceneVectorTextNode* dsSceneVectorTextNode_create(dsAllocator* allocator, const dsText* text,
	void* textUserData, dsTextStyle* styles, uint32_t styleCount, dsTextAlign alignment,
	float maxWidth, float lineScale, uint32_t firstChar, uint32_t charCount, dsShader* shader,
	dsMaterial* material, const char** itemLists, uint32_t itemListCount,
	dsSceneResources** resources, uint32_t resourceCount)
{
	return dsSceneVectorTextNode_createBase(allocator, sizeof(dsSceneVectorTextNode), text,
		textUserData, styles, styleCount, alignment, maxWidth, lineScale, firstChar, charCount,
		shader, material, itemLists, itemListCount, resources, resourceCount);
}

dsSceneVectorTextNode* dsSceneVectorTextNode_createBase(dsAllocator* allocator, size_t structSize,
	const dsText* text, void* textUserData, dsTextStyle* styles, uint32_t styleCount,
	dsTextAlign alignment, float maxWidth, float lineScale, uint32_t firstChar, uint32_t charCount,
	dsShader* shader, dsMaterial* material, const char** itemLists, uint32_t itemListCount,
	dsSceneResources** resources, uint32_t resourceCount)
{
	if (!allocator || !text || !styles || styleCount == 0 || !shader || !material ||
		(!itemLists && itemListCount > 0) || (!resources && resourceCount == 0))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!DS_IS_BUFFER_RANGE_VALID(firstChar, charCount, text->characterCount))
	{
		errno = ERANGE;
		return NULL;
	}

	// Add the style array to the struct size to pool allocations.
	size_t styleOffset = DS_ALIGNED_SIZE(structSize);
	size_t finalStructSize = styleOffset + DS_ALIGNED_SIZE(sizeof(dsTextStyle)*styleCount);

	dsSceneVectorTextNode* node = (dsSceneVectorTextNode*)dsSceneVectorNode_create(
		allocator, finalStructSize, itemLists, itemListCount, resources, resourceCount);
	if (!node)
		return NULL;

	dsSceneNode* baseNode = (dsSceneNode*)node;
	baseNode->type = dsSceneVectorTextNode_setupParentType(NULL);

	node->text = text;
	node->textUserData = textUserData;
	node->shader = shader;
	node->material = material;
	node->styles = (dsTextStyle*)((uint8_t*)node + styleOffset);
	memcpy(node->styles, styles, sizeof(dsTextStyle)*styleCount);
	node->styleCount = styleCount;
	node->alignment = alignment;
	node->maxWidth = maxWidth;
	node->lineScale = lineScale;
	node->firstChar = firstChar;
	node->charCount = charCount;
	node->layoutVersion = 0;
	return node;
}

void dsSceneVectorTextNode_updateLayout(dsSceneVectorTextNode* node)
{
	if (node)
		++node->layoutVersion;
}
