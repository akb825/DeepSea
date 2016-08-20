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

#include <DeepSea/Core/Containers/List.h>
#include <DeepSea/Core/Assert.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

static bool isNodeInList(const dsListNode* node)
{
	return node->previous || node->next;
}

bool dsList_initialize(dsList* list)
{
	if (!list)
		return false;

	list->length = 0;
	list->head = NULL;
	list->tail = NULL;
	return true;
}

bool dsList_prependNode(dsList* list, dsListNode* node)
{
	return dsList_insert(list, NULL, node);
}

bool dsList_appendNode(dsList* list, dsListNode* node)
{
	if (!list)
	{
		errno = EINVAL;
		return false;
	}

	return dsList_insert(list, list->tail, node);
}

bool dsList_insert(dsList* list, dsListNode* previous, dsListNode* node)
{
	if (!list || !node || isNodeInList(node))
	{
		errno = EINVAL;
		return false;
	}

	// Asserts indicate a corrupt list. Don't return false in these cases since it indicates
	// something is seriously wrong, and would probably crash somewhere regardless.
	if (!previous)
	{
		DS_ASSERT(!list->head->previous);
		node->next = list->head;
		if (node->next)
		{
			DS_ASSERT(list->tail);
			list->head->previous = node;
		}
		else
		{
			DS_ASSERT(!list->tail);
			list->tail = node;
		}
		list->head = node;
	}
	else
	{
		node->previous = previous;
		node->next = previous->next;
		if (node->next)
		{
			DS_ASSERT(list->tail != previous);
			DS_ASSERT(node->next->previous = previous);
			node->next->previous = node;
		}
		else
		{
			DS_ASSERT(list->tail == previous);
			list->tail = node;
		}
	}

	++list->length;
	return true;
}

bool dsList_remove(dsList* list, dsListNode* node)
{
	if (!list || !node || !isNodeInList(node))
	{
		errno = EINVAL;
		return false;
	}

	if (list->head == node)
	{
		DS_ASSERT(!node->previous);
		list->head = node->next;
	}

	if (list->tail == node)
	{
		DS_ASSERT(!node->next);
		list->tail = node->previous;
	}

	if (node->previous)
		node->previous->next = node->next;

	if (node->next)
		node->next->previous = node->previous;

	node->previous = NULL;
	node->next = NULL;

	DS_ASSERT(list->length > 0);
	--list->length;
	return true;
}

bool dsList_clear(dsList* list, bool resetNodePointers)
{
	if (!list)
	{
		errno = EINVAL;
		return false;
	}

	if (resetNodePointers)
	{
		for (dsListNode* node = list->head; node;)
		{
			dsListNode* next = node->next;
			node->previous = NULL;
			node->next = NULL;
			node = next;
		}
	}

	list->head = NULL;
	list->tail = NULL;
	list->length = 0;
	return true;
}

#ifdef __cplusplus
}
#endif
