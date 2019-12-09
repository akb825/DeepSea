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

#pragma once

#include <DeepSea/Core/Config.h>
#include "MTLTypes.h"

#import <Metal/MTLSampler.h>

MTLCompareFunction dsGetMTLCompareFunction(mslCompareOp compare, MTLCompareFunction defaultVal);
MTLStencilOperation dsGetMTLStencilOp(mslStencilOp op);
MTLStencilDescriptor* dsCreateMTLStencilDescriptor(const mslStencilOpState* state,
	uint32_t compareMask, uint32_t writeMask);
MTLClearColor dsGetMTLClearColor(dsGfxFormat format, const dsSurfaceColorValue* value);
bool dsIsMTLFormatPVR(dsGfxFormat format);
MTLPixelFormat dsGetMTLDepthFormat(const dsResourceManager* resourceManager, dsGfxFormat format);
MTLPixelFormat dsGetMTLStencilFormat(const dsResourceManager* resourceManager, dsGfxFormat format);
