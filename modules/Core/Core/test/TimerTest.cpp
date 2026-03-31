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

#include <DeepSea/Core/Timer.h>
#include <gtest/gtest.h>

TEST(TimerTest, TicksToSeconds)
{
	dsTimer timer = {1e-3};
	uint64_t ticks = 10000;
	EXPECT_EQ(timer.scale*(double)ticks, dsTimer_ticksToSeconds(timer, ticks));
}

TEST(TimerTest, ConvertTicks)
{
	dsTimer origTimer = {1e-4};
	dsTimer newTimer = {1e-3};

	uint64_t origTicks = 10000;
	uint64_t newTicks = 1000;
	EXPECT_EQ(dsTimer_ticksToSeconds(origTimer, origTicks),
		dsTimer_ticksToSeconds(newTimer, newTicks));
	EXPECT_EQ(newTicks, dsTimer_convertTicks(newTimer, origTimer.scale, origTicks));
}
