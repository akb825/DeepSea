/*
 * Copyright 2019-2025 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/SceneItemList.h>

#include <DeepSea/Core/Containers/Hash.h>

#include <string.h>

uint32_t dsSceneItemList_hash(const dsSceneItemList* itemList)
{
	if (!itemList || !itemList->type)
		return 0;

	uint32_t hash = dsHashPointer(itemList->type);
	uint32_t hashValues[3] = {itemList->nameID, itemList->globalValueCount,
		(itemList->needsCommandBuffer << 8) | itemList->skipPreRenderPass};
	hash = dsHashCombineBytes(hash, hashValues, sizeof(hashValues));

	dsHashSceneItemListFunction hashFunc = itemList->type->hashFunc;
	if (hashFunc)
		hash = hashFunc(itemList, hash);
	return hash;
}

bool dsSceneItemList_equal(const dsSceneItemList* left, const dsSceneItemList* right)
{
	if (left == right)
		return true;
	else if (!left || !right || !left->type || left->type != right->type ||
		left->nameID != right->nameID || left->globalValueCount != right->globalValueCount ||
		left->needsCommandBuffer != right->needsCommandBuffer ||
		left->skipPreRenderPass != right->skipPreRenderPass)
	{
		return false;
	}

	dsSceneItemListsEqualFunction equalFunc = left->type->equalFunc;
	if (equalFunc)
		return equalFunc(left, right);
	return true;
}

void dsSceneItemList_destroy(dsSceneItemList* list)
{
	if (!list || !list->type || !list->type->destroyFunc)
		return;

	list->type->destroyFunc(list);
}
