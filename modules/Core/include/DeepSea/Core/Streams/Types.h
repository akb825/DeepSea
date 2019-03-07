/*
 * Copyright 2016 Aaron Barany
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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#if DS_LINUX
#include <linux/limits.h>
#elif DS_APPLE
#include <sys/syslimits.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used by the Streams portion of the DeepSea/Coroe library.
 */

#if DS_WINDOWS
#define DS_PATH_MAX _MAX_PATH
#define DS_PATH_SEPARATOR '\\'
#define DS_PATH_ALT_SEPARATOR '/'
#else
/**
 * @brief Define for the typical maximum length of a path.
 *
 * There are cases on some filesystems where the length can exceed this path, but this should be
 * sufficient for typical cases.
 */
#define DS_PATH_MAX PATH_MAX

/**
 * @brief The main path separator for the current platform.
 */
#define DS_PATH_SEPARATOR '/'

/**
 * @brief The alternate path separator for the current platform, or 0 if there is no alternate
 * separator.
 */
#define DS_PATH_ALT_SEPARATOR 0
#endif

/**
 * @brief Constant for an invalid stream position.
 */
#define DS_STREAM_INVALID_POS ((uint64_t)-1)

/// @cond Doxygen_Suppress
typedef struct dsAllocator dsAllocator;
/// @endcond

/**
 * @brief Structure that defines a stream.
 *
 * A stream can be used to read and write data from various sources such as a file or memory buffer.
 *
 * This can be "subclassed" by having it as the first member of other allocator structures. This can
 * be done to add additional data to the stream and have it be freely casted between the
 * dsStream and the true stream type.
 *
 * @see Stream.h
 */
typedef struct dsStream dsStream;

/**
 * @brief Enum for the way to seek in a stream.
 * @see Stream.h
 */
typedef enum dsStreamSeekWay
{
	dsStreamSeekWay_Beginning, ///< Relative to the beginning of the stream.
	dsStreamSeekWay_Current,   ///< Relative to the current position of the stream.
	dsStreamSeekWay_End        ///< Relative to the end of the stream.
} dsStreamSeekWay;

/**
 * @brief Enum for the type of resource to load for.
 * @remark Paths for all resource types except for dsFileResourceType_External must be relative
 * paths.
 * @see FileResource.h
 */
typedef enum dsFileResourceType
{
	dsFileResourceType_Embedded,  ///< Resource embedded with the application package.
	dsFileResourceType_Installed, ///< Resource installed with the application.
	dsFileResourceType_Dynamic,   ///< Resource created locally.
	dsFileResourceType_External   ///< Resource external to the application.
} dsFileResourceType;

/**
 * @brief Enum to determine if a file exists, and if so, if it's a file or directory.
 * @see FileUtils.h
 */
typedef enum dsFileStatus
{
	dsFileStatus_Error,          ///< An error occurred in accessing the file.
	dsFileStatus_DoesntExist,    ///< File doesn't exist.
	dsFileStatus_ExistsFile,     ///< File exists as a file or file-like object.
	dsFileStatus_ExistsDirectory ///< File exists as a directory.
} dsFileStatus;

/**
 * @brief Function for reading from a stream.
 * @param stream The stream to read from.
 * @param data The data pointer to hold the data that was read.
 * @param size The number of bytes to read.
 * @return The number of bytes read from the stream.
 */
typedef size_t (*dsStreamReadFunction)(dsStream* stream, void* data, size_t size);

/**
 * @brief Function for writing to a stream.
 * @param stream The stream to write to.
 * @param data The data pointer to write to the stream.
 * @param size The number of bytes to write.
 * @return The number of bytes written to the stream.
 */
typedef size_t (*dsStreamWriteFunction)(dsStream* stream, const void* data, size_t size);

/**
 * @brief Function for seeking in a stream.
 * @param stream The stream to seek in.
 * @param offset The offset from way.
 * @param way The position in the stream to take the offset from.
 * @return False if the seek was invalid.
 */
typedef bool (*dsStreamSeekFunction)(dsStream* stream, int64_t offset, dsStreamSeekWay way);

/**
 * @brief Function for telling the current position in the stream.
 * @param stream The stream to get the position from.
 * @return The position in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 * determined.
 */
typedef uint64_t (*dsStreamTellFunction)(dsStream* stream);

/**
 * @brief Function for flushing the contents of a stream.
 * @param stream The stream to flush.
 */
typedef void (*dsStreamFlushFunction)(dsStream* stream);

/**
 * @brief Function for closing a stream.
 * @param stream The stream to close.
 * @return False if the stream cannot be closed.
 */
typedef bool (*dsStreamCloseFunction)(dsStream* stream);

/** @copydoc dsStream */
struct dsStream
{
	/**
	 * @brief The read function.
	 *
	 * This may be NULL if the stream cannot be read from.
	 */
	dsStreamReadFunction readFunc;

	/**
	 * @brief The write function.
	 *
	 * This may be NULL if the stream cannot be written to.
	 */
	dsStreamWriteFunction writeFunc;

	/**
	 * @brief The seek function.
	 *
	 * This may be NULL if the stream cannot be seeked.
	 */
	dsStreamSeekFunction seekFunc;

	/**
	 * @brief The tell function.
	 *
	 * This may be NULL if the stream cannot be telled.
	 */
	dsStreamTellFunction tellFunc;

	/**
	 * @brief The flush function.
	 *
	 * This may be NULL if the stream cannot be flushed.
	 */
	dsStreamFlushFunction flushFunc;

	/**
	 * @brief The close function.
	 *
	 * This may be NULL if the stream cannot be closed.
	 */
	dsStreamCloseFunction closeFunc;
};

/**
 * @brief Structure that defines a file stream.
 *
 * This is effectively a subclass of dsStream and a pointer to dsFileStream can be freely
 * cast between the two types.
 *
 * @see FileStream.h
 */
typedef struct dsFileStream
{
	/**
	 * @brief The base stream.
	 */
	dsStream stream;

	/**
	 * @brief The file.
	 */
	FILE* file;
} dsFileStream;

/**
 * @brief Structure that defines a memory stream.
 *
 * This is effectively a subclass of dsStream and a pointer to dsMemoryStream can be freely
 * cast between the two types.
 *
 * @see MemoryStream.h
 */
typedef struct dsMemoryStream
{
	/**
	 * @brief The base stream.
	 */
	dsStream stream;

	/**
	 * @brief The memory buffer.
	 */
	void* buffer;

	/**
	 * @brief The size of the buffer.
	 */
	size_t size;

	/**
	 * @brief The current position in the buffer.
	 */
	size_t position;
} dsMemoryStream;

/**
 * @brief Structure that defines a generic stream.
 *
 * This simply holds a user data pointer for the stream. The implementor should set the function
 * pointers for the implementation.
 *
 * This is effectively a subclass of dsStream and a pointer to dsMemoryStream can be freely
 * cast between the two types.
 */
typedef struct dsGenericStream
{
	/**
	 * @brief The base stream.
	 */
	dsStream stream;

	/**
	 * @brief The user data for the stream.
	 */
	void* userData;
} dsGenericStream;

/**
 * @brief Structure that defines a stream for a resource.
 * @see ResourceStream.h
 */
typedef struct dsResourceStream
{
	union
	{
		/**
		 * @brief The file stream.
		 *
		 * This will be used if isFile is true.
		 */
		dsFileStream fileStream;

		/**
		 * @brief The generic stream.
		 *
		 * This will be used if isFile is false.
		 */
		dsGenericStream genericStream;
	};

	/**
	 * @brief True if the stream is a file stream, false if it's a generic stream.
	 */
	bool isFile;
} dsResourceStream;

#ifdef __cplusplus
}
#endif
