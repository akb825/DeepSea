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
#include <DeepSea/Render/Types.h>

#include <Foundation/NSObject.h>

typedef enum dsMTLFeature
{
	// iOS 8, shader language 1.0
	dsMTLFeature_iOS_GPUFamily1_v1 = 0,
	dsMTLFeature_iOS_GPUFamily2_v1 = 1,

	 // iOS 9, shader language 1.1
	dsMTLFeature_iOS_GPUFamily1_v2 = 2,
	dsMTLFeature_iOS_GPUFamily2_v2 = 3,
	dsMTLFeature_iOS_GPUFamily3_v1 = 4,

	// iOS 10, shader language 1.2
	dsMTLFeature_iOS_GPUFamily1_v3 = 5,
	dsMTLFeature_iOS_GPUFamily2_v3 = 6,
	dsMTLFeature_iOS_GPUFamily3_v2 = 7,

	// iOS 11, shader language 2.0
	dsMTLFeature_iOS_GPUFamily1_v4 = 8,
	dsMTLFeature_iOS_GPUFamily2_v4 = 9,
	dsMTLFeature_iOS_GPUFamily3_v3 = 10,
	dsMTLFeature_iOS_GPUFamily4_v1 = 11,

	// iOS 11, shader language 2.1
	dsMTLFeature_iOS_GPUFamily1_v5 = 12,
	dsMTLFeature_iOS_GPUFamily2_v5 = 13,
	dsMTLFeature_iOS_GPUFamily3_v4 = 14,
	dsMTLFeature_iOS_GPUFamily4_v2 = 15,

	// macOS 10.11, shader language 1.1
	dsMTLFeature_macOS_GPUFamily1_v1 = 10000,

	// macOS 10.12, shader language 1.2
	dsMTLFeature_macOS_GPUFamily1_v2 = 10001,

	// macOS 10.13, shader language 2.0
	dsMTLFeature_macOS_GPUFamily1_v3 = 10003,

	// macOS 10.14, shader language 2.1
	dsMTLFeature_macOS_GPUFamily1_v4 = 10004,
	dsMTLFeature_macOS_GPUFamily2_v1 = 10005
} dsMTLFeature;

typedef struct dsMTLRenderer
{
	dsRenderer renderer;

	CFTypeRef mtlDevice;
	dsMTLFeature featureSet;
} dsMTLRenderer;
