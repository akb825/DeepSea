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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Types.h>
#include <MSL/Client/TypesC.h>

void dsGLRenderStates_initialize(mslRenderState* state);
void dsGLRenderStates_updateGLState(const dsRenderer* renderer, mslRenderState* curState,
	const mslRenderState* newState, const dsDynamicRenderStates* dynamicStates);
void dsGLRenderStates_updateDynamicGLStates(const dsRenderer* renderer, mslRenderState* curState,
	const mslRenderState* newState, const dsDynamicRenderStates* dynamicStates);
void dsGLRenderStates_enableAllWriteMasks(const dsRenderer* renderer, mslRenderState* renderState);
