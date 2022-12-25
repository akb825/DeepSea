/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/Math/SIMD/SIMD.h>

#if DS_WINDOWS
#include <intrin.h>
#endif

#if DS_HAS_SIMD

#if DS_X86_32 || DS_X86_64

#if !DS_WINDOWS
static void __cpuid(int cpuInfo[4], int function)
{
	cpuInfo[0] = function;
	cpuInfo[2] = 0;
	asm volatile("cpuid\n\t" : "+a"(cpuInfo[0]), "=b"(cpuInfo[1]), "+c"(cpuInfo[2]),
		"=d"(cpuInfo[3]));
}
#endif

static dsSIMDFeatures detectSIMDFeatures()
{
	// Function 1, edx.
	const int sseBit = 1 << 25;
	const int sse2Bit = 1 << 26;

	// Function 1, ecx
	const int sse3Bit = 1;
	const int fmaBit = 1 << 12;
	const int f16cBit = 1 << 29;

	dsSIMDFeatures features = dsSIMDFeatures_None;

	int cpuInfo[4];
	__cpuid(cpuInfo, 1);
	int ecx = cpuInfo[2];
	int edx = cpuInfo[3];
	if (!(edx & sseBit))
		return features;

	features |= dsSIMDFeatures_Float4;
	if (ecx & sse3Bit)
		features |= dsSIMDFeatures_HAdd;
	if (ecx & fmaBit)
		features |= dsSIMDFeatures_FMA;
	if ((edx & sse2Bit) && (ecx & f16cBit))
		features |= dsSIMDFeatures_HalfFloat;

	return features;
}
#else
static dsSIMDFeatures detectSIMDFeatures()
{
	return dsSIMDFeatures_Float4 | dsSIMDFeatures_HAdd | dsSIMDFeatures_FMA |
		dsSIMDFeatures_HalfFloat;
}
#endif

const dsSIMDFeatures dsHostSIMDFeatures = detectSIMDFeatures();

#endif
