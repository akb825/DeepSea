/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <gtest/gtest.h>

TEST(ThreadStorageTest, GetSetValue)
{
	dsThreadStorage threadStorage;
	EXPECT_TRUE(dsThreadStorage_initialize(&threadStorage));
	EXPECT_EQ(nullptr, dsThreadStorage_get(threadStorage));
	EXPECT_TRUE(dsThreadStorage_set(threadStorage, &threadStorage));
	EXPECT_EQ(&threadStorage, dsThreadStorage_get(threadStorage));
	dsThreadStorage_shutdown(&threadStorage);
}

TEST(ThreadStorageTest, Reinitialize)
{
	dsThreadStorage threadStorage;
	EXPECT_TRUE(dsThreadStorage_initialize(&threadStorage));
	EXPECT_TRUE(dsThreadStorage_set(threadStorage, &threadStorage));
	EXPECT_EQ(&threadStorage, dsThreadStorage_get(threadStorage));
	dsThreadStorage_shutdown(&threadStorage);

	EXPECT_TRUE(dsThreadStorage_initialize(&threadStorage));
	EXPECT_EQ(nullptr, dsThreadStorage_get(threadStorage));
	dsThreadStorage_shutdown(&threadStorage);
}
