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

#include "FixtureBase.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Scene.h>
#include <gtest/gtest.h>

namespace
{

dsSceneNodeType mockSceneNodeType;
const char* testItemListName = "TestItems";

void destroyMockNode(dsSceneNode* node)
{
	dsAllocator_free(node->allocator, node);
}

dsSceneNode* createMockNode(dsAllocator* allocator)
{
	dsSceneNode* node = DS_ALLOCATE_OBJECT(allocator, dsSceneNode);
	if (!node)
		return NULL;

	EXPECT_TRUE(dsSceneNode_initialize(node, allocator, &mockSceneNodeType, &testItemListName, 1,
		&destroyMockNode));
	return node;
}

struct ItemInfo
{
	const dsSceneNode* node;
	const dsMatrix44f* transform;
	uint32_t updateCount;
	uint64_t nodeID;
};

struct MockSceneItemList
{
	dsSceneItemList sceneItemList;
	ItemInfo* items;
	uint32_t itemCount;
	uint32_t maxItems;
	uint64_t nextNodeID;
};

uint64_t addMockSceneItem(dsSceneItemList* itemList, const dsSceneNode* node,
	const dsSceneTreeNode* treeNode, const dsSceneNodeItemData*, void**)
{
	if (!dsSceneNode_isOfType(node, &mockSceneNodeType))
		return DS_NO_SCENE_NODE;

	MockSceneItemList* mockList = (MockSceneItemList*)itemList;
	uint32_t index = mockList->itemCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, mockList->items, mockList->itemCount,
			mockList->maxItems, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	mockList->items[index].node = node;
	mockList->items[index].transform = &treeNode->transform;
	mockList->items[index].updateCount = 0;
	mockList->items[index].nodeID = mockList->nextNodeID;
	return mockList->nextNodeID++;
}

void removeMockSceneItem(dsSceneItemList* itemList, uint64_t nodeID)
{
	MockSceneItemList* mockList = (MockSceneItemList*)itemList;
	for (uint32_t i = 0; i < mockList->itemCount; ++i)
	{
		if (mockList->items[i].nodeID == nodeID)
		{
			mockList->items[i] = mockList->items[mockList->itemCount - 1];
			--mockList->itemCount;
			break;
		}
	}
}

void updateMockSceneItem(dsSceneItemList* itemList, uint64_t nodeID)
{
	MockSceneItemList* mockList = (MockSceneItemList*)itemList;
	for (uint32_t i = 0; i < mockList->itemCount; ++i)
	{
		if (mockList->items[i].nodeID == nodeID)
		{
			++mockList->items[i].updateCount;
			break;
		}
	}
}

void commitMockSceneItems(dsSceneItemList*, const dsView*, dsCommandBuffer*)
{
}

void destroyMockSceneItems(dsSceneItemList* itemList)
{
	MockSceneItemList* mockList = (MockSceneItemList*)itemList;
	EXPECT_TRUE(dsAllocator_free(itemList->allocator, mockList->items));
	EXPECT_TRUE(dsAllocator_free(itemList->allocator, itemList));
}

MockSceneItemList* createMockSceneItems(dsAllocator* allocator)
{
	MockSceneItemList* mockItems = DS_ALLOCATE_OBJECT(allocator, MockSceneItemList);
	if (!mockItems)
		return NULL;

	dsSceneItemList* baseItems = (dsSceneItemList*)mockItems;
	baseItems->allocator = dsAllocator_keepPointer(allocator);
	baseItems->type = 0;
	baseItems->name = testItemListName;
	baseItems->nameID = dsHashString(testItemListName);
	baseItems->globalValueCount = 0;
	baseItems->needsCommandBuffer = false;
	baseItems->addNodeFunc = &addMockSceneItem;
	baseItems->removeNodeFunc = &removeMockSceneItem;
	baseItems->updateNodeFunc = &updateMockSceneItem;
	baseItems->updateFunc = NULL;
	baseItems->commitFunc = &commitMockSceneItems;
	baseItems->destroyFunc = &destroyMockSceneItems;

	mockItems->items = NULL;
	mockItems->itemCount = 0;
	mockItems->maxItems = 0;
	mockItems->nextNodeID = 0;
	return mockItems;
}

bool matricesEqual(const dsMatrix44f* left, const dsMatrix44f* right)
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		for (uint32_t j = 0; j < 4; ++j)
		{
			if (!dsEpsilonEqualf(left->values[i][j], right->values[i][j], 1.0e-4f))
				return false;
		}
	}

	return true;
}

} // namespace

class SceneItemListTest : public FixtureBase
{
public:
	void SetUp() override
	{
		FixtureBase::SetUp();
		mockSceneItems = createMockSceneItems((dsAllocator*)&allocator);
		ASSERT_TRUE(mockSceneItems);
		dsScenePipelineItem pipelineItem = {NULL, (dsSceneItemList*)mockSceneItems};
		scene = dsScene_create((dsAllocator*)&allocator, renderer, NULL, 0, &pipelineItem, 1,
			NULL, NULL);
		ASSERT_TRUE(scene);
	}

	void TearDown() override
	{
		dsScene_destroy(scene);
		FixtureBase::TearDown();
	}

	MockSceneItemList* mockSceneItems;
	dsScene* scene;
};

TEST_F(SceneItemListTest, NodeHierarchy)
{
	dsSceneNode* mockNode1 = createMockNode((dsAllocator*)&allocator);
	ASSERT_TRUE(mockNode1);
	dsSceneNode* mockNode2 = createMockNode((dsAllocator*)&allocator);
	ASSERT_TRUE(mockNode2);

	dsMatrix44f matrix1, matrix2;
	dsMatrix44f_makeRotate(&matrix1, (float)M_PI_2, (float)-M_PI_4, (float)M_PI);
	dsMatrix44f_makeTranslate(&matrix2, 3.2f, -5.3f, 1.3f);
	dsSceneTransformNode* transform1 =
		dsSceneTransformNode_create((dsAllocator*)&allocator, &matrix1);
	ASSERT_TRUE(transform1);
	dsSceneTransformNode* transform2 =
		dsSceneTransformNode_create((dsAllocator*)&allocator, &matrix2);
	ASSERT_TRUE(transform2);

	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform1, mockNode1));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform1, (dsSceneNode*)transform2));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform2, mockNode2));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform2, mockNode1));

	ASSERT_TRUE(dsScene_addNode(scene, (dsSceneNode*)transform1));

	ASSERT_EQ(3U, mockSceneItems->itemCount);
	EXPECT_EQ(mockNode1, mockSceneItems->items[0].node);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[0].transform));

	dsMatrix44f expectedTransform;
	dsMatrix44_affineMul(expectedTransform, matrix1, matrix2);
	EXPECT_EQ(mockNode2, mockSceneItems->items[1].node);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[1].transform));
	EXPECT_EQ(mockNode1, mockSceneItems->items[2].node);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[2].transform));

	EXPECT_TRUE(dsSceneNode_removeChildNode((dsSceneNode*)transform1, (dsSceneNode*)transform2));
	ASSERT_EQ(1U, mockSceneItems->itemCount);
	EXPECT_EQ(mockNode1, mockSceneItems->items[0].node);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[0].transform));

	EXPECT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform1, (dsSceneNode*)transform2));
	ASSERT_EQ(3U, mockSceneItems->itemCount);
	EXPECT_EQ(mockNode1, mockSceneItems->items[0].node);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[0].transform));
	EXPECT_EQ(mockNode2, mockSceneItems->items[1].node);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[1].transform));
	EXPECT_EQ(mockNode1, mockSceneItems->items[2].node);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[2].transform));

	EXPECT_TRUE(dsSceneNode_reparentChildNode((dsSceneNode*)transform2, mockNode2,
		(dsSceneNode*)transform1));
	EXPECT_TRUE(dsScene_update(scene, 0));
	ASSERT_EQ(3U, mockSceneItems->itemCount);
	EXPECT_EQ(mockNode1, mockSceneItems->items[0].node);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[0].transform));
	EXPECT_EQ(mockNode2, mockSceneItems->items[1].node);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[1].transform));
	EXPECT_EQ(mockNode1, mockSceneItems->items[2].node);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[2].transform));

	dsSceneNode_freeRef(mockNode1);
	dsSceneNode_freeRef(mockNode2);
	dsSceneNode_freeRef((dsSceneNode*)transform1);
	dsSceneNode_freeRef((dsSceneNode*)transform2);
}

TEST_F(SceneItemListTest, UpdateTransforms)
{
	dsSceneNode* mockNode1 = createMockNode((dsAllocator*)&allocator);
	ASSERT_TRUE(mockNode1);
	dsSceneNode* mockNode2 = createMockNode((dsAllocator*)&allocator);
	ASSERT_TRUE(mockNode2);

	dsMatrix44f matrix1, matrix2;
	dsMatrix44f_makeRotate(&matrix1, (float)M_PI_2, (float)-M_PI_4, (float)M_PI);
	dsMatrix44f_makeTranslate(&matrix2, 3.2f, -5.3f, 1.3f);
	dsSceneTransformNode* transform1 =
		dsSceneTransformNode_create((dsAllocator*)&allocator, &matrix1);
	ASSERT_TRUE(transform1);
	dsSceneTransformNode* transform2 =
		dsSceneTransformNode_create((dsAllocator*)&allocator, &matrix2);
	ASSERT_TRUE(transform2);

	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform1, mockNode1));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform1, (dsSceneNode*)transform2));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform2, mockNode2));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform2, mockNode1));

	ASSERT_TRUE(dsScene_addNode(scene, (dsSceneNode*)transform1));

	ASSERT_EQ(3U, mockSceneItems->itemCount);
	EXPECT_EQ(mockNode1, mockSceneItems->items[0].node);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[0].transform));

	dsMatrix44f expectedTransform;
	dsMatrix44_affineMul(expectedTransform, matrix1, matrix2);
	EXPECT_EQ(mockNode2, mockSceneItems->items[1].node);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[1].transform));
	EXPECT_EQ(mockNode1, mockSceneItems->items[2].node);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[2].transform));

	dsMatrix44f_makeTranslate(&matrix2, 7.2f, 2.6f, -5.3f);
	EXPECT_TRUE(dsSceneTransformNode_setTransform(transform2, &matrix2));
	EXPECT_TRUE(dsScene_update(scene, 0.0f));

	ASSERT_EQ(3U, mockSceneItems->itemCount);
	EXPECT_EQ(mockNode1, mockSceneItems->items[0].node);
	EXPECT_EQ(0U, mockSceneItems->items[0].updateCount);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[0].transform));

	dsMatrix44_affineMul(expectedTransform, matrix1, matrix2);
	EXPECT_EQ(mockNode2, mockSceneItems->items[1].node);
	EXPECT_EQ(1U, mockSceneItems->items[1].updateCount);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[1].transform));
	EXPECT_EQ(mockNode1, mockSceneItems->items[2].node);
	EXPECT_EQ(1U, mockSceneItems->items[2].updateCount);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[2].transform));

	dsMatrix44f_makeRotate(&matrix1, (float)M_PI_4, (float)M_PI, (float)-M_PI_2);
	EXPECT_TRUE(dsSceneTransformNode_setTransform(transform1, &matrix1));
	EXPECT_TRUE(dsScene_update(scene, 0.0f));

	ASSERT_EQ(3U, mockSceneItems->itemCount);
	EXPECT_EQ(mockNode1, mockSceneItems->items[0].node);
	EXPECT_EQ(1U, mockSceneItems->items[0].updateCount);
	EXPECT_TRUE(matricesEqual(&matrix1, mockSceneItems->items[0].transform));

	dsMatrix44_affineMul(expectedTransform, matrix1, matrix2);
	EXPECT_EQ(mockNode2, mockSceneItems->items[1].node);
	EXPECT_EQ(2U, mockSceneItems->items[1].updateCount);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[1].transform));
	EXPECT_EQ(mockNode1, mockSceneItems->items[2].node);
	EXPECT_EQ(2U, mockSceneItems->items[2].updateCount);
	EXPECT_TRUE(matricesEqual(&expectedTransform, mockSceneItems->items[2].transform));

	EXPECT_TRUE(dsSceneTransformNode_setTransform(transform1, &matrix2));
	dsScene_clearNodes(scene);
	EXPECT_TRUE(dsScene_update(scene, 0.0f));
	ASSERT_EQ(0U, mockSceneItems->itemCount);

	dsSceneNode_freeRef(mockNode1);
	dsSceneNode_freeRef(mockNode2);
	dsSceneNode_freeRef((dsSceneNode*)transform1);
	dsSceneNode_freeRef((dsSceneNode*)transform2);
}
