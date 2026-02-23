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

#include "SceneVectorDrawScratchData.h"

#include "SceneVectorDrawTypes.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/ThreadObjectStorage.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Text/TextSubstitutionData.h>

#include <DeepSea/VectorDraw/VectorScratchData.h>

static dsSceneVectorDrawScratchData* dsSceneVectorDrawScratchData_get(
	dsSceneVectorDrawLoadContext* loadContext)
{
	DS_ASSERT(loadContext);
	DS_ASSERT(loadContext->scratchData);
	dsSceneVectorDrawScratchData* scratchData =
		(dsSceneVectorDrawScratchData*)dsThreadObjectStorage_get(loadContext->scratchData);
	if (scratchData)
		return scratchData;

	scratchData = DS_ALLOCATE_OBJECT(loadContext->allocator, dsSceneVectorDrawScratchData);
	if (!scratchData)
		return NULL;

	scratchData->allocator = loadContext->allocator;
	scratchData->vectorScratchData = NULL;
	scratchData->textSubstitutionData = NULL;
	if (!dsThreadObjectStorage_set(loadContext->scratchData, scratchData))
	{
		DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData));
		return NULL;
	}

	return scratchData;
}

dsVectorScratchData* dsSceneVectorDrawScratchData_getVectorScratchData(
	dsSceneVectorDrawLoadContext* loadContext)
{
	dsSceneVectorDrawScratchData* scratchData = dsSceneVectorDrawScratchData_get(loadContext);
	if (!scratchData)
		return NULL;

	if (!scratchData->vectorScratchData)
		scratchData->vectorScratchData = dsVectorScratchData_create(scratchData->allocator);
	return scratchData->vectorScratchData;
}

dsTextSubstitutionData* dsSceneVectorDrawScratchData_getTextSubstitutionData(
	dsSceneVectorDrawLoadContext* loadContext)
{
	dsSceneVectorDrawScratchData* scratchData = dsSceneVectorDrawScratchData_get(loadContext);
	if (!scratchData)
		return NULL;

	if (!scratchData->textSubstitutionData)
		scratchData->textSubstitutionData = dsTextSubstitutionData_create(scratchData->allocator);
	return scratchData->textSubstitutionData;
}

void dsSceneVectorDrawScratchData_destroy(void* userData)
{
	dsSceneVectorDrawScratchData* scratchData = (dsSceneVectorDrawScratchData*)userData;
	dsVectorScratchData_destroy(scratchData->vectorScratchData);
	dsTextSubstitutionData_destroy(scratchData->textSubstitutionData);
	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData));
}
