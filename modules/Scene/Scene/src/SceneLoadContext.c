/*
 * Copyright 2019-2023 Aaron Barany
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

#include <DeepSea/Scene/SceneLoadContext.h>

#include "SceneLoadContextInternal.h"
#include "SceneTypes.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/ItemLists/SceneModelList.h>
#include <DeepSea/Scene/ItemLists/SceneFullScreenResolve.h>
#include <DeepSea/Scene/ItemLists/ViewCullList.h>
#include <DeepSea/Scene/ItemLists/ViewMipmapList.h>
#include <DeepSea/Scene/Nodes/SceneModelNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/ViewTransformData.h>

#include <string.h>

size_t dsSceneLoadContext_sizeof(void)
{
	return sizeof(dsSceneLoadContext);
}

size_t dsSceneLoadContext_fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsSceneLoadContext));
}

dsSceneLoadContext* dsSceneLoadContext_create(dsAllocator* allocator, dsRenderer* renderer)
{
	if (!allocator || !renderer)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneLoadContext* context = DS_ALLOCATE_OBJECT(allocator, dsSceneLoadContext);
	if (!context)
		return NULL;

	context->allocator = dsAllocator_keepPointer(allocator);
	context->renderer = renderer;
	dsHashTable_initialize(&context->nodeTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);
	dsHashTable_initialize(&context->itemListTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);
	dsHashTable_initialize(&context->instanceDataTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);
	dsHashTable_initialize(&context->customResourceTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);
	dsHashTable_initialize(&context->resourceActionTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);

	// Built-in types.
	dsSceneLoadContext_registerNodeType(context, dsSceneNodeRef_typeName, &dsSceneNodeRef_load,
		NULL, NULL);
	dsSceneLoadContext_registerNodeType(context, dsSceneModelNode_typeName, &dsSceneModelNode_load,
		NULL, NULL);
	dsSceneLoadContext_registerNodeType(context, dsSceneModelNode_reconfigTypeName,
		&dsSceneModelNode_loadReconfig, NULL, NULL);
	dsSceneLoadContext_registerNodeType(context, dsSceneModelNode_remapTypeName,
		&dsSceneModelNode_loadRemap, NULL, NULL);
	dsSceneLoadContext_registerNodeType(context, dsSceneTransformNode_typeName,
		&dsSceneTransformNode_load, NULL, NULL);

	dsSceneLoadContext_registerItemListType(context, dsSceneFullScreenResolve_typeName,
		&dsSceneFullScreenResolve_load, NULL, NULL);
	dsSceneLoadContext_registerItemListType(context, dsSceneModelList_typeName,
		&dsSceneModelList_load, NULL, NULL);
	dsSceneLoadContext_registerItemListType(context, dsViewCullList_typeName,
		&dsViewCullList_load, NULL, NULL);
	dsSceneLoadContext_registerItemListType(context, dsViewMipmapList_typeName,
		&dsViewMipmapList_load, NULL, NULL);
	dsSceneLoadContext_registerItemListType(context, dsViewTransformData_typeName,
		&dsViewTransformData_load, NULL, NULL);

	dsSceneLoadContext_registerInstanceDataType(context, dsInstanceTransformData_typeName,
		&dsInstanceTransformData_load, NULL, NULL);

	// Actions aren't exposed in code so inlined names.
	dsSceneLoadContext_registerResourceActionType(context, "NodeChildren",
		&dsSceneNodeChildren_load, NULL, NULL, 0);

	return context;
}

dsRenderer* dsSceneLoadContext_getRenderer(const dsSceneLoadContext* context)
{
	if (!context)
	{
		errno = EINVAL;
		return NULL;
	}

	return context->renderer;
}

bool dsSceneLoadContext_registerNodeType(dsSceneLoadContext* context, const char* name,
	dsLoadSceneNodeFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->nodeTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Node type name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneNodeItem* nodeType = context->nodeTypes + index;
	memcpy(nodeType->name, name, nameLength + 1);
	nodeType->loadFunc = loadFunc;
	nodeType->userData = userData;
	nodeType->destroyUserDataFunc = destroyUserDataFunc;
	if (!dsHashTable_insert(hashTable, nodeType->name, (dsHashTableNode*)nodeType, NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Node type '%s' has already been registered.", name);
		return false;
	}
	return true;
}

bool dsSceneLoadContext_registerItemListType(dsSceneLoadContext* context, const char* name,
	dsLoadSceneItemListFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->itemListTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Item list type name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneItemListItem* itemListType = context->itemListTypes + index;
	memcpy(itemListType->name, name, nameLength + 1);
	itemListType->loadFunc = loadFunc;
	itemListType->userData = userData;
	itemListType->destroyUserDataFunc = destroyUserDataFunc;
	if (!dsHashTable_insert(hashTable, itemListType->name, (dsHashTableNode*)itemListType, NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Item list type '%s' has already been registered.", name);
		return false;
	}
	return true;
}

bool dsSceneLoadContext_registerInstanceDataType(dsSceneLoadContext* context, const char* name,
	dsLoadSceneInstanceDataFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->instanceDataTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Instance data type name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneInstanceDataItem* instanceDataType = context->instanceDataTypes + index;
	memcpy(instanceDataType->name, name, nameLength + 1);
	instanceDataType->loadFunc = loadFunc;
	instanceDataType->userData = userData;
	instanceDataType->destroyUserDataFunc = destroyUserDataFunc;
	if (!dsHashTable_insert(hashTable, instanceDataType->name, (dsHashTableNode*)instanceDataType,
			NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Instance data type '%s' has already been registered.",
			name);
		return false;
	}
	return true;
}

bool dsSceneLoadContext_registerCustomResourceType(dsSceneLoadContext* context,
	const char* name, const dsCustomSceneResourceType* type,
	dsLoadCustomSceneResourceFunction loadFunc,
	dsDestroyCustomSceneResourceFunction destroyResourceFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, uint32_t additionalResources)
{
	if (!context || !name || !type || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->customResourceTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Custom scene resource type name '%s' exceeds maximum size of %u.", name,
			DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadCustomSceneResourceItem* customResourceType = context->customResourceTypes + index;
	memcpy(customResourceType->name, name, nameLength + 1);
	customResourceType->type = type;
	customResourceType->loadFunc = loadFunc;
	customResourceType->destroyResourceFunc = destroyResourceFunc;
	customResourceType->userData = userData;
	customResourceType->destroyUserDataFunc = destroyUserDataFunc;
	customResourceType->additionalResources = additionalResources;
	if (!dsHashTable_insert(hashTable, customResourceType->name,
			(dsHashTableNode*)customResourceType, NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Custom scene resource type '%s' has already been registered.", name);
		return false;
	}
	return true;
}

uint32_t dsSceneLoadContext_getCustomResourceAdditionalResources(const dsSceneLoadContext* context,
	const char* name)
{
	if (!context || !name)
		return 0;

	dsLoadCustomSceneResourceItem* foundType = (dsLoadCustomSceneResourceItem*)dsHashTable_find(
		&context->customResourceTypeTable.hashTable, name);
	if (!foundType)
		return 0;

	return foundType->additionalResources;
}

bool dsSceneLoadContext_registerResourceActionType(dsSceneLoadContext* context,
	const char* name, dsLoadSceneResourceActionFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, uint32_t additionalResources)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->resourceActionTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Scene resource action type name '%s' exceeds maximum size of %u.", name,
			DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneResourceActionItem* resourceActionType = context->resourceActionTypes + index;
	memcpy(resourceActionType->name, name, nameLength + 1);
	resourceActionType->loadFunc = loadFunc;
	resourceActionType->userData = userData;
	resourceActionType->destroyUserDataFunc = destroyUserDataFunc;
	resourceActionType->additionalResources = additionalResources;
	if (!dsHashTable_insert(hashTable, resourceActionType->name,
			(dsHashTableNode*)resourceActionType, NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Scene resource action type '%s' has already been registered.", name);
		return false;
	}
	return true;
}

uint32_t dsSceneLoadContext_getResourceActionAdditionalResources(const dsSceneLoadContext* context,
	const char* name)
{
	if (!context || !name)
		return 0;

	dsLoadSceneResourceActionItem* foundType = (dsLoadSceneResourceActionItem*)dsHashTable_find(
		&context->resourceActionTypeTable.hashTable, name);
	if (!foundType)
		return 0;

	return foundType->additionalResources;
}

void dsSceneLoadContext_destroy(dsSceneLoadContext* context)
{
	if (!context)
		return;

	dsHashTable* hashTable = &context->nodeTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneNodeItem* nodeType = (dsLoadSceneNodeItem*)node;
		if (nodeType->destroyUserDataFunc)
			nodeType->destroyUserDataFunc(nodeType->userData);
	}

	hashTable = &context->itemListTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneItemListItem* itemListType = (dsLoadSceneItemListItem*)node;
		if (itemListType->destroyUserDataFunc)
			itemListType->destroyUserDataFunc(itemListType->userData);
	}

	hashTable = &context->instanceDataTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneInstanceDataItem* instanceDataType = (dsLoadSceneInstanceDataItem*)node;
		if (instanceDataType->destroyUserDataFunc)
			instanceDataType->destroyUserDataFunc(instanceDataType->userData);
	}

	hashTable = &context->customResourceTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadCustomSceneResourceItem* customResourceType = (dsLoadCustomSceneResourceItem*)node;
		if (customResourceType->destroyUserDataFunc)
			customResourceType->destroyUserDataFunc(customResourceType->userData);
	}

	hashTable = &context->resourceActionTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneResourceActionItem* resourceActionType = (dsLoadSceneResourceActionItem*)node;
		if (resourceActionType->destroyUserDataFunc)
			resourceActionType->destroyUserDataFunc(resourceActionType->userData);
	}

	if (context->allocator)
		DS_VERIFY(dsAllocator_free(context->allocator, context));
}
