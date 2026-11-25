/*
 * Copyright 2026 Aaron Barany
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
#include <DeepSea/Application/Export.h>
#include <DeepSea/Application/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manageing a Windows HDR display manager.
 *
 * The HDR display manager keeps track of the HDR state for each display, which can in turn be used
 * to determine whether HDR is available for a window and the luminance if it is.
 */

#if DS_WINDOWS || defined(DOXYGEN)

/**
 * @brief Windows HDR display manager.
 */
typedef struct dsWindowsHDRDisplayManager dsWindowsHDRDisplayManager;

/**
 * @brief Creates a Windows HDR display manager.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the HDR display manager with. This must support freeing
 *     memeory.
 * @return The HDR display manager or NULL if an error occurred.
 */
DS_APPLICATION_EXPORT dsWindowsHDRDisplayManager* dsWindowsHDRDisplayManager_create(
	dsAllocator* allocator);

/**
 * @brief Updates the state of an HDR display manager.
 *
 * This may be moderately expensive, so it is best called selectively when an update may be
 * reasonably expected, such as a display being added/removed or a window gaining focus.
 *
 * @remark errno will be set on failure.
 * @param displayManager The HDR display manager to update.
 * @return Whether the displays could be updated.
 */
DS_APPLICATION_EXPORT bool dsWindowsHDRDisplayManager_update(
	dsWindowsHDRDisplayManager* displayManager);

/**
 * @brief Gets the luminances for a display.
 * @remark errno will be set on failure.
 * @param[out] outReferenceNits The refernce luminance value in nits.
 * @param[out] outMaxNits The maximum luminance value in nits.
 * @param displayManager The HDR display manager.
 * @param monitor The monitor to get the luminance values for.
 * @return Whether the display could be found.
 */
DS_APPLICATION_EXPORT bool dsWindowsHDRDisplayManager_getDisplayLuminances(float* outReferenceNits,
	float* outMaxNits, const dsWindowsHDRDisplayManager* displayManager, const void* monitor);

/**
 * @brief Destroys an HDR display manager.
 * @param displayManager The HDR display manager to destroy.
 */
DS_APPLICATION_EXPORT void dsWindowsHDRDisplayManager_destroy(
	dsWindowsHDRDisplayManager* displayManager);

#endif // DS_WINDOWS || defined(DOXYGEN)

#ifdef __cplusplus
}
#endif
