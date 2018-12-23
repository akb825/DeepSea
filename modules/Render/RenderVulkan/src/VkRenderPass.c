/*
 * Copyright 2018 Aaron Barany
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

#include "VkRenderPass.h"

#include "Resources/VkResource.h"
#include "Resources/VkShader.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>

bool dsVkRenderPass_addShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;
	dsVkShader* vkShader = (dsVkShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&vkRenderPass->shaderLock));

	for (uint32_t i = 0; i < vkRenderPass->usedShaderCount; ++i)
	{
		void* usedShader = dsLifetime_getObject(vkRenderPass->usedShaders[i]);
		DS_ASSERT(usedShader);
		if (usedShader == shader)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->shaderLock));
			return true;
		}
	}

	uint32_t index = vkRenderPass->usedShaderCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(vkRenderPass->scratchAllocator, vkRenderPass->usedShaders,
		vkRenderPass->usedShaderCount, vkRenderPass->maxUsedShaders, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->shaderLock));
		return false;
	}

	vkRenderPass->usedShaders[index] = dsLifetime_addRef(vkShader->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));
	return true;
}

void dsVkRenderPass_removeShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;
	DS_VERIFY(dsSpinlock_lock(&vkRenderPass->shaderLock));
	for (uint32_t i = 0; i < vkRenderPass->usedShaderCount; ++i)
	{
		void* usedShader = dsLifetime_getObject(vkRenderPass->usedShaders[i]);
		DS_ASSERT(usedShader);
		if (usedShader == shader)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(vkRenderPass->usedShaders,
				vkRenderPass->usedShaderCount, i, 1));
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->shaderLock));
}
