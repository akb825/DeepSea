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
 *
 * @see dsList
 */

/**
 * @brief Initializes a list.
 * @remark errno will be set on failure.
 * @param list The list to initialize.
 * @return False if list is NULL.
 */
DS_CORE_EXPORT bool dsList_initialize(dsList* list);

/**
 * @brief Appends a node to the beginning of the list.
 * @remark errno will be set on failure.
 * @param list The list to append the node to.
 * @param node The node to append.
 * @return False if either list or node is NULL or if the node previous or next pointer is set.
 */
DS_CORE_EXPORT bool dsList_prepend(dsList* list, dsListNode* node);

/**
 * @brief Appends a node to the end of the list.
 * @remark errno will be set on failure.
 * @param list The list to append the node to.
 * @param node The node to append.
 * @return False if either list or node is NULL.
 */
DS_CORE_EXPORT bool dsList_append(dsList* list, dsListNode* node);

/**
 * @brief Inserts a node into the list.
 * @remark errno will be set on failure.
 * @param list The node to add.
 * @param previous The node to insert after. If NULL, node will be inserted at the beginning of
 *     the list.
 * @param node The node to insert.
 * @return False if either list or node is NULL.
 */
DS_CORE_EXPORT bool dsList_insert(dsList* list, dsListNode* previous, dsListNode* node);

/**
 * @brief Removes a node from the list.
 * @remark errno will be set on failure.
 * @param list The list to remove the node from.
 * @param node The node to remove.
 * @return False if either list or node is NULL.
 */
DS_CORE_EXPORT bool dsList_remove(dsList* list, dsListNode* node);

/**
 * @brief Clears the contents of the list.
 * @remark errno will be set on failure.
 * @param list The list to clear.
 * @return False if list is NULL.
 */
DS_CORE_EXPORT bool dsList_clear(dsList* list);

#ifdef __cplusplus
}
#endif
