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

#include <DeepSea/Scene/SceneTick.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Timer.h>

bool dsSceneTick_initialize(dsSceneTick* outTick, float updatePeriod)
{
	if (!outTick || updatePeriod < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	outTick->timer = dsTimer_create();
	outTick->totalTimerTicks = 0;
	outTick->thisTimerTicks = 0;
	outTick->updatePeriod = updatePeriod;
	outTick->stepTime = 0.0f;
	outTick->stepCount = 0;
	// Initialize stepInterp to 1 so that a fixed update period will trigger an update at the start
	// of time rolling over rather than at the end.
	outTick->stepInterp = 1.0f;
	outTick->thisTime = 0.0f;
	return true;
}

bool dsSceneTick_update(dsSceneTick* tick, uint64_t timerTicks)
{
	if (!tick)
	{
		errno = EINVAL;
		return false;
	}

	tick->totalTimerTicks += timerTicks;
	tick->thisTimerTicks = timerTicks;
	tick->thisTime = (float)dsTimer_ticksToSeconds(tick->timer, timerTicks);

	if (tick->updatePeriod == 0.0f)
	{
		tick->stepTime = tick->thisTime;
		tick->stepCount = tick->stepTime != 0.0f;
		tick->stepInterp = 1.0f;
	}
	else
	{
		float normalizedUpdate = tick->stepInterp + tick->thisTime/tick->updatePeriod;
		tick->stepCount = (unsigned int)normalizedUpdate;
		normalizedUpdate -= (float)tick->stepCount;
		tick->stepInterp = normalizedUpdate;
		tick->stepTime = tick->stepCount > 0 ? tick->updatePeriod : 0.0f;
	}

	return true;
}
