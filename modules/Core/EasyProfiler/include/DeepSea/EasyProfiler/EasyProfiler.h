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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Types.h>
#include <DeepSea/EasyProfiler/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to set up profiling using easy_profiler.
 *
 * This will hook up the DeepSea profiling hooks to go through easy_profiler. See
 * https://github.com/yse/easy_profiler for more information.
 */

/**
 * @brief Default port to listen to.
 */
#define DS_DEFAULT_EASY_PROFILER_PORT 28077

/**
 * @brief Starts profiling through easy_profiler.
 * @remark errno will be set on failure.
 * @return False if an error occurred.
 */
DS_EASYPROFILER_EXPORT bool dsEasyProfiler_start(void);

/**
 * @brief Stops profiling through easy_profiler.
 * @remark errno will be set on failure.
 */
DS_EASYPROFILER_EXPORT bool dsEasyProfiler_stop(void);

/**
 * @brief Starts listening for network connections.
 * @remark errno will be set on failure.
 * @param port The port to listen to.
 * @return False if an error occurred.
 */
DS_EASYPROFILER_EXPORT bool dsEasyProfiler_startListening(uint16_t port);

/**
 * @brief Stops listening for network connections.
 * @remark errno will be set on failure.
 * @return False if an error occurred.
 */
DS_EASYPROFILER_EXPORT bool dsEasyProfiler_stopListening(void);

/**
 * @brief Dumps profiling information to a file.
 * @remark errno will be set on failure.
 * @param filePath The path to the file to dump the profiling info to.
 * @return False if an error occurred.
 */
DS_EASYPROFILER_EXPORT bool dsEasyProfiler_dumpToFile(const char* filePath);

#ifdef __cplusplus
}
#endif
