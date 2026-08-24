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
#include <DeepSea/Application/GameInputEventTypes.h>
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
 * @brief Constant for no message box button.
 */
#define DS_MESSAGE_BOX_NO_BUTTON UINT32_MAX

/**
 * @brief The maximum number of buttons that can be used in a message box.
 */
#define DS_MAX_MESSAGE_BOX_BUTTONS 10

/**
 * @brief Log tag used by the application library.
 */
#define DS_APPLICATION_LOG_TAG "application"

/**
 * @brief The standard gravitational constant for use with an accelerometer.
 *
 * An acceleromoter at rest should have a magnitude roughly this magnitude.
 */
#define DS_ACCELEROMOTER_GRAVITY 9.80665f

/**
 * @brief Constant for having no player index assigned to a game input.
 */
#define DS_NO_GAME_INPUT_PLAYER UINT32_MAX

/**
 * @brief Enum for the power state of the current system.
 */
typedef enum dsSystemPowerState
{
	dsSystemPowerState_Unknown,   ///< The power state is unknown.
	dsSystemPowerState_External,  ///< The system uses external power without a battery.
	dsSystemPowerState_OnBattery, ///< The system is unplugged and running off a battery.
	dsSystemPowerState_Charging,  ///< The system is plugged in and charging its battery.
	dsSystemPowerState_Charged    ///< The system is plugged in and battery is fully charged.
} dsSystemPowerState;

/**
 * @brief Enum for the type of a motion sensor.
 */
typedef enum dsMotionSensorType
{
	dsMotionSensorType_Accelerometer,      ///< Accelleromoter to detect lateral movement.
	dsMotionSensorType_Gyroscope,          ///< Gyroscope to detect rotational movement.
	dsMotionSensorType_AccelerometerLeft,  ///< Accelleromoter for the left of a separated pair.
	dsMotionSensorType_GyroscopeLeft,      ///< Gyroscope for the left of a separated pair.
	dsMotionSensorType_AccelerometerRight, ///< Accelleromoter for the right of a separated pair.
	dsMotionSensorType_GyroscopeRight      ///< Gyroscope for the right of a separated pair.
} dsMotionSensorType;

/**
 * @brief Enum for the type of an applicatio event.
 */
typedef enum dsAppEventType
{
	/**
	 * Mouse button was pressed. The mouseButton field will be set.
	 */
	dsAppEventType_MouseButtonDown,
	/**
	 * Mouse button was released. The mouseButton field will be set.
	 */
	dsAppEventType_MouseButtonUp,
	/**
	 * Mouse was moved. The mouseMove field will be set.
	 */
	dsAppEventType_MouseMove,
	/**
	 * Mouse scroll wheel was moved. The mouseMove field will be set.
	 */
	dsAppEventType_MouseWheel,

	/**
	 * Keyboard key was pressed. The key field will be set.
	 */
	dsAppEventType_KeyDown,
	/**
	 * Keyboard key was released. The key field will be set.
	 */
	dsAppEventType_KeyUp,

	/**
	 * Text is being edited. The textEdit field will be set.
	 */
	dsAppEventType_TextEdit,
	/**
	 * Text has been input. The textInput field will be set.
	 */
	dsAppEventType_TextInput,

	/**
	 * A finger was pressed on the touchscreen.. The touch field will be set.
	 */
	dsAppEventType_TouchFingerDown,
	/**
	 * A finger was released on the touchscreen.. The touch field will be set.
	 */
	dsAppEventType_TouchFingerUp,
	/**
	 * Fingers moved on the touchscreen. The touch event will be set.
	 */
	dsAppEventType_TouchMoved,
	/**
	 * Multi-touch gesture was input. The multiTouch field will be set.
	 */
	dsAppEventType_MultiTouch,

	/**
	 * A game input device was disconnected. The gameInputConnect field will be set.
	 */
	dsAppEventType_GameInputConnected,
	/**
	 * A game input device was connected. The gameInputConnect field will be set.
	 */
	dsAppEventType_GameInputDisconnected,
	/**
	 * A game input axis was moved. The gameInputAxis field will be set.
	 */
	dsAppEventType_GameInputAxis,
	/**
	 * A game input button was pressed. The gameInputButton field will be set.
	 */
	dsAppEventType_GameInputButtonDown,
	/**
	 * A game input button was released. The gameInputButton field will be set.
	 */
	dsAppEventType_GameInputButtonUp,
	/**
	 * A game input ball was moved. The gameInputBall field will be set.
	 */
	dsAppEventType_GameInputBall,
	/**
	 * A game input D-pad or hat was moved. The gameInputDPad field will be set.
	 */
	dsAppEventType_GameInputDPad,

	/**
	 * A motion sensor has been updated. The motionSensor field will be set.
	 */
	dsAppEventType_MotionSensor,

	/**
	 * The state of a window has changed. The windowChange field will be set.
	 */
	dsAppEventType_WindowChanged,
	/**
	 * A window has been closed. The window will be hidden, and may either be kept or destroyed.
	 * The window field will be set.
	 */
	dsAppEventType_WindowClosed,
	/**
	 * A window has been destroyed by the windowing system and must be cleaned up on the application
	 * side. The dsWindow instance will be deleted after this event if the application didn't do so
	 * itself. The window filed will be set.
	 */
	dsAppEventType_WindowDestroyed,
	/**
	 * Mouse has entered a window. The window field will be set.
	 */
	dsAppEventType_MouseEntered,
	/**
	 * Mouse has leaved a window. The window field will be set.
	 */
	dsAppEventType_MouseLeft,
	/**
	 * Window focus has been gained. The window field will be set.
	 */
	dsAppEventType_FocusGained,
	/**
	 * Window focus has been lost. The window field will be set.
	 */
	dsAppEventType_FocusLost,
	/**
	 * A window surface has been invalidated and re-created. Any references to the surface must be
	 * updated. The window event field will be set.
	 */
	dsAppEventType_SurfaceInvalidated,

	/**
	 * The application will enter the background. No event field will be set.
	 */
	dsAppEventType_WillEnterBackground,
	/**
	 * The application did enter the background. No event field will be set.
	 */
	dsAppEventType_DidEnterBackground,
	/**
	 * The application will enter the foreground. No event field will be set.
	 */
	dsAppEventType_WillEnterForeground,
	/**
	 * The application did enter the foreground. No event field will be set.
	 */
	dsAppEventType_DidEnterForeground,

	/**
	 * A display has been connected. The display field will be set.
	 */
	dsAppEventType_DisplayConnected,
	/**
	 * A display has been discoonnected. The display field will be set.
	 */
	dsAppEventType_DisplayDisconnected,
	/**
	 * The bounds of a display has changed. The display field will be set.
	 */
	dsAppEventType_DisplayBoundsChanged,
	/**
	 * The scale for a display has changed. The display field will be set.
	 */
	dsAppEventType_DisplayScaleChanged,
	/**
	 * The rotation for a display has changed. The display field will be set.
	 */
	dsAppEventType_DisplayRotated,
	/**
	 * The default display mode of a display has changed. The display field will be set.
	 */
	dsAppEventType_DefaultDisplayModeChanged,
	/**
	 * The primary display has changed. The display field will be set.
	 */
	dsAppEventType_PrimaryDisplayChanged,

	/**
	 * Custom event. The custom field will be set.
	 */
	dsAppEventType_Custom,
} dsAppEventType;

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
	dsWindowFlags_None = 0,                  ///< No special flags.
	dsWindowFlags_Hidden = 0x1,              ///< Window is hidden.
	dsWindowFlags_Resizable = 0x2,           ///< Window can be resized.
	dsWindowFlags_Minimized = 0x4,           ///< Window is minimized.
	dsWindowFlags_Maximized = 0x8,           ///< Window is maximized.
	dsWindowFlags_GrabInput = 0x10,          ///< Grab input and lock to the window.
	dsWindowFlags_DelaySurfaceCreate = 0x40, ///< Delay surface creation until explicitly created.

	/**
	 * Collection of flags that may change dynamically based on the window state.
	 */
	dsWindowFlags_EventMask = dsWindowFlags_Hidden | dsWindowFlags_Minimized |
		dsWindowFlags_Maximized,

	/**
	 * Collection of flags that are only used during initial creation. These will be stripped when
	 * storing the flags for the final window state.
	 */
	dsWindowFlags_InitOnlyMask = dsWindowFlags_DelaySurfaceCreate
} dsWindowFlags;

/**
 * @brief Enum for the type of initial position for a window.
 */
typedef enum dsWindowInitPositionType
{
	/**
	 * Default position with no field set. This will be treated the same as passing NULL for the
	 * init position, but may be more convenient for calling code if the position is conditionally
	 * set.
	 */
	dsWindowInitPositionType_Default,
	dsWindowInitPositionType_Location,          ///< Location as a dsVector2i in screen coordinates.
	dsWindowInitPositionType_DisplayCenter,     ///< The center of a given display.
	dsWindowInitPositionType_DisplayFullScreen, ///< Full screen for a given display mode.
	/**
	 * Standard window drawn full-screen without a border on a given display.
	 */
	dsWindowInitPositionType_DisplayFullScreenBorderless
} dsWindowInitPositionType;

/**
 * @brief The type of text to input into a window.
 */
typedef enum dsWindowTextInputType
{
	dsWindowTextInputType_Default,  ///< Standard text.
	dsWindowTextInputType_Name,     ///< Name or title, where each word is capitalized.
	dsWindowTextInputType_Sentence, ///< Full sentences.
	dsWindowTextInputType_AllCaps,  ///< All letters capitalized.
	dsWindowTextInputType_Email,    ///< e-mail address.
	dsWindowTextInputType_Number    ///< Numerical value.
} dsWindowTextInputType;

/**
 * @brief Flags for the behavior of text input.
 */
typedef enum dsWindowTextInputFlags
{
	dsWindowTextInputFlags_None = 0,          ///< No special flags.
	dsWindowTextInputFlags_Password = 0x1,    ///< Password value that should be hidden.
	dsWindowTextInputFlags_Autocorrect = 0x2, ///< Perform auto-correct for words.
	dsWindowTextInputFlags_Multiline = 0x4    ///< Multiple lines.
} dsWindowTextInputFlags;

/**
 * @brief Flags for changes in a window state within an event.
 *
 * These changes are often inter-related, so they are sent with a single event rather than trying
 * to maintain each change individually.
 */
typedef enum dsWindowChangeFlags
{
	dsWindowChangeFlags_Hidden = 0x1,        ///< The hidden state was toggled.
	dsWindowChangeFlags_Minimized = 0x2,     ///< The minimized state was toggled.
	dsWindowChangeFlags_Maximized = 0x4,     ///< The maximized state was toggled.
	dsWindowChangeFlags_Style = 0x8,         ///< The style of the window has changed.
	dsWindowChangeFlags_Position = 0x10,     ///< The position of the window has changed.
	dsWindowChangeFlags_Size = 0x20,         ///< The size of the window has changed.
	dsWindowChangeFlags_SurfaceSize = 0x40,  ///< The size of the render surface has changed.
	dsWindowChangeFlags_ContentScale = 0x80, ///< The content scale of the window has changed.
	dsWindowChangeFlags_SafeArea = 0x100,    ///< The safe area within the window has changed.
	dsWindowChangeFlags_Display = 0x200      ///< The display of the window has changed.
} dsWindowChangeFlags;

/**
 * @brief Enum for the type of a message box.
 */
typedef enum dsMessageBoxType
{
	dsMessageBoxType_Info,    ///< Information box.
	dsMessageBoxType_Warning, ///< Non-critical warning.
	dsMessageBoxType_Error    ///< Error message.
} dsMessageBoxType;

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
	dsCursor_SizeT,     ///< Size arrow pointer at the top.
	dsCursor_SizeB,     ///< Size arrow pointer at the bottom.
	dsCursor_SizeL,     ///< Size arrow pointer at the left.
	dsCursor_SizeR,     ///< Size arrow pointer at the right.
	dsCursor_Move,      ///< Size arrow pointer in all directions.
	dsCursor_No,        ///< No cursor, such as a circle with a slash.
	dsCursor_Hand,      ///< Hand cursor.
	dsCursor_Count      ///< The number of cursors.
} dsCursor;

/**
 * @brief Enum for the type of game input device.
 *
 * This includes game controllers as well as other form factors such as joysticks and racing wheels.
 */
typedef enum dsGameInputType
{
	dsGameInputType_Unknown,                     ///< Fully unknown type and form factor.
	dsGameInputType_Wheel,                       ///< Racing wheel.
	dsGameInputType_ArcadeStick,                 ///< Arcade stick.
	dsGameInputType_FlightStick,                 ///< Flight simulator stick.
	dsGameInputType_DancePad,                    ///< Dance pad.
	dsGameInputType_Guitar,                      ///< Guitar controller.
	dsGameInputType_DrumKit,                     ///< Drum kit controller.
	dsGameInputType_ArcadePad,                   ///< Arcade pad.
	dsGameInputType_Throttle,                    ///< Throttle control.
	dsGameInputType_UnknownController,           ///< Game controller of unknown type.
	dsGameInputType_XBox360Controller,           ///< XBox 360 game controller.
	dsGameInputType_XBoxOneController,           ///< XBox One game controller.
	dsGameInputType_PS3Controller,               ///< PS3 game controller.
	dsGameInputType_PS4Controller,               ///< PS4 game controller.
	dsGameInputType_PS5Controller,               ///< PS5 game controller.
	dsGameInputType_NintendoSwitchProController, ///< Nintendo Switch Pro or controller.
	dsGameInputType_NintendoSwitchJoyconLeft,    ///< Left joycon only for the Nintendo Switch.
	dsGameInputType_NintendoSwitchJoyconRight,   ///< Right joycon only for the Nintendo Switch.
	dsGameInputType_NintendoSwitchJoyconPair,    ///< Pair of joycons for the Nintendo Switch.
	dsGameInputType_GameCubeController           ///< GameCube controller.
} dsGameInputType;

/**
 * @brief Enum for the type of rumble on a game input.
 */
typedef enum dsGameInputRumble
{
	dsGameInputRumble_LowFrequency,  ///< Low-frequency rumble.
	dsGameInputRumble_HighFrequency, ///< High-frequency rumble.
	dsGameInputRumble_LeftTrigger,   ///< Rumble on the left trigger.
	dsGameInputRumble_RightTrigger   ///< Rumble on the right trugger.
} dsGameInputRumble;

/// @cond
typedef struct dsApplication dsApplication;
typedef struct dsWindow dsWindow;
typedef struct dsGameInput dsGameInput;
typedef struct dsMotionSensor dsMotionSensor;
/// @endcond

/**
 * @brief Struct containing information about the mode for a display.
 */
typedef struct dsDisplayMode
{
	/**
	 * @brief The ID of the display.
	 */
	uint64_t displayID;

	/**
	 * @brief The width in pixels.
	 */
	uint32_t width;

	/**
	 * @brief The height in pixels.
	 */
	uint32_t height;

	/**
	 * @brief The refresh rate of the display in Hz.
	 */
	float refreshRate;
} dsDisplayMode;

/**
 * @brief Struct containing information about a display.
 * @remark None of the members should be modified outside of the implementation.
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
	 * @brief The ID for the display.
	 */
	uint64_t id;

	/**
	 * @brief The number of display modes.
	 */
	uint32_t displayModeCount;

	/**
	 * @brief The default display mode.
	 */
	uint32_t defaultMode;

	/**
	 * @brief The scale for contents within the display.
	 *
	 * This is a scaling factor that's usually user-configurable per display to adjust the size of
	 * the UI per display.
	 */
	float scale;

	/**
	 * @brief The rotation of the display.
	 */
	dsRenderSurfaceRotation rotation;

	/**
	 * @brief The bounds on the overall desktop.
	 */
	dsAlignedBox2i desktopBounds;

	/**
	 * @brief The usable bounds on the overall desktop.
	 *
	 * This is similar to desktopBounds, but subtracts static elements such as global menu bar,
	 * dock, or taskbar.
	 */
	dsAlignedBox2i usableBounds;
} dsDisplayInfo;

/**
 * @brief Struct describing the initial position for a window.
 */
typedef struct dsWindowInitPosition
{
	/**
	 * @brief The type of position.
	 *
	 * This value will be used to determine the initial window style and what field of the union is
	 * set.
	 */
	dsWindowInitPositionType type;

	union
	{
		/**
		 * @brief The position of the window in screen coordinates.
		 *
		 * This will be set when type is dsWindowInitPositionType_Location.
		 */
		dsVector2i position;

		/**
		 * @brief The display for the window.
		 *
		 * This will be set when type is dsWindowInitPositionType_DisplayCenter or
		 * dsWindowInitPositionType_DisplayFullScreenBorderless. When NULL, the primary display will
		 * be implicitly used.
		 */
		const dsDisplayInfo* display;

		/**
		 * @brief The display mode for the window.
		 *
		 * This will be set when type is dsWindowInitPositionType_DisplayFullScreen.
		 */
		const dsDisplayMode* displayMode;
	};
} dsWindowInitPosition;

/**
 * @brief Struct containing information about a window state change.
 */
typedef struct dsWindowChangedEvent
{
	/**
	 * @brief The window that was changed.
	 */
	dsWindow* window;

	/**
	 * @brief The flags denoting what has changed.
	 */
	dsWindowChangeFlags flags;
} dsWindowChangedEvent;

/**
 * @brief Function for cleaning up a custom event.
 * @param eventID User-specified ID describing the event.
 * @param userData The user data to clean up.
 */
typedef void (*dsCustomEventCleanupFunction)(uint32_t eventID, void* userData);

/**
 * @brief Struct containing information about a motion sensor event.
 */
typedef struct dsMotionSensorEvent
{
	/**
	 * @brief The window the that has focus for motion events.
	 */
	const dsWindow* window;

	/**
	 * @brief The motion sensor, or NULL if part of a game input.
	 */
	const dsMotionSensor* sensor;

	/**
	 * @brief The game input device, or NULL if part of a dedicated motion sensor.
	 */
	const dsGameInput* gameInput;

	/**
	 * @brief The type of sensor.
	 */
	dsMotionSensorType type;

	/**
	 * @brief The sensor data.
	 */
	dsVector3f data;
} dsMotionSensorEvent;

/**
 * @brief Struct containing information about a custom event.
 */
typedef struct dsCustomEvent
{
	/**
	 * @brief User-specified ID describing the event.
	 */
	uint32_t eventID;

	/**
	 * @brief The window associated with the event.
	 */
	const dsWindow* window;

	/**
	 * @brief User data provided with the event.
	 */
	void* userData;

	/**
	 * @brief Function for cleaning up the event user data.
	 */
	dsCustomEventCleanupFunction cleanupFunc;
} dsCustomEvent;

/**
 * @brief Struct containing information about an event from the application.
 */
typedef struct dsEvent
{
	/**
	 * @brief The type of the event.
	 */
	dsAppEventType type;

	/**
	 * @brief The relative time of this event in timer ticks.
	 *
	 * This will be relative to the absolute timer ticks reported in the update function.
	 */
	uint64_t time;

	union
	{
		/**
		 * @brief Information about a mouse button press or release.
		 *
		 * This is set for dsAppEventType_MouseButtonDown and dsAppEventType_MouseButtonUp.
		 */
		dsMouseButtonEvent mouseButton;

		/**
		 * @brief Information about a mouse movement.
		 *
		 * This is set for dsAppEventType_MouseMove.
		 */
		dsMouseMoveEvent mouseMove;

		/**
		 * @brief Information about a mouse movement.
		 *
		 * This is set for dsAppEventType_MouseWheel.
		 */
		dsMouseWheelEvent mouseWheel;

		/**
		 * @brief Information about a key press or release.
		 *
		 * This is set for dsAppEventType_KeyDown and dsAppEventType_KeyUp.
		 */
		dsKeyEvent key;

		/**
		 * @brief Information about text being edited.
		 *
		 * This is set for dsAppEventType_TextEdit.
		 */
		dsTextEditEvent textEdit;

		/**
		 * @brief Information about text being input.
		 *
		 * This is set for dsAppEventType_TextInput.
		 */
		dsTextInputEvent textInput;

		/**
		 * @brief Information about a touch input.
		 *
		 * This is set for dsAppEventType_TouchFingerDown, dsAppEventType_TouchFingerUp, and
		 * dsAppEventType_TouchMoved.
		 */
		dsTouchEvent touch;

		/**
		 * @brief Information about a mutli-touch gesture.
		 *
		 * This is set for dsAppEventType_MultiTouch.
		 */
		dsMultiTouchEvent multiTouch;

		/**
		 * @brief The game input being connected or disconnected.
		 *
		 * This is set for dsAppEventType_GameInputConnected and
		 * dsAppEventType_GameInputDisconnected.
		 */
		const dsGameInput* gameInputConnect;

		/**
		 * @brief Information about a game input axis being moved.
		 *
		 * This is set for dsAppEventType_GameInputAxis.
		 */
		dsGameInputAxisEvent gameInputAxis;

		/**
		 * @brief Information about a controller button being pressed or released.
		 *
		 * This is set for dsAppEventType_GameInputButtonDown and
		 * dsAppEventType_GameInputButtonDown.
		 */
		dsGameInputButtonEvent gameInputButton;

		/**
		 * @brief Information about a game input ball being moved.
		 *
		 * This is set for dsAppEventType_GameInputBall.
		 */
		dsGameInputBallEvent gameInputBall;

		/**
		 * @brief Information about a game input D-pad being moved.
		 *
		 * This is set for dsAppEventType_GameInputDPad.
		 */
		dsGameInputDPadEvent gameInputDPad;

		/**
		 * @brief Information about a motion sensor updated.
		 *
		 * This is set for dsAppEventType_MotionSensor.
		 */
		dsMotionSensorEvent motionSensor;

		/**
		 * @brief Information about a change for window state.
		 *
		 * This is set for dsAppEventType_WindowChanged.
		 */
		dsWindowChangedEvent windowChange;

		/**
		 * @brief Window for events reacting to changes on a window.
		 *
		 * This is set for dsAppEventType_MouseEntered, dsAppEvent_MouseLeft,
		 * dsAppEvent_FocusGained, dsAppEvent_FocusLost, dsAppEventType_SurfaceInvalidated,
		 * dsAppEvent_WindowClosed, and dsAppEvent_WindowDestroyed.
		 */
		const dsWindow* window;

		/**
		 * @brief Display for events reacting to changes on a display.
		 *
		 * This is set for dsAppEventType_DisplayConnected, sAppEventType_DisplayDisconnected,
		 * dsAppEventType_DisplayBoundsChanged, dsAppEventType_DisplayDPIChanged,
		 * dsAppEventType_DisplayRotated, dsAppEventType_DefaultDisplayModeChanged, and
		 * dsAppEventType_PrimaryDisplayChanged,
		 */
		const dsDisplayInfo* display;

		/**
		 * @brief Information about a custom event.
		 *
		 * This is set for dsAppEventType_Custom.
		 */
		dsCustomEvent custom;
	};
} dsEvent;

/**
 * @brief Function to update the application.
 * @param application The application.
 * @param absoluteTime The absolute time in timer ticks. Events will be relative to this time.
 * @param lastFrameTime The time it took to execute the last frame in timer ticks.
 * @param userData The user data registered with the function.
 */
typedef void (*dsUpdateApplicationFunction)(
	dsApplication* application, uint64_t absoluteTime, uint64_t lastFrameTime, void* userData);

/**
 * @brief Function to finish the frame in the application.
 * @param application The application.
 * @param userData The user data registered with the function.
 */
typedef void (*dsFinishApplicationFrameFunction)(dsApplication* application, void* userData);

/**
 * @brief Function to set the update rate of the application.
 * @param application The application.
 * @param updateRate The desired update rate.
 * @return False if the update rate couldn't be set.
 */
typedef bool (*dsSetApplicationUpdateRateFunction)(dsApplication* application, float updateRate);

/**
 * @brief Function to show a message box.
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
typedef uint32_t (*dsShowApplicationMessageBoxFunction)(dsApplication* application,
	dsWindow* parentWindow, dsMessageBoxType type, const char* title, const char* message,
	const char* const* buttons, uint32_t buttonCount, uint32_t enterButton, uint32_t escapeButton);

/**
 * @brief Function for quitting the application.
 * @param application The qpplication.
 * @param returnCode The return code for the application.
 */
typedef void (*dsQuitApplicationFunction)(dsApplication* application, int returnCode);

/**
 * @brief Function to add a custom event.
 * @param application The application.
 * @param event The custom event to queue.
 * @return False if the event couldn't be added.
 */
typedef bool (*dsAddCustomApplicationEventFunction)(
	dsApplication* application, const dsCustomEvent* event);

/**
 * @brief Function for getting the current power state of the system.
 * @param[out] outRemainingTime The remaining time on the battery in seconds, or -1 if it cannot be
 *     determined. This may be NULL ifi not needed.
 * @param[out] outBatteryPercent The percent of the battery, or -1 if it cannot be determined.
 *     This may be NULL ifi not needed.
 * @param application The application.
 * @return The current power state, determining if a battery is present and if it is charging.
 */
typedef dsSystemPowerState (*dsGetApplicationPowerStateFunction)(
	int* outRemainingTime, int* outBatteryPercent, const dsApplication* application);

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
typedef void (*dsWindowRemovedFunction)(
	dsApplication* application, dsWindow* window, void* userData);

/**
 * @brief Function to respond to an event.
 * @param application The application.
 * @param event The event that was recieved.
 * @param userData The user data registered with the function.
 * @return True to continue passing the event, false to avoid sending any further.
 */
typedef bool (*dsApplicationEventFunction)(
	dsApplication* application, const dsEvent* event, void* userData);

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
 * @brief Function for getting the mouse position.
 * @param[out] outPosition The position of the mouse in screen coordinates.
 * @param application The application.
 * @return False if the position couldn't be queried.
 */
typedef bool (*dsGetApplicationMousePositionFunction)(
	dsVector2f* outPosition, const dsApplication* application);

/**
 * @brief Function for setting the mouse position.
 * @param application The application.
 * @param window The window to set the mouse position relative to. If NULL, it will be in screen
 *     coordinates.
 * @param position The position of the mouse.
 */
typedef bool (*dsSetApplicationMousePositionFunction)(
	dsApplication* application, dsWindow* window, const dsVector2f* position);

/**
 * @brief Function for getting the currently pressed mouse buttons.
 * @param application The application.
 * @return A bitmask of the currently pressed mouse buttons.
 */
typedef uint32_t (*dsGetApplicationPressedMouseButtonsFunction)(const dsApplication* application);

/**
 * @brief Function to destroy an application.
 * @param application The application to destroy.
 */
typedef void (*dsDestroyApplicationFunction)(dsApplication* application);

/**
 * @brief Function for creating a window.
 * @param application The application.
 * @param allocator The allocator to create the window with.
 * @param title The title of the window.
 * @param surfaceName The name of the render surface.
 * @param position The position of the window, or NULL for the default position.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param flags The flags to control creation of the window.
 * @param renderSurfaceUsage Flags to determine how the render surface for the window will be used.
 */
typedef dsWindow* (*dsCreateWindowFunction)(dsApplication* application, dsAllocator* allocator,
	const char* title, const char* surfaceName, const dsWindowInitPosition* position,
	uint32_t width, uint32_t height, dsWindowFlags flags, dsRenderSurfaceUsage renderSurfaceUsage);

/**
 * @brief Function for destroying a window.
 * @param application The application.
 * @param window The window to destroy.
 * @return False if the window couldn't be destroyed.
 */
typedef bool (*dsDestroyWindowFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function for creating a window surface when initial creation was delayed.
 * @param application The application.
 * @param window The window to create the surface for.
 * @return False if the surface couldn't be created.
 */
typedef bool (*dsCreateWindowSurfaceFunction)(dsApplication* application, dsWindow* window);

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
typedef bool (*dsSetWindowTileFunction)(
	dsApplication* application, dsWindow* window, const char* title);

/**
 * @brief Function for setting the display mode of the window.
 * @param application The application.
 * @param window The window to set the display mode.
 * @param displayMode The new display mode.
 * @return False if the display mode couldn't be set.
 */
typedef bool (*dsSetWindowDisplayModeFunction)(
	dsApplication* application, dsWindow* window, const dsDisplayMode* displayMode);

/**
 * @brief Function for resizing a window.
 * @param application The application.
 * @param window The window to resize.
 * @param width The new width.
 * @param height The new height.
 * @return False if the window couldn't be resized.
 */
typedef bool (*dsResizeWindowFunction)(
	dsApplication* application, dsWindow* window, uint32_t width, uint32_t height);

/**
 * @brief Function for setting a window style.
 * @param application The application.
 * @param window The window to set the style.
 * @param style The new window style.
 * @return False if the window style couldn't be set.
 */
typedef bool (*dsSetWindowStyleFunction)(
	dsApplication* application, dsWindow* window, dsWindowStyle style);

/**
 * @brief Function for setting the position of a window.
 * @param application The application.
 * @param window The window to set the position for.
 * @param position The position of the window, or NULL to use the default position.
 * @return False if the window couldn't be moved.
 */
typedef bool (*dsSetWindowPositionFunction)(
	dsApplication* application, dsWindow* window, const dsVector2i* position);

/**
 * @brief Function for centering .
 * @param application The application.
 * @param window The window to set the position for.
 * @param display The display to center the window on, or NULL to use the primary display.
 * @return False if the window couldn't be moved.
 */
typedef bool (*dsCenterWindowFunction)(
	dsApplication* application, dsWindow* window, const dsDisplayInfo* display);

/**
 * @brief Function to set whether or not a window is hidden.
 * @param application The application.
 * @param window The window to hide or unhide.
 * @param hidden True if the window is hidden.
 * @return False if the window couldn't be hidden.
 */
typedef bool (*dsSetWindowHiddenFunction)(
	dsApplication* application, dsWindow* window, bool hidden);

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
 * @brief Function for setting whether or not a window has grabbed input.
 * @param application The application.
 * @param window The window to set whether or not input is grabbed.
 * @param grab True to grab input.
 * @return False if the input grab state couldn't be set.
 */
typedef bool (*dsSetWindowGrabbedInputFunction)(
	dsApplication* application, dsWindow* window, bool grab);

/**
 * @brief Function for setting whether or not a window is resizable.
 * @param application The application.
 * @param window The window to set whether or not is resizable.
 * @param resizable True to allow resizes.
 * @return False if the resizable state couldn't be set.
 */
typedef bool (*dsSetWindowResizableFunction)(
	dsApplication* application, dsWindow* window, bool resizable);

/**
 * @brief Function to raise a window to the top and gives it focus.
 * @param application The application.
 * @param window The window to raise.
 * @return False if the window couldn't be raised.
 */
typedef bool (*dsRaiseWindowFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function for beginning text input.
 * @param application The application.
 * @param window The window that will accept text input.
 * @param inputType The type of text input.
 * @param inputFlags Flags to control how the input behaves.
 * @return False if input couldn't be begun.
 */
typedef bool (*dsBeginWindowTextInputFunction)(dsApplication* application, dsWindow* window,
	dsWindowTextInputType inputType, dsWindowTextInputFlags inputFlags);

/**
 * @brief Function for ending text input.
 * @param application The application.
 * @param window The window that accepted text input.
 * @return False if input couldn't be ended.
 */
typedef bool (*dsEndWindowTextInputFunction)(dsApplication* application, dsWindow* window);

/**
 * @brief Function for setting the editing rarea for editing text.
 * @param application The application.
 * @param window The window that will accept text input.
 * @param bounds The renctangle to edit text in.
 * @param cursorOffset The offset from the min x of the bounds to show the cursor.
 * @return False if the input rectangle couldn't be set.
 */
typedef bool (*dsSetWindowTextInputAreaFunction)(dsApplication* application, dsWindow* window,
	const dsAlignedBox2i* bounds, uint32_t cursorOffset);

/**
 * @brief Function to get the battery level of a game input.
 * @param[out] outBatteryPercent The percent of the battery, or -1 if it cannot be determined.
 *     This may be NULL ifi not needed.
 * @param application The application.
 * @param gameInput The game input device to get the battery level from.
 * @return The battery level.
 */
typedef dsSystemPowerState (*dsGetGameInputPowerStateFunction)(
	int* outBatteryPercent, const dsApplication* application, const dsGameInput* gameInput);

/**
 * @brief Function for getting the state of a game input axis.
 * @param application The application.
 * @param gameInput The game input device to get the axis from.
 * @param axis The index of the axis.
 * @return The axis value.
 */
typedef float (*dsGetGameInputAxisFunction)(
	const dsApplication* application, const dsGameInput* gameInput, uint32_t axis);

/**
 * @brief Function for getting the state of a game input axis based on the game controller mapping.
 * @param application The application.
 * @param gameInput The game input device to get the axis from.
 * @param mapping The controller mapping.
 * @return The axis value.
 */
typedef float (*dsGetGameInputControllerAxisFunction)(
	const dsApplication* application, const dsGameInput* gameInput, dsGameControllerMap mapping);

/**
 * @brief Function for getting if a game input button is pressed.
 * @param application The application.
 * @param gameInput The game input device to get the button state from.
 * @param button The button to check.
 * @return True if the button is pressed.
 */
typedef bool (*dsIsGameInputButtonPressedFunction)(
	const dsApplication* application, const dsGameInput* gameInput, uint32_t button);

/**
 * @brief Function for getting if a game input button is pressed based on the game controller
 *     mapping.
 * @param application The application.
 * @param gameInput The game input device to get the button state from.
 * @param mapping The controller mapping.
 * @return True if the button is pressed.
 */
typedef bool (*dsIsGameInputControllerButtonPressedFunction)(
	const dsApplication* application, const dsGameInput* gameInput, dsGameControllerMap mapping);

/**
 * @brief Function for setting baseline rumble on a game input.
 * @param application The application.
 * @param gameInput The game input device to set the rumble on.
 * @param rumble The type of rumble to set.
 * @param strength The strength of the rumble in the range [0, 1].
 * @return False if rumble couldn't be set.
 */
typedef bool (*dsSetGameInputBaselineRumbleFunction)(
	dsApplication* application, dsGameInput* gameInput, dsGameInputRumble rumble, float strength);

/**
 * @brief Function for getting baseline rumble on a game input.
 * @param application The application.
 * @param gameInput The game input device to get the rumble on.
 * @param rumble The type of rumble to get.
 * @return The strength of the rumble.
 */
typedef float (*dsGetGameInputBaselineRumbleFunction)(
	dsApplication* application, const dsGameInput* gameInput, dsGameInputRumble rumble);

/**
 * @brief Function for setting timed rumble on a game input.
 * @param application The application.
 * @param gameInput The game input device to set the rumble on.
 * @param rumble The type of rumble to set.
 * @param strength The strength of the rumble in the range [0, 1].
 * @param duration The duration of the rumble in seconds.
 * @return False if rumble couldn't be set.
 */
typedef bool (*dsSetGameInputTimedRumbleFunction)(dsApplication* application,
	dsGameInput* gameInput, dsGameInputRumble rumble, float strength, float duration);

/**
 * @brief Function for getting timed rumble on a game input.
 * @param[out] outDuration The remaining duration in seconds. This may be NULL if not needed.
 * @param application The application.
 * @param gameInput The game input device to get the rumble on.
 * @param rumble The type of rumble to get.
 * @return The strength of the rumble.
 */
typedef float (*dsGetGameInputTimedRumbleFunction)(float* outDuration, dsApplication* application,
	const dsGameInput* gameInput, dsGameInputRumble rumble);

/**
 * @brief Function for setting the LED color on a game input.
 * @param application The application.
 * @param gameInput The game input device to set the LED color on.
 * @param color The color of the LED.
 * @return False if the LED color couldn't be set.
 */
typedef bool (*dsSetGameInputLEDColorFunction)(
	dsApplication* application, dsGameInput* gameInput, dsColor color);

/**
 * @brief Function for setting the player index on a game input.
 * @param application The application.
 * @param gameInput The game input device to set the player index on.
 * @param player The player index.
 * @return False if the player index couldn't be set.
 */
typedef bool (*dsSetGameInputPlayerFunction)(
	dsApplication* application, dsGameInput* gameInput, uint32_t player);

/**
 * @brief Function to get whether or not a game input has a motion sensor.
 * @param application The application.
 * @param gameInput The game input to check.
 * @param type The type of motion sensor to check for.
 * @return Whether or not the motion sensor is present.
 */
typedef bool (*dsGameInputHasMotionSensor)(
	const dsApplication* application, const dsGameInput* gameInput, dsMotionSensorType type);

/**
 * @brief Function to get game input motion sensor data.
 * @param[out] outData The data to populate.
 * @param application The application.
 * @param gameInput The game input to get the data for.
 * @param type The type of motion sensor to get the data for.
 * @return False if the data couldn't be retrieved.
 */
typedef bool (*dsGetGameInputMotionSensorData)(dsVector3f* outData,
	const dsApplication* application, const dsGameInput* gameInput, dsMotionSensorType type);

/**
 * @brief Gets data from a motion sensor on the main device.
 * @param[out] outData The motion sensor data.
 * @param application The application.
 * @param sensor The motion sensor to get the data from.
 * @return False if the motion sensor state couldn't be queried.
 */
typedef bool (*dsGetApplicationMotionSensorDataFunction)(
	dsVector3f* outData, const dsApplication* application, const dsMotionSensor* sensor);

/**
 * @brief Function for getting the game input D-pad direction.
 * @param[out] outDirection The direction the D-pad is in.
 * @param application The application.
 * @param gameInput The game input device to get the D-pad direction from.
 * @param dpad The D-pad to check.
 * @return False if the hat state couldn't be queried.
 */
typedef bool (*dsGetGameInputDPadDirectionFunction)(dsVector2i* outDirection,
	const dsApplication* application, const dsGameInput* gameInput, uint32_t dpad);

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
typedef bool (*dsInterceptCloseWindowFunction)(dsWindow* window, void* userData);

#if DS_ANDROID
/**
 * @brief Function to handle the result of requesting a permission on Android.
 * @param userData The user data provided when passing the function.
 * @param permission The permission that was requested.
 * @param granted Whether the permission was granted.
 */
typedef void (*dsHandleAndroidPermissionResultFunction)(
	void* userData, const char* permission, bool granted);

/**
 * @brief Function to request a permission from Android.
 * @param application The application.
 * @param permission The Android permission name, such as "android.permission.ACCESS_LOCAL_NETWORK".
 * @param resultFunc The function to be called for the result. This may be invoked on another
 *     thread.
 * @param userData The user data to forward to resultFunc.
 * @return False if the request couldn't be sent.
 */
typedef bool (*dsRequestAndroidPermissionFunction)(dsApplication* application,
	const char* permission, dsHandleAndroidPermissionResultFunction resultFunc, void* userData);
#endif

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
	 * @brief Function to destroy the user data.
	 */
	dsDestroyUserDataFunction destroyUserDataFunc;

	/**
	 * @brief The ID of the responder.
	 *
	 * This will be set when added to the application.
	 */
	uint32_t responderID;
} dsWindowResponder;

/**
 * @brief Struct containing information to respond to an event.
 */
typedef struct dsEventResponder
{
	/**
	 * @brief Function called when the event has been recieved.
	 */
	dsApplicationEventFunction eventFunc;

	/**
	 * @brief User data to be passed to eventFunc.
	 */
	void* userData;

	/**
	 * @brief Function to destroy the user data.
	 */
	dsDestroyUserDataFunction destroyUserDataFunc;

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
	uint32_t responderID;
} dsEventResponder;

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
typedef struct dsApplication
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
	 * @brief Timer used to convert ticks to seconds.
	 */
	dsTimer timer;

	/**
	 * @brief Whether this was responsible for initializing dsUniqueNameID.
	 */
	bool uniqueNameIDInitialized;

	/**
	 * @brief The desired rate at which to update the application.
	 *
	 * A positive value is the number of updates per second to target. A value of 0 indicates to
	 * update as quickly as possible, and a negative number will only update when events are
	 * received. Some implementations may not respect this value in all situations.
	 */
	float updateRate;

	/**
	 * @brief The the primary display.
	 */
	const dsDisplayInfo* primaryDisplay;

	/**
	 * @brief Information about the different displays.
	 */
	dsDisplayInfo** displays;

	/**
	 * @brief The number of displays.
	 */
	uint32_t displayCount;

	/**
	 * @brief The number of displays that can be held before the buffer is re-allocated.
	 */
	uint32_t displayCapacity;

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
	 * @brief The game inputs in the application.
	 */
	dsGameInput** gameInputs;

	/**
	 * @brief The number of game inputs.
	 */
	uint32_t gameInputCount;

	/**
	 * @brief The number of game inputs that can be held before the buffer is re-allocated.
	 */
	uint32_t gameInputCapacity;

	/**
	 * @brief The motion sensors in the application.
	 */
	dsMotionSensor** motionSensors;

	/**
	 * @brief The number of motion sensors.
	 */
	uint32_t motionSensorCount;

	/**
	 * @brief The number of motion sensors that can be held before the buffer is re-allocated.
	 */
	uint32_t motionSensorCapacity;

	/**
	 * @brief User data associated with the application.
	 */
	void* userData;

	/**
	 * @brief Function to destroy the application user data.
	 */
	dsDestroyUserDataFunction destroyUserDataFunc;

	/**
	 * @brief Function to call once all resources related to the application are destroyed.
	 */
	dsDestroyUserDataFunction finalizerFunc;

	/**
	 * @brief User data to provide to the finalizer function.
	 */
	void* finalizerUserData;

	/**
	 * @brief Function for updating the application.
	 */
	dsUpdateApplicationFunction updateFunc;

	/**
	 * @brief User data for the update function.
	 */
	void* updateUserData;

	/**
	 * @brief Function to destroy the update user data.
	 */
	dsDestroyUserDataFunction destroyUpdateUserDataFunc;

	/**
	 * @brief Function for finishing a frame in the application.
	 */
	dsFinishApplicationFrameFunction finishFrameFunc;

	/**
	 * @brief User data for the finish frame function.
	 */
	void* finishFrameUserData;

	/**
	 * @brief Function to destroy the finish frame user data.
	 */
	dsDestroyUserDataFunction destroyFinishFrameUserDataFunc;

	/**
	 * @brief Function to set the update rate of the application.
	 */
	dsSetApplicationUpdateRateFunction setUpdateRateFunc;

	/**
	 * @brief Function for showing a message box.
	 */
	dsShowApplicationMessageBoxFunction showMessageBoxFunc;

	/**
	 * @brief Function for quitting the application.
	 */
	dsQuitApplicationFunction quitFunc;

	/**
	 * @brief Function for adding a custom event.
	 */
	dsAddCustomApplicationEventFunction addCustomEventFunc;

	/**
	 * @brief Function to get the current power state.
	 */
	dsGetApplicationPowerStateFunction getPowerStateFunc;

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

#if DS_ANDROID
	/**
	 * @brief Fucntion to request and Android permission.
	 */
	dsRequestAndroidPermissionFunction requestAndroidPermissionFunc;
#endif

	/**
	 * @brief Function to destroy the application.
	 */
	dsDestroyApplicationFunction destroyFunc;

	/**
	 * @brief Function for creating a window.
	 */
	dsCreateWindowFunction createWindowFunc;

	/**
	 * @brief Function for destroying a window.
	 */
	dsDestroyWindowFunction destroyWindowFunc;

	/**
	 * @brief Function for creating a window surface.
	 */
	dsCreateWindowSurfaceFunction createWindowSurfaceFunc;

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
	 * @brief Function for setting the window style.
	 */
	dsSetWindowStyleFunction setWindowStyleFunc;

	/**
	 * @brief Function for setting the position of a window.
	 */
	dsSetWindowPositionFunction setWindowPositionFunc;

	/**
	 * @brief Function for centering a window on a display.
	 */
	dsCenterWindowFunction centerWindowFunc;

	/**
	 * @brief Function for setting if a window is hidden.
	 */
	dsSetWindowHiddenFunction setWindowHiddenFunc;

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
	 * @brief Function for setting if a window has input grabbed.
	 */
	dsSetWindowGrabbedInputFunction setWindowGrabbedInputFunc;

	/**
	 * @brief Function for setting if a window is resizable.
	 */
	dsSetWindowResizableFunction setWindowResizableFunc;

	/**
	 * @brief Function for raising a window.
	 */
	dsRaiseWindowFunction raiseWindowFunc;

	/**
	 * @brief Function for beginning text input.
	 */
	dsBeginWindowTextInputFunction beginTextInputFunc;

	/**
	 * @brief Function for ending text input.
	 */
	dsEndWindowTextInputFunction endTextInputFunc;

	/**
	 * @brief Function for setting the text input area.
	 */
	dsSetWindowTextInputAreaFunction setTextInputAreaFunc;

	/**
	 * @brief Function for getting the game input power state.
	 */
	dsGetGameInputPowerStateFunction getGameInputPowerStateFunc;

	/**
	 * @brief Function for getting a game input axis.
	 */
	dsGetGameInputAxisFunction getGameInputAxisFunc;

	/**
	 * @brief Function for getting a game input axis based on the controller mapping.
	 */
	dsGetGameInputControllerAxisFunction getGameInputControllerAxisFunc;

	/**
	 * @brief Function for getting if a game input button is pressed.
	 */
	dsIsGameInputButtonPressedFunction isGameInputButtonPressedFunc;

	/**
	 * @brief Function for getting if a game input button is pressed based on the controller
	 *     mapping.
	 */
	dsIsGameInputControllerButtonPressedFunction isGameInputControllerButtonPressedFunc;

	/**
	 * @brief Function for getting the game input D-pad direction.
	 */
	dsGetGameInputDPadDirectionFunction getGameInputDPadDirectionFunc;

	/**
	 * @brief Function for setting baseline rumble on a game input.
	 */
	dsSetGameInputBaselineRumbleFunction setGameInputBaselineRumbleFunc;

	/**
	 * @brief Function for getting baseline rumble on a game input.
	 */
	dsGetGameInputBaselineRumbleFunction getGameInputBaselineRumbleFunc;

	/**
	 * @brief Function for setting timed rumble on a game input.
	 */
	dsSetGameInputTimedRumbleFunction setGameInputTimedRumbleFunc;

	/**
	 * @brief Function for getting baseline rumble on a game input.
	 */
	dsGetGameInputTimedRumbleFunction getGameInputTimedRumbleFunc;

	/**
	 * @brief Function to set the LED color on a game input.
	 */
	dsSetGameInputLEDColorFunction setGameInputLEDColorFunc;

	/**
	 * @brief Function to set the plaer index on a game input.
	 */
	dsSetGameInputPlayerFunction setGameInputPlayerFunc;

	/**
	 * @brief Function to get whether or not a game input has a motion sensor.
	 */
	dsGameInputHasMotionSensor gameInputHasMotionSensorFunc;

	/**
	 * @brief Function to get game input motion sensor data.
	 */
	dsGetGameInputMotionSensorData getGameInputMotionSensorDataFunc;

	/**
	 * @brief Function to get the data from a motion sensor.
	 */
	dsGetApplicationMotionSensorDataFunction getMotionSensorDataFunc;
} dsApplication;

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
typedef struct dsWindow
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
	 * @brief The surface for the window.
	 */
	dsRenderSurface* surface;

	/**
	 * @brief Function for drawing the window.
	 */
	dsDrawWindowFunction drawFunc;

	/**
	 * @brief User data for drawing the window.
	 */
	void* drawUserData;

	/**
	 * @brief Function to destroy the draw user data.
	 */
	dsDestroyUserDataFunction destroyDrawUserDataFunc;

	/**
	 * @brief The function to intercept closing the window.
	 */
	dsInterceptCloseWindowFunction closeFunc;

	/**
	 * @brief User data to provide when calling windowCloseFunc.
	 */
	void* closeUserData;

	/**
	 * @brief Function to destroy the close user data.
	 */
	dsDestroyUserDataFunction destroyCloseUserDataFunc;

	/**
	 * @brief The display the window is visible on.
	 *
	 * In rare situations this may be NULL.
	 */
	const dsDisplayInfo* display;

	/**
	 * @brief The flags for the window state.
	 */
	dsWindowFlags flags;

	/**
	 * @brief The style of the window.
	 */
	dsWindowStyle style;

	/**
	 * @brief The display mode of the window when full-screen.
	 */
	dsDisplayMode displayMode;

	/**
	 * @brief The position of the window.
	 */
	dsVector2i position;

	/**
	 * @brief The width of the window.
	 *
	 * This may differ from the surface width if the internal windowing system adjusts the window
	 * coordinates based on the DPI.
	 */
	uint32_t width;

	/**
	 * @brief The height of the window.
	 *
	 * This may differ from the surface height if the internal windowing system adjusts the window
	 * coordinates based on the DPI.
	 */
	uint32_t height;

	/**
	 * @brief The scale to apply to the contents of the window.
	 *
	 * This combines the display scale and any scaling done by the internal windowing system to
	 * adjust for DPI. In most cases it will be the result of (surface->width/width)*display->scale.
	 */
	float contentScale;

	/**
	 * @brief The area within the window that is safe to use.
	 *
	 * This will exclude features such as notches in mobile displays. This uses the window-space
	 * coordinates used by width and height and may differ from the pixel values.
	 */
	dsAlignedBox2i safeArea;
} dsWindow;

/**
 * @brief Struct containing information about a game input device.
 *
 * Game input implementations can effectively subclass this type by having it as the first member
 * of the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsGameInput and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 *
 * @see GameInput.h
 */
typedef struct dsGameInput
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
	 * @brief The name of the device.
	 */
	const char* name;

	/**
	 * @brief The type of the device.
	 */
	dsGameInputType type;

	/**
	 * @brief The vendor ID for the device.
	 */
	uint16_t vendorID;

	/**
	 * @brief The product ID for the device.
	 */
	uint16_t productID;

	/**
	 * @brief The player the game input is associated with, or DS_NO_GAME_INPUT_PLAYER if unset.
	 */
	uint32_t player;

	/**
	 * @brief The number of axes on the device.
	 */
	uint32_t axisCount;

	/**
	 * @brief The number of buttons on the device.
	 */
	uint32_t buttonCount;

	/**
	 * @brief The number of balls on the device, generally for joysticks.
	 */
	uint32_t ballCount;

	/**
	 * @brief The number of D-pads or hats on the device.
	 */
	uint32_t dpadCount;

	/**
	 * @brief The number of touchpads on the device.
	 */
	uint32_t touchpadCount;

	/**
	 * @brief Mapping from a game controller input to the raw game input.
	 */
	dsGameInputMap controllerMapping[dsGameControllerMap_Count];

	/**
	 * @brief The controller map for each axis.
	 *
	 * If hasControllerMappings is true, this will have axisCount entries, otherwise it will be
	 * NULL.
	 */
	const dsGameControllerMap* axisControllerMaps;

	/**
	 * @brief The controller map for each button.
	 *
	 * If hasControllerMappings is true, this will have buttonCount entries, otherwise it will be
	 * NULL.
	 */
	const dsGameControllerMap* buttonControllerMaps;

	/**
	 * @brief The controller map for each dpad.
	 *
	 * If hasControllerMappings is true, this will have 4*dpadCount entries, otherwise it will be
	 * NULL. Each value will correspond to the strictly cardinal directions in dsGameInputDirection.
	 * If a dpad direction is for a full axis, the opposing directions will be duplicated.
	 */
	const dsGameControllerMap* dpadControllerMaps;

	/**
	 * @brief True if any controller mappings are available.
	 */
	bool hasControllerMappings;

	/**
	 * @brief Whether rumble is supported.
	 */
	bool rumbleSupported;

	/**
	 * @brief Whether rumble on trigger buttons is supported.
	 */
	bool triggerRumbleSupported;

	/**
	 * @brief Whether an LED is present.
	 */
	bool hasLED;
} dsGameInput;

/**
 * @brief Struct containing information about a motion sensor.
 *
 * Motion sensor implementations can effectively subclass this type by having it as the first member
 * of the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsMotionSensor and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 *
 * @see MotionSensor.h
 */
typedef struct dsMotionSensor
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
	 * @brief The name of the device.
	 */
	const char* name;

	/**
	 * @brief The type of the motion sensor.
	 */
	dsMotionSensorType type;
} dsMotionSensor;

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsWindowFlags);
DS_ENUM_BITMASK_OPERATORS(dsWindowTextInputFlags);
DS_ENUM_BITMASK_OPERATORS(dsWindowChangeFlags);
/// @endcond
