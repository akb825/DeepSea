/*
 * Copyright 2016-2025 Aaron Barany
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
#include <stdlib.h>

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
#define DS_PATH_MAX (_MAX_PATH + 1)
#define DS_FILE_NAME_MAX DS_PATH_MAX
#define DS_PATH_SEPARATOR '\\'
#define DS_PATH_ALT_SEPARATOR '/'
#else
/**
 * @brief Define for the typical maximum length of a path.
 *
 * There are cases on some filesystems where the length can exceed this limit, but this should be
 * sufficient for typical cases.
 *
 * This includes space for the NULL terminator at the end of the string.
 */
#define DS_PATH_MAX (PATH_MAX + 1)

/**
 * @brief Define for the typical maximum length of a file name.
 *
 * There are cases on some filesystems where the length can exceed this limit, but this should be
 * sufficient for typical cases.
 *
 * This includes space for the NULL terminator at the end of the string.
 */
#define DS_FILE_NAME_MAX (NAME_MAX + 1)

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
typedef struct dsFileArchive dsFileArchive;
typedef struct dsStream dsStream;
/// @endcond

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
 * @brief Enum to determine if a path exists, and if so, if it's a file or directory.
 * @see FileUtils.h
 */
typedef enum dsPathStatus
{
	dsPathStatus_Error,          ///< An error occurred in accessing the path.
	dsPathStatus_Missing,        ///< Path doesn't exist.
	dsPathStatus_ExistsFile,     ///< Path exists as a file or file-like object.
	dsPathStatus_ExistsDirectory ///< Path exists as a directory.
} dsPathStatus;

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
 * @brief Function for restarting a stream to the start.
 * @param stream The stream to restart.
 * @return False if the stream cannot be restarted.
 */
typedef bool (*dsStreamRestartFunction)(dsStream* stream);

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

/**
 * @brief Structure that defines a stream.
 *
 * A stream can be used to read and write data from various sources such as a file or memory buffer.
 *
 * This can be "subclassed" by having it as the first member of other stream structures. This can
 * be done to add additional data to the stream and have it be freely casted between the
 * dsStream and the true stream type.
 *
 * @see Stream.h
 */
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
	 * @brief Function to get the remaining bytes in the stream.
	 *
	 * This may be NULL if the stream cannot get the remaining bytes. If NULL and seekFunc and
	 * tellFunc are set, the remaining bytes will be computed through seeking.
	 */
	dsStreamTellFunction remainingBytesFunc;

	/**
	 * @brief The restart function.
	 *
	 * This may be NULL if the stream cannot be restarted or if seekFunc is set.
	 */
	dsStreamRestartFunction restartFunc;

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

/**
 * @brief Opque type for an iterator over a directory.
 *
 * It is only valid to use a dsDirectoryIterator instance with the stream type that created it.
 */
typedef void* dsDirectoryIterator;

/**
 * @brief Function to get the path status for a file archive.
 * @param archive The archive to get the path status from.
 * @param path The path to get the status for.
 * @return The path status.
 */
typedef dsPathStatus (*dsGetFileArchivePathStatusFunction)(
	const dsFileArchive* archive, const char* path);

/**
 * @brief Function to open a directory for an archive.
 * @param archive The archive to open the directory on.
 * @param path The path to the directory.
 * @return The directory iterator.
 */
typedef dsDirectoryIterator (*dsOpenFileArchiveDirectoryFunction)(
	const dsFileArchive* archive, const char* path);

/**
 * @brief Function to get the next entry in an archive directory.
 * @param[out] result The storage for the result.
 * @param resultSize The maximum size of the result.
 * @param archive The archive the directory is open on.
 * @param iterator The iterator for the directory.
 * @return The result of getting the next entry.
 */
typedef dsPathStatus (*dsNextFileArchiveDirectoryEntryFunction)(
	char* result, size_t resultSize, const dsFileArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Function to close a directory for an archive.
 * @param archive The archive the directory is open on.
 * @param iterator The iterator for the directory to close.
 * @return False if the directory couldn't be closed.
 */
typedef bool (*dsCloseFileArchiveDirectoryFunction)(
	const dsFileArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Function to open a file on an archive.
 * @param archive The archive to open the file from.
 * @param path The path to the file.
 * @return The opened stream or NULL if the file couldn't be opened.
 */
typedef dsStream* (*dsOpenFileArchiveFileFunction)(const dsFileArchive* archive, const char* path);

/**
 * @brief Function to close a file on an archive.
 * @param archive The archive to close the file from.
 * @param stream The stream previously opened.
 * @return False if the file couldn't be closed.
 */
typedef bool (*dsCloseFileArchiveFileFunction)(const dsFileArchive* archive, dsStream* stream);

/**
 * @brief Struct describing an archive of files.
 *
 * Archives are read-only, and intended to group multiple files and directories into a single unit,
 * often with compression. Implementations should ensure that multiple files can be opened
 * simultaneously across multiple threads.
 *
 * This can be "subclassed" by having it as the first member of other archive structures. This can
 * be done to add additional data to the archive and have it be freely casted between the
 * dsFileArchive and the true stream type.
 *
 * @see FileArchive.h
 */
struct dsFileArchive
{
	/**
	 * @brief Function to get the status of a path within the archive.
	 */
	dsGetFileArchivePathStatusFunction getPathStatusFunc;

	/**
	 * @brief Function to open a directory within the archive.
	 */
	dsOpenFileArchiveDirectoryFunction openDirectoryFunc;

	/**
	 * @brief Function to get the next directory entry within the archive.
	 */
	dsNextFileArchiveDirectoryEntryFunction nextDirectoryEntryFunc;

	/**
	 * @brief Function to close a directory within the archive.
	 */
	dsCloseFileArchiveDirectoryFunction closeDirectoryFunc;

	/**
	 * @brief Function to open a file within the archive.
	 */
	dsOpenFileArchiveFileFunction openFileFunc;

	/**
	 * @brief Function to close a file within the archive.
	 */
	dsCloseFileArchiveFileFunction closeFileFunc;
};

#ifdef __cplusplus
}
#endif
