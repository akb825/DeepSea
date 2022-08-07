/*
 * Copyright 2022 Aaron Barany
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
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for querying random bytes from the device.
 */

/**
 * @brief Queries random bytes from the device.
 *
 * This provides true random bytes, or as close as possible on the current device, and is suitable
 * for cryptographic purposes.
 *
 * Intended uses of the output of this function include, but are not limited to:
 * - Seeding a random number generator (either cryptographic or non-cryptographic depending on the
 *   requirements) for fast generation of bulk random numbers.
 * - Initialization vector for block encryption algorithms.
 * - Random IDs, such as a session ID, that cannot be predicted.
 *
 * This is intended to be used relatively infrequently and relatively small sizes. For bulk
 * generation of random numbers, a dedicated random number generator (such as dsRandom when security
 * isn't a requirement, or a separate cryptographic random number generator when it is) is highly
 * recommended.
 *
 * @remark errno will be set on failure.
 * @remark This function is thread safe.
 * @param[out] outData Storage for the output.
 * @param size The size of the output in bytes.
 * @return False if an error occurred.
 */
DS_CORE_EXPORT bool dsDeviceRandomBytes(void* outData, size_t size);

#ifdef __cplusplus
}
#endif
