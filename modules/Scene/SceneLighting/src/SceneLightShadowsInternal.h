/*
 * Copyright 2021 Aaron Barany
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

#pragma once

#include <DeepSea/SceneLighting/SceneLightShadows.h>

typedef struct ShadowBufferInfo ShadowBufferInfo;

struct dsSceneLightShadows
{
	dsAllocator* allocator;
	const char* name;
	dsResourceManager* resourceManager;
	const dsSceneLightSet* lightSet;
	dsSceneLightType lightType;
	uint32_t nameID;
	uint32_t lightID;
	uint32_t transformGroupID;
	bool cascaded;

	const dsView* view;
	uint32_t committedMatrices;
	uint32_t totalMatrices;

	dsSceneShadowParams shadowParams;
	dsShadowCullVolume cullVolumes[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];
	dsShadowProjection projections[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];
	dsMatrix44f projectionMatrices[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];
	float minBoxSizes[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];
	float largeBoxSize;
	uint32_t projectionSet[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];

	ShadowBufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
	uint32_t curBuffer;
	void* curBufferData;

	dsShaderVariableGroup* fallback;

	dsSpinlock lock;
};
