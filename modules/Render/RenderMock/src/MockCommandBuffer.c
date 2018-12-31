/*
 * Copyright 2017 Aaron Barany
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

#include "MockCommandBuffer.h"
#include <DeepSea/Core/Assert.h>

bool dsMockCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);

	return true;
}

bool dsMockCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);

	return true;
}

bool dsMockCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(submitBuffer);
	DS_UNUSED(submitBuffer);

	return true;
}
