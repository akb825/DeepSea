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

#include <DeepSea/Math/Core.h>

bool dsSceneTick_initialize(dsSceneTick* outTick, float updatePeriod, float maxTime)
{
	if (!outTick || updatePeriod < 0.0f || maxTime < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	outTick->timer = dsTimer_create();
	outTick->absoluteTimerTicks = 0;
	outTick->totalTimerTicks = 0;
	outTick->thisTimerTicks = 0;
	if (maxTime == 0.0f)
		outTick->maxTimerTicks = UINT64_MAX;
	else
		outTick->maxTimerTicks = dsTimer_secondsToTicks(outTick->timer, maxTime);
	outTick->updatePeriod = updatePeriod;
	outTick->stepTime = 0.0f;
	outTick->stepCount = 1;
	// Initialize stepInterp to 1 so that a fixed update period will trigger an update at the start
	// of time rolling over rather than at the end.
	outTick->stepInterp = 1.0f;
	outTick->thisTime = 0.0f;
	return true;
}

bool dsSceneTick_update(dsSceneTick* tick, uint64_t absoluteTicks, uint64_t elapsedTicks)
{
	if (!tick)
	{
		errno = EINVAL;
		return false;
	}

	if (elapsedTicks > tick->maxTimerTicks)
		elapsedTicks = 0;

	tick->absoluteTimerTicks = absoluteTicks;
	tick->totalTimerTicks += elapsedTicks;
	tick->thisTimerTicks = elapsedTicks;
	tick->thisTime = (float)dsTimer_ticksToSeconds(tick->timer, elapsedTicks);

	if (tick->updatePeriod == 0.0f)
	{
		tick->stepTime = tick->thisTime;
		tick->stepCount = 1;
		tick->stepInterp = 1.0f;
	}
	else
	{
		float normalizedUpdate = tick->stepInterp + tick->thisTime/tick->updatePeriod;
		unsigned int stepCount = (unsigned int)normalizedUpdate;
		normalizedUpdate -= (float)stepCount;
		tick->stepCount = dsMax(stepCount, 1U);
		tick->stepInterp = normalizedUpdate;
		tick->stepTime = stepCount > 0 ? tick->updatePeriod : 0.0f;
	}

	return true;
}
