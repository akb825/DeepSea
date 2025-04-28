/*
 * Copyright 2017-2025 Aaron Barany
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
#ifndef __AnyGLConfig_h_
#define __AnyGLConfig_h_ 1

#if defined(_WIN32)
#	define ANYGL_WINDOWS 1
#elif defined(__ANDROID__)
#	define ANYGL_ANDROID 1
#elif defined(__APPLE__)
#	define ANYGL_APPLE 1
#	include "TargetConditionals.h"
#	if TARGET_OS_IPHONE
#		define ANYGL_IOS 1
#	endif
#endif

#ifndef ANYGL_WINDOWS
#define ANYGL_WINDOWS 0
#endif

#ifndef ANYGL_ANDROID
#define ANYGL_ANDROID 0
#endif

#ifndef ANYGL_APPLE
#define ANYGL_APPLE 0
#endif

#ifndef ANYGL_IOS
#define ANYGL_IOS 0
#endif

/* #define this to override whether or not to use OpenGL ES. */
#ifndef ANYGL_GLES
#define ANYGL_GLES (ANYGL_ANDROID || ANYGL_IOS)
#endif

/* @define this to force usage of EGL, even when platform-specific libraries are available. */
#ifndef ANYGL_FORCE_EGL
#define ANYGL_FORCE_EGL 0
#endif

/* #define this to the OpenGL version (times 10) to include when loading via function pointer. */
#ifndef ANYGL_GL_VERSION
#define ANYGL_GL_VERSION 33
#endif

/* #define this to the OpenGL ES version (times 10) to include when loading via function pointer. */
#ifndef ANYGL_GLES_VERSION
#define ANYGL_GLES_VERSION 30
#endif

/*
 * Libraries for loading OpenGL functions.
 * ANYGL_LOAD_FPTR takes the function pointer from the system OpenGL includes.
 */
#define ANYGL_LOAD_FPTR 0
#define ANYGL_LOAD_EGL  1
#define ANYGL_LOAD_WGL  2
#define ANYGL_LOAD_GLX  3

/* Which load methods are available. These can be manually defined if needed. */
#ifndef ANYGL_HAS_FPTR
#if ANGL_APPLE
#	define ANYGL_HAS_FPTR 1
#else
#	define ANYGL_HAS_FPTR 0
#endif
#endif

#ifndef ANYGL_HAS_EGL
#if ANYGL_GLES || ANYGL_FORCE_EGL
#	define ANYGL_HAS_EGL 1
#else
#	define ANYGL_HAS_EGL 0
#endif
#endif

#ifndef ANYGL_HAS_WGL
#if ANYGL_WINDOWS
#	define ANYGL_HAS_WGL 1
#else
#	define ANYGL_HAS_WGL 0
#endif
#endif

#ifndef ANYGL_HAS_GLX
#if !ANYGL_APPLE && !ANYGL_GLES && !ANYGL_WINDOWS
#	define ANYGL_HAS_GLX 1
#else
#	define ANYGL_HAS_GLX 0
#endif
#endif

/* #define this to override the default library. */
#ifndef ANYGL_LOAD_DEFAULT
#if ANYGL_APPLE
#	define ANYGL_LOAD_DEFAULT ANYGL_LOAD_FPTR
#elif ANYGL_GLES || ANYGL_FORCE_EGL
#	define ANYGL_LOAD_DEFAULT ANYGL_LOAD_EGL
#elif ANYGL_WINDOWS
#	define ANYGL_LOAD_DEFAULT ANYGL_LOAD_WGL
#else
#	define ANYGL_LOAD_DEFAULT ANYGL_LOAD_GLX
#endif
#endif

/*
 * #define ANYGL_DYNAMIC to use dynamic linking.
 * #define ANYGL_BUILD to export symbols, otherwise they will be imported.
 */
#ifndef ANYGL_EXPORT
#if ANYGL_DYNAMIC
#	ifdef _MSC_VER
#		ifdef ANYGL_BUILD
#			define ANYGL_EXPORT __declspec(dllexport)
#		else
#			define ANYGL_EXPORT __declspec(dllimport)
#		endif
#	else
#		define ANYGL_EXPORT __attribute__((visibility("default")))
#	endif
#else
#	define ANYGL_EXPORT
#endif
#endif

/* #define this to override the calling convention. */
#ifndef APIENTRY
#if ANYGL_WINDOWS
#	define APIENTRY __stdcall
#else
#	define APIENTRY
#endif
#endif

/*
 * #define this to 1 if you want to allow debugging OpenGL functions. Useful for debugging, but
 * adds some overhead, so not suitable for release builds.
 */
#ifndef ANYGL_ALLOW_DEBUG
#ifdef NDEBUG
#	define ANYGL_ALLOW_DEBUG 0
#else
#	define ANYGL_ALLOW_DEBUG 1
#endif
#endif

#endif
