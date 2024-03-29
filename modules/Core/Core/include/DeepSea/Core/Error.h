/*
 * Copyright 2016-2022 Aaron Barany
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
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for handling error codes and additional error code values.
 */

/**
 * @brief Error code for an index that is out of range.
 */
#define EINDEX 10000

/**
 * @brief Error code for a size that is invalid.
 */
#define ESIZE 10001

/**
 * @brief Error code for invalid file format.
 */
#define EFORMAT 10002

/**
 * @brief Error for an element not found.
 */
#define ENOTFOUND 10003

/**
 * @brief Executes a statement and prints an error if it fails.
 *
 * This will do the following:
 * 1. Execute the statement provided.
 * 2. If the function returns false, prints an error message with the function call and the
 *    error string for errno.
 * 3. Return the result of the function. This allows calling code to perform additional opeations
 *    based on whether or not the call statement succeeded.
 *
 * @param tag The log tag for the message.
 * @param statement The statement to execute. This will usually be a function call.
 * @return The bool result of the stement.
 */
#define DS_CHECK(tag, statement) \
	dsPerformCheck(tag, __FILE__, __LINE__, __FUNCTION__, (statement), #statement)

/**
 * @brief Executes a statement and prints an error if it fails.
 *
 * This will do the following:
 * 1. Execute the statement provided.
 * 2. If the function returns false, prints an error message with the function call and the
 *    error string for errno.
 * 3. Return the result of the function. This allows calling code to perform additional opeations
 *    based on whether or not the call statement succeeded.
 *
 * @param tag The log tag for the message.
 * @param statement The statement to execute. This can be a function call or previous result.
 * @param failStr The string to log on failure.
 * @return The bool result of the stement.
 */
#define DS_CHECK_MESSAGE(tag, statement, failStr) \
	dsPerformCheck(tag, __FILE__, __LINE__, __FUNCTION__, (statement), failStr)

/**
 * @brief Gets the string for an error number.
 *
 * This is thread-safe, using a statically allocated thread-local buffer for the string. This also
 * handles custom error codes.
 *
 * @param errorCode The error code to get the string for. This will typically be errno.
 * @return A string for the error number.
 */
DS_CORE_EXPORT const char* dsErrorString(int errorCode);

/**
 * @brief Performs the check for the DS_CHECK macro.
 * @param tag The tag for the message.
 * @param file The file for the message.
 * @param line The line for the message.
 * @param result The result of the statement.
 * @param statementStr The string form of the statement.
 * @param function The function for the message.
 */
DS_CORE_EXPORT bool dsPerformCheck(const char* tag, const char* file,
	unsigned int line, const char* function, bool result, const char* statementStr);

#ifdef __cplusplus
}
#endif
