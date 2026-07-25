/*
 * Copyright 2017-2026 Aaron Barany
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
 * @brief Functions for working with a window.
 *
 * A window is a drawable surface that displays graphics drawn with a dsRenderer. It may be a
 * standalone window or a part of a larger window surrounded by widgets.
 *
 * @see dsWindow
 */

/**
 * @brief Creates a new window and adds it to the application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param allocator The allocator to create the window with. If NULL, it will use the same allocator
 *     as the application.
 * @param title The title of the window. This will be copied.
 * @param surfaceName The name of the render surface. If NULL, it will be the same as title used on
 *     creation. This will be copied.
 * @param position The position of the window. If NULL, an arbitrary position will be chosen and the
 *     window style will be dsWindowStyle_Normal.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param flags Flags to control the behavior of the window.
 * @param renderSurfaceUsage Flags to determine how the render surface for the window will be used.
 * @return The created window or NULL if an error occurred.
 */
DS_APPLICATION_EXPORT dsWindow* dsWindow_create(dsApplication* application, dsAllocator* allocator,
	const char* title, const char* surfaceName, const dsWindowInitPosition* position,
	uint32_t width, uint32_t height, dsWindowFlags flags, dsRenderSurfaceUsage renderSurfaceUsage);

/**
 * @brief Creates a surface that was delayed with the dsWindowFlags_DelaySurfaceCreate flag.
 *
 * This will do nothing if the surface was already created.
 *
 * @remark errno will be set on failure.
 * @param window The window to create the surface for.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsWindow_createSurface(dsWindow* window);

/**
 * @brief Sets the draw function for a window.
 * @remark errno will be set on failure.
 * @param window The window.
 * @param drawFunc The draw function to set.
 * @param userData The user data to provide to the draw function.
 * @param destroyUserDataFunc The function to destroy the user data when the window is
 *     destoyed, the draw function is changed, or setting the function fails.
 * @return True if the draw function was set.
 */
DS_APPLICATION_EXPORT bool dsWindow_setDrawFunction(dsWindow* window, dsDrawWindowFunction drawFunc,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Sets the function to respond to closing the window.
 *
 * This function may be used to avoid closing the window when requested by the user.
 *
 * @remark errno will be set on failure.
 * @param window The window.
 * @param closeFunc The close function to set.
 * @param userData The user data to provide to the draw function.
 * @param destroyUserDataFunc The function to destroy the user data when the window is
 *     destoyed, the close function is changed, or setting the function fails.
 * @return True if the draw function was set.
 */
DS_APPLICATION_EXPORT bool dsWindow_setCloseFunction(dsWindow* window,
	dsInterceptCloseWindowFunction closeFunc, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Sets the title of a window.
 * @remark errno will be set on failure.
 * @param window The window to set the title on.
 * @param title The new title. This will be copied.
 * @return False if the title couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsWindow_setTitle(dsWindow* window, const char* title);

/**
 * @brief Sets the display mode of the window.
 *
 * This only takes affect with full-screen windows.
 *
 * @remark errno will be set on failure.
 * @param window The window to set the display mode.
 * @param displayMode The new display mode.
 * @return False if the display mode couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsWindow_setDisplayMode(
	dsWindow* window, const dsDisplayMode* displayMode);

/**
 * @brief Resizes a window.
 *
 * This only takes affect with a normal window.
 *
 * @remark errno will be set on failure.
 * @param window The window to resize.
 * @param width The new width in display coordinates.
 * @param height The new height display coordinates.
 * @return False if the window couldn't be resized.
 */
DS_APPLICATION_EXPORT bool dsWindow_resize(dsWindow* window, uint32_t width, uint32_t height);

/**
 * @brief Sets the style of the window.
 *
 * This may resize the window or change the desktop resolution.
 *
 * @remark errno will be set on failure.
 * @param window The window to set the style.
 * @param style The new window style.
 * @return False if the window style couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsWindow_setStyle(dsWindow* window, dsWindowStyle style);

/**
 * @brief Sets the position of a window.
 * @remark errno will be set on failure.
 * @param window The window to set the position for.
 * @param position The position of the window in screen space, or NULL to use the default position.
 * @return False if the window couldn't be moved.
 */
DS_APPLICATION_EXPORT bool dsWindow_setPosition(dsWindow* window, const dsVector2i* position);

/**
 * @brief Centers the window on a display.
 * @remark errno will be set on failure.
 * @param window The window to set the position for.
 * @param display The display to center the window on, or NULL to use the primary display.
 * @return False if the window couldn't be moved.
 */
DS_APPLICATION_EXPORT bool dsWindow_center(dsWindow* window, const dsDisplayInfo* display);

/**
 * @brief Sets whether or not a window is hidden.
 * @remark errno will be set on failure.
 * @param window The window to hide or unhide.
 * @param hidden True if the window is hidden.
 * @return False if the window couldn't be hidden.
 */
DS_APPLICATION_EXPORT bool dsWindow_setHidden(dsWindow* window, bool hidden);

/**
 * @brief Minimizes a window.
 * @remark errno will be set on failure.
 * @param window The window to minimize.
 * @return False if the window couldn't be minimized.
 */
DS_APPLICATION_EXPORT bool dsWindow_minimize(dsWindow* window);

/**
 * @brief Maximizes a window.
 * @remark errno will be set on failure.
 * @param window The window to maximize.
 * @return False if the window couldn't be maximized.
 */
DS_APPLICATION_EXPORT bool dsWindow_maximize(dsWindow* window);

/**
 * @brief Restores a minimized or maximized window.
 * @remark errno will be set on failure.
 * @param window The window to maximize.
 * @return False if the window couldn't be restored.
 */
DS_APPLICATION_EXPORT bool dsWindow_restore(dsWindow* window);

/**
 * @brief Sets whether or not a window has grabbed input.
 * @remark errno will be set on failure.
 * @param window The window to set whether or not input is grabbed.
 * @param grab True to grab input.
 * @return False if the input grab state couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsWindow_setGrabbedInput(dsWindow* window, bool grab);

/**
 * @brief Sets whether or not a window is resizable.
 * @remark errno will be set on failure.
 * @param window The window to set whether or not is resizable.
 * @param resizable True to allow resizes.
 * @return False if the resizable state couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsWindow_setResizable(dsWindow* window, bool resizable);

/**
 * @brief Raises a window to the top and gives it focus.
 * @remark errno will be set on failure.
 * @param window The window to raise.
 * @return False if the window couldn't be raised.
 */
DS_APPLICATION_EXPORT bool dsWindow_raise(dsWindow* window);

/**
 * @brief Begins accepting text input on a window.
 * @remark errno will be set on failure.
 * @param window The window that will accept text input.
 * @param inputType The type of text input.
 * @param inputFlags Flags to control how the input behaves.
 * @return False if input couldn't be begun.
 */
DS_APPLICATION_EXPORT bool dsApplication_beginTextInput(
	dsWindow* window, dsWindowTextInputType inputType, dsWindowTextInputFlags inputFlags);

/**
 * @brief Ends accepting text input.
 * @remark errno will be set on failure.
 * @param window The window that accepted text input.
 * @return False if input couldn't be ended.
 */
DS_APPLICATION_EXPORT bool dsApplication_endTextInput(dsWindow* window);

/**
 * @brief Sets the editing area for editing text.
 *
 * This is generally used for suggestions for unicode entry.
 *
 * @remark errno will be set on failure.
 * @param window The window that will accept text input.
 * @param bounds The renctangle to edit text in.
 * @param cursorOffset The offset from the min x of the bounds to show the cursor.
 * @return False if the input rectangle couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsApplication_setTextInputArea(
	dsWindow* window, const dsAlignedBox2i* bounds, uint32_t cursorOffset);

/**
 * @brief Destroys a window and removes it from the application.
 * @remark errno will be set on failure.
 * @param window The window to destroy.
 * @return True if the window was destroyed.
 */
DS_APPLICATION_EXPORT bool dsWindow_destroy(dsWindow* window);

#ifdef __cplusplus
}
#endif
