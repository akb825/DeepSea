/*
 * Copyright 2016-2023 Aaron Barany
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
#include <DeepSea/Core/Error.h>

static void insertImpl(dsList* list, dsListNode* previous, dsListNode* node)
{
	// Asserts indicate a corrupt list. Don't return false in these cases since it indicates
	// something is seriously wrong, and would probably crash somewhere regardless.
	if (!previous)
	{
		DS_ASSERT(!list->head || !list->head->previous);
		node->previous = NULL;
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
		previous->next = node;
		if (node->next)
		{
			DS_ASSERT(list->tail != previous);
			DS_ASSERT(node->next->previous == previous);
			node->next->previous = node;
		}
		else
		{
			DS_ASSERT(list->tail == previous);
			list->tail = node;
		}
	}

	++list->length;
}

bool dsList_initialize(dsList* list)
{
	if (!list)
	{
		errno = EINVAL;
		return false;
	}

	list->length = 0;
	list->head = NULL;
	list->tail = NULL;
	return true;
}

bool dsList_prepend(dsList* list, dsListNode* node)
{
	if (!list || !node)
	{
		errno = EINVAL;
		return false;
	}

	insertImpl(list, NULL, node);
	return true;
}

bool dsList_append(dsList* list, dsListNode* node)
{
	if (!list || !node)
	{
		errno = EINVAL;
		return false;
	}

	insertImpl(list, list->tail, node);
	return true;
}

bool dsList_insert(dsList* list, dsListNode* previous, dsListNode* node)
{
	if (!list || !node)
	{
		errno = EINVAL;
		return false;
	}

	insertImpl(list, previous, node);
	return true;
}

bool dsList_remove(dsList* list, dsListNode* node)
{
	if (!list || !node)
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

bool dsList_clear(dsList* list)
{
	if (!list)
	{
		errno = EINVAL;
		return false;
	}

	list->head = NULL;
	list->tail = NULL;
	list->length = 0;
	return true;
}
