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
 * The default behavior is documented in dsLog_defaultPrint().
 */

/**
 * @brief The maximum length of a formatted log message, including the null terminator.
 */
#define DS_LOG_MAX_LENGTH 1024

/**
 * @brief The level of the log message.
 */
typedef enum dsLogLevel
{
	dsLogLevel_Trace,	///< A trace message for status updates.
	dsLogLevel_Debug,	///< A debug message.
	dsLogLevel_Info,	///< An info message.
	dsLogLevel_Warning,	///< A warning message that could indicate a problem.
	dsLogLevel_Error,	///< An error message that indicates a problem.
	dsLogLevel_Fatal	///< A fatal message that indicates execution cannot continue.
} dsLogLevel;

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
typedef void (*dsLog_FunctionType)(void* userData, dsLogLevel level, const char* tag,
	const char* file, unsigned int line, const char* function, const char* message);

/**
 * @brief Default log printing function, which will be called when the logging function is NULL.
 *
 * When compiling to debug, only messages of debug and higher will be printed. When compiling to
 * release, only messages of info and higher will be printed. Messages below the level of warning
 * will be printed to stdout, while messages warning and higher will be printed to stderr.
 *
 * On Windows, messages will also be printed to the debug console.
 *
 * @param level The level of the message.
 * @param tag The tag for the message.
 * @param file The file for the message.
 * @param line The line for the message.
 * @param function The function for the message.
 * @param message The log message.
 */
DS_CORE_EXPORT void dsLog_defaultPrint(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message);

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
DS_CORE_EXPORT void dsLog_message(dsLogLevel level, const char* tag, const char* file,
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
DS_CORE_EXPORT void dsLog_messagef(dsLogLevel level, const char* tag, const char* file,
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
DS_CORE_EXPORT void dsLog_vmessagef(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message, va_list args);

/**
 * @brief Logs a message, populating the current file, line, and function.
 * @param level The log level.
 * @param tag The tag for the message.
 * @param message The message to log.
 */
#define DS_LOG(level, tag, message) \
	dsLog_message(level, tag, __FILE__, __LINE__, __FUNCTION__, message)

/**
 * @brief Logs a trace message, populating the current file, line, and function.
 * @param tag The tag for the message.
 * @param message The message to log.
 */
#define DS_LOG_TRACE(tag, message) DS_LOG(dsLogLevel_Trace, tag, message)

/**
 * @brief Logs a debug message, populating the current file, line, and function.
 * @param tag The tag for the message.
 * @param message The message to log.
 */
#define DS_LOG_DEBUG(tag, message) DS_LOG(dsLogLevel_Debug, tag, message)

/**
 * @brief Logs an info message, populating the current file, line, and function.
 * @param tag The tag for the message.
 * @param message The message to log.
 */
#define DS_LOG_INFO(tag, message) DS_LOG(dsLogLevel_Info, tag, message)

/**
 * @brief Logs a warning message, populating the current file, line, and function.
 * @param tag The tag for the message.
 * @param message The message to log.
 */
#define DS_LOG_WARNING(tag, message) DS_LOG(dsLogLevel_Warning, tag, message)

/**
 * @brief Logs an error message, populating the current file, line, and function.
 * @param tag The tag for the message.
 * @param message The message to log.
 */
#define DS_LOG_ERROR(tag, message) DS_LOG(dsLogLevel_Error, tag, message)

/**
 * @brief Logs a fatal message, populating the current file, line, and function.
 * @param tag The tag for the message.
 * @param message The message to log.
 */
#define DS_LOG_FATAL(tag, message) DS_LOG(dsLogLevel_Fatal, tag, message)

/**
 * @brief Logs a message with formatting, populating the current file, line, and function.
 * @param level The log level.
 * @param tag The tag for the message.
 */
#define DS_LOG_F(level, tag, ...) \
	dsLog_messagef(level, tag, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/**
 * @brief Logs a trace message with formatting, populating the current file, line, and function.
 * @param tag The tag for the message.
 */
#define DS_LOG_TRACE_F(tag, ...) DS_LOG_F(dsLogLevel_Trace, tag, __VA_ARGS__)

/**
 * @brief Logs a debug message with formatting, populating the current file, line, and function.
 * @param tag The tag for the message.
 */
#define DS_LOG_DEBUG_F(tag, ...) DS_LOG_F(dsLogLevel_Debug, tag, __VA_ARGS__)

/**
 * @brief Logs an info message with formatting, populating the current file, line, and function.
 * @param tag The tag for the message.
 */
#define DS_LOG_INFO_F(tag, ...) DS_LOG_F(dsLogLevel_Info, tag, __VA_ARGS__)

/**
 * @brief Logs a warning message with formatting, populating the current file, line, and function.
 * @param tag The tag for the message.
 */
#define DS_LOG_WARNING_F(tag, ...) DS_LOG_F(dsLogLevel_Warning, tag, __VA_ARGS__)

/**
 * @brief Logs an error message with formatting, populating the current file, line, and function.
 * @param tag The tag for the message.
 */
#define DS_LOG_ERROR_F(tag, ...) DS_LOG_F(dsLogLevel_Error, tag, __VA_ARGS__)

/**
 * @brief Logs a fatal message with formatting, populating the current file, line, and function.
 * @param tag The tag for the message.
 */
#define DS_LOG_FATAL_F(tag, ...) DS_LOG_F(dsLogLevel_Fatal, tag, __VA_ARGS__)

#ifdef __cplusplus
}
#endif
