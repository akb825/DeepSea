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

#include "Helpers.h"
#include <DeepSea/Core/Containers/StringPool.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <gtest/gtest.h>

TEST(StringPoolTest, InsertStrings)
{
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsStringPool stringPool;
	EXPECT_FALSE_ERRNO(EINVAL, dsStringPool_initialize(NULL));
	ASSERT_TRUE(dsStringPool_initialize(&stringPool));

	EXPECT_FALSE_ERRNO(EINVAL, dsStringPool_reserve(NULL, "foo"));
	EXPECT_FALSE_ERRNO(EINVAL, dsStringPool_reserve(&stringPool, NULL));
	ASSERT_TRUE(dsStringPool_reserve(&stringPool, "foo"));
	ASSERT_TRUE(dsStringPool_reserve(&stringPool, "foobar"));
	ASSERT_TRUE(dsStringPool_reserve(&stringPool, "foobarbaz"));

	EXPECT_FALSE_ERRNO(EPERM, dsStringPool_insert(&stringPool, "foo"));

	EXPECT_FALSE_ERRNO(EINVAL, dsStringPool_allocate(NULL, (dsAllocator*)&allocator));
	EXPECT_FALSE_ERRNO(EINVAL, dsStringPool_allocate(&stringPool, NULL));
	EXPECT_TRUE(dsStringPool_allocate(&stringPool, (dsAllocator*)&allocator));
	EXPECT_FALSE_ERRNO(EPERM, dsStringPool_allocate(&stringPool, (dsAllocator*)&allocator));
	EXPECT_FALSE_ERRNO(EPERM, dsStringPool_reserve(&stringPool, "foobarbaz"));

	EXPECT_FALSE_ERRNO(EINVAL, dsStringPool_insert(NULL, "foo"));
	EXPECT_FALSE_ERRNO(EINVAL, dsStringPool_insert(&stringPool, NULL));
	EXPECT_STREQ("foo", dsStringPool_insert(&stringPool, "foo"));
	EXPECT_STREQ("foobar", dsStringPool_insert(&stringPool, "foobar"));
	EXPECT_STREQ("foobarbaz", dsStringPool_insert(&stringPool, "foobarbaz"));
	EXPECT_FALSE_ERRNO(ENOMEM, dsStringPool_insert(&stringPool, "foo"));

	dsStringPool_shutdown(&stringPool);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}
