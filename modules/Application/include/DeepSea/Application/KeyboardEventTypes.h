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
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used for keyboard events.
 */

/**
 * @brief Enum for a keyboard key code.
 */
typedef enum dsKeyCode
{
	dsKeyCode_A,
	dsKeyCode_B,
	dsKeyCode_C,
	dsKeyCode_D,
	dsKeyCode_E,
	dsKeyCode_F,
	dsKeyCode_G,
	dsKeyCode_H,
	dsKeyCode_I,
	dsKeyCode_J,
	dsKeyCode_K,
	dsKeyCode_L,
	dsKeyCode_M,
	dsKeyCode_N,
	dsKeyCode_O,
	dsKeyCode_P,
	dsKeyCode_Q,
	dsKeyCode_R,
	dsKeyCode_S,
	dsKeyCode_T,
	dsKeyCode_U,
	dsKeyCode_V,
	dsKeyCode_W,
	dsKeyCode_X,
	dsKeyCode_Y,
	dsKeyCode_Z,
	dsKeyCode_1,
	dsKeyCode_2,
	dsKeyCode_3,
	dsKeyCode_4,
	dsKeyCode_5,
	dsKeyCode_6,
	dsKeyCode_7,
	dsKeyCode_8,
	dsKeyCode_9,
	dsKeyCode_0,
	dsKeyCode_Enter,
	dsKeyCode_Escape,
	dsKeyCode_Backspace,
	dsKeyCode_Tab,
	dsKeyCode_Space,
	dsKeyCode_Minus,
	dsKeyCode_Equals,
	dsKeyCode_LeftBracket,
	dsKeyCode_RightBracket,
	dsKeyCode_Backslash,
	dsKeyCode_NonUSHash,
	dsKeyCode_Semicolon,
	dsKeyCode_Apostrophe,
	dsKeyCode_Grave,
	dsKeyCode_Comma,
	dsKeyCode_Period,
	dsKeyCode_Slash,
	dsKeyCode_Capslock,
	dsKeyCode_F1,
	dsKeyCode_F2,
	dsKeyCode_F3,
	dsKeyCode_F4,
	dsKeyCode_F5,
	dsKeyCode_F6,
	dsKeyCode_F7,
	dsKeyCode_F8,
	dsKeyCode_F9,
	dsKeyCode_F10,
	dsKeyCode_F11,
	dsKeyCode_F12,
	dsKeyCode_F13,
	dsKeyCode_F14,
	dsKeyCode_F15,
	dsKeyCode_F16,
	dsKeyCode_F17,
	dsKeyCode_F18,
	dsKeyCode_F19,
	dsKeyCode_F20,
	dsKeyCode_F21,
	dsKeyCode_F22,
	dsKeyCode_F23,
	dsKeyCode_F24,
	dsKeyCode_PrintScreen,
	dsKeyCode_ScrollLock,
	dsKeyCode_Pause,
	dsKeyCode_Insert,
	dsKeyCode_Home,
	dsKeyCode_End,
	dsKeyCode_PageUp,
	dsKeyCode_PageDown,
	dsKeyCode_Delete,
	dsKeyCode_Left,
	dsKeyCode_Right,
	dsKeyCode_Up,
	dsKeyCode_Down,
	dsKeyCode_NumLock,
	dsKeyCode_KpDivide,
	dsKeyCode_KpMultiply,
	dsKeyCode_KpMinus,
	dsKeyCode_KpPlus,
	dsKeyCode_KpEquals,
	dsKeyCode_KpEqualsAS400,
	dsKeyCode_KpEnter,
	dsKeyCode_Kp1,
	dsKeyCode_Kp2,
	dsKeyCode_Kp3,
	dsKeyCode_Kp4,
	dsKeyCode_Kp5,
	dsKeyCode_Kp6,
	dsKeyCode_Kp7,
	dsKeyCode_Kp8,
	dsKeyCode_Kp9,
	dsKeyCode_Kp0,
	dsKeyCode_Kp00,
	dsKeyCode_Kp000,
	dsKeyCode_KpPeriod,
	dsKeyCode_KpComma,
	dsKeyCode_KpLeftParen,
	dsKeyCode_KpRightParen,
	dsKeyCode_KpLeftBrace,
	dsKeyCode_KpRightBrace,
	dsKeyCode_KpTab,
	dsKeyCode_KpBackspace,
	dsKeyCode_KpA,
	dsKeyCode_KpB,
	dsKeyCode_KpC,
	dsKeyCode_KpD,
	dsKeyCode_KpE,
	dsKeyCode_KpF,
	dsKeyCode_KpXOr,
	dsKeyCode_KpPercent,
	dsKeyCode_KpLess,
	dsKeyCode_KpGreater,
	dsKeyCode_KpAmperstand,
	dsKeyCode_KpDoubleAmperstand,
	dsKeyCode_KpPipe,
	dsKeyCode_KpDoublePipe,
	dsKeyCode_KpColon,
	dsKeyCode_KpHash,
	dsKeyCode_KpSpace,
	dsKeyCode_KpAt,
	dsKeyCode_KpExclamation,
	dsKeyCode_KpMemStore,
	dsKeyCode_KpMemRecall,
	dsKeyCode_KpMemClear,
	dsKeyCode_KpMemAdd,
	dsKeyCode_KpMemSubtract,
	dsKeyCode_KpMemMultiply,
	dsKeyCode_KpMemDivide,
	dsKeyCode_KpPlusMinus,
	dsKeyCode_KpClear,
	dsKeyCode_KpClearEntry,
	dsKeyCode_KpBinary,
	dsKeyCode_KpOctal,
	dsKeyCode_KpDecimal,
	dsKeyCode_KpHex,
	dsKeyCode_KpPower,
	dsKeyCode_NonUSBackslash,
	dsKeyCode_ContextMenu,
	dsKeyCode_Power,
	dsKeyCode_Execute,
	dsKeyCode_Help,
	dsKeyCode_Menu,
	dsKeyCode_Select,
	dsKeyCode_Stop,
	dsKeyCode_Again,
	dsKeyCode_Undo,
	dsKeyCode_Cut,
	dsKeyCode_Copy,
	dsKeyCode_Paste,
	dsKeyCode_Find,
	dsKeyCode_Mute,
	dsKeyCode_VolumeUp,
	dsKeyCode_VolumeDown,
	dsKeyCode_International1,
	dsKeyCode_International2,
	dsKeyCode_International3,
	dsKeyCode_International4,
	dsKeyCode_International5,
	dsKeyCode_International6,
	dsKeyCode_International7,
	dsKeyCode_International8,
	dsKeyCode_International9,
	dsKeyCode_Lang1,
	dsKeyCode_Lang2,
	dsKeyCode_Lang3,
	dsKeyCode_Lang4,
	dsKeyCode_Lang5,
	dsKeyCode_Lang6,
	dsKeyCode_Lang7,
	dsKeyCode_Lang8,
	dsKeyCode_Lang9,
	dsKeyCode_AltErase,
	dsKeyCode_SysReq,
	dsKeyCode_Cancel,
	dsKeyCode_Clear,
	dsKeyCode_ClearAgain,
	dsKeyCode_Prior,
	dsKeyCode_Return,
	dsKeyCode_Separator,
	dsKeyCode_Out,
	dsKeyCode_Oper,
	dsKeyCode_CrSel,
	dsKeyCode_ExSel,
	dsKeyCode_ThousandsSeparator,
	dsKeyCode_DecimalSeparator,
	dsKeyCode_CurrencyUnit,
	dsKeyCode_CurrencySubUnit,
	dsKeyCode_LControl,
	dsKeyCode_LShift,
	dsKeyCode_LAlt,
	dsKeyCode_LWinCmd,
	dsKeyCode_RControl,
	dsKeyCode_RShift,
	dsKeyCode_RAlt,
	dsKeyCode_RWinCmd,
	dsKeyCode_Mode,
	dsKeyCode_AudioNext,
	dsKeyCode_AudioPrev,
	dsKeyCode_AudioStop,
	dsKeyCode_AudioPlay,
	dsKeyCode_AudioMute,
	dsKeyCode_AudioRewind,
	dsKeyCode_AudioFastForward,
	dsKeyCode_MediaSelect,
	dsKeyCode_WWW,
	dsKeyCode_Mail,
	dsKeyCode_Calculator,
	dsKeyCode_Computer,
	dsKeyCode_ACSearch,
	dsKeyCode_ACHome,
	dsKeyCode_ACBack,
	dsKeyCode_ACForward,
	dsKeyCode_ACStop,
	dsKeyCode_ACRefresh,
	dsKeyCode_ACBookmarks,
	dsKeyCode_BrightnessDown,
	dsKeyCode_BrightnessUp,
	dsKeyCode_DisplaySwitch,
	dsKeyCode_KbdIllumToggle,
	dsKeyCode_KbdIllumDown,
	dsKeyCode_KbdIllumUp,
	dsKeyCode_Eject,
	dsKeyCode_Sleep,
	dsKeyCode_App1,
	dsKeyCode_App2,

	dsKeyCode_Count
} dsKeyCode;

/**
 * @brief Enum for a key modifier.
 */
typedef enum dsKeyModifier
{
	dsKeyModifier_None = 0,         ///< No modifiers.
	dsKeyModifier_LShift = 0x1,     ///< Left shift key.
	dsKeyModifier_RShift = 0x2,     ///< Right shift key.
	dsKeyModifier_LCtrl = 0x4,      ///< Left control key.
	dsKeyModifier_RCtrl = 0x8,      ///< Right control key.
	dsKeyModifier_LAlt = 0x10,      ///< Left alt/option key.
	dsKeyModifier_RAlt = 0x20,      ///< Right alt/option key.
	dsKeyModifier_LWinCmd = 0x40,   ///< Left windows/command key.
	dsKeyModifier_RWinCmd = 0x80,   ///< Right windows/command key.
	dsKeyModifier_NumLock = 0x100,  ///< Numlock
	dsKeyModifier_CapsLock = 0x200, ///< Capslock
	dsKeyModifier_Mode = 0x400,     ///< Mode

	dsKeyModifier_Shift = dsKeyModifier_LShift | dsKeyModifier_RShift, ///< Either shift key.
	dsKeyModifier_Ctrl = dsKeyModifier_LCtrl | dsKeyModifier_RCtrl,    ///< Either control key.
	dsKeyModifier_Alt = dsKeyModifier_LCtrl | dsKeyModifier_RCtrl,     ///< Either alt key.
	/** Either windows/command key. */
	dsKeyModifier_WinCmd = dsKeyModifier_LWinCmd | dsKeyModifier_RWinCmd
} dsKeyModifier;

/**
 * @brief Struct containing information about a key event.
 */
typedef struct dsKeyEvent
{
	/**
	 * @brief The key that was pressed or released.
	 */
	dsKeyCode key;

	/**
	 * @brief The key modifiers currently pressed.
	 */
	dsKeyModifier modifiers;

	/**
	 * @brief True if the press was due to the key still being pressed.
	 */
	bool repeat;
} dsKeyEvent;

/**
 * @brief Struct containing information about text editing.
 *
 * This is done when editing is in progress, but not yet committed. One example is editing
 * interanational text, such as Chinese and Japanese, or input on mobile platforms.
 */
typedef struct dsTextEditEvent
{
	/**
	 * @brief The cursor position.
	 */
	int32_t cursor;

	/**
	 * The length of the selection.
	 */
	int32_t selectionLength;

	/**
	 * @brief The text being edited.
	 */
	char text[32];
} dsTextEditEvent;

/**
 * @brief Struct containing text that has been input.
 *
 * This is text that has been committed, potentially after a series of dsTextEditEvents.
 */
typedef struct dsTextInputEvent
{
	/**
	 * @brief The text that was input.
	 */
	char text[32];
} dsTextInputEvent;

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsKeyModifier);
/// @endcond
