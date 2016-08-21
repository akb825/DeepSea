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

#include "Helpers.h"
#include <DeepSea/Core/Containers/List.h>
#include <gtest/gtest.h>

struct TestNode
{
	dsListNode node;
	unsigned int value;
};

TEST(ListTest, Initialize)
{
	dsList list;
	EXPECT_FALSE_ERRNO(EINVAL, dsList_initialize(NULL));
	EXPECT_TRUE(dsList_initialize(&list));
}

TEST(ListTest, PrependNode)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_prepend(NULL, (dsListNode*)&node1));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_prepend(&list, NULL));

	EXPECT_TRUE(dsList_prepend(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_prepend(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_prepend(&list, (dsListNode*)&node3));

	EXPECT_EQ(3U, list.length);
	EXPECT_EQ((dsListNode*)&node3, list.head);
	EXPECT_EQ((dsListNode*)&node1, list.tail);

	EXPECT_EQ((dsListNode*)&node2, node1.node.previous);
	EXPECT_EQ(nullptr, node1.node.next);

	EXPECT_EQ((dsListNode*)&node3, node2.node.previous);
	EXPECT_EQ((dsListNode*)&node1, node2.node.next);

	EXPECT_EQ(nullptr, node3.node.previous);
	EXPECT_EQ((dsListNode*)&node2, node3.node.next);
}

TEST(ListTest, AppendNode)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_append(NULL, (dsListNode*)&node1));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_append(&list, NULL));

	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node3));

	EXPECT_EQ(3U, list.length);
	EXPECT_EQ((dsListNode*)&node1, list.head);
	EXPECT_EQ((dsListNode*)&node3, list.tail);

	EXPECT_EQ(nullptr, node1.node.previous);
	EXPECT_EQ((dsListNode*)&node2, node1.node.next);

	EXPECT_EQ((dsListNode*)&node1, node2.node.previous);
	EXPECT_EQ((dsListNode*)&node3, node2.node.next);

	EXPECT_EQ((dsListNode*)&node2, node3.node.previous);
	EXPECT_EQ(nullptr, node3.node.next);
}

TEST(ListTest, InsertNode)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));

	EXPECT_TRUE(dsList_insert(&list, NULL, (dsListNode*)&node2));

	EXPECT_FALSE_ERRNO(EINVAL, dsList_insert(NULL, (dsListNode*)&node2, (dsListNode*)&node1));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_insert(&list,(dsListNode*)&node2,  NULL));

	EXPECT_TRUE(dsList_insert(&list, (dsListNode*)&node2, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_insert(&list, (dsListNode*)&node2, (dsListNode*)&node3));

	EXPECT_EQ(3U, list.length);
	EXPECT_EQ((dsListNode*)&node2, list.head);
	EXPECT_EQ((dsListNode*)&node1, list.tail);

	EXPECT_EQ(nullptr, node2.node.previous);
	EXPECT_EQ((dsListNode*)&node3, node2.node.next);

	EXPECT_EQ((dsListNode*)&node2, node3.node.previous);
	EXPECT_EQ((dsListNode*)&node1, node3.node.next);

	EXPECT_EQ((dsListNode*)&node3, node1.node.previous);
	EXPECT_EQ(nullptr, node1.node.next);
}

TEST(ListTest, RemoveHead)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));

	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node3));

	EXPECT_FALSE_ERRNO(EINVAL, dsList_remove(&list, nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_remove(nullptr, (dsListNode*)&node1));

	EXPECT_TRUE(dsList_remove(&list, (dsListNode*)&node1));

	EXPECT_EQ(2U, list.length);
	EXPECT_EQ((dsListNode*)&node2, list.head);
	EXPECT_EQ((dsListNode*)&node3, list.tail);

	EXPECT_EQ(nullptr, node2.node.previous);
	EXPECT_EQ((dsListNode*)&node3, node2.node.next);

	EXPECT_EQ((dsListNode*)&node2, node3.node.previous);
	EXPECT_EQ(nullptr, node3.node.next);
}

TEST(ListTest, RemoveTail)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));

	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node3));

	EXPECT_FALSE_ERRNO(EINVAL, dsList_remove(&list, nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_remove(nullptr, (dsListNode*)&node3));

	EXPECT_TRUE(dsList_remove(&list, (dsListNode*)&node3));

	EXPECT_EQ(2U, list.length);
	EXPECT_EQ((dsListNode*)&node1, list.head);
	EXPECT_EQ((dsListNode*)&node2, list.tail);

	EXPECT_EQ(nullptr, node1.node.previous);
	EXPECT_EQ((dsListNode*)&node2, node1.node.next);

	EXPECT_EQ((dsListNode*)&node1, node2.node.previous);
	EXPECT_EQ(nullptr, node2.node.next);
}

TEST(ListTest, RemoveMiddle)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));

	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node3));

	EXPECT_FALSE_ERRNO(EINVAL, dsList_remove(&list, nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsList_remove(nullptr, (dsListNode*)&node3));

	EXPECT_TRUE(dsList_remove(&list, (dsListNode*)&node2));

	EXPECT_EQ(2U, list.length);
	EXPECT_EQ((dsListNode*)&node1, list.head);
	EXPECT_EQ((dsListNode*)&node3, list.tail);

	EXPECT_EQ(nullptr, node1.node.previous);
	EXPECT_EQ((dsListNode*)&node3, node1.node.next);

	EXPECT_EQ((dsListNode*)&node1, node3.node.previous);
	EXPECT_EQ(nullptr, node3.node.next);
}

TEST(ListTest, RemoveAll)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));

	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node3));

	EXPECT_TRUE(dsList_remove(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_remove(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_remove(&list, (dsListNode*)&node3));

	EXPECT_EQ(0U, list.length);
	EXPECT_EQ(nullptr, list.head);
	EXPECT_EQ(nullptr, list.tail);
}

TEST(ListTest, Clear)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	dsList list;
	EXPECT_TRUE(dsList_initialize(&list));

	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node1));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node2));
	EXPECT_TRUE(dsList_append(&list, (dsListNode*)&node3));

	EXPECT_FALSE_ERRNO(EINVAL, dsList_clear(nullptr));

	EXPECT_TRUE(dsList_clear(&list));

	EXPECT_EQ(0U, list.length);
	EXPECT_EQ(nullptr, list.head);
	EXPECT_EQ(nullptr, list.tail);
}
