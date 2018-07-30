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

#include <DeepSea/EasyProfiler/EasyProfiler.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

// Order is important, since arbitrary_value.h doesn't #include all of its dependencies.
#include <easy/profiler.h>
#include <easy/arbitrary_value.h>

#include <cstdio>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>

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

	const char* uniqueString(const char* string)
	{
		DS_VERIFY(dsSpinlock_lock(&m_spinlock));

		// NOTE: insertion will allocate even if already present, so search first.
		uint32_t hash = dsHashString(string);
		auto iter = m_uniqueStrings.find(hash);
		const char* finalString;
		if (iter == m_uniqueStrings.end())
			finalString = m_uniqueStrings.emplace(hash, string).first->second.c_str();
		else
		{
			DS_ASSERT(iter->second == string);
			finalString = iter->second.c_str();
		}

		DS_VERIFY(dsSpinlock_unlock(&m_spinlock));
		return finalString;
	}

private:
	std::unordered_map<uint32_t, std::string> m_uniqueStrings;
	dsSpinlock m_spinlock;
};

const char* uniqueString(const char* string)
{
	static UniqueStringContainer uniqueStrings;
	return uniqueStrings.uniqueString(string);
}

profiler::color_t getColor(ExpandedProfileType type)
{
	switch (type)
	{
		case ExpandedProfileType::Function:
			return profiler::colors::LightGreen500;
		case ExpandedProfileType::Scope:
			return profiler::colors::Cyan300;
		case ExpandedProfileType::Wait:
			return profiler::colors::Red500;
		case ExpandedProfileType::Lock:
			return profiler::colors::Orange400;
		case ExpandedProfileType::Value:
			return profiler::colors::Brown500;
		case ExpandedProfileType::GPU:
			return profiler::colors::Purple700;
	}
	return profiler::colors::Black;
}

void registerProfilerBlock(void** blockData, const char* name, const char* file, unsigned int line,
	profiler::BlockType blockType, ExpandedProfileType profileType,
	const std::function<void (char*, uint32_t, const char*)>& customNameFunc = nullptr)
{
	void* ptrValue;
	DS_ATOMIC_LOAD_PTR(blockData, &ptrValue);
	if (ptrValue)
		return;

	// Max size: max path + ':' + max uint32_t digits + null terminator
	constexpr uint32_t maxSize = DS_PATH_MAX + 1 + 10 + 1;
	char buffer[maxSize];

	int result = std::snprintf(buffer, maxSize, "%s:%u", file, line);
	DS_UNUSED(result);
	DS_ASSERT(result > 0 && (uint32_t)result < maxSize);
	const char* uniqueName = uniqueString(buffer);

	bool copyName = false;
	if (customNameFunc)
	{
		copyName = true;
		customNameFunc(buffer, maxSize, name);
		name = buffer;
	}

	const profiler::BaseBlockDescriptor* block = profiler::registerDescription(profiler::ON,
		uniqueName, name, file, line, blockType, getColor(profileType), copyName);
	bool succeeded = DS_ATOMIC_COMPARE_EXCHANGE_PTR(blockData, &ptrValue, (void**)&block, false);
	DS_UNUSED(succeeded);

	// This might have happened concurrently across threads, in which case only the first one
	// executed will assign the pointer. easy_profiler uses a map internally to identify the blocks,
	// so this should return the same value if another thread also tried to register the same block.
	DS_ASSERT(succeeded || ptrValue == block);
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
	const char*, unsigned int line)
{
	registerProfilerBlock(blockData, name, file, line, profiler::BlockType::Block,
		(ExpandedProfileType)type);
	profiler::beginNonScopedBlock((const profiler::BaseBlockDescriptor*)*blockData);
}

void pop(void*, dsProfileType, const char*, const char*, unsigned int)
{
	profiler::endBlock();
}

void statValue(void*, void** blockData, const char* category, const char* name, double value,
	const char* file, const char*, unsigned int line)
{
	registerProfilerBlock(blockData, name, file, line, profiler::BlockType::Value,
		ExpandedProfileType::Value,
		[category](char* buffer, uint32_t maxSize, const char* name)
		{
			int result = std::snprintf(buffer, maxSize, "%s: %s", category, name);
			DS_UNUSED(result);
			DS_ASSERT(result >= 0 && (uint32_t)result < maxSize);
		});
	profiler::setValue((const profiler::BaseBlockDescriptor*)*blockData, value,
		profiler::ValueId(blockData));
}

void gpuValue(void*, const char* surface, const char* pass, uint64_t timeNs)
{
	// NOTE: This is a little slow, but we can't just cache the block data in this case. There
	// shouldn't be a massive number of render passes, so it shouldn't be a large impact.
	constexpr uint32_t maxSize = 256;
	char buffer[maxSize];
	int result = std::snprintf(buffer, maxSize, "%s: %s (ms)", surface, pass);
	DS_UNUSED(result);
	DS_ASSERT(result >= 0 && (uint32_t)result < maxSize);

	const char* uniqueName = uniqueString(buffer);

	const profiler::BaseBlockDescriptor* block = profiler::registerDescription(profiler::ON,
		uniqueName, uniqueName, __FILE__, __LINE__, profiler::BlockType::Value,
		getColor(ExpandedProfileType::GPU));
	double timeMs = (double)timeNs*1000000.0;
	profiler::setValue(block, timeMs, profiler::ValueId(uniqueName));
}

} // namespace

extern "C"
{

bool dsEasyProfiler_start(void)
{
	const dsProfileFunctions* curFunctions = dsProfile_getFunctions();
	if (curFunctions->pushFunc)
	{
		errno = EPERM;
		return false;
	}

	dsProfileFunctions functions =
	{
		&registerThread, &startFrame, &endFrame, &push, &pop, &statValue, &gpuValue
	};
	dsProfile_setFunctions(NULL, &functions);

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
