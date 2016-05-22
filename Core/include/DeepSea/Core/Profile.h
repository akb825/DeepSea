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
 */

#if DS_PROFILING_ENABLED

/**
 * @brief Marks the start of a frame.
 */
#define DS_PROFILE_FRAME_START() dsProfile_startFrame(__FILE__, __FUNCTION__, __LINE__)

/**
 * @brief Marks the end of a frame.
 */
#define DS_PROFILE_FRAME_END() dsProfile_endFrame(__FILE__, __FUNCTION__, __LINE__)

/**
 * @brief Profiles the start of a function.
 */
#define DS_PROFILE_FUNC_START() dsProfile_push(dsProfileType_Function, __FUNCTION__, \
	__FILE__, __FUNCTION__, __LINE__)

/**
 * @brief Profiles the start of a function with a custon name.
 * @param name The custom function name.
 */
#define DS_PROFILE_FUNC_START_NAME(name) dsProfile_push(dsProfileType_Function, name,  __FILE__, \
	__FUNCTION__, __LINE__)

/**
 * @brief Returns void from a profiled function.
 */
#define DS_PROFILE_FUNC_RETURN_VOID() \
	do \
	{ \
		dsProfile_pop(dsProfileType_Function, __FILE__, __FUNCTION__, __LINE__); \
		return; \
	} while(0)

/**
 * @brief Returns from a profiled function.
 * @param retVal The return value.
 */
#define DS_PROFILE_FUNC_RETURN(retVal) \
	do \
	{ \
		dsProfile_pop(dsProfileType_Function, __FILE__, __FUNCTION__, __LINE__); \
		return retVal; \
	} while(0)

/**
 * @brief Profiles the start of a scope.
 * @param name The name of the scope.
 */
#define DS_PROFILE_SCOPE_START(name) dsProfile_push(dsProfileType_Scope, name, __FILE__, \
	__FUNCTION__, __LINE__)

/**
 * @brief Profiles the end of a scope.
 */
#define DS_PROFILE_SCOPE_END() dsProfile_pop(dsProfileType_Scope, __FILE__,  __FUNCTION__, __LINE__)

/**
 * @brief Profiles the start of a wait.
 * @param name The name of what's being waited on.
 */
#define DS_PROFILE_WAIT_START(name) dsProfile_push(dsProfileType_Wait, name, __FILE__, \
	__FUNCTION__, __LINE__)

/**
 * @brief Profiles the end of a wait.
 */
#define DS_PROFILE_WAIT_END() dsProfile_pop(dsProfileType_Wait, __FILE__,  __FUNCTION__, __LINE__)

/**
 * @brief Profiles the start of a lock.
 * @param name The name of what's being locked.
 */
#define DS_PROFILE_LOCK_START(name) dsProfile_push(dsProfileType_Lock, name, __FILE__, \
	__FUNCTION__, __LINE__)

/**
 * @brief Profiles the end of a lock.
 */
#define DS_PROFILE_LOCK_END() dsProfile_pop(dsProfileType_Lock, __FILE__,  __FUNCTION__, __LINE__)

/**
 * @brief Profiles a statistic.
 * @param category The category of the statistic.
 * @param name The name of the statistic.
 * @param value The value of the statistic.
 */
#define DS_PROFILE_STAT(category, name, value) dsProfile_stat(category, name, value, __FILE__, \
		__FUNCTION__, __LINE__)

#else
#define DS_PROFILE_FRAME_START() do {} while(0)
#define DS_PROFILE_FRAME_END() do {} while(0)
#define DS_PROFILE_FUNC_START() do {} while(0)
#define DS_PROFILE_FUNC_START_NAME(name) do {} while(0)
#define DS_PROFILE_FUNC_RETURN_VOID() return
#define DS_PROFILE_FUNC_RETURN(retVal) return retVal
#define DS_PROFILE_SCOPE_START(name) do {} while(0)
#define DS_PROFILE_SCOPE_END() do {} while(0)
#define DS_PROFILE_WAIT_START(name) do {} while(0)
#define DS_PROFILE_WAIT_END() do {} while(0)
#define DS_PROFILE_LOCK_START(name) do {} while(0)
#define DS_PROFILE_LOCK_END() do {} while(0)
#define DS_PROFILE_STAT(category, name, value) do {} while(0)
#endif

/**
 * @brief Sets the functions used by the profiler.
 *
 * If any function is NULL, this will return false and not change the state of the functions.
 *
 * @param userData Data provided to each profiler function.
 * @param startFrameFunc Function called at the start of a frame.
 * @param endFrameFunc Function called at the end of a frame.
 * @param pushFunc The function called when pushing a profile scope.
 * @param popFunc The function called when popping a profile scope.
 * @param statFunc The function called when profiling a statistic.
 * @return False if not all of the functions are provided.
 */
DS_CORE_EXPORT bool dsProfile_setFunctions(void* userData, dsProfileFrameFunction startFrameFunc,
	dsProfileFrameFunction endFrameFunc, dsProfilePushFunction pushFunc,
	dsProfilePopFunction popFunc, dsProfileStatFunction statFunc);

/**
 * @brief Gets the user data for the profiler.
 * @return The user data.
 */
DS_CORE_EXPORT void* dsProfile_getUserData();

/**
 * @brief Gets the start frame function.
 * @return The start frame function.
 */
DS_CORE_EXPORT dsProfileFrameFunction dsProfile_getStartFrameFunction();

/**
 * @brief Gets the end frame function.
 * @return The end frame function.
 */
DS_CORE_EXPORT dsProfileFrameFunction dsProfile_getEndFrameFunction();

/**
 * @brief Gets the profile push function.
 * @return The push function.
 */
DS_CORE_EXPORT dsProfilePushFunction dsProfile_getPushFunction();

/**
 * @brief Gets the profile pop function.
 * @return The pop function.
 */
DS_CORE_EXPORT dsProfilePopFunction dsProfile_getPopFunction();

/**
 * @brief Gets the profile stat function.
 * @return The stat function.
 */
DS_CORE_EXPORT dsProfileStatFunction dsProfile_getStatFunction();

/**
 * @brief Clears the functions used by the profiler.
 *
 * All profile operations will be NOPs.
 */
DS_CORE_EXPORT void dsProfile_clearFunctions();

/**
 * @brief Returns whether or not profiling is enabled.
 *
 * Profiling is disabled when no functions are set.
 *
 * @return True if profiling is enabled.
 */
DS_CORE_EXPORT bool dsProfile_enabled();

/**
 * @brief Marks the start of a frame.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
DS_CORE_EXPORT void dsProfile_startFrame(const char* file, const char* function, unsigned int line);

/**
 * @brief Marks the end of a frame.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
DS_CORE_EXPORT void dsProfile_endFrame(const char* file, const char* function, unsigned int line);

/**
 * @brief Pushes a profile scope.
 * @param type What is being profiled.
 * @param name The name for what is being profiled.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
DS_CORE_EXPORT void dsProfile_push(dsProfileType type, const char* name, const char* file,
	const char* function, unsigned int line);

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
 * @param category The category for the statistic.
 * @param name The name of the value.
 * @param value The value for the statistic.
 * @param file The name of the source file.
 * @param function The function calling this.
 * @param line The line of the function call.
 */
DS_CORE_EXPORT void dsProfile_stat(const char* category, const char* name, double value,
	const char* file, const char* function, unsigned int line);

#ifdef __cplusplus
}
#endif
