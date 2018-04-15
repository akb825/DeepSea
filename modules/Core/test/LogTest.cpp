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

#include <DeepSea/Core/Log.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace
{

class LogTest : public testing::Test
{
public:
	struct Message
	{
		dsLogLevel level;
		std::string tag;
		std::string file;
		unsigned int line;
		std::string function;
		std::string message;
	};

	static void logFunc(void* userData, dsLogLevel level, const char* tag,
		const char* file, unsigned int line, const char* function, const char* messageStr)
	{
		Message message;
		message.level = level;
		message.tag = tag;
		message.file = file;
		message.line = line;
		message.function = function;
		std::size_t scope = message.function.find_last_of(':');
		if (scope != std::string::npos)
			message.function = message.function.substr(scope + 1);
		message.message = messageStr;

		LogTest* self = (LogTest*)userData;
		self->messages.push_back(message);
	}

	void SetUp() override
	{
		dsLog_setFunction(this, &LogTest::logFunc);
	}

	void TearDown() override
	{
		dsLog_clearFunction();
	}

	std::vector<Message> messages;
};

} // namespace

TEST(Log, SetFunction)
{
	EXPECT_EQ(NULL, dsLog_getUserData());
	EXPECT_EQ(NULL, dsLog_getFunction());

	int test;
	dsLog_setFunction(&test, &LogTest::logFunc);

	EXPECT_EQ(&test, dsLog_getUserData());
	EXPECT_EQ(&LogTest::logFunc, dsLog_getFunction());

	dsLog_clearFunction();
	EXPECT_EQ(NULL, dsLog_getUserData());
	EXPECT_EQ(NULL, dsLog_getFunction());
}

TEST_F(LogTest, Log)
{
	DS_LOG_TRACE("trace", "Trace log test.");
	DS_LOG_DEBUG("debug", "Debug log test.");
	DS_LOG_INFO("info", "Info log test.");
	DS_LOG_WARNING("warning", "Warning log test.");
	DS_LOG_ERROR("error", "Error log test.");
	DS_LOG_FATAL("fatal", "Fatal log test.");

	ASSERT_EQ(6U, messages.size());

	unsigned int firstLine = messages[0].line;
	std::size_t separatorIndex;
	EXPECT_EQ(dsLogLevel_Trace, messages[0].level);
	EXPECT_EQ("trace", messages[0].tag);
	separatorIndex = messages[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[0].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine, messages[0].line);
	EXPECT_EQ("TestBody", messages[0].function);
	EXPECT_EQ("Trace log test.", messages[0].message);

	EXPECT_EQ(dsLogLevel_Debug, messages[1].level);
	EXPECT_EQ("debug", messages[1].tag);
	separatorIndex = messages[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[1].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 1, messages[1].line);
	EXPECT_EQ("TestBody", messages[1].function);
	EXPECT_EQ("Debug log test.", messages[1].message);

	EXPECT_EQ(dsLogLevel_Info, messages[2].level);
	EXPECT_EQ("info", messages[2].tag);
	separatorIndex = messages[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[2].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 2, messages[2].line);
	EXPECT_EQ("TestBody", messages[2].function);
	EXPECT_EQ("Info log test.", messages[2].message);

	EXPECT_EQ(dsLogLevel_Warning, messages[3].level);
	EXPECT_EQ("warning", messages[3].tag);
	separatorIndex = messages[3].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[3].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 3, messages[3].line);
	EXPECT_EQ("TestBody", messages[3].function);
	EXPECT_EQ("Warning log test.", messages[3].message);

	EXPECT_EQ(dsLogLevel_Error, messages[4].level);
	EXPECT_EQ("error", messages[4].tag);
	separatorIndex = messages[4].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[4].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 4, messages[4].line);
	EXPECT_EQ("TestBody", messages[4].function);
	EXPECT_EQ("Error log test.", messages[4].message);

	EXPECT_EQ(dsLogLevel_Fatal, messages[5].level);
	EXPECT_EQ("fatal", messages[5].tag);
	separatorIndex = messages[5].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[5].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 5, messages[5].line);
	EXPECT_EQ("TestBody", messages[5].function);
	EXPECT_EQ("Fatal log test.", messages[5].message);
}

TEST_F(LogTest, Logf)
{
	DS_LOG_TRACE_F("trace", "%s log test.", "Trace");
	DS_LOG_DEBUG_F("debug", "%s log test.", "Debug");
	DS_LOG_INFO_F("info", "%s log test.", "Info");
	DS_LOG_WARNING_F("warning", "%s log test.", "Warning");
	DS_LOG_ERROR_F("error", "%s log test.", "Error");
	DS_LOG_FATAL_F("fatal", "%s log test.", "Fatal");

	ASSERT_EQ(6U, messages.size());

	unsigned int firstLine = messages[0].line;
	std::size_t separatorIndex;
	EXPECT_EQ(dsLogLevel_Trace, messages[0].level);
	EXPECT_EQ("trace", messages[0].tag);
	separatorIndex = messages[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[0].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine, messages[0].line);
	EXPECT_EQ("TestBody", messages[0].function);
	EXPECT_EQ("Trace log test.", messages[0].message);

	EXPECT_EQ(dsLogLevel_Debug, messages[1].level);
	EXPECT_EQ("debug", messages[1].tag);
	separatorIndex = messages[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[1].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 1, messages[1].line);
	EXPECT_EQ("TestBody", messages[1].function);
	EXPECT_EQ("Debug log test.", messages[1].message);

	EXPECT_EQ(dsLogLevel_Info, messages[2].level);
	EXPECT_EQ("info", messages[2].tag);
	separatorIndex = messages[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[2].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 2, messages[2].line);
	EXPECT_EQ("TestBody", messages[2].function);
	EXPECT_EQ("Info log test.", messages[2].message);

	EXPECT_EQ(dsLogLevel_Warning, messages[3].level);
	EXPECT_EQ("warning", messages[3].tag);
	separatorIndex = messages[3].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[3].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 3, messages[3].line);
	EXPECT_EQ("TestBody", messages[3].function);
	EXPECT_EQ("Warning log test.", messages[3].message);

	EXPECT_EQ(dsLogLevel_Error, messages[4].level);
	EXPECT_EQ("error", messages[4].tag);
	separatorIndex = messages[4].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[4].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 4, messages[4].line);
	EXPECT_EQ("TestBody", messages[4].function);
	EXPECT_EQ("Error log test.", messages[4].message);

	EXPECT_EQ(dsLogLevel_Fatal, messages[5].level);
	EXPECT_EQ("fatal", messages[5].tag);
	separatorIndex = messages[5].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("LogTest.cpp", messages[5].file.c_str() + separatorIndex + 1);
	EXPECT_EQ(firstLine + 5, messages[5].line);
	EXPECT_EQ("TestBody", messages[5].function);
	EXPECT_EQ("Fatal log test.", messages[5].message);
}
