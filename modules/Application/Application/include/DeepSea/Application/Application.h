/*
 * Copyright 2017-2023 Aaron Barany
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
 * @brief Functions for working with an application.
 *
 * An application provides an interface to create and destroy windows, update and draw the
 * application, and respond to events. The underlying implementation may differ, such as SDL,
 * Qt, or a platform-specifc GUI API.
 *
 * @see dsApplication
 */

/**
 * @brief Adds a window responder to an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responder The responder to add.
 * @return The ID of the responder, or 0 if an error occurred.
 */
DS_APPLICATION_EXPORT uint32_t dsApplication_addWindowResponder(dsApplication* application,
	const dsWindowResponder* responder);

/**
 * @brief Removes a window responder from an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responderID The ID of the responder to remove.
 * @return True if the responder was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeWindowResponder(dsApplication* application,
	uint32_t responderID);

/**
 * @brief Adds an event responder to an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responder The responder to add.
 * @return The ID of the responder, or 0 if an error occurred.
 */
DS_APPLICATION_EXPORT uint32_t dsApplication_addEventResponder(dsApplication* application,
	const dsEventResponder* responder);

/**
 * @brief Removes an event responder from an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responderID The ID of the responder to remove.
 * @return True if the responder was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeEventResponder(dsApplication* application,
	uint32_t responderID);

/**
 * @brief Sets the pre-input update function for the application.
 *
 * This function will be called before processing input and before the update function. This can be
 * used for potentially expensive operations before input is processed to reduce input latency. Any
 * CPU-based framerate limiting should be done inside this function.
 *
 * The update time passed won't include the previous frame's pre-input update function. When using
 * this to perform CPU-side framerate limiting, this should sleep for the difference between the
 * target frame time and the update time.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param function The udpate function.
 * @param userData The user data to provide to the function.
 * @return True if the function was set.
 */
DS_APPLICATION_EXPORT bool dsApplication_setPreInputUpdateFunction(dsApplication* application,
	dsUpdateApplicationFunction function, void* userData);

/**
 * @brief Sets the update function for the application.
 *
 * This function will be called before drawing all of the windows.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param function The udpate function.
 * @param userData The user data to provide to the function.
 * @return True if the function was set.
 */
DS_APPLICATION_EXPORT bool dsApplication_setUpdateFunction(dsApplication* application,
	dsUpdateApplicationFunction function, void* userData);

/**
 * @brief Sets the function for finishing a frame in the application.
 *
 * This function will be called after drawing all of the windows.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param function The finish frame function.
 * @param userData The user data to provide to the function.
 * @return True if the function was set.
 */
DS_APPLICATION_EXPORT bool dsApplication_setFinishFrameFunction(dsApplication* application,
	dsFinishApplicationFrameFunction function, void* userData);

/**
 * @brief Adds an existing window to the application.
 *
 * This is useful when more control is required when setting up a window. For example, if it's
 * actually a sub-element of a surrounding window that also contains widgets.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param window The window to add.
 * @return True if the window was added.
 */
DS_APPLICATION_EXPORT bool dsApplication_addWindow(dsApplication* application, dsWindow* window);

/**
 * @brief Removes a window from the application without destroying it.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param window The window to remove.
 * @return True if the window was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeWindow(dsApplication* application, dsWindow* window);

/**
 * @brief Adds an existing game input to the application.
 *
 * This is usually called by the implementation and not directly.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param gameInput The game input device to add.
 * @return True if the game input that was added.
 */
DS_APPLICATION_EXPORT bool dsApplication_addGameInput(dsApplication* application,
	dsGameInput* gameInput);

/**
 * @brief Removes a game input from the application without destroying it.
 *
 * THis is usually called by the implementation and not directly.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param gameInput The game input device to remove.
 * @return True if the game input was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeGameInput(dsApplication* application,
	dsGameInput* gameInput);

/**
 * @brief Adds an existing motion sensor to the application.
 *
 * This is usually called by the implementation and not directly.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param sensor The motion sensor device to add.
 * @return True if the motion sensor that was added.
 */
DS_APPLICATION_EXPORT bool dsApplication_addMotionSensor(dsApplication* application,
	dsMotionSensor* sensor);

/**
 * @brief Removes a motion sensor from the application without destroying it.
 *
 * THis is usually called by the implementation and not directly.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param sensor The motion sensor to remove.
 * @return True if the motion sensor was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeMotionSensor(dsApplication* application,
	dsMotionSensor* sensor);

/**
 * @brief Shows a message box and blocks execution until it's dismissed.
 * @param application The application.
 * @param parentWindow The parent window for the dialog, or NULL to be unparented.
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
DS_APPLICATION_EXPORT uint32_t dsApplication_showMessageBox(dsApplication* application,
	dsWindow* parentWindow, dsMessageBoxType type, const char* title, const char* message,
	const char* const* buttons, uint32_t buttonCount, uint32_t enterButton, uint32_t escapeButton);

/**
 * @brief Runs the application.
 * @param application The application.
 * @return The error code of the application. This is intended to be returned from main().
 */
DS_APPLICATION_EXPORT int dsApplication_run(dsApplication* application);

/**
 * @brief Quits the application.
 * @remark The application may not quit until the next iteration of the update loop.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param exitCode The exit code for the application.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_quit(dsApplication* application, int exitCode);

/**
 * @brief Adds a custom event to be placed on the event queue.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param window The window associated with the event.
 * @param event The custom event to queue.
 * @return False if the event couldn't be added.
 */
DS_APPLICATION_EXPORT bool dsApplication_addCustomEvent(dsApplication* application,
	dsWindow* window, const dsCustomEvent* event);

/**
 * @brief Gets the current event time.
 *
 * This can be used to find what an event's time is relative to the current one, as it is
 * implementation defined what the start point is.
 *
 * @remark Some implementations use 32-bit internal timers. For example, a 32-bit millisecond value
 * will wrap around in ~49 days. While unlikely to occur in real-world scenarios, a sanity check
 * (e.g. clamping relative times so they don't become negative) can be used to minimize the impact.
 *
 * @remark Some implementations use
 * @param application The application.
 * @return The current event time in seconds.
 */
DS_APPLICATION_EXPORT double dsApplication_getCurrentEventTime(const dsApplication* application);

/**
 * @brief Gets the current power state of the system.
 *
 * This will return the overview of the power state, whether there's a battery and if it's charging,
 * and optionally the remaining time in seconds and the percent of the battery.
 *
 * @remark Some devices may only report time left or percent remaining, so it is best to query both.
 * Additionally, the values are estimates and may not be accurate at times or for older and less
 * reliable hardware. For example, if you want to show a low battery warning, you may want to check
 * for < 10 minutes or 10% remaining across multiple queries over several seconds.
 *
 * @param[out] outRemainingTime The remaining time on the battery in seconds, or -1 if it cannot be
 *     determined. This may be NULL ifi not needed.
 * @param[out] outBatteryPercent The percent of the battery, or -1 if it cannot be determined.
 *     This may be NULL ifi not needed.
 * @param application The application.
 * @return The current power state, determining if a battery is present and if it is charging.
 */
DS_APPLICATION_EXPORT dsSystemPowerState dsApplication_getPowerState(int* outRemainingTime,
	int* outBatteryPercent, const dsApplication* application);

/**
 * @brief Gets the bounds for the display in display coordinates.
 * @remark errno will be set on failure.
 * @param[out] outBounds The bounds for the display.
 * @param application The application.
 * @param display The index of the display.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_getDisplayBounds(dsAlignedBox2i* outBounds,
	const dsApplication* application, uint32_t display);

/**
 * @brief Adjusts the size of the window based on the DPI.
 *
 * On Linux and Windows this will multiply the size based on the DPI of the display compared to the
 * reference DPI. Other platforms won't perform this step, since it's either already done by the OS
 * (for macOS) or doesn't apply due to not having standard windows. (e.g. Android)
 *
 * @param application The application.
 * @param display The index of the display.
 * @param size The size to adjust.
 * @return The adjusted size.
 */
DS_APPLICATION_EXPORT uint32_t dsApplication_adjustWindowSize(const dsApplication* application,
	uint32_t display, uint32_t size);

/**
 * @brief Adjusts an arbitary size based on the DPI.
 *
 * This will multiply the size based on the DPI of the display compared to the reference DPI.
 *
 * @param application The application.
 * @param display The index of the display.
 * @param size The size to adjust.
 * @return The adjusted size.
 */
DS_APPLICATION_EXPORT float dsApplication_adjustSize(const dsApplication* application,
	uint32_t display, float size);

/**
 * @brief Gets the cursor used by the application.
 * @param application The application.
 * @return The cursor.
 */
DS_APPLICATION_EXPORT dsCursor dsApplication_getCursor(const dsApplication* application);

/**
 * @brief Sets the cursor used by the application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param cursor The new cursor type.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_setCursor(dsApplication* application, dsCursor cursor);

/**
 * @brief Gets whether or not the cursor is hidden.
 *
 * This is used for the OS-provided cursor. For example, the hardware cursor could be hidden and a
 * texture shown for the cursor instead.
 *
 * @param application The application.
 * @return True if the cursor is hidden.
 */
DS_APPLICATION_EXPORT bool dsApplication_getCursorHidden(const dsApplication* application);

/**
 * @brief Sets whether or not the cursor is hidden.
 *
 * This is used for the OS-provided cursor. For example, the hardware cursor could be hidden and a
 * texture shown for the cursor instead.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param hidden True if the cursor is hidden.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_setCursorHidden(dsApplication* application, bool hidden);

/**
 * @brief Gets whether or not a keyboard key is currently pressed.
 * @param application The application.
 * @param key The key to check.
 * @return True if the key is pressed.
 */
DS_APPLICATION_EXPORT bool dsApplication_isKeyPressed(const dsApplication* application,
	dsKeyCode key);

/**
 * @brief Gets the currently pressed key modifiers.
 * @param application The application.
 * @return The currently pressed modifiers.
 */
DS_APPLICATION_EXPORT dsKeyModifier dsApplication_getKeyModifiers(const dsApplication* application);

/**
 * @brief Begins accepting text input.
 * @remark errno will be set on failure.
 * @param application The application.
 * @return False if input couldn't be begun.
 */
DS_APPLICATION_EXPORT bool dsApplication_beginTextInput(dsApplication* application);

/**
 * @brief Ends accepting text input.
 * @remark errno will be set on failure.
 * @param application The application.
 * @return False if input couldn't be ended.
 */
DS_APPLICATION_EXPORT bool dsApplication_endTextInput(dsApplication* application);

/**
 * @brief Sets the editing rectangle for editing text.
 *
 * This is generally used for suggestions for unicode entry.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param rect The renctangle to edit text in.
 * @return False if the input rectangle couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsApplication_setTextInputRect(dsApplication* application,
	const dsAlignedBox2i* rect);

/**
 * @brief Gets the current position of the mouse.
 * @remark errno will be set on failure.
 * @param[out] outPosition The position of the mouse in screen coordinates.
 * @param application The application.
 * @return False if the position couldn't be queried.
 */
DS_APPLICATION_EXPORT bool dsApplication_getMousePosition(dsVector2i* outPosition,
	const dsApplication* application);

/**
 * @brief Sets the current position of the mouse.
 *
 * This may generate a mouse move event.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param window The window to set the mouse position relative to. If NULL, it will be in screen
 *     coordinates.
 * @param position The position of the mouse.
 */
DS_APPLICATION_EXPORT bool dsApplication_setMousePosition(dsApplication* application,
	dsWindow* window, const dsVector2i* position);

/**
 * @brief Gets the currently pressed mouse buttons.
 * @param application The application.
 * @return A bitmask of the currently pressed mouse buttons.
 */
DS_APPLICATION_EXPORT uint32_t dsApplication_getPressedMouseButtons(
	const dsApplication* application);

/**
 * @brief Gets the window with focus.
 * @param application The application.
 * @return The window with focus, or NULL if no window has focus.
 */
DS_APPLICATION_EXPORT dsWindow* dsApplication_getFocusWindow(const dsApplication* application);

/**
 * @brief Dispatches an event through the application.
 *
 * This is usually called by the application, but could also be used to generate fake events.
 *
 * @param application The application.
 * @param window The window that the event originated fromm.
 * @param event The event to send.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_dispatchEvent(dsApplication* application, dsWindow* window,
	const dsEvent* event);

/**
 * @brief Initializes the members of an application.
 *
 * This will initiialize all members to 0 and set up any internal structures. This is called by the
 * application implementation.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_initialize(dsApplication* application);

/**
 * @brief Destroys the private members of an application.
 *
 * This is called by the application implementation. The following pointers are freed:
 * - windowResponders
 * - eventResponders
 * - windows
 * - controllers
 * Only the base pointers are destroyed, not the elements within the arrays.
 *
 * @param application The application.
 */
DS_APPLICATION_EXPORT void dsApplication_shutdown(dsApplication* application);

#ifdef __cplusplus
}
#endif
