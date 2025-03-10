/*
 * Copyright 2018-2024 Aaron Barany
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

#include <DeepSea/EasyProfiler/EasyProfiler.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#define LOG_TAG "easy_profiler"

#if DS_PROFILING_ENABLED

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Profile.h>

// Order is important, since arbitrary_value.h doesn't #include all of its dependencies.
#include <easy/profiler.h>
#include <easy/arbitrary_value.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <unordered_set>

namespace
{

enum class ExpandedProfileType
{
	Function = dsProfileType_Function,
	Scope = dsProfileType_Scope,
	Wait = dsProfileType_Wait,
	Lock = dsProfileType_Lock,
	Value,
	GPU
};

struct StringStorage
{
	const char* str;
	std::unique_ptr<char[]> strStorage;
	uint32_t hash;

	StringStorage(const char* s, uint32_t h, bool copy = false)
	{
		if (copy)
		{
			size_t strLen = std::strlen(s) + 1;
			strStorage.reset(new char[strLen]);
			std::memcpy(strStorage.get(), s, strLen);
			str = strStorage.get();
		}
		else
			str = s;
		hash = h;
	}

	bool operator==(const StringStorage& other) const
	{
		return hash == other.hash && std::strcmp(str, other.str) == 0;
	}

	bool operator!=(const StringStorage& other) const
	{
		return !(*this == other);
	}
};

struct StringStorageHash
{
	std::size_t operator()(const StringStorage& str) const
	{
		return str.hash;
	}
};

class UniqueStringContainer
{
public:
	UniqueStringContainer()
	{
		dsSpinlock_initialize(&m_spinlock);
	}

	~UniqueStringContainer()
	{
		dsSpinlock_shutdown(&m_spinlock);
	}

	const char* uniqueString(const char* string, uint32_t hash)
	{
		DS_VERIFY(dsSpinlock_lock(&m_spinlock));

		// NOTE: insertion will allocate even if already present, so search first.
		auto iter = m_uniqueStrings.find(StringStorage(string, hash));
		const char* finalString;
		if (iter == m_uniqueStrings.end())
			finalString = m_uniqueStrings.emplace(string, hash, true).first->str;
		else
			finalString = iter->str;

		DS_VERIFY(dsSpinlock_unlock(&m_spinlock));
		return finalString;
	}

private:
	std::unordered_set<StringStorage, StringStorageHash> m_uniqueStrings;
	dsSpinlock m_spinlock;
};

const char* uniqueString(const char* string, uint32_t hash)
{
	static UniqueStringContainer uniqueStrings;
	return uniqueStrings.uniqueString(string, hash);
}

const char* uniqueString(const char* string)
{
	return uniqueString(string, dsHashString(string));
}

profiler::color_t hsvColor(float hue, float saturation, float value)
{
	// https://www.rapidtables.com/convert/color/hsv-to-rgb.html
	float c = value*saturation;
	float x = c*(1.0f - std::fabs(std::fmod(hue/60.0f, 2.0f) - 1.0f));
	float m = value - c;
	c += m;
	x += m;

	float r, g, b;
	if (hue >= 0.0f && hue < 60.0f)
	{
		r = c;
		g = x;
		b = m;
	}
	else if (hue >= 60.0f && hue < 120.0f)
	{
		r = x;
		g = c;
		b = m;
	}
	else if (hue >= 120.0f && hue < 180.0f)
	{
		r = m;
		g = c;
		b = x;
	}
	else if (hue >= 180.0f && hue < 240.0f)
	{
		r = m;
		g = x;
		b = c;
	}
	else if (hue >= 240.0f && hue < 300.0f)
	{
		r = x;
		g = m;
		b = c;
	}
	else
	{
		r = c;
		g = m;
		b = x;
	}

	return profiler::colors::color((uint8_t)std::roundf(r*255.0f), (uint8_t)std::roundf(g*255.0f),
		(uint8_t)std::roundf(b*255.0f));
}

float hashToHue(uint32_t hash)
{
	return (float)((double)hash/(double)std::numeric_limits<uint32_t>::max())*360.0f;
}

profiler::color_t getColor(ExpandedProfileType type, uint32_t hash = 0)
{
	switch (type)
	{
		case ExpandedProfileType::Function:
		case ExpandedProfileType::Scope:
			return hsvColor(hashToHue(hash), 0.2f, 1.0f);
		case ExpandedProfileType::Wait:
			return profiler::colors::Red900;
		case ExpandedProfileType::Lock:
			return profiler::colors::Orange800;
		case ExpandedProfileType::Value:
		case ExpandedProfileType::GPU:
			return hsvColor(hashToHue(hash), 0.5f, 0.8f);
	}
	return profiler::colors::Black;
}

const profiler::BaseBlockDescriptor* registerProfilerBlock(void** blockData, const char* category,
	const char* name, const char* file, unsigned int line, profiler::BlockType blockType,
	ExpandedProfileType profileType, bool copyName)
{
	void* ptrValue;
	DS_ATOMIC_LOAD_PTR(blockData, &ptrValue);
	if (ptrValue)
		return (const profiler::BaseBlockDescriptor*)*blockData;

	// Max size: max path + ':' + max uint32_t digits + null terminator
	constexpr uint32_t maxSize = DS_PATH_MAX + 1 + 10 + 1;
	char buffer[maxSize];

	if (category)
	{
		copyName = true;
		int result = std::snprintf(buffer, maxSize, "%u %s: %s", (uint32_t)profileType, category,
			name);
		DS_UNUSED(result);
		DS_ASSERT(result > 0 && (uint32_t)result < maxSize);

		constexpr uint32_t nameOffset = 2;
		name = buffer + nameOffset;
	}
	else
	{
		int result = std::snprintf(buffer, maxSize, "%s:%u", file, line);
		DS_UNUSED(result);
		DS_ASSERT(result > 0 && (uint32_t)result < maxSize);

		if (!name)
			name = buffer;
	}

	uint32_t hash = dsHashString(buffer);
	const char* uniqueName = uniqueString(buffer, hash);

	const profiler::BaseBlockDescriptor* block = profiler::registerDescription(profiler::ON,
		uniqueName, name, file, line, blockType, getColor(profileType, hash), copyName);
	bool succeeded = DS_ATOMIC_COMPARE_EXCHANGE_PTR(blockData, &ptrValue, (void**)&block, false);
	DS_UNUSED(succeeded);

	// This might have happened concurrently across threads, in which case only the first one
	// executed will assign the pointer. easy_profiler uses a map internally to identify the blocks,
	// so this should return the same value if another thread also tried to register the same block.
	DS_ASSERT(succeeded || ptrValue == block);

	return block;
}

const profiler::BaseBlockDescriptor* registerDynamicProfilerBlock(const char* category,
	const char* name, const char* units, const char* file, unsigned int line,
	profiler::BlockType blockType, ExpandedProfileType profileType)
{
	constexpr uint32_t maxSize = 256;
	char buffer[maxSize];

	constexpr uint32_t nameOffset = 2;
	int result;
	if (units)
	{
		result = std::snprintf(buffer, maxSize, "%u %s: %s (%s)", (uint32_t)profileType, category,
			name, units);
	}
	else
		result = std::snprintf(buffer, maxSize, "%u %s: %s", (uint32_t)profileType, category, name);

	DS_UNUSED(result);
	DS_ASSERT(result > 0 && (uint32_t)result < maxSize);

	uint32_t hash = dsHashString(buffer);
	const char* uniqueName = uniqueString(buffer, hash);
	const char* blockName = uniqueString(buffer + nameOffset);

	return profiler::registerDescription(profiler::ON, uniqueName, blockName, file, line,
		blockType, getColor(profileType, hash), true);
}

void registerThread(void*, const char* name)
{
	profiler::registerThread(name);
}

void startFrame(void*)
{
	EASY_NONSCOPED_BLOCK("Frame", profiler::colors::BlueGrey100);
}

void endFrame(void*)
{
	EASY_END_BLOCK;
}

void push(void*, void** blockData, dsProfileType type, const char* name, const char* file,
	const char*, unsigned int line, bool dynamic)
{
	constexpr uint32_t maxSize = 256;
	char nameBuffer[maxSize];
	bool copyName = false;
	if (type == dsProfileType_Wait || type == dsProfileType_Lock)
	{
		const char* prefix = NULL;
		switch (type)
		{
			case dsProfileType_Wait:
				prefix = "Wait";
				break;
			case dsProfileType_Lock:
				prefix = "Lock";
				break;
			default:
				DS_ASSERT(false);
				break;
		}

		int result = std::snprintf(nameBuffer, maxSize, "%s: %s", prefix, name);
		DS_UNUSED(result);
		DS_ASSERT(result > 0 && (uint32_t)result < maxSize);
		name = nameBuffer;
		copyName = true;
	}
	auto block = registerProfilerBlock(blockData, nullptr, dynamic ? nullptr : name, file, line,
		profiler::BlockType::Block, (ExpandedProfileType)type, copyName);
	profiler::beginNonScopedBlock(block, dynamic ? name : "");
}

void pop(void*, dsProfileType, const char*, const char*, unsigned int)
{
	profiler::endBlock();
}

void statValue(void*, void** blockData, const char* category, const char* name, double value,
	const char* file, const char*, unsigned int line, bool dynamic)
{
	const profiler::BaseBlockDescriptor* block;
	if (dynamic)
	{
		block = registerProfilerBlock(blockData, category, name, file, line,
			profiler::BlockType::Value, ExpandedProfileType::Value, true);
	}
	else
	{
		block = registerDynamicProfilerBlock(category, name, nullptr, file, line,
			profiler::BlockType::Value, ExpandedProfileType::Value);
	}

	profiler::setValue(block, value, profiler::ValueId(*block));
}

void gpuValue(void*, const char* category, const char* name, uint64_t timeNs)
{
	auto block = registerDynamicProfilerBlock(category, name, "GPU ms", __FILE__, __LINE__,
		profiler::BlockType::Value, ExpandedProfileType::Value);

	double timeMs = (double)timeNs/1000000.0;
	profiler::setValue(block, timeMs, profiler::ValueId(*block));
}

} // namespace

extern "C"
{

bool dsEasyProfiler_start(bool beginCapture)
{
	const dsProfileFunctions* curFunctions = dsProfile_getFunctions();
	if (curFunctions->pushFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(LOG_TAG, "Profiler already started.");
		return false;
	}

	dsProfileFunctions functions =
	{
		&registerThread, &startFrame, &endFrame, &push, &pop, &statValue, &gpuValue
	};
	dsProfile_setFunctions(NULL, &functions);

	EASY_MAIN_THREAD;
	if (beginCapture)
		profiler::startCapture();
	return true;
}

bool dsEasyProfiler_stop(void)
{
	profiler::stopCapture();
	dsProfile_clearFunctions();
	return true;
}

bool dsEasyProfiler_startListening(uint16_t port)
{
	profiler::startListen(port);
	return true;
}

bool dsEasyProfiler_stopListening(void)
{
	profiler::stopCapture();
	return true;
}

bool dsEasyProfiler_dumpToFile(const char* filePath)
{
	if (!filePath)
	{
		errno = EINVAL;
		return false;
	}

	return profiler::dumpBlocksToFile(filePath) > 0;
}

} // extern "C"

#else

extern "C"
{

bool dsEasyProfiler_start(bool)
{
	errno = EPERM;
	return false;
}

bool dsEasyProfiler_stop(void)
{
	errno = EPERM;
	return false;
}

bool dsEasyProfiler_startListening(uint16_t)
{
	errno = EPERM;
	return false;
}

bool dsEasyProfiler_stopListening(void)
{
	errno = EPERM;
	return false;
}

bool dsEasyProfiler_dumpToFile(const char*)
{
	errno = EPERM;
	DS_LOG_WARNING(LOG_TAG, "Profiling disabled.");
	return false;
}

} // extern "C"

#endif
