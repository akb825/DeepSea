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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Streams/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Structure that defines a file stream.
 */

/**
 * @brief Opens a file stream with a file path.
 * @param stream The stream to open.
 * @param filePath The file path to open.
 * @param mode The mode to open the file with. See fopen.
 * @return False if the file couldn't be opened.
 */
DS_CORE_EXPORT bool dsFileStream_openPath(dsFileStream* stream, const char* filePath,
	const char* mode);

/**
 * @brief Opens a file stream with a FILE pointer.
 * @param stream The stream to open.
 * @param file The file to use.
 * @return False if stream or file are NULL.
 */
DS_CORE_EXPORT bool dsFileStream_openFile(dsFileStream* stream, FILE* file);

/**
 * @brief Reads from a file stream.
 * @param stream The stream to read from.
 * @param data The data pointer to hold the data that was read.
 * @param size The number of bytes to read.
 * @return The number of bytes read from the stream.
 */
DS_CORE_EXPORT size_t dsFileStream_read(dsFileStream* stream, void* data, size_t size);

/**
 * @brief Writes to a file stream.
 * @param stream The stream to write to.
 * @param data The data pointer to write to the stream.
 * @param size The number of bytes to write.
 * @return The number of bytes written to the stream.
 */
DS_CORE_EXPORT size_t dsFileStream_write(dsFileStream* stream, const void* data, size_t size);

/**
 * @brief Seeks in a file stream.
 * @param stream The stream to seek in.
 * @param offset The offset from way.
 * @param way The position in the stream to take the offset from.
 * @return False if the seek was invalid.
 */
DS_CORE_EXPORT bool dsFileStream_seek(dsFileStream* stream, int64_t offset, dsStreamSeekWay way);

/**
 * @brief Tells the current position in a file stream.
 * @param stream The stream to get the position from.
 * @return The position in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 * determined.
 */
DS_CORE_EXPORT uint64_t dsFileStream_tell(dsFileStream* stream);

/**
 * @brief Flushes the contents of a file stream.
 * @param stream The stream to flush.
 */
DS_CORE_EXPORT void dsFileStream_flush(dsFileStream* stream);

/**
 * @brief Closes a file stream.
 * @param stream The stream to close.
 * @return False if the stream cannot be closed.
 */
DS_CORE_EXPORT bool dsFileStream_close(dsFileStream* stream);

#ifdef __cplusplus
}
#endif
