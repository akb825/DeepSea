/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the basic node types provided by the DeepSea/Scene library.
 */

/**
 * @brief ID for a type of a scene node.
 *
 * The type should be declared as a static variable. See dsSceneNode_setupParentType() for
 * information for how to set up the parent type.
 */
typedef struct dsSceneNodeType dsSceneNodeType;

/** @copydoc dsSceneNodeType */
struct dsSceneNodeType
{
	/**
	 * @brief The parent type of the node, or NULL if there is no base type.
	 */
	const dsSceneNodeType* parent;
};

/**
 * @brief Struct for a node within a scene graph.
 *
 * Scene nodes are reference counted. They may be referenced multiple times, or even within
 * different scenes. The reference count starts at 1 on creation and once the last reference has
 * been freed the node will be deleted.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsSceneItemList and the true internal type.
 *
 * @remark A node may not be a sibbling with itself, sharing the same direct parent. If you want to
 * have the same node appear multiple times, there must be a separate parent between them. For
 * example, the following is not allowed:
 * ```
 *     A
 *    / \
 *   B   B
 * ```
 * However, the following is allowed:
 * ```
 *     A
 *    / \
 *   C   D
 *   |   |
 *   B   B
 * ```
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see SceneNode.h
 */
typedef struct dsSceneNode dsSceneNode;

/**
 * @brief Struct for a node in the scene tree, which reflects the scene graph.
 */
typedef struct dsSceneTreeNode dsSceneTreeNode;

/**
 * @brief Function for destroying a scene ndoe.
 * @param node The node to destroy.
 */
typedef void (*dsDestroySceneNodeFunction)(dsSceneNode* node);

/// @cond
typedef struct dsSceneResources dsSceneResources;
typedef void (*dsDestroySceneUserDataFunction)(void* userData);
/// @endcond

/** @copydoc dsSceneNode */
struct dsSceneNode
{
	/**
	 * @brief The allocator for the node.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of the node.
	 */
	const dsSceneNodeType* type;

	/**
	 * @brief The children of the node.
	 */
	dsSceneNode** children;

	/**
	 * @brief The item lists that will use the node.
	 */
	const char** itemLists;

	/**
	 * @brief The tree nodes that correspond to this node in various scenes.
	 *
	 * This is for internal management of the scene graph.
	 */
	dsSceneTreeNode** treeNodes;

	/**
	 * @brief The number of children.
	 */
	uint32_t childCount;

	/**
	 * @brief The maximum number of children.
	 */
	uint32_t maxChildren;

	/**
	 * @brief The number of item lists.
	 */
	uint32_t itemListCount;

	/**
	 * @brief The number of tree nodes.
	 */
	uint32_t treeNodeCount;

	/**
	 * @brief The maximum number of tree nodes.
	 */
	uint32_t maxTreeNodes;

	/**
	 * @brief The reference count for the node.
	 *
	 * This will start at 1 on creation.
	 */
	uint32_t refCount;

	/**
	 * @brief Custom user data to store with the node.
	 */
	void* userData;

	/**
	 * @brief Function called on destruction to destroy the user data.
	 */
	dsDestroySceneUserDataFunction destroyUserDataFunc;

	/**
	 * @brief Destroy function.
	 */
	dsDestroySceneNodeFunction destroyFunc;
};

/**
 * @brief Scene node implementation that contains a transform for any subnodes.
 * @remark None of the members should be modified outside of the implementation.
 * @see SceneTransformNode
 */
typedef struct dsSceneTransformNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The transform for the node.
	 *
	 * This is the local transform for this node relative to any parent nodes.
	 *
	 * This should not be assigned directly since it won't udpate the transforms for any children.
	 * Instead, dsSceneTransformNode_setTransform() should be called. The children will then have
	 * their transforms updated in the call to dsScene_update().
	 */
	dsMatrix44f transform;
} dsSceneTransformNode;

/**
 * @brief Info for what to draw inside a model node when initializing.
 * @see SceneDrawNode.h
 */
typedef struct dsSceneModelInitInfo
{
	/**
	 * @brief The name of the model info.
	 *
	 * This is optional, and can be used for material remapping when set. The string will be copied
	 * when set.
	 */
	const char* name;

	/**
	 * @brief The shader to draw the model with.
	 */
	dsShader* shader;

	/**
	 * @brief The material to draw the model with.
	 */
	dsMaterial* material;

	/**
	 * @brief Geometry instance to draw.
	 */
	dsDrawGeometry* geometry;

	/**
	 * @brief The distance range to draw the model.
	 *
	 * Lower range is inclusive, upperrange is exclusive. If the x value is larger than the y value,
	 * then the model will always be drawn.
	 */
	dsVector2f distanceRange;

	union
	{
		/**
		 * @brief The draw range.
		 *
		 * This will be used if geometry doesn't have an index buffer.
		 */
		dsDrawRange drawRange;

		/**
		 * @brief The indexed draw range.
		 *
		 * This will be used if geometry has an index buffer.
		 */
		dsDrawIndexedRange drawIndexedRange;
	};

	/**
	 * @brief The primitive type for the draw.
	 */
	dsPrimitiveType primitiveType;

	/**
	 * @brief The name for the list to use the model with.
	 */
	const char* listName;
} dsSceneModelInitInfo;

/**
 * @brief Info for what to draw inside a model node.
 * @see SceneDrawNode.h
 */
typedef struct dsSceneModelInfo
{
	/**
	 * @brief The name of the model info.
	 *
	 * This is optional, and can be used for material remapping when set.
	 */
	const char* name;

	/**
	 * @brief The shader to draw the model with.
	 */
	dsShader* shader;

	/**
	 * @brief The material to draw the model with.
	 */
	dsMaterial* material;

	/**
	 * @brief Geometry instance to draw.
	 */
	dsDrawGeometry* geometry;

	/**
	 * @brief The distance range to draw the model.
	 *
	 * Lower range is inclusive, upperrange is exclusive. If the x value is larger than the y value,
	 * then the model will always be drawn.
	 */
	dsVector2f distanceRange;

	union
	{
		/**
		 * @brief The draw range.
		 *
		 * This will be used if geometry doesn't have an index buffer.
		 */
		dsDrawRange drawRange;

		/**
		 * @brief The indexed draw range.
		 *
		 * This will be used if geometry has an index buffer.
		 */
		dsDrawIndexedRange drawIndexedRange;
	};

	/**
	 * @brief The primitive type for the draw.
	 */
	dsPrimitiveType primitiveType;

	/**
	 * @brief The name ID for the list to use the model with.
	 */
	uint32_t listNameID;
} dsSceneModelInfo;

/**
 * @brief Scene node implementation that contains model geometry to draw.
 * @remark None of the members should be modified outside of the implementation.
 * @see SceneModelNode.h
 */
typedef struct dsSceneModelNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The models that will be drawn within the ndoe.
	 */
	dsSceneModelInfo* models;

	/**
	 * @brief The resources to keep a reference to.
	 *
	 * This will ensure that any resources used within models are kept alive.
	 */
	dsSceneResources** resources;

	/**
	 * @brief The number of models.
	 */
	uint32_t modelCount;

	/**
	 * @brief The number of resources.
	 */
	uint32_t resourceCount;

	/**
	 * @brief The bounding box for the model.
	 */
	dsOrientedBox3f bounds;
} dsSceneModelNode;

/**
 * @brief Struct defining remapping for a material.
 * @see ModelNode.h
 */
typedef struct dsSceneMaterialRemap
{
	/**
	 * @brief The name of the model to replace the material.
	 */
	const char* name;

	/**
	 * @brief The new shader to use, or NULL to leave the same.
	 */
	dsShader* shader;

	/**
	 * @brief The new material to use, or NULL to leave the same.
	 */
	dsMaterial* material;
} dsSceneMaterialRemap;

/**
 * @brief Struct holding data for an item in a scene item list.
 */
typedef struct dsSceneItemData
{
	/**
	 * @brief The name ID for the corresponding scene item list.
	 */
	uint32_t nameID;

	/**
	 * @brief The data associated with the item.
	 */
	void* data;
} dsSceneItemData;

/**
 * @brief Struct holding all of the item list data for a scene node.
 *
 * A separate instance is maintained for each time the node is present in the scene graph.
 *
 * @see SceneNodeItemData.h
 */
typedef struct dsSceneNodeItemData
{
	/**
	 * @brief The data associated with each item list the node is used with.
	 */
	dsSceneItemData* itemData;

	/**
	 * @brief The number of item list data instances.
	 */
	uint32_t count;
} dsSceneNodeItemData;

#ifdef __cplusplus
}
#endif
