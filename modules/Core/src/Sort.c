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

#define _GNU_SOURCE
#include <DeepSea/Core/Sort.h>
#include <stdlib.h>
#if DS_WINDOWS
#include <search.h>
#endif

#if DS_APPLE || DS_BSD || DS_WINDOWS
typedef struct ContextWrapper
{
	dsSortCompareFunction func;
	void* context;
} ContextWrapper;

static int sortWrapper(void* context, const void* left, const void* right)
{
	ContextWrapper* contextWrapper = (ContextWrapper*)context;
	return contextWrapper->func(left, right, contextWrapper->context);
}
#endif

void dsSort(void* array, size_t memberCount, size_t memberSize, dsSortCompareFunction compareFunc,
	void* context)
{
#if DS_WINDOWS
	ContextWrapper wrapper = {compareFunc, context};
	qsort_s(array, memberCount, memberSize, &sortWrapper, &wrapper);
#elif DS_APPLE
	ContextWrapper wrapper = {compareFunc, context};
	qsort_r(array, memberCount, memberSize, &wrapper, &sortWrapper);
#else
	qsort_r(array, memberCount, memberSize, compareFunc, context);
#endif
}

void* dsBinarySearch(const void* key, const void* array, size_t memberCount,
	size_t memberSize, dsSortCompareFunction compareFunc, void* context)
{
	if (!array || memberCount == 0 || memberSize == 0 || !compareFunc)
		return NULL;

	uint8_t* arrayBytes = (uint8_t*)array;
	size_t start = 0;
	size_t count = memberCount;
	while (count > 0)
	{
		size_t halfCount = count/2;
		size_t half = start + halfCount;
		int compare = compareFunc(key, arrayBytes + half*memberSize, context);
		if (compare == 0)
			return arrayBytes + half*memberSize;
		else if (compare < 0)
			count = halfCount;
		else
		{
			if (count == 1)
				break;
			start = half;
			count -= halfCount;
		}
	}

	return NULL;
}

void* dsBinarySearchLowerBound(const void* key, const void* array, size_t memberCount,
	size_t memberSize, dsSortCompareFunction compareFunc, void* context)
{
	if (!array || memberCount == 0 || memberSize == 0 || !compareFunc)
		return NULL;

	uint8_t* arrayBytes = (uint8_t*)array;
	size_t start = 0;
	size_t count = memberCount;
	int compare = 0;
	while (count > 0)
	{
		size_t halfCount = count/2;
		size_t half = start + halfCount;
		compare = compareFunc(key, arrayBytes + half*memberSize, context);
		if (compare == 0)
		{
			start = half;
			break;
		}
		else if (compare < 0)
			count = halfCount;
		else
		{
			if (count == 1)
				break;
			start = half;
			count -= halfCount;
		}
	}

	// May need to go up to the element that's >= the key.
	while (compare > 0 && start < memberCount)
		compare = compareFunc(key, arrayBytes + ++start*memberSize, context);
	if (start == memberCount)
		return NULL;

	// Go to the first matching element.
	while (compare <= 0 && start > 0)
	{
		compare = compareFunc(key, arrayBytes + (start - 1)*memberSize, context);
		if (compare <= 0)
			--start;
	}
	return arrayBytes + start*memberSize;
}

void* dsBinarySearchUpperBound(const void* key, const void* array, size_t memberCount,
	size_t memberSize, dsSortCompareFunction compareFunc, void* context)
{
	if (!array || memberCount == 0 || memberSize == 0 || !compareFunc)
		return NULL;

	uint8_t* arrayBytes = (uint8_t*)array;
	size_t start = 0;
	size_t count = memberCount;
	int compare = 0;
	while (count > 0)
	{
		size_t halfCount = count/2;
		size_t half = start + halfCount;
		compare = compareFunc(key, arrayBytes + half*memberSize, context);
		if (compare == 0)
		{
			start = half;
			break;
		}
		else if (compare < 0)
			count = halfCount;
		else
		{
			if (count == 1)
				break;
			start = half;
			count -= halfCount;
		}
	}

	// May need to go up to the element that's > the key.
	while (compare >= 0 && start < memberCount)
		compare = compareFunc(key, arrayBytes + ++start*memberSize, context);
	if (start == memberCount)
		return NULL;

	// Go to the first matching element.
	while (compare < 0 && start > 0)
	{
		compare = compareFunc(key, arrayBytes + (start - 1)*memberSize, context);
		if (compare < 0)
			--start;
	}
	return arrayBytes + start*memberSize;
}
