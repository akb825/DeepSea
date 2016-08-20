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

#include <DeepSea/Math/Random.h>
#include <DeepSea/Core/Assert.h>

uint32_t dsRandom(uint32_t* seed)
{
	DS_ASSERT(seed);
	uint64_t temp = *seed ? *seed : 1;
	temp = temp*48271 % (DS_RANDOM_MAX + 1);
	return *seed = (uint32_t)temp;
}
