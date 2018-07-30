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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Containers/Types.h>
#include <DeepSea/Core/Memory/Types.h>
#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Core/Thread/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes (directly or indirectly) all of the types used in the DeepSea/Core library.
 */

/**
 * @brief Log tag used by the core library.
 */
#define DS_CORE_LOG_TAG "core"

/**
 * @brief The level of the log message.
 * @see Log.h
 */
typedef enum dsLogLevel
{
	dsLogLevel_Trace,   ///< A trace message for status updates.
	dsLogLevel_Debug,   ///< A debug message.
	dsLogLevel_Info,    ///< An info message.
	dsLogLevel_Warning, ///< A warning message that could indicate a problem.
	dsLogLevel_Error,   ///< An error message that indicates a problem.
	dsLogLevel_Fatal    ///< A fatal message that indicates execution cannot continue.
} dsLogLevel;

/**
 * @brief The type describing what is being profiled.
 */
typedef enum dsProfileType
{
	dsProfileType_Function, ///< Profiling a function.
	dsProfileType_Scope,    ///< Profiling a scope within a function.
	dsProfileType_Wait,     ///< Waiting for an an asynchronous operation, mutex lock, etc.
	dsProfileType_Lock      ///< Locked, such as with a mutex.
} dsProfileType;

/**
 * @brief Struct containing a range of indices.
 */
typedef struct dsIndexRange
{
	/**
	 * @brief The first index in the range.
	 */
	uint32_t start;

	/**
	 * @brief The number of elements in the range.
	 */
	uint32_t count;
} dsIndexRange;

/**
 * @brief Type of the sort comparisson function.
 * @param left The left item.
 * @param right The right item.
 * @param context The context passed with the sort.
 * @return < 0 if left < right, > 9 if left > right, or 0 if left == right.
 */
typedef int (*dsSortCompareFunction)(const void* left, const void* right, void* context);

/**
 * @brief Type of the logging function.
 * @remark This may be called across multiple threads.
 * @param userData User data for the logging function.
 * @param level The level of the message.
 * @param tag The tag for the message.
 * @param file The file for the message.
 * @param line The line for the message.
 * @param function The function for the message.
 * @param message The log message.
 */
typedef void (*dsLogFunction)(void* userData, dsLogLevel level, const char* tag,
	const char* file, unsigned int line, const char* function, const char* message);

/**
 * @brief Type for a function registering a thread name.
 * @param userData The user data for profiling functions.
 * @param name The name of the thread.
 */
typedef void (*dsProfileRegisterThreadFunction)(void* userData, const char* name);

/**
 * @brief Type for the function marking the start and end of a frame.
 * @remark This may be called across multiple threads.
 * @param userData The user data for profiling functions.
 */
typedef void (*dsProfileFrameFunction)(void* userData);

/**
 * @brief Type for the function pushing a profile scope.
 * @remark This may be called across multiple threads.
 * @param userData User data for the profiling functions.
 * @param localData A pointer to a void* for data unique to the call site.
 * @param type What is being profiled.
 * @param name The name for what is being profiled.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
typedef void (*dsProfilePushFunction)(void* userData, void** localData, dsProfileType type,
	const char* name, const char* file, const char* function, unsigned int line);

/**
 * @brief Type for the function popping a profile scope.
 * @remark This may be called across multiple threads.
 * @param userData User data for the profiling functions.
 * @param type What is being profiled.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
typedef void (*dsProfilePopFunction)(void* userData, dsProfileType type, const char* file,
	const char* function, unsigned int line);

/**
 * @brief Type for the function profiling a statistic.
 * @remark This may be called across multiple threads.
 * @param userData The user data for profiling functions.
 * @param localData A pointer to a void* for data unique to the call site.
 * @param category The category for the statistic.
 * @param name The name of the value.
 * @param value The value for the statistic.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
typedef void (*dsProfileStatFunction)(void* userData, void** localData, const char* category,
	const char* name, double value, const char* file, const char* function, unsigned int line);

/**
 * @brief Type for a function reporting time spent on the GPU.
 * @remark This will only be called from the main thread.
 * @param userData The user data for profiling functions.
 * @param view The name of the view being profiled.
 * @param pass The name of the pass being profiled.
 * @param timeNs The time spent for rendering in nanoseconds.
 */
typedef void (*dsProfileGpuFunction)(void* userData, const char* view, const char* pass,
	uint64_t timeNs);

/**
 * @brief Struct containing the function pointers for the profiler.
 */
typedef struct dsProfileFunctions
{
	/**
	 * @brief Thread registration function.
	 */
	dsProfileRegisterThreadFunction registerThreadFunc;

	/**
	 * @brief Frame start function.
	 */
	dsProfileFrameFunction startFrameFunc;

	/**
	 * @brief Frame end function.
	 */
	dsProfileFrameFunction endFrameFunc;

	/**
	 * @brief CPU push function.
	 */
	dsProfilePushFunction pushFunc;

	/**
	 * @brief CPU pop function.
	 */
	dsProfilePopFunction popFunc;

	/**
	 * @brief Stat registration function.
	 */
	dsProfileStatFunction statFunc;

	/**
	 * @brief GPU timing function.
	 */
	dsProfileGpuFunction gpuFunc;
} dsProfileFunctions;

/**
 * @brief Structure that holds the system data for a timer.
 * @see Timer.h
 */
typedef struct dsTimer
{
	/** Implementation specific scale. */
	double scale;
} dsTimer;

#ifdef __cplusplus
}
#endif
