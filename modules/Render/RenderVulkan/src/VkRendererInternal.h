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

#pragma once

#include "Types.h"

void dsVkRenderer_flush(dsRenderer* renderer);
dsGfxFenceResult dsVkRenderer_waitForSubmit(dsRenderer* renderer, uint64_t submitCount,
	uint64_t timeout);

void dsVkRenderer_processGfxBuffer(dsRenderer* renderer, dsVkGfxBufferData* buffer);
void dsVkRenderer_deleteGfxBuffer(dsRenderer* renderer, dsVkGfxBufferData* buffer);
