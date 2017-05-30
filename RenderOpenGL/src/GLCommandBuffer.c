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

#include "GLCommandBuffer.h"
#include <DeepSea/Core/Assert.h>

bool dsGLCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferDataFunc(commandBuffer, buffer, offset, data, size);
}

bool dsGLCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferFunc(commandBuffer, srcBuffer, srcOffset, dstBuffer, dstOffset,
		size);
}

bool dsGLCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)submitBuffer)->functions;
	return functions->submitFunc(commandBuffer, submitBuffer);
}
