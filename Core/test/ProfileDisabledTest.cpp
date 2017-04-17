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

#undef DS_PROFILING_ENABLED
#define DS_PROFILING_ENABLED 0

#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace
{

struct PushInfo
{
	PushInfo(dsProfileType type_, const char* name_, const char* file_, const char* function_,
		unsigned int line_)
		: type(type_), name(name_), file(file_), function(function_), line(line_) {}

	dsProfileType type;
	std::string name;
	std::string file;
	std::string function;
	unsigned int line;
};

struct PopInfo
{
	PopInfo(dsProfileType type_, const char* file_, const char* function_, unsigned int line_)
		: type(type_), file(file_), function(function_), line(line_) {}

	dsProfileType type;
	std::string file;
	std::string function;
	unsigned int line;
};

struct StatInfo
{
	StatInfo(const char* category_, const char* name_, double value_, const char* file_,
		const char* function_, unsigned int line_)
		: category(category_), name(name_), value(value_), file(file_), function(function_),
		  line(line_) {}

	std::string category;
	std::string name;
	double value;
	std::string file;
	std::string function;
	unsigned int line;
};

struct ProfileInfo
{
	std::vector<PushInfo> push;
	std::vector<PopInfo> pop;
	std::vector<StatInfo> stat;
};

void profilePush(void* userData, void**, dsProfileType type, const char* name, const char* file,
	const char* function, unsigned int line)
{
	((ProfileInfo*)userData)->push.emplace_back(type, name, file, function, line);
}

void profilePop(void* userData, dsProfileType type, const char* file, const char* function,
	unsigned int line)
{
	((ProfileInfo*)userData)->pop.emplace_back(type, file, function, line);
}

void profileStat(void* userData, void**, const char* category, const char* name, double value,
	const char* file, const char* function, unsigned int line)
{
	((ProfileInfo*)userData)->stat.emplace_back(category, name, value, file, function, line);
}

void voidFunction()
{
	DS_PROFILE_FUNC_START();
	DS_PROFILE_FUNC_RETURN_VOID();
}

int intFunction(int retVal)
{
	DS_PROFILE_FUNC_START_NAME("Custom Function");
	DS_PROFILE_FUNC_RETURN(retVal);
}

} // namespace

TEST(ProfileDisabled, Macros)
{
	ProfileInfo info;
	dsProfileFunctions functions =
	{
		NULL, NULL, NULL, &profilePush, &profilePop, &profileStat, NULL
	};
	dsProfile_setFunctions(&info, &functions);

	voidFunction();
	EXPECT_EQ(10, intFunction(10));
	DS_PROFILE_SCOPE_START("Scope");
	DS_PROFILE_SCOPE_END();
	DS_PROFILE_WAIT_START("Wait");
	DS_PROFILE_WAIT_END();
	DS_PROFILE_LOCK_START("Lock");
	DS_PROFILE_LOCK_END();
	DS_PROFILE_STAT("Category", "Name", 10);

	dsProfile_clearFunctions();

	EXPECT_EQ(0U, info.push.size());
	EXPECT_EQ(0U, info.pop.size());
	EXPECT_EQ(0U, info.stat.size());
}
