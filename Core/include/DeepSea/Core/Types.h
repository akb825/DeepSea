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
#include <DeepSea/Core/Memory/Types.h>
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
 * @brief The level of the log message.
 */
typedef enum dsLogLevel
{
	dsLogLevel_Trace,	///< A trace message for status updates.
	dsLogLevel_Debug,	///< A debug message.
	dsLogLevel_Info,	///< An info message.
	dsLogLevel_Warning,	///< A warning message that could indicate a problem.
	dsLogLevel_Error,	///< An error message that indicates a problem.
	dsLogLevel_Fatal	///< A fatal message that indicates execution cannot continue.
} dsLogLevel;

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
 * @brief Structure that holds the system data for a timer.
 */
typedef struct dsTimer
{
	/** Implementation specific scale. */
	double scale;
} dsTimer;

#ifdef __cplusplus
}
#endif
