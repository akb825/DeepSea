#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Export.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions and macros used to print log messages.
 *
 * By default messages will output to stdout (trace, debug, and info) or stderr (warning, error,
 * fatal). The logging function can be overridden by calling dsLog_setFunction().
 */

/**
 * @brief The maximum length of a formatted log message.
 */
#define DS_LOG_MAX_LENGTH 1024

/**
 * @brief The level of the log message.
 */
typedef enum dsLog_Level
{
	dsLog_Level_Trace,		///< A trace message for status updates.
	dsLog_Level_Debug,		///< A debug message.
	dsLog_Level_Info,		///< An info message.
	dsLog_Level_Warning,	///< A warning message that could indicate a problem.
	dsLog_Level_Error,		///< An error message that indicates a problem.
	dsLog_Level_Fatal		///< A fatal message that indicates execution cannot continue.
} dsLog_Level;

/**
 * @brief Type of the logging function.
 * @remark This may be called across multiple threads.
 * @param userData User data for the logging function.
 * @param level The level of the message.
 * @param tag The tag for the message.
 * @param file The file for the message.
 * @param line The line for the message.
 * @param function The function for the message.
 * @param message The log message.
 */
typedef void (*dsLog_FunctionType)(void* userData, dsLog_Level level, const char* tag,
	const char* file, unsigned int line, const char* function, const char* message);

/**
 * @brief Sets the logging function.
 * @param userData The user data for the logging function.
 * @param function The function to send the log messages to.
 */
DS_CORE_EXPORT void dsLog_setFunction(void* userData, dsLog_FunctionType function);

/**
 * @brief Gets the logging user data.
 * @return The user data.
 */
DS_CORE_EXPORT void* dsLog_getUserData();

/**
 * @brief Gets the logging function.
 * @return The user data.
 */
DS_CORE_EXPORT void* dsLog_getFunction();

/**
 * @brief Clears the function.
 */
DS_CORE_EXPORT void dsLog_clearFunction();

/**
 * @brief Logs a message without formatting.
 * @param level The level of the message.
 * @param tag The tag for the message.
 * @param file The file for the message.
 * @param line The line for the message.
 * @param function The function for the message.
 * @param message The log message.
 */
DS_CORE_EXPORT void dsLog_message(dsLog_Level level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message);

/**
 * @brief Logs a message with printf style formatting.
 * @param level The level of the message.
 * @param tag The tag for the message.
 * @param file The file for the message.
 * @param line The line for the message.
 * @param function The function for the message.
 * @param message The log message.
 */
DS_CORE_EXPORT void dsLog_messagef(dsLog_Level level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message, ...);

/**
 * @brief Logs a message with printf style formatting.
 * @param level The level of the message.
 * @param tag The tag for the message.
 * @param file The file for the message.
 * @param line The line for the message.
 * @param function The function for the message.
 * @param message The log message.
 * @param args The formatting arguments.
 */
DS_CORE_EXPORT void dsLog_vmessagef(dsLog_Level level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message, va_list args);

#ifdef __cplusplus
}
#endif
