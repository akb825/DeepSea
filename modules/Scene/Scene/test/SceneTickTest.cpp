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
#include <DeepSea/Scene/SceneTick.h>
#include <gtest/gtest.h>

TEST(SceneTickTest, UpdateDynamicPeriod)
{
	dsSceneTick tick;
	EXPECT_FALSE(dsSceneTick_initialize(nullptr, 0.0f, 0.0f));
	EXPECT_FALSE(dsSceneTick_initialize(&tick, -1.0f, 0.0f));
	EXPECT_FALSE(dsSceneTick_initialize(&tick, 0.0f, -1.0f));
	ASSERT_TRUE(dsSceneTick_initialize(&tick, 0.0f, 0.0f));

	// Use a dummy timer scale to allow for fixed tick values.
	EXPECT_NE(0U, tick.timer.scale);
	tick.timer.scale = 0.001;

	EXPECT_EQ(0U, tick.absoluteTimerTicks);
	EXPECT_EQ(0U, tick.totalTimerTicks);
	EXPECT_EQ(0U, tick.totalSteps);
	EXPECT_EQ(0U, tick.thisTimerTicks);
	EXPECT_EQ(UINT64_MAX, tick.maxTimerTicks);
	EXPECT_EQ(0.0f, tick.updatePeriod);
	EXPECT_EQ(0.0f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_EQ(1.0f, tick.stepInterp);
	EXPECT_EQ(0.0f, tick.thisTime);

	EXPECT_FALSE(dsSceneTick_update(nullptr, 123, 32));
	EXPECT_TRUE(dsSceneTick_update(&tick, 123, 32));
	EXPECT_EQ(123U, tick.absoluteTimerTicks);
	EXPECT_EQ(32U, tick.totalTimerTicks);
	EXPECT_EQ(1U, tick.totalSteps);
	EXPECT_EQ(32U, tick.thisTimerTicks);
	EXPECT_EQ(0.0f, tick.updatePeriod);
	EXPECT_FLOAT_EQ(0.032f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_EQ(1.0f, tick.stepInterp);
	EXPECT_FLOAT_EQ(0.032f, tick.thisTime);

	EXPECT_TRUE(dsSceneTick_update(&tick, 125, 0));
	EXPECT_EQ(125U, tick.absoluteTimerTicks);
	EXPECT_EQ(32U, tick.totalTimerTicks);
	EXPECT_EQ(1U, tick.totalSteps);
	EXPECT_EQ(0U, tick.thisTimerTicks);
	EXPECT_EQ(0.0f, tick.updatePeriod);
	EXPECT_EQ(0.0f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_EQ(1.0f, tick.stepInterp);
	EXPECT_EQ(0.0f, tick.thisTime);

	EXPECT_TRUE(dsSceneTick_update(&tick, 154, 17));
	EXPECT_EQ(154U, tick.absoluteTimerTicks);
	EXPECT_EQ(49U, tick.totalTimerTicks);
	EXPECT_EQ(2U, tick.totalSteps);
	EXPECT_EQ(17U, tick.thisTimerTicks);
	EXPECT_EQ(0.0f, tick.updatePeriod);
	EXPECT_FLOAT_EQ(0.017f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_EQ(1.0f, tick.stepInterp);
	EXPECT_FLOAT_EQ(0.017f, tick.thisTime);
}

TEST(SceneTickTest, UpdateFixedPeriod)
{
	const float epsilon = 1e-6f;

	float updatePeriod = 1/60.0f;
	dsSceneTick tick;
	ASSERT_TRUE(dsSceneTick_initialize(&tick, updatePeriod, 5.0f));

	// Use a dummy timer scale to allow for fixed tick values.
	EXPECT_NE(0U, tick.timer.scale);
	double origScale = tick.timer.scale;
	tick.timer.scale = 0.001;
	tick.maxTimerTicks = dsTimer_convertTicks(tick.timer, origScale, tick.maxTimerTicks);

	EXPECT_EQ(0U, tick.absoluteTimerTicks);
	EXPECT_EQ(0U, tick.totalTimerTicks);
	EXPECT_EQ(0U, tick.totalSteps);
	EXPECT_EQ(0U, tick.thisTimerTicks);
	EXPECT_EQ(5000, tick.maxTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(0.0f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_EQ(1.0f, tick.stepInterp);
	EXPECT_EQ(0.0f, tick.thisTime);

	EXPECT_TRUE(dsSceneTick_update(&tick, 123, 0));
	EXPECT_EQ(123U, tick.absoluteTimerTicks);
	EXPECT_EQ(0U, tick.totalTimerTicks);
	EXPECT_EQ(1U, tick.totalSteps);
	EXPECT_EQ(0U, tick.thisTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(updatePeriod, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_EQ(0.0f, tick.stepInterp);
	EXPECT_EQ(0.0f, tick.thisTime);

	EXPECT_TRUE(dsSceneTick_update(&tick, 124, 1));
	EXPECT_EQ(124U, tick.absoluteTimerTicks);
	EXPECT_EQ(1U, tick.totalTimerTicks);
	EXPECT_EQ(1U, tick.totalSteps);
	EXPECT_EQ(1U, tick.thisTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(0.0f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_NEAR(0.001f/updatePeriod, tick.stepInterp, epsilon);
	EXPECT_FLOAT_EQ(0.001f, tick.thisTime);

	EXPECT_TRUE(dsSceneTick_update(&tick, 136, 12));
	EXPECT_EQ(136U, tick.absoluteTimerTicks);
	EXPECT_EQ(13U, tick.totalTimerTicks);
	EXPECT_EQ(1U, tick.totalSteps);
	EXPECT_EQ(12U, tick.thisTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(0.0f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_NEAR(0.013f/updatePeriod, tick.stepInterp, epsilon);
	EXPECT_FLOAT_EQ(0.012f, tick.thisTime);

	EXPECT_TRUE(dsSceneTick_update(&tick, 145, 9));
	EXPECT_EQ(145U, tick.absoluteTimerTicks);
	EXPECT_EQ(22U, tick.totalTimerTicks);
	EXPECT_EQ(2U, tick.totalSteps);
	EXPECT_EQ(9U, tick.thisTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(updatePeriod, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_NEAR((0.022f - updatePeriod)/updatePeriod, tick.stepInterp, epsilon);
	EXPECT_FLOAT_EQ(0.009f, tick.thisTime);

	EXPECT_TRUE(dsSceneTick_update(&tick, 145, 0));
	EXPECT_EQ(145U, tick.absoluteTimerTicks);
	EXPECT_EQ(22U, tick.totalTimerTicks);
	EXPECT_EQ(2U, tick.totalSteps);
	EXPECT_EQ(0U, tick.thisTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(0.0f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_NEAR((0.022f - updatePeriod)/updatePeriod, tick.stepInterp, epsilon);
	EXPECT_EQ(0.0f, tick.thisTime);
	EXPECT_EQ(tick.totalSteps, dsSceneTick_absoluteStepNumber(&tick, 0));

	EXPECT_TRUE(dsSceneTick_update(&tick, 178, 33));
	EXPECT_EQ(178U, tick.absoluteTimerTicks);
	EXPECT_EQ(55U, tick.totalTimerTicks);
	EXPECT_EQ(4U, tick.totalSteps);
	EXPECT_EQ(33U, tick.thisTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(updatePeriod, tick.stepTime);
	EXPECT_EQ(2U, tick.stepCount);
	EXPECT_NEAR((0.055f - 3*updatePeriod)/updatePeriod, tick.stepInterp, epsilon);
	EXPECT_FLOAT_EQ(0.033f, tick.thisTime);
	EXPECT_EQ(2U, dsSceneTick_absoluteStepNumber(&tick, 0));
	EXPECT_EQ(3U, dsSceneTick_absoluteStepNumber(&tick, 1));
	EXPECT_EQ(1.0f, dsSceneTick_interpForStep(&tick, 0));
	EXPECT_EQ(tick.stepInterp, dsSceneTick_interpForStep(&tick, 1));

	EXPECT_TRUE(dsSceneTick_update(&tick, 20000, 10000));
	EXPECT_EQ(20000U, tick.absoluteTimerTicks);
	EXPECT_EQ(55U, tick.totalTimerTicks);
	EXPECT_EQ(4U, tick.totalSteps);
	EXPECT_EQ(0U, tick.thisTimerTicks);
	EXPECT_EQ(updatePeriod, tick.updatePeriod);
	EXPECT_EQ(0.0f, tick.stepTime);
	EXPECT_EQ(1U, tick.stepCount);
	EXPECT_NEAR((0.055f - 3*updatePeriod)/updatePeriod, tick.stepInterp, epsilon);
	EXPECT_FLOAT_EQ(0.0f, tick.thisTime);
}
