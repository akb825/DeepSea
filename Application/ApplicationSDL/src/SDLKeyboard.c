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

#include "SDLKeyboard.h"

dsKeyCode dsFromSDLScancode(SDL_Scancode scanCode)
{
	switch (scanCode)
	{
		case SDL_SCANCODE_UNKNOWN:
			return dsKeyCode_Count;
		case SDL_SCANCODE_A:
			return dsKeyCode_A;
		case SDL_SCANCODE_B:
			return dsKeyCode_B;
		case SDL_SCANCODE_C:
			return dsKeyCode_C;
		case SDL_SCANCODE_D:
			return dsKeyCode_D;
		case SDL_SCANCODE_E:
			return dsKeyCode_E;
		case SDL_SCANCODE_F:
			return dsKeyCode_F;
		case SDL_SCANCODE_G:
			return dsKeyCode_G;
		case SDL_SCANCODE_H:
			return dsKeyCode_H;
		case SDL_SCANCODE_I:
			return dsKeyCode_I;
		case SDL_SCANCODE_J:
			return dsKeyCode_J;
		case SDL_SCANCODE_K:
			return dsKeyCode_K;
		case SDL_SCANCODE_L:
			return dsKeyCode_L;
		case SDL_SCANCODE_M:
			return dsKeyCode_M;
		case SDL_SCANCODE_N:
			return dsKeyCode_N;
		case SDL_SCANCODE_O:
			return dsKeyCode_O;
		case SDL_SCANCODE_P:
			return dsKeyCode_P;
		case SDL_SCANCODE_Q:
			return dsKeyCode_Q;
		case SDL_SCANCODE_R:
			return dsKeyCode_R;
		case SDL_SCANCODE_S:
			return dsKeyCode_S;
		case SDL_SCANCODE_T:
			return dsKeyCode_T;
		case SDL_SCANCODE_U:
			return dsKeyCode_U;
		case SDL_SCANCODE_V:
			return dsKeyCode_V;
		case SDL_SCANCODE_W:
			return dsKeyCode_W;
		case SDL_SCANCODE_X:
			return dsKeyCode_X;
		case SDL_SCANCODE_Y:
			return dsKeyCode_Y;
		case SDL_SCANCODE_Z:
			return dsKeyCode_Z;
		case SDL_SCANCODE_1:
			return dsKeyCode_1;
		case SDL_SCANCODE_2:
			return dsKeyCode_2;
		case SDL_SCANCODE_3:
			return dsKeyCode_3;
		case SDL_SCANCODE_4:
			return dsKeyCode_4;
		case SDL_SCANCODE_5:
			return dsKeyCode_5;
		case SDL_SCANCODE_6:
			return dsKeyCode_6;
		case SDL_SCANCODE_7:
			return dsKeyCode_7;
		case SDL_SCANCODE_8:
			return dsKeyCode_8;
		case SDL_SCANCODE_9:
			return dsKeyCode_9;
		case SDL_SCANCODE_0:
			return dsKeyCode_0;
		case SDL_SCANCODE_RETURN:
			return dsKeyCode_Enter;
		case SDL_SCANCODE_ESCAPE:
			return dsKeyCode_Escape;
		case SDL_SCANCODE_BACKSPACE:
			return dsKeyCode_Backspace;
		case SDL_SCANCODE_TAB:
			return dsKeyCode_Tab;
		case SDL_SCANCODE_SPACE:
			return dsKeyCode_Space;
		case SDL_SCANCODE_MINUS:
			return dsKeyCode_Minus;
		case SDL_SCANCODE_EQUALS:
			return dsKeyCode_Equals;
		case SDL_SCANCODE_LEFTBRACKET:
			return dsKeyCode_LeftBracket;
		case SDL_SCANCODE_RIGHTBRACKET:
			return dsKeyCode_RightBracket;
		case SDL_SCANCODE_BACKSLASH:
			return dsKeyCode_Backslash;
		case SDL_SCANCODE_NONUSHASH:
			return dsKeyCode_NonUSHash;
		case SDL_SCANCODE_SEMICOLON:
			return dsKeyCode_Semicolon;
		case SDL_SCANCODE_APOSTROPHE:
			return dsKeyCode_Apostrophe;
		case SDL_SCANCODE_GRAVE:
			return dsKeyCode_Grave;
		case SDL_SCANCODE_COMMA:
			return dsKeyCode_Comma;
		case SDL_SCANCODE_PERIOD:
			return dsKeyCode_Period;
		case SDL_SCANCODE_SLASH:
			return dsKeyCode_Slash;
		case SDL_SCANCODE_CAPSLOCK:
			return dsKeyCode_Capslock;
		case SDL_SCANCODE_F1:
			return dsKeyCode_F1;
		case SDL_SCANCODE_F2:
			return dsKeyCode_F2;
		case SDL_SCANCODE_F3:
			return dsKeyCode_F3;
		case SDL_SCANCODE_F4:
			return dsKeyCode_F4;
		case SDL_SCANCODE_F5:
			return dsKeyCode_F5;
		case SDL_SCANCODE_F6:
			return dsKeyCode_F6;
		case SDL_SCANCODE_F7:
			return dsKeyCode_F7;
		case SDL_SCANCODE_F8:
			return dsKeyCode_F8;
		case SDL_SCANCODE_F9:
			return dsKeyCode_F9;
		case SDL_SCANCODE_F10:
			return dsKeyCode_F10;
		case SDL_SCANCODE_F11:
			return dsKeyCode_F11;
		case SDL_SCANCODE_F12:
			return dsKeyCode_F12;
		case SDL_SCANCODE_PRINTSCREEN:
			return dsKeyCode_PrintScreen;
		case SDL_SCANCODE_SCROLLLOCK:
			return dsKeyCode_ScrollLock;
		case SDL_SCANCODE_PAUSE:
			return dsKeyCode_Pause;
		case SDL_SCANCODE_INSERT:
			return dsKeyCode_Insert;
		case SDL_SCANCODE_HOME:
			return dsKeyCode_Home;
		case SDL_SCANCODE_PAGEUP:
			return dsKeyCode_PageUp;
		case SDL_SCANCODE_DELETE:
			return dsKeyCode_Delete;
		case SDL_SCANCODE_END:
			return dsKeyCode_End;
		case SDL_SCANCODE_PAGEDOWN:
			return dsKeyCode_PageDown;
		case SDL_SCANCODE_RIGHT:
			return dsKeyCode_Right;
		case SDL_SCANCODE_LEFT:
			return dsKeyCode_Left;
		case SDL_SCANCODE_DOWN:
			return dsKeyCode_Down;
		case SDL_SCANCODE_UP:
			return dsKeyCode_Up;
		case SDL_SCANCODE_NUMLOCKCLEAR:
			return dsKeyCode_NumLock;
		case SDL_SCANCODE_KP_DIVIDE:
			return dsKeyCode_KpDivide;
		case SDL_SCANCODE_KP_MULTIPLY:
			return dsKeyCode_KpMultiply;
		case SDL_SCANCODE_KP_MINUS:
			return dsKeyCode_KpMinus;
		case SDL_SCANCODE_KP_PLUS:
			return dsKeyCode_KpPlus;
		case SDL_SCANCODE_KP_ENTER:
			return dsKeyCode_KpEnter;
		case SDL_SCANCODE_KP_1:
			return dsKeyCode_Kp1;
		case SDL_SCANCODE_KP_2:
			return dsKeyCode_Kp2;
		case SDL_SCANCODE_KP_3:
			return dsKeyCode_Kp3;
		case SDL_SCANCODE_KP_4:
			return dsKeyCode_Kp4;
		case SDL_SCANCODE_KP_5:
			return dsKeyCode_Kp5;
		case SDL_SCANCODE_KP_6:
			return dsKeyCode_Kp6;
		case SDL_SCANCODE_KP_7:
			return dsKeyCode_Kp7;
		case SDL_SCANCODE_KP_8:
			return dsKeyCode_Kp8;
		case SDL_SCANCODE_KP_9:
			return dsKeyCode_Kp9;
		case SDL_SCANCODE_KP_0:
			return dsKeyCode_Kp0;
		case SDL_SCANCODE_KP_PERIOD:
			return dsKeyCode_KpPeriod;
		case SDL_SCANCODE_NONUSBACKSLASH:
			return dsKeyCode_NonUSBackslash;
		case SDL_SCANCODE_APPLICATION:
			return dsKeyCode_ContextMenu;
		case SDL_SCANCODE_POWER:
			return dsKeyCode_Power;
		case SDL_SCANCODE_KP_EQUALS:
			return dsKeyCode_KpEquals;
		case SDL_SCANCODE_F13:
			return dsKeyCode_F13;
		case SDL_SCANCODE_F14:
			return dsKeyCode_F14;
		case SDL_SCANCODE_F15:
			return dsKeyCode_F15;
		case SDL_SCANCODE_F16:
			return dsKeyCode_F16;
		case SDL_SCANCODE_F17:
			return dsKeyCode_F17;
		case SDL_SCANCODE_F18:
			return dsKeyCode_F18;
		case SDL_SCANCODE_F19:
			return dsKeyCode_F19;
		case SDL_SCANCODE_F20:
			return dsKeyCode_F20;
		case SDL_SCANCODE_F21:
			return dsKeyCode_F21;
		case SDL_SCANCODE_F22:
			return dsKeyCode_F22;
		case SDL_SCANCODE_F23:
			return dsKeyCode_F23;
		case SDL_SCANCODE_F24:
			return dsKeyCode_F24;
		case SDL_SCANCODE_EXECUTE:
			return dsKeyCode_Execute;
		case SDL_SCANCODE_HELP:
			return dsKeyCode_Help;
		case SDL_SCANCODE_MENU:
			return dsKeyCode_Menu;
		case SDL_SCANCODE_SELECT:
			return dsKeyCode_Select;
		case SDL_SCANCODE_STOP:
			return dsKeyCode_Stop;
		case SDL_SCANCODE_AGAIN:
			return dsKeyCode_Again;
		case SDL_SCANCODE_UNDO:
			return dsKeyCode_Undo;
		case SDL_SCANCODE_CUT:
			return dsKeyCode_Cut;
		case SDL_SCANCODE_COPY:
			return dsKeyCode_Copy;
		case SDL_SCANCODE_PASTE:
			return dsKeyCode_Paste;
		case SDL_SCANCODE_FIND:
			return dsKeyCode_Find;
		case SDL_SCANCODE_MUTE:
			return dsKeyCode_Mute;
		case SDL_SCANCODE_VOLUMEUP:
			return dsKeyCode_VolumeUp;
		case SDL_SCANCODE_VOLUMEDOWN:
			return dsKeyCode_VolumeDown;
		case SDL_SCANCODE_KP_COMMA:
			return dsKeyCode_KpComma;
		case SDL_SCANCODE_KP_EQUALSAS400:
			return dsKeyCode_KpEqualsAS400;
		case SDL_SCANCODE_INTERNATIONAL1:
			return dsKeyCode_International1;
		case SDL_SCANCODE_INTERNATIONAL2:
			return dsKeyCode_International2;
		case SDL_SCANCODE_INTERNATIONAL3:
			return dsKeyCode_International3;
		case SDL_SCANCODE_INTERNATIONAL4:
			return dsKeyCode_International4;
		case SDL_SCANCODE_INTERNATIONAL5:
			return dsKeyCode_International5;
		case SDL_SCANCODE_INTERNATIONAL6:
			return dsKeyCode_International6;
		case SDL_SCANCODE_INTERNATIONAL7:
			return dsKeyCode_International7;
		case SDL_SCANCODE_INTERNATIONAL8:
			return dsKeyCode_International8;
		case SDL_SCANCODE_INTERNATIONAL9:
			return dsKeyCode_International9;
		case SDL_SCANCODE_LANG1:
			return dsKeyCode_Lang1;
		case SDL_SCANCODE_LANG2:
			return dsKeyCode_Lang2;
		case SDL_SCANCODE_LANG3:
			return dsKeyCode_Lang3;
		case SDL_SCANCODE_LANG4:
			return dsKeyCode_Lang4;
		case SDL_SCANCODE_LANG5:
			return dsKeyCode_Lang5;
		case SDL_SCANCODE_LANG6:
			return dsKeyCode_Lang6;
		case SDL_SCANCODE_LANG7:
			return dsKeyCode_Lang7;
		case SDL_SCANCODE_LANG8:
			return dsKeyCode_Lang8;
		case SDL_SCANCODE_LANG9:
			return dsKeyCode_Lang9;
		case SDL_SCANCODE_ALTERASE:
			return dsKeyCode_AltErase;
		case SDL_SCANCODE_SYSREQ:
			return dsKeyCode_SysReq;
		case SDL_SCANCODE_CANCEL:
			return dsKeyCode_Cancel;
		case SDL_SCANCODE_CLEAR:
			return dsKeyCode_Clear;
		case SDL_SCANCODE_PRIOR:
			return dsKeyCode_Prior;
		case SDL_SCANCODE_RETURN2:
			return dsKeyCode_Return;
		case SDL_SCANCODE_SEPARATOR:
			return dsKeyCode_Separator;
		case SDL_SCANCODE_OUT:
			return dsKeyCode_Out;
		case SDL_SCANCODE_OPER:
			return dsKeyCode_Oper;
		case SDL_SCANCODE_CLEARAGAIN:
			return dsKeyCode_ClearAgain;
		case SDL_SCANCODE_CRSEL:
			return dsKeyCode_CrSel;
		case SDL_SCANCODE_EXSEL:
			return dsKeyCode_ExSel;
		case SDL_SCANCODE_KP_00:
			return dsKeyCode_Kp00;
		case SDL_SCANCODE_KP_000:
			return dsKeyCode_Kp000;
		case SDL_SCANCODE_THOUSANDSSEPARATOR:
			return dsKeyCode_ThousandsSeparator;
		case SDL_SCANCODE_DECIMALSEPARATOR:
			return dsKeyCode_DecimalSeparator;
		case SDL_SCANCODE_CURRENCYUNIT:
			return dsKeyCode_CurrencyUnit;
		case SDL_SCANCODE_CURRENCYSUBUNIT:
			return dsKeyCode_CurrencySubUnit;
		case SDL_SCANCODE_KP_LEFTPAREN:
			return dsKeyCode_KpLeftParen;
		case SDL_SCANCODE_KP_RIGHTPAREN:
			return dsKeyCode_KpRightParen;
		case SDL_SCANCODE_KP_LEFTBRACE:
			return dsKeyCode_KpLeftBrace;
		case SDL_SCANCODE_KP_RIGHTBRACE:
			return dsKeyCode_KpRightBrace;
		case SDL_SCANCODE_KP_TAB:
			return dsKeyCode_KpTab;
		case SDL_SCANCODE_KP_BACKSPACE:
			return dsKeyCode_KpBackspace;
		case SDL_SCANCODE_KP_A:
			return dsKeyCode_KpA;
		case SDL_SCANCODE_KP_B:
			return dsKeyCode_KpB;
		case SDL_SCANCODE_KP_C:
			return dsKeyCode_KpC;
		case SDL_SCANCODE_KP_D:
			return dsKeyCode_KpD;
		case SDL_SCANCODE_KP_E:
			return dsKeyCode_KpE;
		case SDL_SCANCODE_KP_F:
			return dsKeyCode_KpF;
		case SDL_SCANCODE_KP_XOR:
			return dsKeyCode_KpXOr;
		case SDL_SCANCODE_KP_POWER:
			return dsKeyCode_KpPower;
		case SDL_SCANCODE_KP_PERCENT:
			return dsKeyCode_KpPercent;
		case SDL_SCANCODE_KP_LESS:
			return dsKeyCode_KpLess;
		case SDL_SCANCODE_KP_GREATER:
			return dsKeyCode_KpGreater;
		case SDL_SCANCODE_KP_AMPERSAND:
			return dsKeyCode_KpAmperstand;
		case SDL_SCANCODE_KP_DBLAMPERSAND:
			return dsKeyCode_KpDoubleAmperstand;
		case SDL_SCANCODE_KP_VERTICALBAR:
			return dsKeyCode_KpPipe;
		case SDL_SCANCODE_KP_DBLVERTICALBAR:
			return dsKeyCode_KpDoublePipe;
		case SDL_SCANCODE_KP_COLON:
			return dsKeyCode_KpColon;
		case SDL_SCANCODE_KP_HASH:
			return dsKeyCode_KpHash;
		case SDL_SCANCODE_KP_SPACE:
			return dsKeyCode_KpSpace;
		case SDL_SCANCODE_KP_AT:
			return dsKeyCode_KpAt;
		case SDL_SCANCODE_KP_EXCLAM:
			return dsKeyCode_KpExclamation;
		case SDL_SCANCODE_KP_MEMSTORE:
			return dsKeyCode_KpMemStore;
		case SDL_SCANCODE_KP_MEMRECALL:
			return dsKeyCode_KpMemRecall;
		case SDL_SCANCODE_KP_MEMCLEAR:
			return dsKeyCode_KpMemClear;
		case SDL_SCANCODE_KP_MEMADD:
			return dsKeyCode_KpMemAdd;
		case SDL_SCANCODE_KP_MEMSUBTRACT:
			return dsKeyCode_KpMemSubtract;
		case SDL_SCANCODE_KP_MEMMULTIPLY:
			return dsKeyCode_KpMemMultiply;
		case SDL_SCANCODE_KP_MEMDIVIDE:
			return dsKeyCode_KpMemDivide;
		case SDL_SCANCODE_KP_PLUSMINUS:
			return dsKeyCode_KpPlusMinus;
		case SDL_SCANCODE_KP_CLEAR:
			return dsKeyCode_KpClear;
		case SDL_SCANCODE_KP_CLEARENTRY:
			return dsKeyCode_KpClearEntry;
		case SDL_SCANCODE_KP_BINARY:
			return dsKeyCode_KpBinary;
		case SDL_SCANCODE_KP_OCTAL:
			return dsKeyCode_KpOctal;
		case SDL_SCANCODE_KP_DECIMAL:
			return dsKeyCode_KpDecimal;
		case SDL_SCANCODE_KP_HEXADECIMAL:
			return dsKeyCode_KpHex;
		case SDL_SCANCODE_LCTRL:
			return dsKeyCode_LControl;
		case SDL_SCANCODE_LSHIFT:
			return dsKeyCode_LShift;
		case SDL_SCANCODE_LALT:
			return dsKeyCode_LAlt;
		case SDL_SCANCODE_LGUI:
			return dsKeyCode_LWinCmd;
		case SDL_SCANCODE_RCTRL:
			return dsKeyCode_RControl;
		case SDL_SCANCODE_RSHIFT:
			return dsKeyCode_RShift;
		case SDL_SCANCODE_RALT:
			return dsKeyCode_RAlt;
		case SDL_SCANCODE_RGUI:
			return dsKeyCode_RWinCmd;
		case SDL_SCANCODE_MODE:
			return dsKeyCode_Mode;
		case SDL_SCANCODE_AUDIONEXT:
			return dsKeyCode_AudioNext;
		case SDL_SCANCODE_AUDIOPREV:
			return dsKeyCode_AudioPrev;
		case SDL_SCANCODE_AUDIOSTOP:
			return dsKeyCode_AudioStop;
		case SDL_SCANCODE_AUDIOPLAY:
			return dsKeyCode_AudioPlay;
		case SDL_SCANCODE_AUDIOMUTE:
			return dsKeyCode_AudioMute;
		case SDL_SCANCODE_MEDIASELECT:
			return dsKeyCode_MediaSelect;
		case SDL_SCANCODE_WWW:
			return dsKeyCode_WWW;
		case SDL_SCANCODE_MAIL:
			return dsKeyCode_Mail;
		case SDL_SCANCODE_CALCULATOR:
			return dsKeyCode_Calculator;
		case SDL_SCANCODE_COMPUTER:
			return dsKeyCode_Computer;
		case SDL_SCANCODE_AC_SEARCH:
			return dsKeyCode_ACSearch;
		case SDL_SCANCODE_AC_HOME:
			return dsKeyCode_ACHome;
		case SDL_SCANCODE_AC_BACK:
			return dsKeyCode_ACBack;
		case SDL_SCANCODE_AC_FORWARD:
			return dsKeyCode_ACForward;
		case SDL_SCANCODE_AC_STOP:
			return dsKeyCode_ACStop;
		case SDL_SCANCODE_AC_REFRESH:
			return dsKeyCode_ACRefresh;
		case SDL_SCANCODE_AC_BOOKMARKS:
			return dsKeyCode_ACBookmarks;
		case SDL_SCANCODE_BRIGHTNESSDOWN:
			return dsKeyCode_BrightnessDown;
		case SDL_SCANCODE_BRIGHTNESSUP:
			return dsKeyCode_BrightnessUp;
		case SDL_SCANCODE_DISPLAYSWITCH:
			return dsKeyCode_DisplaySwitch;
		case SDL_SCANCODE_KBDILLUMTOGGLE:
			return dsKeyCode_KbdIllumToggle;
		case SDL_SCANCODE_KBDILLUMDOWN:
			return dsKeyCode_KbdIllumDown;
		case SDL_SCANCODE_KBDILLUMUP:
			return dsKeyCode_KbdIllumUp;
		case SDL_SCANCODE_EJECT:
			return dsKeyCode_Eject;
		case SDL_SCANCODE_SLEEP:
			return dsKeyCode_Sleep;
		case SDL_SCANCODE_APP1:
			return dsKeyCode_App1;
		case SDL_SCANCODE_APP2:
			return dsKeyCode_App2;
		case SDL_SCANCODE_AUDIOREWIND:
			return dsKeyCode_AudioRewind;
		case SDL_SCANCODE_AUDIOFASTFORWARD:
			return dsKeyCode_AudioFastForward;
		case SDL_NUM_SCANCODES:
			return dsKeyCode_Count;
	}

	return dsKeyCode_Count;
}

SDL_Scancode dsToSDLScancode(dsKeyCode keyCode)
{
	switch (keyCode)
	{
		case dsKeyCode_A:
			return SDL_SCANCODE_A;
		case dsKeyCode_B:
			return SDL_SCANCODE_B;
		case dsKeyCode_C:
			return SDL_SCANCODE_C;
		case dsKeyCode_D:
			return SDL_SCANCODE_D;
		case dsKeyCode_E:
			return SDL_SCANCODE_E;
		case dsKeyCode_F:
			return SDL_SCANCODE_F;
		case dsKeyCode_G:
			return SDL_SCANCODE_G;
		case dsKeyCode_H:
			return SDL_SCANCODE_H;
		case dsKeyCode_I:
			return SDL_SCANCODE_I;
		case dsKeyCode_J:
			return SDL_SCANCODE_J;
		case dsKeyCode_K:
			return SDL_SCANCODE_K;
		case dsKeyCode_L:
			return SDL_SCANCODE_L;
		case dsKeyCode_M:
			return SDL_SCANCODE_M;
		case dsKeyCode_N:
			return SDL_SCANCODE_N;
		case dsKeyCode_O:
			return SDL_SCANCODE_O;
		case dsKeyCode_P:
			return SDL_SCANCODE_P;
		case dsKeyCode_Q:
			return SDL_SCANCODE_Q;
		case dsKeyCode_R:
			return SDL_SCANCODE_R;
		case dsKeyCode_S:
			return SDL_SCANCODE_S;
		case dsKeyCode_T:
			return SDL_SCANCODE_T;
		case dsKeyCode_U:
			return SDL_SCANCODE_U;
		case dsKeyCode_V:
			return SDL_SCANCODE_V;
		case dsKeyCode_W:
			return SDL_SCANCODE_W;
		case dsKeyCode_X:
			return SDL_SCANCODE_X;
		case dsKeyCode_Y:
			return SDL_SCANCODE_Y;
		case dsKeyCode_Z:
			return SDL_SCANCODE_Z;
		case dsKeyCode_1:
			return SDL_SCANCODE_1;
		case dsKeyCode_2:
			return SDL_SCANCODE_2;
		case dsKeyCode_3:
			return SDL_SCANCODE_3;
		case dsKeyCode_4:
			return SDL_SCANCODE_4;
		case dsKeyCode_5:
			return SDL_SCANCODE_5;
		case dsKeyCode_6:
			return SDL_SCANCODE_6;
		case dsKeyCode_7:
			return SDL_SCANCODE_7;
		case dsKeyCode_8:
			return SDL_SCANCODE_8;
		case dsKeyCode_9:
			return SDL_SCANCODE_9;
		case dsKeyCode_0:
			return SDL_SCANCODE_0;
		case dsKeyCode_Enter:
			return SDL_SCANCODE_RETURN;
		case dsKeyCode_Escape:
			return SDL_SCANCODE_ESCAPE;
		case dsKeyCode_Backspace:
			return SDL_SCANCODE_BACKSPACE;
		case dsKeyCode_Tab:
			return SDL_SCANCODE_TAB;
		case dsKeyCode_Space:
			return SDL_SCANCODE_SPACE;
		case dsKeyCode_Minus:
			return SDL_SCANCODE_MINUS;
		case dsKeyCode_Equals:
			return SDL_SCANCODE_EQUALS;
		case dsKeyCode_LeftBracket:
			return SDL_SCANCODE_LEFTBRACKET;
		case dsKeyCode_RightBracket:
			return SDL_SCANCODE_RIGHTBRACKET;
		case dsKeyCode_Backslash:
			return SDL_SCANCODE_BACKSLASH;
		case dsKeyCode_NonUSHash:
			return SDL_SCANCODE_NONUSHASH;
		case dsKeyCode_Semicolon:
			return SDL_SCANCODE_SEMICOLON;
		case dsKeyCode_Apostrophe:
			return SDL_SCANCODE_APOSTROPHE;
		case dsKeyCode_Grave:
			return SDL_SCANCODE_GRAVE;
		case dsKeyCode_Comma:
			return SDL_SCANCODE_COMMA;
		case dsKeyCode_Period:
			return SDL_SCANCODE_PERIOD;
		case dsKeyCode_Slash:
			return SDL_SCANCODE_SLASH;
		case dsKeyCode_Capslock:
			return SDL_SCANCODE_CAPSLOCK;
		case dsKeyCode_F1:
			return SDL_SCANCODE_F1;
		case dsKeyCode_F2:
			return SDL_SCANCODE_F2;
		case dsKeyCode_F3:
			return SDL_SCANCODE_F3;
		case dsKeyCode_F4:
			return SDL_SCANCODE_F4;
		case dsKeyCode_F5:
			return SDL_SCANCODE_F5;
		case dsKeyCode_F6:
			return SDL_SCANCODE_F6;
		case dsKeyCode_F7:
			return SDL_SCANCODE_F7;
		case dsKeyCode_F8:
			return SDL_SCANCODE_F8;
		case dsKeyCode_F9:
			return SDL_SCANCODE_F9;
		case dsKeyCode_F10:
			return SDL_SCANCODE_F10;
		case dsKeyCode_F11:
			return SDL_SCANCODE_F11;
		case dsKeyCode_F12:
			return SDL_SCANCODE_F12;
		case dsKeyCode_PrintScreen:
			return SDL_SCANCODE_PRINTSCREEN;
		case dsKeyCode_ScrollLock:
			return SDL_SCANCODE_SCROLLLOCK;
		case dsKeyCode_Pause:
			return SDL_SCANCODE_PAUSE;
		case dsKeyCode_Insert:
			return SDL_SCANCODE_INSERT;
		case dsKeyCode_Home:
			return SDL_SCANCODE_HOME;
		case dsKeyCode_PageUp:
			return SDL_SCANCODE_PAGEUP;
		case dsKeyCode_Delete:
			return SDL_SCANCODE_DELETE;
		case dsKeyCode_End:
			return SDL_SCANCODE_END;
		case dsKeyCode_PageDown:
			return SDL_SCANCODE_PAGEDOWN;
		case dsKeyCode_Right:
			return SDL_SCANCODE_RIGHT;
		case dsKeyCode_Left:
			return SDL_SCANCODE_LEFT;
		case dsKeyCode_Down:
			return SDL_SCANCODE_DOWN;
		case dsKeyCode_Up:
			return SDL_SCANCODE_UP;
		case dsKeyCode_NumLock:
			return SDL_SCANCODE_NUMLOCKCLEAR;
		case dsKeyCode_KpDivide:
			return SDL_SCANCODE_KP_DIVIDE;
		case dsKeyCode_KpMultiply:
			return SDL_SCANCODE_KP_MULTIPLY;
		case dsKeyCode_KpMinus:
			return SDL_SCANCODE_KP_MINUS;
		case dsKeyCode_KpPlus:
			return SDL_SCANCODE_KP_PLUS;
		case dsKeyCode_KpEnter:
			return SDL_SCANCODE_KP_ENTER;
		case dsKeyCode_Kp1:
			return SDL_SCANCODE_KP_1;
		case dsKeyCode_Kp2:
			return SDL_SCANCODE_KP_2;
		case dsKeyCode_Kp3:
			return SDL_SCANCODE_KP_3;
		case dsKeyCode_Kp4:
			return SDL_SCANCODE_KP_4;
		case dsKeyCode_Kp5:
			return SDL_SCANCODE_KP_5;
		case dsKeyCode_Kp6:
			return SDL_SCANCODE_KP_6;
		case dsKeyCode_Kp7:
			return SDL_SCANCODE_KP_7;
		case dsKeyCode_Kp8:
			return SDL_SCANCODE_KP_8;
		case dsKeyCode_Kp9:
			return SDL_SCANCODE_KP_9;
		case dsKeyCode_Kp0:
			return SDL_SCANCODE_KP_0;
		case dsKeyCode_KpPeriod:
			return SDL_SCANCODE_KP_PERIOD;
		case dsKeyCode_NonUSBackslash:
			return SDL_SCANCODE_NONUSBACKSLASH;
		case dsKeyCode_ContextMenu:
			return SDL_SCANCODE_APPLICATION;
		case dsKeyCode_Power:
			return SDL_SCANCODE_POWER;
		case dsKeyCode_KpEquals:
			return SDL_SCANCODE_KP_EQUALS;
		case dsKeyCode_F13:
			return SDL_SCANCODE_F13;
		case dsKeyCode_F14:
			return SDL_SCANCODE_F14;
		case dsKeyCode_F15:
			return SDL_SCANCODE_F15;
		case dsKeyCode_F16:
			return SDL_SCANCODE_F16;
		case dsKeyCode_F17:
			return SDL_SCANCODE_F17;
		case dsKeyCode_F18:
			return SDL_SCANCODE_F18;
		case dsKeyCode_F19:
			return SDL_SCANCODE_F19;
		case dsKeyCode_F20:
			return SDL_SCANCODE_F20;
		case dsKeyCode_F21:
			return SDL_SCANCODE_F21;
		case dsKeyCode_F22:
			return SDL_SCANCODE_F22;
		case dsKeyCode_F23:
			return SDL_SCANCODE_F23;
		case dsKeyCode_F24:
			return SDL_SCANCODE_F24;
		case dsKeyCode_Execute:
			return SDL_SCANCODE_EXECUTE;
		case dsKeyCode_Help:
			return SDL_SCANCODE_HELP;
		case dsKeyCode_Menu:
			return SDL_SCANCODE_MENU;
		case dsKeyCode_Select:
			return SDL_SCANCODE_SELECT;
		case dsKeyCode_Stop:
			return SDL_SCANCODE_STOP;
		case dsKeyCode_Again:
			return SDL_SCANCODE_AGAIN;
		case dsKeyCode_Undo:
			return SDL_SCANCODE_UNDO;
		case dsKeyCode_Cut:
			return SDL_SCANCODE_CUT;
		case dsKeyCode_Copy:
			return SDL_SCANCODE_COPY;
		case dsKeyCode_Paste:
			return SDL_SCANCODE_PASTE;
		case dsKeyCode_Find:
			return SDL_SCANCODE_FIND;
		case dsKeyCode_Mute:
			return SDL_SCANCODE_MUTE;
		case dsKeyCode_VolumeUp:
			return SDL_SCANCODE_VOLUMEUP;
		case dsKeyCode_VolumeDown:
			return SDL_SCANCODE_VOLUMEDOWN;
		case dsKeyCode_KpComma:
			return SDL_SCANCODE_KP_COMMA;
		case dsKeyCode_KpEqualsAS400:
			return SDL_SCANCODE_KP_EQUALSAS400;
		case dsKeyCode_International1:
			return SDL_SCANCODE_INTERNATIONAL1;
		case dsKeyCode_International2:
			return SDL_SCANCODE_INTERNATIONAL2;
		case dsKeyCode_International3:
			return SDL_SCANCODE_INTERNATIONAL3;
		case dsKeyCode_International4:
			return SDL_SCANCODE_INTERNATIONAL4;
		case dsKeyCode_International5:
			return SDL_SCANCODE_INTERNATIONAL5;
		case dsKeyCode_International6:
			return SDL_SCANCODE_INTERNATIONAL6;
		case dsKeyCode_International7:
			return SDL_SCANCODE_INTERNATIONAL7;
		case dsKeyCode_International8:
			return SDL_SCANCODE_INTERNATIONAL8;
		case dsKeyCode_International9:
			return SDL_SCANCODE_INTERNATIONAL9;
		case dsKeyCode_Lang1:
			return SDL_SCANCODE_LANG1;
		case dsKeyCode_Lang2:
			return SDL_SCANCODE_LANG2;
		case dsKeyCode_Lang3:
			return SDL_SCANCODE_LANG3;
		case dsKeyCode_Lang4:
			return SDL_SCANCODE_LANG4;
		case dsKeyCode_Lang5:
			return SDL_SCANCODE_LANG5;
		case dsKeyCode_Lang6:
			return SDL_SCANCODE_LANG6;
		case dsKeyCode_Lang7:
			return SDL_SCANCODE_LANG7;
		case dsKeyCode_Lang8:
			return SDL_SCANCODE_LANG8;
		case dsKeyCode_Lang9:
			return SDL_SCANCODE_LANG9;
		case dsKeyCode_AltErase:
			return SDL_SCANCODE_ALTERASE;
		case dsKeyCode_SysReq:
			return SDL_SCANCODE_SYSREQ;
		case dsKeyCode_Cancel:
			return SDL_SCANCODE_CANCEL;
		case dsKeyCode_Clear:
			return SDL_SCANCODE_CLEAR;
		case dsKeyCode_Prior:
			return SDL_SCANCODE_PRIOR;
		case dsKeyCode_Return:
			return SDL_SCANCODE_RETURN2;
		case dsKeyCode_Separator:
			return SDL_SCANCODE_SEPARATOR;
		case dsKeyCode_Out:
			return SDL_SCANCODE_OUT;
		case dsKeyCode_Oper:
			return SDL_SCANCODE_OPER;
		case dsKeyCode_ClearAgain:
			return SDL_SCANCODE_CLEARAGAIN;
		case dsKeyCode_CrSel:
			return SDL_SCANCODE_CRSEL;
		case dsKeyCode_ExSel:
			return SDL_SCANCODE_EXSEL;
		case dsKeyCode_Kp00:
			return SDL_SCANCODE_KP_00;
		case dsKeyCode_Kp000:
			return SDL_SCANCODE_KP_000;
		case dsKeyCode_ThousandsSeparator:
			return SDL_SCANCODE_THOUSANDSSEPARATOR;
		case dsKeyCode_DecimalSeparator:
			return SDL_SCANCODE_DECIMALSEPARATOR;
		case dsKeyCode_CurrencyUnit:
			return SDL_SCANCODE_CURRENCYUNIT;
		case dsKeyCode_CurrencySubUnit:
			return SDL_SCANCODE_CURRENCYSUBUNIT;
		case dsKeyCode_KpLeftParen:
			return SDL_SCANCODE_KP_LEFTPAREN;
		case dsKeyCode_KpRightParen:
			return SDL_SCANCODE_KP_RIGHTPAREN;
		case dsKeyCode_KpLeftBrace:
			return SDL_SCANCODE_KP_LEFTBRACE;
		case dsKeyCode_KpRightBrace:
			return SDL_SCANCODE_KP_RIGHTBRACE;
		case dsKeyCode_KpTab:
			return SDL_SCANCODE_KP_TAB;
		case dsKeyCode_KpBackspace:
			return SDL_SCANCODE_KP_BACKSPACE;
		case dsKeyCode_KpA:
			return SDL_SCANCODE_KP_A;
		case dsKeyCode_KpB:
			return SDL_SCANCODE_KP_B;
		case dsKeyCode_KpC:
			return SDL_SCANCODE_KP_C;
		case dsKeyCode_KpD:
			return SDL_SCANCODE_KP_D;
		case dsKeyCode_KpE:
			return SDL_SCANCODE_KP_E;
		case dsKeyCode_KpF:
			return SDL_SCANCODE_KP_F;
		case dsKeyCode_KpXOr:
			return SDL_SCANCODE_KP_XOR;
		case dsKeyCode_KpPower:
			return SDL_SCANCODE_KP_POWER;
		case dsKeyCode_KpPercent:
			return SDL_SCANCODE_KP_PERCENT;
		case dsKeyCode_KpLess:
			return SDL_SCANCODE_KP_LESS;
		case dsKeyCode_KpGreater:
			return SDL_SCANCODE_KP_GREATER;
		case dsKeyCode_KpAmperstand:
			return SDL_SCANCODE_KP_AMPERSAND;
		case dsKeyCode_KpDoubleAmperstand:
			return SDL_SCANCODE_KP_DBLAMPERSAND;
		case dsKeyCode_KpPipe:
			return SDL_SCANCODE_KP_VERTICALBAR;
		case dsKeyCode_KpDoublePipe:
			return SDL_SCANCODE_KP_DBLVERTICALBAR;
		case dsKeyCode_KpColon:
			return SDL_SCANCODE_KP_COLON;
		case dsKeyCode_KpHash:
			return SDL_SCANCODE_KP_HASH;
		case dsKeyCode_KpSpace:
			return SDL_SCANCODE_KP_SPACE;
		case dsKeyCode_KpAt:
			return SDL_SCANCODE_KP_AT;
		case dsKeyCode_KpExclamation:
			return SDL_SCANCODE_KP_EXCLAM;
		case dsKeyCode_KpMemStore:
			return SDL_SCANCODE_KP_MEMSTORE;
		case dsKeyCode_KpMemRecall:
			return SDL_SCANCODE_KP_MEMRECALL;
		case dsKeyCode_KpMemClear:
			return SDL_SCANCODE_KP_MEMCLEAR;
		case dsKeyCode_KpMemAdd:
			return SDL_SCANCODE_KP_MEMADD;
		case dsKeyCode_KpMemSubtract:
			return SDL_SCANCODE_KP_MEMSUBTRACT;
		case dsKeyCode_KpMemMultiply:
			return SDL_SCANCODE_KP_MEMMULTIPLY;
		case dsKeyCode_KpMemDivide:
			return SDL_SCANCODE_KP_MEMDIVIDE;
		case dsKeyCode_KpPlusMinus:
			return SDL_SCANCODE_KP_PLUSMINUS;
		case dsKeyCode_KpClear:
			return SDL_SCANCODE_KP_CLEAR;
		case dsKeyCode_KpClearEntry:
			return SDL_SCANCODE_KP_CLEARENTRY;
		case dsKeyCode_KpBinary:
			return SDL_SCANCODE_KP_BINARY;
		case dsKeyCode_KpOctal:
			return SDL_SCANCODE_KP_OCTAL;
		case dsKeyCode_KpDecimal:
			return SDL_SCANCODE_KP_DECIMAL;
		case dsKeyCode_KpHex:
			return SDL_SCANCODE_KP_HEXADECIMAL;
		case dsKeyCode_LControl:
			return SDL_SCANCODE_LCTRL;
		case dsKeyCode_LShift:
			return SDL_SCANCODE_LSHIFT;
		case dsKeyCode_LAlt:
			return SDL_SCANCODE_LALT;
		case dsKeyCode_LWinCmd:
			return SDL_SCANCODE_LGUI;
		case dsKeyCode_RControl:
			return SDL_SCANCODE_RCTRL;
		case dsKeyCode_RShift:
			return SDL_SCANCODE_RSHIFT;
		case dsKeyCode_RAlt:
			return SDL_SCANCODE_RALT;
		case dsKeyCode_RWinCmd:
			return SDL_SCANCODE_RGUI;
		case dsKeyCode_Mode:
			return SDL_SCANCODE_MODE;
		case dsKeyCode_AudioNext:
			return SDL_SCANCODE_AUDIONEXT;
		case dsKeyCode_AudioPrev:
			return SDL_SCANCODE_AUDIOPREV;
		case dsKeyCode_AudioStop:
			return SDL_SCANCODE_AUDIOSTOP;
		case dsKeyCode_AudioPlay:
			return SDL_SCANCODE_AUDIOPLAY;
		case dsKeyCode_AudioMute:
			return SDL_SCANCODE_AUDIOMUTE;
		case dsKeyCode_MediaSelect:
			return SDL_SCANCODE_MEDIASELECT;
		case dsKeyCode_WWW:
			return SDL_SCANCODE_WWW;
		case dsKeyCode_Mail:
			return SDL_SCANCODE_MAIL;
		case dsKeyCode_Calculator:
			return SDL_SCANCODE_CALCULATOR;
		case dsKeyCode_Computer:
			return SDL_SCANCODE_COMPUTER;
		case dsKeyCode_ACSearch:
			return SDL_SCANCODE_AC_SEARCH;
		case dsKeyCode_ACHome:
			return SDL_SCANCODE_AC_HOME;
		case dsKeyCode_ACBack:
			return SDL_SCANCODE_AC_BACK;
		case dsKeyCode_ACForward:
			return SDL_SCANCODE_AC_FORWARD;
		case dsKeyCode_ACStop:
			return SDL_SCANCODE_AC_STOP;
		case dsKeyCode_ACRefresh:
			return SDL_SCANCODE_AC_REFRESH;
		case dsKeyCode_ACBookmarks:
			return SDL_SCANCODE_AC_BOOKMARKS;
		case dsKeyCode_BrightnessDown:
			return SDL_SCANCODE_BRIGHTNESSDOWN;
		case dsKeyCode_BrightnessUp:
			return SDL_SCANCODE_BRIGHTNESSUP;
		case dsKeyCode_DisplaySwitch:
			return SDL_SCANCODE_DISPLAYSWITCH;
		case dsKeyCode_KbdIllumToggle:
			return SDL_SCANCODE_KBDILLUMTOGGLE;
		case dsKeyCode_KbdIllumDown:
			return SDL_SCANCODE_KBDILLUMDOWN;
		case dsKeyCode_KbdIllumUp:
			return SDL_SCANCODE_KBDILLUMUP;
		case dsKeyCode_Eject:
			return SDL_SCANCODE_EJECT;
		case dsKeyCode_Sleep:
			return SDL_SCANCODE_SLEEP;
		case dsKeyCode_App1:
			return SDL_SCANCODE_APP1;
		case dsKeyCode_App2:
			return SDL_SCANCODE_APP2;
		case dsKeyCode_AudioRewind:
			return SDL_SCANCODE_AUDIOREWIND;
		case dsKeyCode_AudioFastForward:
			return SDL_SCANCODE_AUDIOFASTFORWARD;
		case dsKeyCode_Count:
			return SDL_SCANCODE_UNKNOWN;
	}

	return SDL_SCANCODE_UNKNOWN;
}

dsKeyModifier dsFromSDLKeyMod(Uint16 modifiers)
{
	uint32_t curMods = 0;
	if (modifiers & KMOD_LSHIFT)
		curMods |= dsKeyModifier_LShift;
	if (modifiers & KMOD_RSHIFT)
		curMods |= dsKeyModifier_RShift;
	if (modifiers & KMOD_LCTRL)
		curMods |= dsKeyModifier_LCtrl;
	if (modifiers & KMOD_RCTRL)
		curMods |= dsKeyModifier_RCtrl;
	if (modifiers & KMOD_LALT)
		curMods |= dsKeyModifier_LAlt;
	if (modifiers & KMOD_RALT)
		curMods |= dsKeyModifier_RAlt;
	if (modifiers & KMOD_LGUI)
		curMods |= dsKeyModifier_LWinCmd;
	if (modifiers & KMOD_RGUI)
		curMods |= dsKeyModifier_RWinCmd;
	if (modifiers & KMOD_NUM)
		curMods |= dsKeyModifier_NumLock;
	if (modifiers & KMOD_CAPS)
		curMods |= dsKeyModifier_CapsLock;
	if (modifiers & KMOD_MODE)
		curMods |= dsKeyModifier_Mode;

	return (dsKeyModifier)curMods;
}
