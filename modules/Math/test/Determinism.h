/*
 * Copyright 2026 Aaron Barany
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

#include <gtest/gtest.h>

#if DS_DETERMINISTIC_MATH
#define EXPECT_EQ_DETERMINISTIC(val1, val2, epsilon) \
	do \
	{ \
		(void)epsilon; \
		EXPECT_EQ(val1, val2); \
	} while (0)
#else
#define EXPECT_EQ_DETERMINISTIC(val1, val2, epsilon) EXPECT_NEAR(val1, val2, epsilon)
#endif
