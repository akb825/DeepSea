/*
 * Copyright 2022-2026 Aaron Barany
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
#elif DS_X86
#include <cpuid.h>
#endif

#if DS_HAS_SIMD

#if DS_X86

#if DS_WINDOWS
static void __get_cpuid(
	unsigned int level, unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx)
{
	int cpuInfo[4];
	__cpuid(cpuInfo, level);
	*eax = cpuInfo[0];
	*ebx = cpuInfo[1];
	*ecx = cpuInfo[2];
	*edx = cpuInfo[3];
}

static void __get_cpuid_count(unsigned int level, unsigned int sublevel, unsigned int* eax,
	unsigned int* ebx, unsigned int* ecx, unsigned int* edx)
{
	int cpuInfo[4];
	__cpuidex(cpuInfo, level, sublevel);
	*eax = cpuInfo[0];
	*ebx = cpuInfo[1];
	*ecx = cpuInfo[2];
	*edx = cpuInfo[3];
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
	const int sse41Bit = 1 << 19;
	const int f16cBit = 1 << 29;

	// Function 7, sub 0, ebx
	const int avx2Bit = 1 << 5;

	dsSIMDFeatures features = dsSIMDFeatures_None;

	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	if (!(edx & sseBit))
		return features;

	features |= dsSIMDFeatures_Float4;
	if (edx & sse2Bit)
		features |= dsSIMDFeatures_Int | dsSIMDFeatures_Double2;
	if (ecx & sse3Bit)
		features |= dsSIMDFeatures_HAdd;
	if (ecx & sse41Bit)
		features |= dsSIMDFeatures_Rounding;
	if ((ecx & fmaBit) && !DS_DETERMINISTIC_MATH)
		features |= dsSIMDFeatures_FMA;
	if ((edx & sse2Bit) && (ecx & f16cBit))
		features |= dsSIMDFeatures_HalfFloat;

	__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);
	if (ebx & avx2Bit)
		features |= dsSIMDFeatures_Double4;

	return features;
}
#else
static dsSIMDFeatures detectSIMDFeatures()
{
	dsSIMDFeatures features =
		dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_HAdd | dsSIMDFeatures_HalfFloat;
#if !DS_DETERMINISTIC_MATH
	features |= dsSIMDFeatures_FMA;
#endif
#if DS_ARM_64
	features |= dsSIMDFeatures_Double2 | dsSIMDFeatures_Rounding;
#endif
	return features;
}
#endif

const dsSIMDFeatures dsHostSIMDFeatures = detectSIMDFeatures();

#endif
