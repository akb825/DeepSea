/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/Application/Types.h>
#include <DeepSea/ApplicationSDL/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @file
 * @brief Functions for creating an application that's implemented with SDL.
 */

/**
 * @brief Shows a message box and blocks execution until it's dismissed.
 *
 * This is similar to dsApplication_showMessageBox, except it doesn't require a dsApplication
 * instance. This is ideal for showing a message box if there was an error before the application
 * has been created.
 *
 * @param type The type of the message box.
 * @param title The title of the message box.
 * @param message The message to display.
 * @param buttons The list of button names.
 * @param buttonCount The number of buttons.
 * @param enterButton The index of the button to trigger with the enter key, or
 *     DS_MESSAGE_BOX_NO_BUTTON for no button.
 * @param escapeButton The index of the button to trigger with the escape key, or
 *     DS_MESSAGE_BOX_NO_BUTTON for no button.
 * @return The index of the pressed button, or DS_MESSAGE_BOX_NO_BUTTON if an error occurred.
 */
DS_APPLICATIONSDL_EXPORT uint32_t dsSDLApplication_showMessageBox(dsMessageBoxType type,
	const char* title, const char* message, const char** buttons, uint32_t buttonCount,
	uint32_t enterButton, uint32_t escapeButton);

/**
 * @brief Creates an SDL application.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the application with.
 * @param renderer The renderer to use with the application.
 * @return The application, or NULL if the application couldn't be created.
 */
DS_APPLICATIONSDL_EXPORT dsApplication* dsSDLApplication_create(dsAllocator* allocator,
	dsRenderer* renderer);

/**
 * @brief Destroys an SDL application.
 * @param application The application to destroy.
 */
DS_APPLICATIONSDL_EXPORT void dsSDLApplication_destroy(dsApplication* application);

#ifdef __cplusplus
}
#endif
