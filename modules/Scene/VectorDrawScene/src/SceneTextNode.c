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

#include <DeepSea/VectorDrawScene/SceneTextNode.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/VectorDrawScene/SceneVectorNode.h>

#include <string.h>

const char* const dsSceneTextNode_typeName = "TextNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneTextNode_type(void)
{
	return &nodeType;
}

const dsSceneNodeType* dsSceneTextNode_setupParentType(dsSceneNodeType* type)
{
	dsSceneNode_setupParentType(&nodeType, dsSceneVectorNode_type());
	return dsSceneNode_setupParentType(type, &nodeType);
}

dsSceneTextNode* dsSceneTextNode_create(dsAllocator* allocator, const dsText* text,
	void* textUserData, const dsTextStyle* styles, uint32_t styleCount, dsTextAlign alignment,
	float maxWidth, float lineScale, int32_t z, uint32_t firstChar, uint32_t charCount,
	dsShader* shader, dsMaterial* material, uint32_t fontTextureElement, const char** itemLists,
	uint32_t itemListCount, dsSceneResources** resources, uint32_t resourceCount)
{
	return dsSceneTextNode_createBase(allocator, sizeof(dsSceneTextNode), text,
		textUserData, styles, styleCount, alignment, maxWidth, lineScale, z, firstChar, charCount,
		shader, material, fontTextureElement, itemLists, itemListCount, resources, resourceCount);
}

dsSceneTextNode* dsSceneTextNode_createBase(dsAllocator* allocator, size_t structSize,
	const dsText* text, void* textUserData, const dsTextStyle* styles, uint32_t styleCount,
	dsTextAlign alignment, float maxWidth, float lineScale, int32_t z, uint32_t firstChar,
	uint32_t charCount, dsShader* shader, dsMaterial* material, uint32_t fontTextureElement,
	const char** itemLists, uint32_t itemListCount, dsSceneResources** resources,
	uint32_t resourceCount)
{
	if (!allocator || !text || !styles || styleCount == 0 || !shader || !material ||
		(!itemLists && itemListCount > 0) || (!resources && resourceCount == 0))
	{
		errno = EINVAL;
		return NULL;
	}

	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	DS_ASSERT(materialDesc);
	const dsMaterialElement* fontTextureMatElement = materialDesc->elements + fontTextureElement;
	if (fontTextureElement >= materialDesc->elementCount ||
		fontTextureMatElement->type != dsMaterialType_Texture ||
		fontTextureMatElement->binding != dsMaterialBinding_Instance)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR(DS_VECTOR_DRAW_SCENE_LOG_TAG,
			"Font texture element must be a texture with instance binding.");
		return NULL;
	}

	dsTextLayout* layout = dsTextLayout_create(allocator, text, styles, styleCount);
	if (!layout)
		return NULL;

	// Add the style array to the struct size to pool allocations.
	size_t styleOffset = DS_ALIGNED_SIZE(structSize);
	size_t finalStructSize = styleOffset + DS_ALIGNED_SIZE(sizeof(dsTextStyle)*styleCount);

	dsSceneTextNode* node = (dsSceneTextNode*)dsSceneVectorNode_create(
		allocator, finalStructSize, z, itemLists, itemListCount, resources, resourceCount);
	if (!node)
	{
		dsTextLayout_destroy(layout);
		return NULL;
	}

	dsSceneNode* baseNode = (dsSceneNode*)node;
	baseNode->type = dsSceneTextNode_setupParentType(NULL);

	node->layout = layout;
	node->textUserData = textUserData;
	node->shader = shader;
	node->material = material;
	node->styles = (dsTextStyle*)((uint8_t*)node + styleOffset);
	memcpy(node->styles, styles, sizeof(dsTextStyle)*styleCount);
	node->styleCount = styleCount;
	node->fontTextureElement = fontTextureElement;
	node->alignment = alignment;
	node->maxWidth = maxWidth;
	node->lineScale = lineScale;
	node->firstChar = firstChar;
	node->charCount = charCount;
	node->layoutVersion = 0;

	return node;
}

void dsSceneTextNode_updateLayout(dsSceneTextNode* node)
{
	if (node)
		++node->layoutVersion;
}

void dsSceneTextNode_destroy(dsSceneNode* node)
{
	DS_ASSERT(dsSceneNode_isOfType(node, dsSceneTextNode_type()));
	dsSceneTextNode* textNode = (dsSceneTextNode*)node;
	dsTextLayout_destroy(textNode->layout);
}
