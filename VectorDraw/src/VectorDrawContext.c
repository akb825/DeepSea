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

#include <DeepSea/VectorDraw/VectorDrawContext.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>

dsVectorDrawContext* dsVectorDrawContext_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule)
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) || !shaderModule)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	dsMaterial* material = dsMaterial_create(allocator, shaderModule->materialDesc);
	if (!material)
		return NULL;

	dsShaderVariableGroup* transformGroup = dsShaderVariableGroup_create(resourceManager, allocator,
		allocator, shaderModule->transformDesc);
	if (!transformGroup)
	{
		dsMaterial_destroy(material);
		return NULL;
	}

	dsVectorDrawContext* context = DS_ALLOCATE_OBJECT(allocator, dsVectorDrawContext);
	if (!context)
	{
		dsMaterial_destroy(material);
		DS_VERIFY(dsShaderVariableGroup_destroy(transformGroup));
	}

	DS_VERIFY(dsMaterial_setVariableGroup(material, shaderModule->transformElement,
		transformGroup));

	context->allocator = dsAllocator_keepPointer(allocator);
	context->shaderModule = shaderModule;
	context->material = material;
	context->transformGroup = transformGroup;

	return context;
}

bool dsVectorDrawContext_destroy(dsVectorDrawContext* context)
{
	if (!context)
		return true;

	if (!dsShaderVariableGroup_destroy(context->transformGroup))
		return false;
	dsMaterial_destroy(context->material);
	if (context->allocator)
		DS_VERIFY(dsAllocator_free(context->allocator, context));
	return true;
}
