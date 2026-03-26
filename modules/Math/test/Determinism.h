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

#include <DeepSea/Math/Core.h>
#include <gtest/gtest.h>

#define EXPECT_RELATIVE_EQ(val1, val2, absoluteEps, relativeEps) \
	EXPECT_PRED4(dsRelativeEpsilonEquald, val1, val2, absoluteEps, relativeEps)

#if DS_DETERMINISTIC_MATH
#define EXPECT_EQ_DETERMINISTIC(val1, val2, epsilon) EXPECT_EQ(val1, val2)
#define EXPECT_RELATIVE_EQ_DETERMINISTIC(val1, val2, absoluteEps, relativeEps) EXPECT_EQ(val1, val2)
#else
#define EXPECT_EQ_DETERMINISTIC(val1, val2, epsilon) EXPECT_NEAR(val1, val2, epsilon)
#define EXPECT_RELATIVE_EQ_DETERMINISTIC(val1, val2, absoluteEps, relativeEps) \
	EXPECT_RELATIVE_EQ(val1, val2, absoluteEps, relativeEps)
#endif
