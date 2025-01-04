/*
 * Copyright 2018-2025 Aaron Barany
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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Core/Streams/Stream.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Sets the context for resource streams.
 *
 * This should be called by the application to set the information for opening resource files.
 *
 * @remark errno will be set on failure.
 * @param globalContext The context for the global state.
 * @param applicationContext The context for the current application.
 * @param embeddedDir The directory for embedded resources.
 * @param localDir The directory for local resources.
 * @param dynamicDir The directory for dynamic resources.
 */
DS_CORE_EXPORT bool dsResourceStream_setContext(void* globalContext, void* applicationContext,
	const char* embeddedDir, const char* localDir, const char* dynamicDir);

/**
 * @brief Gets the directory for embedded resources.
 * @return The embedded directory.
 */
DS_CORE_EXPORT const char* dsResourceStream_getEmbeddedDirectory(void);

/**
 * @brief Sets the directory for embedded resources.
 * @param dir The embedded directory.
 */
DS_CORE_EXPORT void dsResourceStream_setEmbeddedDirectory(const char* dir);

/**
 * @brief Gets the directory for local resources.
 * @return The local directory.
 */
DS_CORE_EXPORT const char* dsResourceStream_getLocalDirectory(void);

/**
 * @brief Sets the directory for local resources.
 * @param dir The local directory.
 */
DS_CORE_EXPORT void dsResourceStream_setLocalDirectory(const char* dir);

/**
 * @brief Gets the directory for dynamic resources.
 * @return The dynamic directory.
 */
DS_CORE_EXPORT const char* dsResourceStream_getDynamicDirectory(void);

/**
 * @brief Sets the directory for dynamic resources.
 * @param dir The dynamic directory.
 */
DS_CORE_EXPORT void dsResourceStream_setDynamicDirectory(const char* dir);

/**
 * @brief Gets whether or not a resource type will be a file.
 * @param type The resource type.
 * @return True if the resource will be a file.
 */
DS_CORE_EXPORT bool dsResourceStream_isFile(dsFileResourceType type);

/**
 * @brief Gets the directory path for a resource type.
 * @param type The resource type.
 * @return The path, or NULL if no directory is used for the given resource type.
 */
DS_CORE_EXPORT const char* dsResourceStream_getDirectory(dsFileResourceType type);

/**
 * @brief Gets the path for a resource on the filesystem.
 * @remark errno will be set on failure.
 * @param[out] outResult The path for the resource.
 * @param resultSize The size of the result buffer, including the null terminator.
 * @param type The resource type.
 * @param path The path to get the resource path for.
 * @return False if the parameters are invalid or there isn't enough space in result.
 */
DS_CORE_EXPORT bool dsResourceStream_getPath(char* outResult, size_t resultSize,
	dsFileResourceType type, const char* path);

/**
 * @brief Starts iterating over a directory from a resource.
 * @remark errno will be set on failure.
 * @param type The resource type.
 * @param path The path to the directory.
 * @return The directory iterator or NULL if the directory cannot be iterated.
 */
DS_CORE_EXPORT dsDirectoryIterator dsResourceStream_openDirectory(
	dsFileResourceType type, const char* path);

/**
 * @brief Gets the next entry in a directory.
 *
 * The . and .. entries will be implicitly skipped.
 *
 * @remark When listing a directory for an embedded path on Android, due to limitations of the
 *     NDK only file entries will be listed.
 * @remark errno will be set on failure.
 * @param[out] outEntry The entry to populate.
 * @param iterator The iterator to get the next entry with.
 * @return The result of getting the next entry.
 */
DS_CORE_EXPORT dsDirectoryEntryResult dsResourceStream_nextDirectoryEntry(
	dsDirectoryEntry* outEntry, dsDirectoryIterator iterator);

/**
 * @brief Closes a directory.
 * @remark errno will be set on failure.
 * @param iterator The directory iterator to close.
 * @return False if the directory couldn't be closed.
 */
DS_CORE_EXPORT bool dsResourceStream_closeDirectory(dsDirectoryIterator iterator);

/**
 * @brief Opens a stream for a resource.
 * @remark errno will be set on failure.
 * @param stream The stream to open.
 * @param type The resource type.
 * @param path The path to open.
 * @param mode The mode to open the file with. See fopen.
 * @return False if the file couldn't be opened.
 */
DS_CORE_EXPORT bool dsResourceStream_open(dsResourceStream* stream, dsFileResourceType type,
	const char* path, const char* mode);

/**
 * @brief Reads from a resource stream.
 * @remark errno will be set on failure.
 * @param stream The stream to read from.
 * @param data The data pointer to hold the data that was read.
 * @param size The number of bytes to read.
 * @return The number of bytes read from the stream.
 */
DS_CORE_EXPORT inline size_t dsResourceStream_read(dsResourceStream* stream, void* data,
	size_t size);

/**
 * @brief Writes to a resource stream.
 * @remark errno will be set on failure.
 * @param stream The stream to write to.
 * @param data The data pointer to write to the stream.
 * @param size The number of bytes to write.
 * @return The number of bytes written to the stream.
 */
DS_CORE_EXPORT inline size_t dsResourceStream_write(dsResourceStream* stream, const void* data,
	size_t size);

/**
 * @brief Seeks in a resource stream.
 * @remark errno will be set on failure.
 * @param stream The stream to seek in.
 * @param offset The offset from way.
 * @param way The position in the stream to take the offset from.
 * @return False if the seek was invalid.
 */
DS_CORE_EXPORT inline bool dsResourceStream_seek(dsResourceStream* stream, int64_t offset,
	dsStreamSeekWay way);

/**
 * @brief Tells the current position in a resource stream.
 * @remark errno will be set on failure.
 * @param stream The stream to get the position from.
 * @return The position in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 *     determined.
 */
DS_CORE_EXPORT inline uint64_t dsResourceStream_tell(dsResourceStream* stream);

/**
 * @brief Gets the remaining bytes in the stream at the current location.
 * @remark errno will be set on failure.
 * @param stream The stream to get the remaining bytes from.
 * @return The remeaning bytes in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 *     determined.
 */
DS_CORE_EXPORT inline uint64_t dsResourceStream_remainingBytes(dsResourceStream* stream);

/**
 * @brief Flushes the contents of a resource stream.
 * @param stream The stream to flush.
 */
DS_CORE_EXPORT inline void dsResourceStream_flush(dsFileStream* stream);

/**
 * @brief Closes a resource stream.
 * @remark errno will be set on failure.
 * @param stream The stream to close.
 * @return False if the stream cannot be closed.
 */
DS_CORE_EXPORT inline bool dsResourceStream_close(dsResourceStream* stream);

inline size_t dsResourceStream_read(dsResourceStream* stream, void* data, size_t size)
{
	return dsStream_read((dsStream*)stream, data, size);
}

inline size_t dsResourceStream_write(dsResourceStream* stream, const void* data, size_t size)
{
	return dsStream_write((dsStream*)stream, data, size);
}

inline bool dsResourceStream_seek(dsResourceStream* stream, int64_t offset, dsStreamSeekWay way)
{
	return dsStream_seek((dsStream*)stream, offset, way);
}

inline uint64_t dsResourceStream_tell(dsResourceStream* stream)
{
	return dsStream_tell((dsStream*)stream);
}

inline uint64_t dsResourceStream_remainingBytes(dsResourceStream* stream)
{
	return dsStream_remainingBytes((dsStream*)stream);
}

inline void dsResourceStream_flush(dsFileStream* stream)
{
	dsStream_flush((dsStream*)stream);
}

inline bool dsResourceStream_close(dsResourceStream* stream)
{
	return dsStream_close((dsStream*)stream);
}

#ifdef __cplusplus
}
#endif
