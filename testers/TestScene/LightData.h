/*
 * Copyright 2019-2020 Aaron Barany
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

#include <DeepSea/Core/Config.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

dsSceneGlobalData* dsLightData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize);
dsSceneGlobalData* dsLightData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* lightDesc);
void dsLightData_setDirection(dsSceneGlobalData* globalData, const dsVector3f* direction);
void dsLightData_setColor(dsSceneGlobalData* globalData, const dsVector3f* color);
void dsLightData_setAmbientColor(dsSceneGlobalData* globalData, const dsVector3f* color);

#ifdef __cplusplus
}
#endif
