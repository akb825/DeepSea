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

#include <errno.h>

#define EXPECT_FALSE_ERRNO(err, actual) \
	do \
	{ \
		errno = 0; \
		EXPECT_FALSE(actual); \
		EXPECT_EQ(err, errno); \
	} while (false)

#define EXPECT_NULL_ERRNO(err, actual) \
	do \
	{ \
		errno = 0; \
		EXPECT_EQ(nullptr, actual); \
		EXPECT_EQ(err, errno); \
	} while (false)

#define EXPECT_EQ_ERRNO(err, expected, actual) \
	do \
	{ \
		errno = 0; \
		EXPECT_EQ(expected, actual); \
		EXPECT_EQ(err, errno); \
	} while (false)
