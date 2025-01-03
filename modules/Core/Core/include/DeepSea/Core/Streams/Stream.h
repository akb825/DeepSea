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
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for operating on streams of any type.
 * @see dsStream
 */

/**
 * @brief Reads from a stream.
 * @remark errno will be set on failure.
 * @param stream The stream to read from.
 * @param data The data pointer to hold the data that was read.
 * @param size The number of bytes to read.
 * @return The number of bytes read from the stream.
 */
DS_CORE_EXPORT inline size_t dsStream_read(dsStream* stream, void* data, size_t size);

/**
 * @brief Reads from the current position in the stream until the end.
 * @remark errno will be set on failure.
 * @param[out] outSize The size of the buffer.
 * @param stream The stream to read from.
 * @param allocator The allocator to create the buffer with.
 * @return The read buffer, or NULL if an error occurred. This buffer will need to be freed by the
 *     caller.
 */
DS_CORE_EXPORT void* dsStream_readUntilEnd(size_t* outSize, dsStream* stream,
	dsAllocator* allocator);

/**
 * @brief Reads from the current position in the stream until the end, re-using the buffer when
 *     possible.
 * @remark errno will be set on failure.
 * @param[inout] buffer A pointer to the buffer. This may initially point to a non-null value, in
 *     which case the memory will be re-used if possible. Once the buffer is no longer needed, it
 *     must be freed by the caller.
 * @param[inout] size The size of the buffer corresponding to the read data.
 * @param[inout] capacity The total allocated size of the buffer.
 * @param stream The stream to read from.
 * @param allocator The allocator to create the buffer with.
 * @return False if an error occurred.
 */
DS_CORE_EXPORT bool dsStream_readUntilEndReuse(void** buffer, size_t* size, size_t* capacity,
	dsStream* stream, dsAllocator* allocator);

/**
 * @brief Writes to a stream.
 * @remark errno will be set on failure.
 * @param stream The stream to write to.
 * @param data The data pointer to write to the stream.
 * @param size The number of bytes to write.
 * @return The number of bytes written to the stream.
 */
DS_CORE_EXPORT inline size_t dsStream_write(dsStream* stream, const void* data, size_t size);

/**
 * @brief Seeks in a stream.
 * @remark errno will be set on failure.
 * @param stream The stream to seek in.
 * @param offset The offset from way.
 * @param way The position in the stream to take the offset from.
 * @return False if the seek was invalid.
 */
DS_CORE_EXPORT inline bool dsStream_seek(dsStream* stream, int64_t offset, dsStreamSeekWay way);

/**
 * @brief Tells the current position in a stream.
 * @remark errno will be set on failure.
 * @param stream The stream to get the position from.
 * @return The position in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 *     determined.
 */
DS_CORE_EXPORT inline uint64_t dsStream_tell(dsStream* stream);

/**
 * @brief Checks whether the remaining bytes can be queried from the stream.
 *
 * This will check whether the remaining bytes can be computed directly or through seeking.
 *
 * @param stream The stream the remaining bytes may be queried from.
 * @return False if dsStream_remainingBytes() is guaranteed to fail. If true,
 *     dsStream_remainingBytes() will likely succeed, but may still fail in some situations.
 */
DS_CORE_EXPORT inline bool dsStream_canGetRemainingBytes(dsStream* stream);

/**
 * @brief Gets the remaining bytes in the stream at the current location.
 * @remark errno will be set on failure.
 * @param stream The stream to get the remaining bytes from.
 * @return The remeaning bytes in the stream, or DS_STREAM_INVALID_POS if the position cannot be
 *     determined.
 */
DS_CORE_EXPORT inline uint64_t dsStream_remainingBytes(dsStream* stream);

/**
 * @brief Skips bytes in a stream.
 *
 * This will attempt to do a seek, otherwise it will read the bytes to an empty buffer to skip them.
 *
 * @remark Some implementations will seek past the end and not throw an error until the next read.
 * @remark errno will be set on failure.
 * @param stream The stream to read from.
 * @param size The number of bytes to skip.
 * @return The number of bytes skipped. If implemented as a seek, this will be 0 if the seek fails.
 */
DS_CORE_EXPORT uint64_t dsStream_skip(dsStream* stream, uint64_t size);

/**
 * @brief Checks whether a stream may be restarted to the beginning.
 *
 * This will check whether the stream can be restarted directly or through seeking.
 *
 * @param stream The stream that may be restarted.
 * @return True if dsStream_restart() can be executed.
 */
DS_CORE_EXPORT inline bool dsStream_canRestart(dsStream* stream);

/**
 * @brief Restarts a stream back to the beginning.
 * @remark errno will be set on failure.
 * @param stream The stream to restart.
 */
DS_CORE_EXPORT inline bool dsStream_restart(dsStream* stream);

/**
 * @brief Flushes the contents of a stream.
 * @param stream The stream to flush.
 */
DS_CORE_EXPORT inline void dsStream_flush(dsStream* stream);

/**
 * @brief Closes a stream.
 * @remark errno will be set on failure.
 * @param stream The stream to close.
 * @return False if the stream cannot be closed. A NULL close function is considered a success.
 */
DS_CORE_EXPORT inline bool dsStream_close(dsStream* stream);

inline size_t dsStream_read(dsStream* stream, void* data, size_t size)
{
	if (!stream || !stream->readFunc || !data)
	{
		errno = EINVAL;
		return 0;
	}

	return stream->readFunc(stream, data, size);
}

inline size_t dsStream_write(dsStream* stream, const void* data, size_t size)
{
	if (!stream || !stream->writeFunc || !data)
	{
		errno = EINVAL;
		return 0;
	}

	return stream->writeFunc(stream, data, size);
}

inline bool dsStream_seek(dsStream* stream, int64_t offset, dsStreamSeekWay way)
{
	if (!stream || !stream->seekFunc)
	{
		errno = EINVAL;
		return false;
	}

	return stream->seekFunc(stream, offset, way);
}

inline uint64_t dsStream_tell(dsStream* stream)
{
	if (!stream || !stream->tellFunc)
	{
		errno = EINVAL;
		return DS_STREAM_INVALID_POS;
	}

	return stream->tellFunc(stream);
}

inline bool dsStream_canGetRemainingBytes(dsStream* stream)
{
	return stream && (stream->remainingBytesFunc || (stream->seekFunc && stream->tellFunc));
}

inline uint64_t dsStream_remainingBytes(dsStream* stream)
{
	if (!dsStream_canGetRemainingBytes(stream))
	{
		errno = EINVAL;
		return DS_STREAM_INVALID_POS;
	}

	if (stream->remainingBytesFunc)
		return stream->remainingBytesFunc(stream);

	uint64_t position = dsStream_tell(stream);
	if (position == DS_STREAM_INVALID_POS || !dsStream_seek(stream, 0, dsStreamSeekWay_End))
		return DS_STREAM_INVALID_POS;

	uint64_t end = dsStream_tell(stream);
	if (end == DS_STREAM_INVALID_POS ||
		(end != position && !dsStream_seek(stream, position, dsStreamSeekWay_Beginning)))
	{
		return DS_STREAM_INVALID_POS;
	}

	return end - position;
}

inline void dsStream_flush(dsStream* stream)
{
	if (!stream || !stream->flushFunc)
		return;

	stream->flushFunc(stream);
}

inline bool dsStream_canRestart(dsStream* stream)
{
	return stream && (stream->seekFunc || stream->restartFunc);
}

inline bool dsStream_restart(dsStream* stream)
{
	if (!stream || !(stream->seekFunc || stream->restartFunc))
	{
		errno = EINVAL;
		return false;
	}

	if (stream->restartFunc)
		return stream->restartFunc(stream);
	return stream->seekFunc(stream, 0, dsStreamSeekWay_Beginning);
}

inline bool dsStream_close(dsStream* stream)
{
	if (!stream)
	{
		errno = EINVAL;
		return false;
	}

	if (!stream->closeFunc)
		return true;

	return stream->closeFunc(stream);
}

#ifdef __cplusplus
}
#endif
