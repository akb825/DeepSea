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

#include <DeepSea/Core/Config.h>

#include "SDLShared.h"
#include <stdbool.h>

#if DS_MAC
#include <AppKit/NSWindow.h>
#else
#include <UIKit/UIWindow.h>
#endif

#if DS_APPLE

void* dsSDLWindow_getUsableWindowHandle(void* window)
{
#if DS_MAC
	NSView* view = [(__bridge NSWindow*)window contentView];
	if (view.subviews.count > 0)
		view = view.subviews[0];
	return (void*)CFBridgingRetain(view);
#else
#define METALVIEW_TAG 255
	DS_UNUSED(rendererID);
	return (void*)CFBridgingRetain([(__bridge UIWindow*)window viewWithTag: METALVIEW_TAG]);
#endif
}

void dsSDLWindow_releaseUsableWindowHandle(void* handle)
{
	if (handle)
		CFRelease(handle);
}

#endif // DS_MAC
