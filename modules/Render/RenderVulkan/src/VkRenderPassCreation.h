/*
 * Copyright 2019-2021 Aaron Barany
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

#include "VkTypes.h"

bool dsVkAttachmentHasResolve(const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	uint32_t attachment, uint32_t samples, uint32_t surfaceSamples, uint32_t defaultSamples);
bool dsCreateUnderlyingVkRenderPass(dsVkRenderPassData* renderPassData,
	uint32_t resolveAttachmentCount);
