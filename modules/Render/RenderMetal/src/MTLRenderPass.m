/*
 * Copyright 2019 Aaron Barany
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

#include "MTLRenderPass.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

bool dsMTLRenderPass_addShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsMTLRenderPass* mtlRenderPass = (dsMTLRenderPass*)renderPass;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&mtlRenderPass->shaderLock));

	for (uint32_t i = 0; i < mtlRenderPass->usedShaderCount; ++i)
	{
		if (mtlRenderPass->usedShaders[i] == mtlShader->lifetime)
		{
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
			return true;
		}
	}

	uint32_t index = mtlRenderPass->usedShaderCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(renderPass->allocator, mtlRenderPass->usedShaders,
		mtlRenderPass->usedShaderCount, mtlRenderPass->maxUsedShaders, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
		return false;
	}

	mtlRenderPass->usedShaders[index] = dsLifetime_addRef(mtlShader->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
	return true;
}

void dsMTLRenderPass_removeShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsMTLRenderPass* mtlRenderPass = (dsMTLRenderPass*)renderPass;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&mtlRenderPass->shaderLock));
	for (uint32_t i = 0; i < mtlRenderPass->usedShaderCount; ++i)
	{
		dsLifetime* shaderLifetime = mtlRenderPass->usedShaders[i];
		if (shaderLifetime == mtlShader->lifetime)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(mtlRenderPass->usedShaders,
				mtlRenderPass->usedShaderCount, i, 1));
			dsLifetime_freeRef(shaderLifetime);
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
}
