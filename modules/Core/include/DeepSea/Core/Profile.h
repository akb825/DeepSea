/*
 * Copyright 2016-2018 Aaron Barany
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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Hooks for profiling code.
 *
 * The macros should be used rather than the functions directly. This allows the profiling
 * code to be stripped out when DS_PROFILING_ENABLED is defined to 0.
 *
 * Most profiling types may use either static or dynamic names. When a choice is available, static
 * is default, with dynamic using a separate DYNAMIC version of the macro. The static name versions
 * assume that the same name will be passed in each invocation, and can optimize under this
 * assumption. The dynamic versions offer more flexibility by allowing you to provide a separate
 * name each invocation, but at the expense of higher overhead.
 */

#if DS_PROFILING_ENABLED

///\{
#define DS_PROFILE_PUSH_IMPL(type, name, dynamic) \
	do \
	{ \
		static void* _dsProfileLocalData; \
		dsProfile_push(&_dsProfileLocalData, type, name, __FILE__, __FUNCTION__, __LINE__, \
			dynamic); \
	} while(0)

#define DS_PROFILE_POP_IMPL(type) dsProfile_pop(type, __FILE__, __FUNCTION__, __LINE__)
///\}

/**
 * @brief Profiles the start of a function.
 */
#define DS_PROFILE_FUNC_START() DS_PROFILE_PUSH_IMPL(dsProfileType_Function, __FUNCTION__, false)

/**
 * @brief Profiles the start of a function with a custon name.
 * @param name The custom function name.
 */
#define DS_PROFILE_FUNC_START_NAME(name) DS_PROFILE_PUSH_IMPL(dsProfileType_Function, name, false)

/**
 * @brief Returns void from a profiled function.
 */
#define DS_PROFILE_FUNC_RETURN_VOID() \
	do \
	{ \
		DS_PROFILE_POP_IMPL(dsProfileType_Function); \
		return; \
	} while(0)

/**
 * @brief Returns from a profiled function.
 * @param retVal The return value.
 */
#define DS_PROFILE_FUNC_RETURN(retVal) \
	do \
	{ \
		DS_PROFILE_POP_IMPL(dsProfileType_Function); \
		return retVal; \
	} while(0)

/**
 * @brief Profiles the end of the function.
 *
 * In nearly all cases you want to use DS_PROFILE_FUNC_RETURN() or DS_PROFILE_FUNC_RETURN_VOID()
 * instead. One example when you would want to use this is if you need to maintain an explicit scope
 * within a function.
 */
#define DS_PROFILE_FUNC_END() DS_PROFILE_POP_IMPL(dsProfileType_Function)

/**
 * @brief Profiles the start of a scope.
 * @param name The name of the scope. This must be consistent across calls.
 */
#define DS_PROFILE_SCOPE_START(name) DS_PROFILE_PUSH_IMPL(dsProfileType_Scope, name, false)

/**
 * @brief Profiles the start of a scope with a dynamic name.
 * @param name The name of the scope.
 */
#define DS_PROFILE_DYNAMIC_SCOPE_START(name) DS_PROFILE_PUSH_IMPL(dsProfileType_Scope, name, true)

/**
 * @brief Profiles the end of a scope.
 */
#define DS_PROFILE_SCOPE_END() DS_PROFILE_POP_IMPL(dsProfileType_Scope)

/**
 * @brief Profiles the start of a wait.
 * @param name The name of what's being waited on. This must be consistent across calls.
 */
#define DS_PROFILE_WAIT_START(name) DS_PROFILE_PUSH_IMPL(dsProfileType_Wait, name, false)

/**
 * @brief Profiles the start of a wait with a dynamic name.
 * @param name The name of what's being wated on.
 */
#define DS_PROFILE_DYNAMIC_WAIT_START(name) DS_PROFILE_PUSH_IMPL(dsProfileType_Wait, name, true)

/**
 * @brief Profiles the end of a wait.
 */
#define DS_PROFILE_WAIT_END() DS_PROFILE_POP_IMPL(dsProfileType_Wait)

/**
 * @brief Profiles the start of a lock.
 * @param name The name of what's being locked. This must be consistent across calls.
 */
#define DS_PROFILE_LOCK_START(name) DS_PROFILE_PUSH_IMPL(dsProfileType_Lock, name, false)

/**
 * @brief Profiles the start of a lock with a dynamic name.
 * @param name The name of what's being locked.
 */
#define DS_PROFILE_DYNAMIC_LOCK_START(name) \
	DS_PROFILE_PUSH_IMPL(dsProfileType_Lock, name, true)

/**
 * @brief Profiles the end of a lock.
 */
#define DS_PROFILE_LOCK_END() DS_PROFILE_POP_IMPL(dsProfileType_Lock)

/**
 * @brief Profiles a statistic.
 * @remark It's assumed that the category and name paired will uniquely identify the statistic.
 * @param category The category of the statistic. This must be consistent across calls.
 * @param name The name of the statistic. This must be consistent across calls.
 * @param value The value of the statistic as a double.
 */
#define DS_PROFILE_STAT(category, name, value) \
	do \
	{ \
		static void* _dsProfileLocalData; \
		dsProfile_stat(&_dsProfileLocalData, category, name, value, __FILE__, __FUNCTION__, \
			__LINE__, false); \
	} while (0)

/**
 * @brief Profiles a statistic with a dynamic name.
 * @remark It's assumed that the category and name paired will uniquely identify the statistic.
 * @param category The category of the statistic.
 * @param name The name of the statistic.
 * @param value The value of the statistic as a double.
 */
#define DS_PROFILE_DYNAMIC_STAT(category, name, value) \
	do \
	{ \
		static void* _dsProfileLocalData; \
		dsProfile_stat(&_dsProfileLocalData, category, name, value, __FILE__, __FUNCTION__, \
			__LINE__, true); \
	} while (0)

#else
#define DS_PROFILE_FUNC_START() do {} while(0)
#define DS_PROFILE_FUNC_START_NAME(name) do {} while(0)
#define DS_PROFILE_FUNC_RETURN_VOID() return
#define DS_PROFILE_FUNC_RETURN(retVal) return retVal
#define DS_PROFILE_SCOPE_START(name) do {} while(0)
#define DS_PROFILE_DYNAMIC_SCOPE_START(name) do {} while(0)
#define DS_PROFILE_SCOPE_END() do {} while(0)
#define DS_PROFILE_WAIT_START(name) do {} while(0)
#define DS_PROFILE_DYNAMIC_WAIT_START(name) do {} while(0)
#define DS_PROFILE_WAIT_END() do {} while(0)
#define DS_PROFILE_LOCK_START(name) do {} while(0)
#define DS_PROFILE_DYNAMIC_LOCK_START(name) do {} while(0)
#define DS_PROFILE_LOCK_END() do {} while(0)
#define DS_PROFILE_STAT(category, name, value) do {} while(0)
#define DS_PROFILE_DYNAMIC_STAT(category, name, value) do {} while(0)
#endif

/**
 * @brief Sets the functions used by the profiler.
 * @param userData Data provided to each profiler function.
 * @param functions The profiler function functions. If NULL, this will be the same as calling
 *     dsProfile_clearFunctions().
 */
DS_CORE_EXPORT void dsProfile_setFunctions(void* userData, const dsProfileFunctions* functions);

/**
 * @brief Gets the user data for the profiler.
 * @return The user data.
 */
DS_CORE_EXPORT void* dsProfile_getUserData(void);

/**
 * @brief Gets the registered functions.
 * @return The registered functions.
 */
DS_CORE_EXPORT const dsProfileFunctions* dsProfile_getFunctions(void);

/**
 * @brief Clears the functions used by the profiler.
 *
 * All profile operations will be NOPs.
 */
DS_CORE_EXPORT void dsProfile_clearFunctions(void);

/**
 * @brief Registers a thread.
 * @remark This will automatically be called when dsThread_setThisThreadName() is called. It is also
 *     automatic when creating a thread using dsTHread.
 * @param name The name of the thread.
 */
DS_CORE_EXPORT void dsProfile_registerThread(const char* name);

/**
 * @brief Marks the start of a frame.
 */
DS_CORE_EXPORT void dsProfile_startFrame(void);

/**
 * @brief Marks the end of a frame.
 */
DS_CORE_EXPORT void dsProfile_endFrame(void);

/**
 * @brief Pushes a profile scope.
 * @param localData A pointer to a void* for data unique to the call site.
 * @param type What is being profiled.
 * @param name The name for what is being profiled.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 * @param dynamicName True if the name can be dynamic.
 */
DS_CORE_EXPORT void dsProfile_push(void** localData, dsProfileType type, const char* name,
	const char* file, const char* function, unsigned int line, bool dynamicName);

/**
 * @brief Pops a profile scope.
 * @param type What is being profiled.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
DS_CORE_EXPORT void dsProfile_pop(dsProfileType type, const char* file, const char* function,
	unsigned int line);

/**
 * @brief Profiles a statistic.
 * @remark It's assumed that the category and name paired will uniquely identify the statistic.
 * @param localData A pointer to a void* for data unique to the call site.
 * @param category The category for the statistic.
 * @param name The name of the value.
 * @param value The value for the statistic.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 * @param dynamicName True if the name can be dynamic.
 */
DS_CORE_EXPORT void dsProfile_stat(void** localData, const char* category, const char* name,
	double value, const char* file, const char* function, unsigned int line, bool dynamicName);

/**
 * @brief Profiles a GPU timing.
 *
 * GPU timings are always dynamic, meaning the names may change each call.
 *
 * @remark It's assumed that the category and name paired will uniquely identify the statistic.
 * @param category The category for the time.
 * @param name The name of the time.
 * @param timeNs The time spent in nanoseconds.
 */
DS_CORE_EXPORT void dsProfile_gpu(const char* category, const char* name, uint64_t timeNs);

#ifdef __cplusplus
}
#endif
