#pragma once
#ifndef __AnyGL_h_
#define __AnyGL_h_ 1

#include "AnyGLConfig.h"

/**
 * @brief Initializes the AnyGL library.
 *
 * This will initialize the loading library, such as GLX or WGL. On some platforms this is
 * necessary to load extensions in the loading library required to properly create an OpenGL
 * context.
 *
 * @return 1 if the initialization succeeded, 0 if it failed.
 */
ANYGL_EXPORT int AnyGL_initialize(void);

/**
 * @brief Loads the OpenGL functions based on the currently bound context.
 * @return 1 if the load succeedd, 0 if it failed.
 */
ANYGL_EXPORT int AnyGL_load(void);

/**
 * @brief Gets the version of OpenGL.
 * @param[out] major The major version.
 * @param[out] minor The minor viersion.
 * @param[out] es 1 if OpenGL ES, 0 if desktop OpenGL.
 */
ANYGL_EXPORT void AnyGL_getGLVersion(int* major, int* minor, int* es);

/**
 * @brief Checks to see if the OpenGL version is at least the version provided.
 * @param major The major version.
 * @param minor The minor version.
 * @param es 1 if OpenGL ES, 0 if desktop OpenGL.
 * @return 1 if the OpenGL version is less than or equal to the major and minor version and es
 *     matches whether or not OpenGL ES is enabled.
 */
ANYGL_EXPORT int AnyGL_atLeastVersion(int major, int minor, int es);

/**
 * @brief Shuts down the AnyGL library, freeing any persistently held resources.
 */
ANYGL_EXPORT void AnyGL_shutdown(void);

/**
 * @brief Gets the string for an OpenGL error.
 * @param error The error code returned from glGetError() or glCheckFramebufferStatus().
 * @return The string for the error, or "UNKNOWN" if not a known error code.
 */
ANYGL_EXPORT const char* AnyGL_errorString(unsigned int error);

/**
 * @brief Function pointer type for the error function.
 * @param file The file for the original call into OpenGL.
 * @param function The function for the original call into OpenGL.
 * @Param line The line for the original call into OpenGL.
 * @param glError The OpenGL error code.
 * @param glFunction The GL function called, including parameters passed in.
 */
typedef void (*AnyGLErrorFunc)(const char* file, const char* function, unsigned int line,
	unsigned int glError, const char* glFunction);

/**
 * @brief Sets the error function.
 *
 * This is used when debugging is enabled and a GL error is is thrown.
 *
 * @param errorFunc The function to set. If NULL, the default error function will be used.
 */
ANYGL_EXPORT void AnyGL_setErrorFunc(AnyGLErrorFunc func);

/**
 * @brief Gets whether or not debugging is enabled.
 * @return 1 if debugging is enabled, 0 of not.
 */
ANYGL_EXPORT int AnyGL_getDebugEnabled(void);

/**
 * @brief Sets whether or not debugging is enabled.
 *
 * This will check for an OpenGL error for each function call, calling the error function if one
 * occurred, passing in a string version of the function call with parameter values and information
 * about the callsite. This will do nothing if ANYGL_ALLOW_DEBUG is defined to 0.
 *
 * @param enabled 1 if debugging should be enabled, 0 if not.
 */
ANYGL_EXPORT void AnyGL_setDebugEnabled(int enabled);

/**
 * @brief Gets whether or not error checking is enabled for debug functions.
 * @return 1 if error checking is enabled, 0 if not.
 */
ANYGL_EXPORT int AnyGL_getErrorCheckingEnabled(void);

/**
 * @brief Sets whether or not error checking is enabled for debug functions.
 *
 * This can be used to disable error checking for situations such as loading resources where you
 * may want to check for error codes yourself.

 * @remark This is maintained separately for each thread.
 * @param enabled 1 if error checking should be enabled, 0 if not.
 */
ANYGL_EXPORT void AnyGL_setErrorCheckingEnabled(int enabled);

/**
 * @brief Gets the last OpenGL callsite.
 * @remark This is maintained separately for each thread and is only recorded when
 * ANYGL_ALLOW_DEBUG is 1, though debugging doesn't need to be enabled.
 * @param[out] file The file of the callsite.
 * @param[out] function The function of the callsite.
 * @param[out] line The line of the callsite.
 */
ANYGL_EXPORT void AnyGL_getLastCallsite(const char** file, const char** function,
	unsigned int* line);

#endif
