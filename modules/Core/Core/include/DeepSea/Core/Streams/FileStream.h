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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Streams/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for operating on file streams.
 * @see dsFileStream
 */

/**
 * @brief Gets the status of a file or directory on the filesystem.
 * @remark errno will be set on failure.
 * @param path The path to a file or directory.
 * @return The status of the file.
 */
DS_CORE_EXPORT dsPathStatus dsFileStream_getPathStatus(const char* path);

/**
 * @brief Creates a directory on the filesystem.
 * @remark errno will be set on failure.
 * @param path The path to the directory.
 * @return False if the directory couldn't be created. If errno is EEXIST, the directory already
 *     existed.
 */
DS_CORE_EXPORT bool dsFileStream_createDirectory(const char* path);

/**
 * @brief Removes a file from the filesystem.
 * @remark errno will be set on failure.
 * @param path The path to remove.
 * @return False if the file couldn't be removed.
 */
DS_CORE_EXPORT bool dsFileStream_removeFile(const char* path);

/**
 * @brief Removes a directory from the filesystem.
 *
 * The directory must be empty before removal.
 *
 * @remark errno will be set on failure.
 * @param path The path to remove.
 * @return False if the path couldn't be removed.
 */
DS_CORE_EXPORT bool dsFileStream_removeDirectory(const char* path);

/**
 * @brief Starts iterating over a directory on the filesystem.
 * @remark errno will be set on failure.
 * @param path The path to the directory.
 * @return The directory iterator or NULL if the directory cannot be iterated.
 */
DS_CORE_EXPORT dsDirectoryIterator dsFileStream_openDirectory(const char* path);

/**
 * @brief Gets the next entry in a directory.
 *
 * The . and .. entries will be implicitly skipped.
 *
 * @remark errno will be set on failure.
 * @param[out] outEntry The entry to populate.
 * @param iterator The iterator to get the next entry with.
 * @return The result of getting the next entry.
 */
DS_CORE_EXPORT dsDirectoryEntryResult dsFileStream_nextDirectoryEntry(
	dsDirectoryEntry* outEntry, dsDirectoryIterator iterator);

/**
 * @brief Closes a directory.
 * @remark errno will be set on failure.
 * @param iterator The directory iterator to close.
 * @return False if the directory couldn't be closed.
 */
DS_CORE_EXPORT bool dsFileStream_closeDirectory(dsDirectoryIterator iterator);

/**
 * @brief Opens a file stream with a file path.
 * @remark errno will be set on failure.
 * @param stream The stream to open.
 * @param path The file path to open.
 * @param mode The mode to open the file with. See fopen.
 * @return False if the file couldn't be opened.
 */
DS_CORE_EXPORT bool dsFileStream_openPath(dsFileStream* stream, const char* path, const char* mode);

/**
 * @brief Opens a file stream with a FILE pointer.
 * @remark errno will be set on failure.
 * @param stream The stream to open.
 * @param file The file to use.
 * @return False if stream or file are NULL.
 */
DS_CORE_EXPORT bool dsFileStream_openFile(dsFileStream* stream, FILE* file);

/**
 * @brief Reads from a file stream.
 * @remark errno will be set on failure.
 * @param stream The stream to read from.
 * @param data The data pointer to hold the data that was read.
 * @param size The number of bytes to read.
 * @return The number of bytes read from the stream.
 */
DS_CORE_EXPORT size_t dsFileStream_read(dsFileStream* stream, void* data, size_t size);

/**
 * @brief Writes to a file stream.
 * @remark errno will be set on failure.
 * @param stream The stream to write to.
 * @param data The data pointer to write to the stream.
 * @param size The number of bytes to write.
 * @return The number of bytes written to the stream.
 */
DS_CORE_EXPORT size_t dsFileStream_write(dsFileStream* stream, const void* data, size_t size);

/**
 * @brief Seeks in a file stream.
 * @remark errno will be set on failure.
 * @param stream The stream to seek in.
 * @param offset The offset from way.
 * @param way The position in the stream to take the offset from.
 * @return False if the seek was invalid.
 */
DS_CORE_EXPORT bool dsFileStream_seek(dsFileStream* stream, int64_t offset, dsStreamSeekWay way);

/**
 * @brief Tells the current position in a file stream.
 * @remark errno will be set on failure.
 * @param stream The stream to get the position from.
 * @return The position in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 *     determined.
 */
DS_CORE_EXPORT uint64_t dsFileStream_tell(dsFileStream* stream);

/**
 * @brief Gets the remaining bytes in the stream at the current location.
 * @remark errno will be set on failure.
 * @param stream The stream to get the remaining bytes from.
 * @return The remeaning bytes in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 *     determined.
 */
DS_CORE_EXPORT uint64_t dsFileStream_remainingBytes(dsFileStream* stream);

/**
 * @brief Flushes the contents of a file stream.
 * @param stream The stream to flush.
 */
DS_CORE_EXPORT void dsFileStream_flush(dsFileStream* stream);

/**
 * @brief Closes a file stream.
 * @remark errno will be set on failure.
 * @param stream The stream to close.
 * @return False if the stream cannot be closed.
 */
DS_CORE_EXPORT bool dsFileStream_close(dsFileStream* stream);

#ifdef __cplusplus
}
#endif
