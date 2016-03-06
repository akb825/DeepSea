/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Core/Atomic.h>

#if defined(_MSC_VER) && !DS_64BIT

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

__int64 dsAtomic_interlockedOr64Impl(__int64* xPtr, __int64 value)
{
	return _InterlockedOr64(xPtr, value);
}

__int64 dsAtomic_interlockedExchange64Impl(__int64* xPtr, __int64 value)
{
	return _InterlockedExchange64(xPtr, value);
}

__int64 dsAtomic_interlockedCompareExchange64Impl(__int64* xPtr, __int64 value, __int64 expected)
{
	return _InterlockedCompareExchange64(xPtr, value, expected);
}

__int64 dsAtomic_interlockedExchangeAdd64Impl(__int64* xPtr, __int64 value)
{
	return _InterlockedExchangeAdd64(xPtr, value);
}

#endif
