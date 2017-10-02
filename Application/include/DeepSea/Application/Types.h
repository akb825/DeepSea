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
#include <DeepSea/Application/MouseEventTypes.h>
#include <DeepSea/Application/KeyboardEventTypes.h>
#include <DeepSea/Application/TouchEventTypes.h>
#include <DeepSea/Core/Types.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Application library.
 */

/**
 * @brief Default DPI for the current platform.
 */
#if DS_APPLE
#define DS_DEFAULT_DPI 72.0f
#else
#define DS_DEFAULT_DPI 96.0f
#endif

/**
 * @brief Log tag used by the render library.
 */
#define DS_APPLICATION_LOG_TAG "application"

/**
 * @brief Enum for the type of an event.
 */
typedef enum dsEventType
{
	dsEventType_MouseButtonDown, ///< Mouse button was pressed. The mouseButton field will be set.
	dsEventType_MouseButtonUp,   ///< Mouse button was released. The mouseButton field will be set.
	dsEventType_MouseMove,       ///< Mouse was moved. The mouseMove field will be set.
	dsEventType_MouseWheel,      ///< Mouse scroll wheel was moved. The mouseMove field will be set.
	dsEventType_KeyDown,         ///< Keyboard key was pressed. The key field will be set.
	dsEventType_KeyUp,           ///< Keyboard key was released. The key field will be set.
	dsEventType_TextEdit,        ///< Text is being edited. The textEdit field will be set.
	dsEventType_TextInput,       ///< Text has been input. The textInput field will be set.
	dsEventType_Touch,           ///< Touchscreen was used. The touch field will be set.
	dsEventType_MultiTouch,      ///< Multi-touch gesture was input. The multiTouch field will be
	                             ///< set.
	dsEventType_WindowShown,     ///< A window has been shown. No event field will be set.
	dsEventType_WindowHidden,    ///< A window has been hidden. No event field will be set.
	dsEventType_WindowMinimized, ///< A window has been minimized. No event field will be set.
	dsEventType_WindowRestored,  ///< A window has been restored after minimized. No event field
	                             ///< will be set.
	dsEventType_WindowResized,   ///< A window has been resized. The resize field will be set.
	dsEventType_MouseEnter,      ///< Mouse has entered a window. No event field will be set.
	dsEventType_MouseLeave,      ///< Mouse has leaved a window. No event field will be set.
	dsEventType_FocusGained,     ///< Window focus has been gained. No event field will be set.
	dsEventType_FocusLost,       ///< Window focus has been lost. No event field will be set.
} dsEventType;

/**
 * @brief Enum describing the style of a window.
 */
typedef enum dsWindowStyle
{
	dsWindowStyle_Normal,     ///< Normal window.
	dsWindowStyle_FullScreen, ///< Full-screen window, changing the desktop resolution if necessary.

	/**
	 * Standard window that's drawn full-screen without a boarder. Avoids capturing the screen and
	 * changing the desktop resolution.
	 */
	dsWindowStyle_FullScreenBorderless
} dsWindowStyle;

/**
 * @brief Flags for the behavior of the window.
 */
typedef enum dsWindowFlags
{
	dsWindowFlags_None = 0,         ///< No special flags.
	dsWindowFlags_Hidden = 0x1,     ///< Window is hidden.
	dsWindowFlags_Resizeable = 0x2, ///< Window can be resized.
	dsWindowFlags_Minimized = 0x4,  ///< Window is minimized.
	dsWindowFlags_Maximize = 0x8,   ///< Window is maximized.
	dsWindowFlags_GrabInput = 0x10, ///< Grab input and lock to the window.
	dsWindowFlags_Center = 0x20     ///< Center the window on the target display.
} dsWindowFlags;

/**
 * @brief Enum for the cursor to be displayed.
 */
typedef enum dsCursor
{
	dsCursor_Arrow,     ///< Default arrow cursor.
	dsCursor_IBeam,     ///< I-beam cursor usually used for text.
	dsCursor_Wait,      ///< Wait cursor.
	dsCursor_Crosshair, ///< Crosshair cursor.
	dsCursor_WaitArrow, ///< Arrow cursor with a wait icon.
	dsCursor_SizeTLBR,  ///< Size arrow pointer to the top-left and bottom-right.
	dsCursor_SizeTRBL,  ///< Size arrow pointer to the top-right and bottom-left.
	dsCursor_SizeTB,    ///< Size arrow pointer to the top and bottom.
	dsCursor_SizeLR,    ///< Size arrow pointer to the left and right.
	dsCursor_SizeAll,   ///< Size arrow pointer in all directions.
	dsCursor_No,        ///< No cursor, such as a circle with a slash.
	dsCursor_Hand       ///< Hand cursor.
} dsCursor;

/**
 * @brief Enum for the type of controller.
 */
typedef enum dsControllerType
{
	dsControllerType_Unknown,     ///< Unknown type.
	dsControllerType_Controller,  ///< Standard game controller.
	dsControllerType_Wheel,       ///< Racing wheel.
	dsControllerType_ArcadeStick, ///< Arcade stick.
	dsControllerType_FlightStick, ///< Flight simulator stick.
	dsControllerType_DancePad,    ///< Dance pad.
	dsControllerType_Guitar,      ///< Guitar controller.
	dsControllerType_DrumKit,     ///< Drum kit controller.
	dsControllerType_ArcadePad,   ///< Arcade pad.
	dsControllerType_Throttle     ///< Throttle control.
} dsControllerType;

/**
 * @brief Enum for the battery level of a controller.
 */
typedef enum dsControllerBattery
{
	dsControllerBattery_Unknown, ///< Unknown battery level.
	dsControllerBattery_Empty,   ///< Battery is empty.
	dsControllerBattery_Low,     ///< Battery is low.
	dsControllerBattery_Medium,  ///< Battery is partially depleted.
	dsControllerBattery_Full,    ///< Battery is fully charged.
	dsControllerBattery_Wired    ///< Controller is wired.
} dsControllerBattery;

/**
 * @brief Struct containing information about the mode for a display.
 */
typedef struct dsDisplayMode
{
	/**
	 * @brief The index of the display.
	 */
	uint32_t displayIndex;

	/**
	 * @brief The width in pixels.
	 */
	uint32_t width;

	/**
	 * @brief The height in pixels.
	 */
	uint32_t height;

	/**
	 * @brief The refresh rate of the display.
	 */
	uint32_t refreshRate;
} dsDisplayMode;

/**
 * @brief Struct containing information about a display.
 */
typedef struct dsDisplayInfo
{
	/**
	 * @brief The name of the display.
	 */
	const char* name;

	/**
	 * @brief The display modes.
	 */
	const dsDisplayMode* displayModes;

	/**
	 * @brief The number of display modes.
	 */
	uint32_t displayModeCount;

	/**
	 * @brief The display DPI.
	 *
	 * This can be compared to DS_DEFAULT_DPI to determine a scale factor.
	 */
	float dpi;

	/**
	 * @brief The dimensions of the display.
	 */
	dsAlignedBox2i dimensions;
} dsDisplayInfo;

/**
 * @brief Struct containing information about a window being resized.
 */
typedef struct dsResizeEvent
{
	/**
	 * @brief The new width of the window.
	 */
	uint32_t width;

	/**
	 * @brief The new height of the window.
	 */
	uint32_t height;
} dsResizeEvent;

/**
 * @brief Base object for an application that uses DeepSea.
 *
 * Application implementations can effectively subclass this type by having it as the first member
 * of the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsApplication and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 *
 * @see Application.h
 */
typedef struct dsApplication dsApplication;

/**
 * @brief Base object for a window that displays graphics.
 *
 * Window implementations can effectively subclass this type by having it as the first member
 * of the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsWindow and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 *
 * @see Window.h
 */
typedef struct dsWindow dsWindow;

/**
 * @brief Struct containing information about an event from the application.
 */
typedef struct dsEvent
{
	/**
	 * @brief The type of the event.
	 */
	dsEventType type;

	union
	{
		/**
		 * @brief Information about a mouse button press or release.
		 *
		 * This is set for dsEventType_MouseButtonUp and dsEventType_MouseButtonDown.
		 */
		dsMouseButtonEvent mouseButton;

		/**
		 * @brief Information about a mouse movement.
		 *
		 * This is set for dsEventType_MouseMove.
		 */
		dsMouseMoveEvent mouseMove;

		/**
		 * @brief Information about a mouse movement.
		 *
		 * This is set for dsEventType_MouseWheel.
		 */
		dsMouseMoveEvent mouseWheel;

		/**
		 * @brief Information about a key press or release.
		 *
		 * This is set for dsEventType_KeyDown and dsEventType_KeyDown.
		 */
		dsKeyEvent key;

		/**
		 * @brief Information about text being edited.
		 *
		 * This is set for dsEventType_TextEdit.
		 */
		dsTextEditEvent textEdit;

		/**
		 * @brief Information about text being input.
		 *
		 * This is set for dsEventType_TextInput.
		 */
		dsTextInputEvent textInput;

		/**
		 * @brief Information about a touch input.
		 *
		 * This is set for dsEventType_Touch.
		 */
		dsTouchEvent touch;

		/**
		 * @brief Information about a mutli-touch gesture.
		 *
		 * This is set for dsEventType_MultiTouch.
		 */
		dsMultiTouchEvent multiTouch;

		/**
		 * @brief Information about a window resize.
		 */
		dsResizeEvent resize;
	};
} dsEvent;

/**
 * @brief Struct containing information about a controller.
 *
 * Controller implementations can effectively subclass this type by having it as the first member
 * of the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsWindow and the true internal type.
 */
typedef struct dsController
{
	/**
	 * @brief The application this was created with.
	 */
	dsApplication* application;

	/**
	 * @brief The allocator the this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The name of the controller.
	 */
	const char* name;

	/**
	 * @brief The type of the controller.
	 */
	dsControllerType type;

	/**
	 * @brief The number of axes on the controller.
	 */
	uint32_t axisCount;

	/**
	 * @brief The number of buttons on the controller.
	 */
	uint32_t buttonCount;

	/**
	 * @brief The number of balls on the controller, generally for joysticks.
	 */
	uint32_t ballCount;

	/**
	 * @brief The number of hats on the controller, generally for joysticks.
	 */
	uint32_t hatCount;

	/**
	 * @brief True if rumble is supported.
	 */
	bool rumbleSupported;
} dsController;

/**
 * @brief Function called when a window is added.
 * @param application The application.
 * @param window The window that was added.
 * @param userData The user data registered with the function.
 */
typedef void (*dsWindowAddedFunction)(dsApplication* application, dsWindow* window, void* userData);

/**
 * @brief Function called when a window is removed.
 *
 * This is called immediately before the window is destroyed.
 *
 * @param application The application.
 * @param window The window that was removed.
 * @param userData The user data registered with the function.
 */
typedef void (*dsWindowRemovedFunction)(dsApplication* application, dsWindow* window,
	void* userData);

/**
 * @brief Function to respond to an event.
 * @param application The application.
 * @param window The window that recieved the event.
 * @param event The event that was recieved.
 * @param userData The user data registered with the function.
 * @return True to continue passing the event, false to avoid sending any further.
 */
typedef bool (*dsWindowEventFunction)(dsApplication* application, dsWindow* window,
	const dsEvent* event, void* userData);

/**
 * @brief Function to update the application.
 * @param application The application.
 * @param lastFrameTime The time it took to execute the last frame in seconds.
 * @param userData The user data registered with the function.
 */
typedef void (*dsUpdateApplicationFunction)(dsApplication* application, double lastFrameTime,
	void* userData);

/**
 * @brief Function to draw a window.
 * @param application The application.
 * @param window The window to draw.
 * @param userData The user data registered with the function.
 */
typedef void (*dsDrawWindowFunction)(dsApplication* application, dsWindow* window, void* userData);

/**
 * @brief Function to respond to a window close request.
 * @param window The window to be closed.
 * @param userData The user data registered with the function.
 * @return True to close the window, false to leave it open.
 */
typedef bool (*dsWindowCloseFunction)(dsWindow* window, void* userData);

/**
 * @brief Function for running an application.
 * @param application The application.
 * @return The return code.
 */
typedef int (*dsRunApplicationFunction)(dsApplication* application);

/**
 * @brief Function for quitting the application.
 * @param application The qpplication.
 * @param returnCode The return code for the application.
 */
typedef void (*dsQuitApplicationFunction)(dsApplication* application, int returnCode);

/**
 * @brief Function to get the current cursor.
 * @param application The application.
 * @return The current cursor.
 */
typedef dsCursor (*dsGetApplicationCursorFunction)(const dsApplication* application);

/**
 * @brief Function to set the current cursor.
 * @param application The application.
 * @param cursor The cursor.
 * @return False if the cursor couldn't be set.
 */
typedef bool (*dsSetApplicationCursorFunction)(dsApplication* application, dsCursor cursor);

/**
 * @brief Function to get whether or not the cursor is hidden.
 * @param application The application.
 * @return True if the cursor is hidden.
 */
typedef bool (*dsGetApplicationCursorHiddenFunction)(const dsApplication* application);

/**
 * @brief Function to set whether or not the cursor is hidden.
 * @param application The application.
 * @param hidden True if the cursor is hidden.
 * @return False if the cursor couldn't be hidden or unhidden.
 */
typedef bool (*dsSetApplicationCursorHiddenFunction)(dsApplication* application, bool hidden);

/**
 * @brief Function for getting whether or not a key is pressed.
 * @param application The application.
 * @param key The key to check.
 * @return True if the key is pressed.
 */
typedef bool (*dsIsApplicationKeyPressedFunction)(const dsApplication* application, dsKeyCode key);

/**
 * @brief Function for getting the currently pressed key modifiers.
 * @param application The application.
 * @return The currently pressed modifiers.
 */
typedef dsKeyModifier (*dsGetApplicationKeyModifiersFunction)(const dsApplication* application);

/**
 * @brief Function for beginning text input.
 * @param application The application.
 * @return False if input couldn't be begun.
 */
typedef bool (*dsBeginApplicationTextInputFunction)(dsApplication* application);

/**
 * @brief Function for ending text input.
 * @param application The application.
 * @return False if input couldn't be ended.
 */
typedef bool (*dsEndApplicationTextInputFunction)(dsApplication* application);

/**
 * @brief Function for setting the editing rectangle for editing text.
 * @param application The application.
 * @param rect The renctangle to edit text in.
 * @return False if the input rectangle couldn't be set.
 */
typedef bool (*dsSetApplicationTextInputRectFunction)(dsApplication* application,
	const dsAlignedBox2i* rect);

/**
 * @brief Function for getting the mouse position.
 * @param[out] outPosition The position of the mouse in screen coordinates.
 * @param application The application.
 * @return False if the position couldn't be queried.
 */
typedef bool (*dsGetApplicationMousePositionFunction)(dsVector2i* outPosition,
	const dsApplication* application);

/**
 * @brief Function for setting the mouse position.
 * @param application The application.
 * @param window The window to set the mouse position relative to. If NULL, it will be in screen
 *     coordinates.
 * @param position The position of the mouse.
 */
typedef bool (*dsSetApplicationMousePositionFunction)(dsApplication* application, dsWindow* window,
	const dsVector2i* position);

/**
 * @brief Function for getting the currently pressed mouse buttons.
 * @param application The application.
 * @return A bitmask of the currently pressed mouse buttons.
 */
typedef uint32_t (*dsGetApplicationPressedMouseButtonsFunction)(const dsApplication* application);

/**
 * @brief Function for creating a window.
 * @param application The application.
 * @param allocator The allocator to create the window with.
 * @param title The title of the window.
 * @param position The position of the window, or NULL for the default position.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param style The style of the window.
 * @param flags Combination of dsWindowFlags values to create the window.
 */
typedef dsWindow* (*dsCreateWindowFunction)(dsApplication* application, dsAllocator* allocator,
	const char* title, const dsVector2i* position, uint32_t width, uint32_t height, int flags);

/**
 * @brief Function for destroying a window.
 * @param application The application.
 * @param window The window to destroy.
 */
typedef void (*dsDestroyWindowFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function for getting the window with focus.
 * @param application The application.
 * @return The window with focus.
 */
typedef dsWindow* (*dsGetFocusWindowFunction)(const dsApplication* application);

/**
 * @brief Function for setting a window title.
 * @param application The application.
 * @param window The window to set the title on.
 * @param title The new title.
 */
typedef bool (*dsSetWindowTileFunction)(dsApplication* application, dsWindow* window,
	const char* title);

/**
 * @brief Function for setting the display mode of the window.
 * @param application The application.
 * @param window The window to set the display mode.
 * @param displayMode The new display mode.
 * @return False if the display mode couldn't be set.
 */
typedef bool (*dsSetWindowDisplayModeFunction)(dsApplication* application, dsWindow* window,
	const dsDisplayMode* displayMode);

/**
 * @brief Function for resizing a window.
 * @param application The application.
 * @param window The window to resize.
 * @param width The new width.
 * @param height The new height.
 * @return False if the window couldn't be resized.
 */
typedef bool (*dsResizeWindowFunction)(dsApplication* application, dsWindow* window, uint32_t width,
	uint32_t height);

/**
 * @brief Function for getting the size of a window.
 * @param[out] outWidth The width of the window. This may be NULL.
 * @param[out] outHeight THe height of the window. This may be NULL.
 * @param application The resi.
 * @param window The window to get the size for.
 * @return False if the size couldn't be queried.
 */
typedef bool (*dsGetWindowSizeFunction)(uint32_t* outWidth, uint32_t* outHeight,
	const dsApplication* application, const dsWindow* window);

/**
 * @brief Function for setting a window style.
 * @param application The application.
 * @param window The window to set the style.
 * @param style The new window style.
 * @return False if the window style couldn't be set.
 */
typedef bool (*dsSetWindowStyleFunction)(dsApplication* application, dsWindow* window,
	dsWindowStyle style);

/**
 * @brief Function for getting the position of a window.
 * @param[out] outPosition The position of the window.
 * @param application The application.
 * @param window The window to set the position for.
 * @return False if the window position couldn't be queried.
 */
typedef bool (*dsGetWindowPositionFunction)(dsVector2i* outPosition,
	const dsApplication* application, const dsWindow* window);

/**
 * @brief Function for setting the position of a window.
 * @param application The application.
 * @param window The window to set the position for.
 * @param position The position of the window, or NULL to use the default position.
 * @param center True to center the window.
 * @return False if the window couldn't be moved.
 */
typedef bool (*dsSetWindowPositionFunction)(dsApplication* application, dsWindow* window,
	const dsVector2i* position, bool center);

/**
 * @brief Function to get whether or not a window is hidden.
 * @param application The application.
 * @param window The window to check.
 * @return True if the window is hidden.
 */
typedef bool (*dsGetWindowHiddenFunction)(const dsApplication* application, const dsWindow* window);

/**
 * @brief Function to set whether or not a window is hidden.
 * @param application The application.
 * @param window The window to hide or unhide.
 * @param hidden True if the window is hidden.
 * @return False if the window couldn't be hidden.
 */
typedef bool (*dsSetWindowHiddenFunction)(dsApplication* application, dsWindow* window,
	bool hidden);

/**
 * @brief Function to get whether or not a window is minimized.
 * @param application The application.
 * @param window The window to check.
 * @return True if the window is minimized.
 */
typedef bool (*dsGetWindowMinimizedFunction)(const dsApplication* application,
	const dsWindow* window);

/**
 * @brief Function to get whether or not a window is maximized.
 * @param application The application.
 * @param window The window to check.
 * @return True if the window is maximized.
 */
typedef bool (*dsGetWindowMaximizedFunction)(const dsApplication* application,
	const dsWindow* window);

/**
 * @brief Function to minimize a window.
 * @param application The application.
 * @param window The window to minimize.
 * @return False if the window couldn't be minimized.
 */
typedef bool (*dsMinimizeWindowFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function to maximize a window.
 * @param application The application.
 * @param window The window to maximize.
 * @return False if the window couldn't be maximized.
 */
typedef bool (*dsMaximizeWindowFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function to restore a minimized or maximized a window.
 * @param application The application.
 * @param window The window to restore.
 * @return False if the window couldn't be restored.
 */
typedef bool (*dsRestoreWindowFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function for getting whether or not a window has grabbed input.
 * @param application The application.
 * @param window The window to check.
 * @return True if the window has grabbed input.
 */
typedef bool (*dsGetWindowGrabbedInputFunction)(const dsApplication* application,
	const dsWindow* window);

/**
 * @brief Function for setting whether or not a window has grabbed input.
 * @param application The application.
 * @param window The window to set whether or not input is grabbed.
 * @param grab True to grab input.
 * @return False if the input grab state couldn't be set.
 */
typedef bool (*dsSetWindowGrabbedInputFunction)(dsApplication* application, dsWindow* window,
	bool grab);

/**
 * @brief Function to raise a window to the top and gives it focus.
 * @param application The application.
 * @param window The window to raise.
 * @return False if the window couldn't be raised.
 */
typedef bool (*dsRaiseWindowFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function for getting the state of a controller axis.
 * @param application The application.
 * @param controller The controller to get the axis from.
 * @param axis The index of the axis.
 * @return The axis value.
 */
typedef float (*dsGetControllerAxisFunction)(const dsApplication* application,
	const dsController* controller, uint32_t axis);

/**
 * @brief Function for getting if a controller button is pressed.
 * @param application The application.
 * @param controller The controller to get the button state from.
 * @param button The button to check.
 * @return True if the button is pressed.
 */
typedef bool (*dsIsControllerButtonPressedFunction)(const dsApplication* application,
	const dsController* controller, uint32_t button);

/**
 * @brief Function for getting the hat direction.
 * @param[out] outDirection The direction the hat is in.
 * @param application The application.
 * @param controller The controller to get the hat direction from.
 * @param hat The hat to check.
 * @return False if the hat state couldn't be queried.
 */
typedef bool (*dsGetControllerHatDirectionFunction)(dsVector2i* outDirection,
	const dsApplication* application, const dsController* controller, uint32_t hat);

/**
 * @brief Function for starting rumble on a controller.
 * @param application The application.
 * @param controller The controller to start the rumble on.
 * @param strength The strength of the rumble.
 * @param duration The duration to rumble for.
 * @return False if rumble couldn't be started.
 */
typedef bool (*dsStartControllerRumbleFunction)(dsApplication* application,
	dsController* controller, float strength, float duration);

/**
 * @brief Function for stopping rumble on a controller.
 * @param application The application.
 * @param controller The controller to stop the rumble on.
 * @return False if rumble couldn't be stopped.
 */
typedef bool (*dsStopControllerRumbleFunction)(dsApplication* application,
	dsController* controller);

/**
 * @brief Struct containing information to respond to a window being added or removed.
 */
typedef struct dsWindowResponder
{
	/**
	 * @brief Function called when a window has been added.
	 */
	dsWindowAddedFunction windowAddedFunc;

	/**
	 * @brief Function called when a window will be removed.
	 */
	dsWindowRemovedFunction windowRemovedFunc;

	/**
	 * @brief User data to be passed to windowAddedFunc and windowRemovedFunc.
	 */
	void* userData;

	/**
	 * @brief The ID of the responder.
	 *
	 * This will be set when added to the application.
	 */
	uint32_t responderId;
} dsWindowResponder;

/**
 * @brief Struct containing information to respond to an event.
 */
typedef struct dsEventResponder
{
	/**
	 * @brief Function called when the event has been recieved.
	 */
	dsWindowEventFunction eventFunc;

	/**
	 * @brief User data to be passed to eventFunc.
	 */
	void* userData;

	/**
	 * @brief The priority of the responder.
	 *
	 * Lower numbers will be executed first.
	 */
	int32_t priority;

	/**
	 * @brief The ID of the responder.
	 *
	 * This will be set when added to the application.
	 */
	uint32_t responderId;
} dsEventResponder;

/** @copydoc dsApplication */
struct dsApplication
{
	/**
	 * @brief The renderer used with the application.
	 */
	dsRenderer* renderer;

	/**
	 * @brief The allocator the this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Information about the different displays.
	 */
	const dsDisplayInfo* displays;

	/**
	 * @brief The number of displays.
	 */
	uint32_t displayCount;

	/**
	 * @brief The controllers in the application.
	 */
	dsController** controllers;

	/**
	 * @brief The number of controllers.
	 */
	uint32_t controllerCount;

	/**
	 * @brief The window responders.
	 */
	dsWindowResponder* windowResponders;

	/**
	 * @brief The number of window responders.
	 */
	uint32_t windowResponderCount;

	/**
	 * @brief The number of window responders that can be held before the buffer is re-allocated.
	 */
	uint32_t windowResponderCapacity;

	/**
	 * @brief The event responders.
	 */
	dsEventResponder* eventResponders;

	/**
	 * @brief The number of event responders.
	 */
	uint32_t eventResponderCount;

	/**
	 * @brief The number of window responders that can be held before the buffer is re-allocated.
	 */
	uint32_t eventResponderCapacity;

	/**
	 * @brief The windows in the application.
	 */
	dsWindow** windows;

	/**
	 * @brief The number of windows.
	 */
	uint32_t windowCount;

	/**
	 * @brief The number of windows that can be held before the buffer is re-allocated.
	 */
	uint32_t windowCapacity;

	/**
	 * @brief Function for updating the application.
	 */
	dsUpdateApplicationFunction updateFunc;

	/**
	 * @brief User data for update function.
	 */
	void* updateUserData;

	/**
	 * @brief Function for running the application.
	 */
	dsRunApplicationFunction runFunc;

	/**
	 * @brief Function for quitting the application.
	 */
	dsQuitApplicationFunction quitFunc;

	/**
	 * @brief Function for getting the cursor.
	 */
	dsGetApplicationCursorFunction getCursorFunc;

	/**
	 * @brief Function for setting the cursor.
	 */
	dsSetApplicationCursorFunction setCursorFunc;

	/**
	 * @brief Function for getting if the cursor is hidden.
	 */
	dsGetApplicationCursorHiddenFunction getCursorHiddenFunc;

	/**
	 * @brief Function for setting if the cursor is hidden.
	 */
	dsSetApplicationCursorHiddenFunction setCursorHiddenFunc;

	/**
	 * @brief Function for getting if a key is pressed.
	 */
	dsIsApplicationKeyPressedFunction isKeyPressedFunc;

	/**
	 * @brief Function for getting the key modifiers.
	 */
	dsGetApplicationKeyModifiersFunction getKeyModifiersFunc;

	/**
	 * @brief Function for beginning text input.
	 */
	dsBeginApplicationTextInputFunction beginTextInputFunc;

	/**
	 * @brief Function for ending text input.
	 */
	dsEndApplicationTextInputFunction endTextInputFunc;

	/**
	 * @brief Function for setting the text input rectangle.
	 */
	dsSetApplicationTextInputRectFunction setTextInputRectFunc;

	/**
	 * @brief Function for getting the mouse position.
	 */
	dsGetApplicationMousePositionFunction getMousePositionFunc;

	/**
	 * @brief Function for setting the mouse position.
	 */
	dsSetApplicationMousePositionFunction setMousePositionFunc;

	/**
	 * @brief Function for getting the pressed mouse buttons.
	 */
	dsGetApplicationPressedMouseButtonsFunction getPressedMouseButtonsFunc;

	/**
	 * @brief Function for creating a window.
	 */
	dsCreateWindowFunction createWindowFunc;

	/**
	 * @brief Function for destroying a window.
	 */
	dsDestroyWindowFunction destroyWindowFunc;

	/**
	 * @brief Function to get the window with focus.
	 */
	dsGetFocusWindowFunction getFocusWindowFunc;

	/**
	 * @brief Function to set a window title.
	 */
	dsSetWindowTileFunction setWindowTitleFunc;

	/**
	 * @brief Function for setting the window display mode.
	 */
	dsSetWindowDisplayModeFunction setWindowDisplayModeFunc;

	/**
	 * @brief Function for resizing a window.
	 */
	dsResizeWindowFunction resizeWindowFunc;

	/**
	 * @brief Function for getting the size fo a window.
	 */
	dsGetWindowSizeFunction getWindowSizeFunc;

	/**
	 * @brief Function for setting the window style.
	 */
	dsSetWindowStyleFunction setWindowStyleFunc;

	/**
	 * @brief Function for getting the window position.
	 */
	dsGetWindowPositionFunction getWindowPositionFunc;

	/**
	 * @brief Function for setting the window position.
	 */
	dsSetWindowPositionFunction setWindowPositionFunc;

	/**
	 * @brief Function for getting if a window is hidden.
	 */
	dsGetWindowHiddenFunction getWindowHiddenFunc;

	/**
	 * @brief Function for setting if a window is hidden.
	 */
	dsSetWindowHiddenFunction setWindowHiddenFunc;

	/**
	 * @brief Function for getting if a window is minimized.
	 */
	dsGetWindowMinimizedFunction getWindowMinimizedFunc;

	/**
	 * @brief Function for getting if a window is maximized.
	 */
	dsGetWindowMaximizedFunction getWindowMaximizedFunc;

	/**
	 * @brief Function for minimizing a window.
	 */
	dsMinimizeWindowFunction minimizeWindowFunc;

	/**
	 * @brief Function for maximizing a window.
	 */
	dsMaximizeWindowFunction maximizeWindowFunc;

	/**
	 * @brief Function for restoring a window.
	 */
	dsRestoreWindowFunction restoreWindowFunc;

	/**
	 * @brief Function for getting if a window has input grabbed.
	 */
	dsGetWindowGrabbedInputFunction getWindowGrabbedInputFunc;

	/**
	 * @brief Function for setting if a window has input grabbed.
	 */
	dsSetWindowGrabbedInputFunction setWindowGrabbedInputFunc;

	/**
	 * @brief Function for raising a window.
	 */
	dsRaiseWindowFunction raiseWindowFunc;

	/**
	 * @brief Function for getting a controller axis.
	 */
	dsGetControllerAxisFunction getControllerAxisFunc;

	/**
	 * @brief Function for getting if a controller button is pressed.
	 */
	dsIsControllerButtonPressedFunction isControllerButtonPressedFunc;

	/**
	 * @brief Function for getting the controller hat direction.
	 */
	dsGetControllerHatDirectionFunction getControllerHatDirectionFunc;

	/**
	 * @brief Function for starting rumble on a controller.
	 */
	dsStartControllerRumbleFunction startControllerRumbleFunc;

	/**
	 * @brief Function for stopping rumble on a controller.
	 */
	dsStopControllerRumbleFunction stopControllerRumbleFunc;
};

/** @copydoc dsWindow */
struct dsWindow
{
	/**
	 * @brief The application this was created with.
	 */
	dsApplication* application;

	/**
	 * @brief The allocator the this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The title of the window.
	 */
	const char* title;

	/**
	 * @brief The color surface for the window.
	 */
	dsRenderSurface* colorSurface;

	/**
	 * @brief The depth/stencil surface for the window.
	 */
	dsRenderSurface* depthStencilSurface;

	/**
	 * @brief Function for drawing the window.
	 */
	dsDrawWindowFunction drawWindowFunc;

	/**
	 * @brief User data for drawing the window.
	 */
	void* drawWindowUserData;

	/**
	 * @brief The function to intercept closing the window.
	 */
	dsWindowCloseFunction windowCloseFunc;

	/**
	 * @brief User data to provide when calling windowCloseFunc.
	 */
	void* windowCloseUserData;

	/**
	 * @brief The style of the window.
	 */
	dsWindowStyle windowStyle;

	/**
	 * @brief The display mode of the window.
	 */
	dsDisplayMode displayMode;
};

#ifdef __cplusplus
}
#endif
