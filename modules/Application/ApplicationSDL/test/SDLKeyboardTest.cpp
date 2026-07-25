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

#include "SDLKeyboard.h"
#include <gtest/gtest.h>

TEST(SDLKeyboardTest, KeyCode)
{
	for (int i = 0; i < dsKeyCode_Count; ++i)
	{
		dsKeyCode keyCode = (dsKeyCode)i;
		// The following keycodes aren't supported on all versions of SDL.
		switch (keyCode)
		{
			case dsKeyCode_WWW:
			case dsKeyCode_Mail:
			case dsKeyCode_Calculator:
			case dsKeyCode_Computer:
			case dsKeyCode_BrightnessDown:
			case dsKeyCode_BrightnessUp:
			case dsKeyCode_DisplaySwitch:
			case dsKeyCode_KeyboardIlluminationToggle:
			case dsKeyCode_KeyboardIlluminationDown:
			case dsKeyCode_KeyboardIlluminationUp:
				continue;
			default:
				break;
		}
		EXPECT_EQ(keyCode, dsFromSDLScancode(dsToSDLScancode(keyCode)));
	}
}
