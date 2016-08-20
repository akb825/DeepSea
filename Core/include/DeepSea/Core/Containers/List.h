/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Containers/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for managing a linked list.
 *
 * None of these functions allocate or deallocate memory, so it is up to the caller to allocate or
 * free if necessary.
 */

/**
 * @brief Initializes a list node.
 *
 * This clears out the previous and next pointers, which is required to add it to a list.
 *
 * @param node The node to initialize.
 */
inline bool dsListNode_initialize(dsListNode* node);

/**
 * @brief Initializes a list.
 * @param list The list to initialize.
 * @return False if list is NULL.
 */
DS_CORE_EXPORT bool dsList_initialize(dsList* list);

/**
 * @brief Appends a node to the beginning of the list.
 * @param list The list to append the node to.
 * @param node The node to append.
 * @return False if either parameter is NULL or if the node previous or next pointer is set.
 */
DS_CORE_EXPORT bool dsList_prependNode(dsList* list, dsListNode* node);

/**
 * @brief Appends a node to the end of the list.
 * @param list The list to append the node to.
 * @param node The node to append.
 * @return False if either parameter is NULL or if the node previous or next pointer is set.
 */
DS_CORE_EXPORT bool dsList_appendNode(dsList* list, dsListNode* node);

/**
 * @brief Inserts a node into the list.
 * @param list The node to add.
 * @param previous The node to insert after. If NULL, node will be inserted at the beginning of
 * the list.
 * @param node The node to insert.
 * @return False if either list or node is NULL or if the node previous or next pointer is set.
 */
DS_CORE_EXPORT bool dsList_insert(dsList* list, dsListNode* previous, dsListNode* node);

/**
 * @brief Removes a node from the list.
 * @param list The list to remove the node from.
 * @param node The node to remove.
 * @return False if either list or node is NULL.
 */
DS_CORE_EXPORT bool dsList_remove(dsList* list, dsListNode* node);

/**
 * @brief Clears the contents of the list.
 * @param list The list to clear.
 * @param resetNodePointers True to set the previous and next pointers for each node to NULL. This
 * is useful if the nodes will be inserted into another list.
 * @return False if the list is NULL.
 */
DS_CORE_EXPORT bool dsList_clear(dsList* list, bool resetNodePointers);

inline bool dsListNode_initialize(dsListNode* node)
{
	if (!node)
		return false;

	node->previous = NULL;
	node->next = NULL;
	return true;
}

#ifdef __cplusplus
}
#endif
