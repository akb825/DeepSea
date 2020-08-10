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

#include <DeepSea/VectorDrawScene/SceneVectorImageNode.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/VectorDrawScene/SceneVectorNode.h>

const char* const dsSceneVectorImageNode_typeName = "VectorImageNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneVectorImageNode_type(void)
{
	return &nodeType;
}

const dsSceneNodeType* dsSceneVectorImageNode_setupParentType(dsSceneNodeType* type)
{
	dsSceneNode_setupParentType(&nodeType, dsSceneVectorNode_type());
	return dsSceneNode_setupParentType(type, &nodeType);
}

dsSceneVectorImageNode* dsSceneVectorImageNode_create(dsAllocator* allocator,
	dsVectorImage* vectorImage, int32_t z, const dsVectorShaders* shaders, dsMaterial* material,
	const char** itemLists, uint32_t itemListCount, dsSceneResources** resources,
	uint32_t resourceCount)
{
	return dsSceneVectorImageNode_createBase(allocator, sizeof(dsSceneVectorImageNode), vectorImage,
		z, shaders, material, itemLists, itemListCount, resources, resourceCount);
}

dsSceneVectorImageNode* dsSceneVectorImageNode_createBase(dsAllocator* allocator, size_t structSize,
	dsVectorImage* vectorImage, int32_t z, const dsVectorShaders* shaders, dsMaterial* material,
	const char** itemLists, uint32_t itemListCount, dsSceneResources** resources,
	uint32_t resourceCount)
{
	if (!vectorImage || !shaders || !material)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneVectorImageNode* node = (dsSceneVectorImageNode*)dsSceneVectorNode_create(
		allocator, structSize, z, itemLists, itemListCount, resources, resourceCount);
	if (!node)
		return NULL;

	dsSceneNode* baseNode = (dsSceneNode*)node;
	baseNode->type = dsSceneVectorImageNode_setupParentType(NULL);

	node->vectorImage = vectorImage;
	node->shaders = shaders;
	node->material = material;
	return node;
}

