/*
 * Copyright 2025 Aaron Barany
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

#include <DeepSea/Core/Streams/ZipArchive.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Streams/Endian.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>

#include <string.h>

#define DS_READ_BUFFER_SIZE 4096
// 1 MB
#define DS_DEFAULT_DECOMPRESS_BUFFER_SIZE 1048576

#if DS_PATH_SEPARATOR != '/' || DS_PATH_ALT_SEPARATOR != 0
#define DS_NEEDS_PATH_SEPARATOR_FIXUP 1
#else
#define DS_NEEDS_PATH_SEPARATOR_FIXUP 0
#endif

typedef enum CompressionMethod
{
	CompressionMethod_None = 0,
	CompressionMethod_Deflate = 8
} CompressionMethod;

typedef struct EndOfCentralDirectoryRecord
{
	uint32_t signature;
	uint16_t diskNumber;
	uint16_t startDiskNumber;
	uint16_t thisDiskEntryCount;
	uint16_t totalEntryCount;
	uint32_t centralDirectorySize;
	uint32_t centralDirectoryOffset;
	uint16_t commentSize;
} EndOfCentralDirectoryRecord;

typedef struct Zip64EndOfCentralDirectoryLocator
{
	uint32_t signature;
	uint32_t diskNumber;
	uint64_t offset;
	uint32_t diskCount;
} Zip64EndOfCentralDirectoryLocator;

typedef struct Zip64EndOfCentralDirectoryRecord
{
	uint32_t signature;
	uint64_t size;
	uint16_t madeByVersion;
	uint16_t requiredVersion;
	uint32_t diskNumber;
	uint32_t startDiskNumber;
	uint64_t thisDiskEntryCount;
	uint64_t totalEntryCount;
	uint64_t centralDirectorySize;
	uint64_t centralDirectoryOffset;
} Zip64EndOfCentralDirectoryRecord;

typedef struct CentralDirectoryHeader
{
	uint32_t signature;
	uint16_t madeByVersion;
	uint16_t requiredVersion;
	uint16_t generalPurposeFlags;
	uint16_t compressionMethod;
	uint16_t lastModFileTime;
	uint16_t lastModFileDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;
	uint16_t fileCommentLength;
	uint16_t startDisk;
	uint16_t internalFileAttribs;
	uint32_t externalFileAttribs;
	uint32_t localHeaderOffset;
} CentralDirectoryHeader;

typedef struct LocalFileHeader
{
	uint32_t signature;
	uint16_t requiredVersion;
	uint16_t generalPurposeFlags;
	uint16_t compressionMethod;
	uint16_t lastModFileTime;
	uint16_t lastModFileDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;
} LocalFileHeader;

typedef struct FileEntry
{
	const char* fileName;
	uint64_t offset;
	uint64_t compressed : 1;
	uint64_t compressedSize : 63;
	uint64_t unompressedSize;
} FileEntry;

typedef struct dsDirectoryIteratorInfo
{
	const char* prefix;
	size_t prefixLen;
	const FileEntry* curEntry;
	const FileEntry* endEntry;
} dsDirectoryIteratorInfo;

struct dsZipArchive
{
	dsFileArchive archive;
	dsAllocator* allocator;

	dsFileResourceType resourceType;
	const char* path;

	FileEntry* entries;
	size_t entryCount;
	size_t decompressBufferSize;
};

static bool readUInt16(uint16_t* result, dsStream* stream)
{
	size_t readSize = dsStream_read(stream, result, sizeof(*result));
	*result = dsEndian_swapUInt16OnBig(*result);
	return readSize == sizeof(*result);
}

static bool readUInt32(uint32_t* result, dsStream* stream)
{
	size_t readSize = dsStream_read(stream, result, sizeof(*result));
	*result = dsEndian_swapUInt32OnBig(*result);
	return readSize == sizeof(*result);
}

static bool readUInt64(uint64_t* result, dsStream* stream)
{
	size_t readSize = dsStream_read(stream, result, sizeof(*result));
	*result = dsEndian_swapUInt64OnBig(*result);
	return readSize == sizeof(*result);
}

static uint64_t findEndOfCentralDirectoryRecord(uint64_t* outFullSize, dsStream* stream)
{
	// Search from the end for the directory record.
	const char endOfCentralDirSignature[4] = {0x50, 0x4B, 0x05, 0x06};
	const size_t minEndOfCCentralDirRecordSize = 22;
	// Max comment length 64k, so 22 + 2^16 - 1.
	const size_t maxEndOfCCentralDirRecordSize = 65557;
	const unsigned int overlap = 3;
	char buffer[DS_READ_BUFFER_SIZE];

	if (!dsStream_seek(stream, 0, dsStreamSeekWay_End))
		return DS_STREAM_INVALID_POS;

	uint64_t remainingSize = dsStream_tell(stream);
	if (remainingSize == DS_STREAM_INVALID_POS ||
		remainingSize < minEndOfCCentralDirRecordSize)
	{
		return DS_STREAM_INVALID_POS;
	}
	*outFullSize = remainingSize;

	size_t checkedSize = 0;
	do
	{
		// Need some overlap since the signature isn't guaranteed to be aligned.
		size_t offset = DS_READ_BUFFER_SIZE - overlap;
		if (checkedSize + offset > maxEndOfCCentralDirRecordSize)
			offset = maxEndOfCCentralDirRecordSize - checkedSize;

		if (offset > remainingSize)
			offset = (size_t)remainingSize;

		remainingSize -= offset;
		checkedSize += offset;

		if (!dsStream_seek(stream, remainingSize, dsStreamSeekWay_Beginning))
			return DS_STREAM_INVALID_POS;
		size_t readSize = dsStream_read(stream, buffer, offset + overlap);

		// Search from the end for the signature.
		for (size_t i = readSize - overlap; i-- > 0;)
		{
			if (memcmp(buffer + i, endOfCentralDirSignature, sizeof(endOfCentralDirSignature)) == 0)
			{
				if (checkedSize - i < minEndOfCCentralDirRecordSize)
					return DS_STREAM_INVALID_POS;
				return remainingSize + i;
			}
		}
	} while (checkedSize < maxEndOfCCentralDirRecordSize && remainingSize > 0);
	return DS_STREAM_INVALID_POS;
}

static bool readEndOfCentralDirectoryRecord(uint64_t* outFirstDirRecordOffset,
	size_t* outEntryCount, dsStream* stream, const char* path, uint64_t endOfCentralDirOffset,
	uint64_t fullSize)
{
	const uint32_t endOfCentralDirSignature = 0x06054B50;

	// Read the central directory record, with some validation.
	EndOfCentralDirectoryRecord endOfCentralDirRecord;
	if (!dsStream_seek(stream, endOfCentralDirOffset, dsStreamSeekWay_Beginning) ||
		!readUInt32(&endOfCentralDirRecord.signature, stream) ||
		endOfCentralDirRecord.signature != endOfCentralDirSignature ||
		!readUInt16(&endOfCentralDirRecord.diskNumber, stream) ||
		!readUInt16(&endOfCentralDirRecord.startDiskNumber, stream) ||
		!readUInt16(&endOfCentralDirRecord.thisDiskEntryCount, stream) ||
		!readUInt16(&endOfCentralDirRecord.totalEntryCount, stream) ||
		!readUInt32(&endOfCentralDirRecord.centralDirectorySize, stream) ||
		!readUInt32(&endOfCentralDirRecord.centralDirectoryOffset, stream) ||
		!readUInt16(&endOfCentralDirRecord.commentSize, stream) ||
		dsStream_tell(stream) + endOfCentralDirRecord.commentSize != fullSize)
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
		errno = EFORMAT;
		return false;
	}

	bool maybeZip64 = endOfCentralDirRecord.diskNumber == 0xFFFF ||
		endOfCentralDirRecord.startDiskNumber == 0xFFFF ||
		endOfCentralDirRecord.thisDiskEntryCount == 0xFFFF ||
		endOfCentralDirRecord.totalEntryCount == 0xFFFF ||
		endOfCentralDirRecord.centralDirectorySize == 0xFFFFFFFF ||
		endOfCentralDirRecord.centralDirectoryOffset == 0xFFFFFFFF;
	if (maybeZip64)
	{
		// Try to read the zip64 end of central directory locator. If not present, then the size may
		// actually be the max value.
		const uint32_t zip64EndOfCentralDirLocatorSignature = 0x07064B50;
		const size_t zip64EndOfCentralDirLocatorSize = 20;
		Zip64EndOfCentralDirectoryLocator zip64EndOfCentralDirLocator;
		if (endOfCentralDirOffset >= zip64EndOfCentralDirLocatorSize &&
			dsStream_seek(stream, endOfCentralDirOffset - zip64EndOfCentralDirLocatorSize,
				dsStreamSeekWay_Beginning) &&
			readUInt32(&zip64EndOfCentralDirLocator.signature, stream) &&
			zip64EndOfCentralDirLocator.signature == zip64EndOfCentralDirLocatorSignature)
		{
			if (!readUInt32(&zip64EndOfCentralDirLocator.diskNumber, stream) ||
				!readUInt64(&zip64EndOfCentralDirLocator.offset, stream) ||
				!readUInt32(&zip64EndOfCentralDirLocator.diskCount, stream))
			{
				DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
				errno = EFORMAT;
				return false;
			}

			const uint32_t zip64EndOfCentralDirSignature = 0x06064B50;
			const size_t minZp64EndOfCentralDirSize = 44;
			Zip64EndOfCentralDirectoryRecord zip64EndOfCentralDirRecord;
			if (!dsStream_seek(stream, zip64EndOfCentralDirLocator.offset,
					dsStreamSeekWay_Beginning) ||
				!readUInt32(&zip64EndOfCentralDirRecord.signature, stream) ||
				zip64EndOfCentralDirRecord.signature != zip64EndOfCentralDirSignature ||
				!readUInt64(&zip64EndOfCentralDirRecord.size, stream) ||
				zip64EndOfCentralDirRecord.size < minZp64EndOfCentralDirSize ||
				!readUInt16(&zip64EndOfCentralDirRecord.madeByVersion, stream) ||
				!readUInt16(&zip64EndOfCentralDirRecord.requiredVersion, stream) ||
				!readUInt32(&zip64EndOfCentralDirRecord.diskNumber, stream) ||
				!readUInt32(&zip64EndOfCentralDirRecord.startDiskNumber, stream) ||
				!readUInt64(&zip64EndOfCentralDirRecord.thisDiskEntryCount, stream) ||
				!readUInt64(&zip64EndOfCentralDirRecord.totalEntryCount, stream) ||
				!readUInt64(&zip64EndOfCentralDirRecord.centralDirectorySize, stream) ||
				!readUInt64(&zip64EndOfCentralDirRecord.centralDirectoryOffset, stream))
			{
				DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
				errno = EFORMAT;
				return false;
			}


			if (zip64EndOfCentralDirRecord.diskNumber != 0 ||
				zip64EndOfCentralDirRecord.startDiskNumber != 0 ||
				zip64EndOfCentralDirRecord.thisDiskEntryCount !=
					zip64EndOfCentralDirRecord.totalEntryCount)
			{
				DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Multi-disk .zip file '%s' not supported.", path);
				errno = EFORMAT;
				return false;
			}

	#if !DS_64BIT
			if (zip64EndOfCentralDirRecord.totalEntryCount > 0xFFFFFFFF)
			{
				DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Too many entries in .zip file '%s'.", path);
				errno = EFORMAT;
				return false;
			}
	#endif

			*outFirstDirRecordOffset = zip64EndOfCentralDirRecord.centralDirectoryOffset;
			*outEntryCount = (size_t)zip64EndOfCentralDirRecord.totalEntryCount;
			return true;
		}
	}


	if (endOfCentralDirRecord.diskNumber != 0 || endOfCentralDirRecord.startDiskNumber != 0 ||
		endOfCentralDirRecord.thisDiskEntryCount != endOfCentralDirRecord.totalEntryCount)
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Multi-disk .zip file '%s' not supported.", path);
		errno = EFORMAT;
		return false;
	}

	*outFirstDirRecordOffset = endOfCentralDirRecord.centralDirectoryOffset;
	*outEntryCount = endOfCentralDirRecord.totalEntryCount;
	return true;
}

static bool readCentralDirectoryHeader(
	CentralDirectoryHeader* outHeader, dsStream* stream, const char* path)
{
	const uint32_t centralDirectoryHeaderSignature = 0x02014B50;
	if (!readUInt32(&outHeader->signature, stream) ||
		outHeader->signature != centralDirectoryHeaderSignature ||
		!readUInt16(&outHeader->madeByVersion, stream) ||
		!readUInt16(&outHeader->requiredVersion, stream) ||
		!readUInt16(&outHeader->generalPurposeFlags, stream) ||
		!readUInt16(&outHeader->compressionMethod, stream) ||
		!readUInt16(&outHeader->lastModFileTime, stream) ||
		!readUInt16(&outHeader->lastModFileDate, stream) ||
		!readUInt32(&outHeader->crc32, stream) ||
		!readUInt32(&outHeader->compressedSize, stream) ||
		!readUInt32(&outHeader->uncompressedSize, stream) ||
		!readUInt16(&outHeader->fileNameLength, stream) ||
		!readUInt16(&outHeader->extraFieldLength, stream) ||
		!readUInt16(&outHeader->fileCommentLength, stream) ||
		!readUInt16(&outHeader->startDisk, stream) ||
		!readUInt16(&outHeader->internalFileAttribs, stream) ||
		!readUInt32(&outHeader->externalFileAttribs, stream) ||
		!readUInt32(&outHeader->localHeaderOffset, stream))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
		errno = EFORMAT;
		return false;
	}

	return true;
}

static size_t getFullAllocSize(
	dsStream* stream, const char* path, uint64_t firstDirRecordOffset, size_t entryCount)
{
	const uint16_t encryptionFlag = 0x1;

	size_t fullAllocSize = DS_ALIGNED_SIZE(sizeof(dsZipArchive)) +
		DS_ALIGNED_SIZE(strlen(path) + 1) + DS_ALIGNED_SIZE(sizeof(FileEntry)*entryCount);

	if (!dsStream_seek(stream, firstDirRecordOffset, dsStreamSeekWay_Beginning))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
		errno = EFORMAT;
		return 0;
	}

	for (size_t i = 0; i < entryCount; ++i)
	{
		CentralDirectoryHeader header;
		if (!readCentralDirectoryHeader(&header, stream, path))
			return 0;

		if (header.generalPurposeFlags & encryptionFlag)
		{
			DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Encrypted .zip file '%s' not supported.", path);
			errno = EFORMAT;
			return 0;
		}

		switch (header.compressionMethod)
		{
			case CompressionMethod_None:
			case CompressionMethod_Deflate:
				break;
			default:
				DS_LOG_ERROR_F(DS_CORE_LOG_TAG,
					"Compression method for .zip file '%s' not supported.", path);
				errno = EFORMAT;
				return 0;
		}

		if (header.startDisk != 0 && header.startDisk != 0xFFFF)
		{
			DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Multi-disk .zip file '%s' not supported.", path);
			errno = EFORMAT;
			return false;
		}

		fullAllocSize += DS_ALIGNED_SIZE(header.fileNameLength + 1);
		if (!dsStream_seek(stream,
				header.fileNameLength + header.extraFieldLength + header.fileCommentLength,
				dsStreamSeekWay_Current))
		{
			DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
			errno = EFORMAT;
			return false;
		}
	}

	return fullAllocSize;
}

static bool readFileEntries(dsBufferAllocator* allocator, dsStream* stream, const char* path,
	uint64_t firstDirRecordOffset, FileEntry* entries, uint64_t entryCount)
{
	const uint16_t zip64ExtraInfoSignature = 0x0001;
	const uint32_t localFileHeaderSignature = 0x04034B50;

	if (!dsStream_seek(stream, firstDirRecordOffset, dsStreamSeekWay_Beginning))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
		errno = EFORMAT;
		return false;
	}

	// First pass: read the directory headers.
	for (size_t i = 0; i < entryCount; ++i)
	{
		FileEntry* entry = entries + i;
		CentralDirectoryHeader header;
		if (!readCentralDirectoryHeader(&header, stream, path))
			return false;

		char* fileName = DS_ALLOCATE_OBJECT_ARRAY(allocator, char, header.fileNameLength + 1);
		DS_ASSERT(fileName);
		if (!dsStream_read(stream, fileName, header.fileNameLength))
		{
			DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
			errno = EFORMAT;
			return false;
		}
		fileName[header.fileNameLength] = 0;

		entry->fileName = fileName;
		entry->compressed = header.compressedSize != CompressionMethod_None;

		// Read extended zip64 info if needed.
		size_t skipSize = header.fileCommentLength;
		if (header.fileNameLength > 0 && fileName[header.fileNameLength - 1] == '/')
		{
			// Don't need to get any further info for directory entries.
			skipSize += header.extraFieldLength;
			entry->compressedSize = 0;
			entry->unompressedSize = 0;
			entry->offset = DS_STREAM_INVALID_POS;
		}
		else
		{
			bool hasZip64 = false;
			if (header.uncompressedSize == 0xFFFFFFFF || header.compressedSize == 0xFFFFFFFF ||
				header.localHeaderOffset == 0xFFFFFFFF || header.startDisk == 0xFFFF)
			{
				// Read extended zip64 info.
				size_t readExtraBytes = 0;
				while (readExtraBytes < header.extraFieldLength)
				{
					uint16_t extraSignature, extraSize;
					readExtraBytes += sizeof(uint16_t)*2;
					if (!readUInt16(&extraSignature, stream) || !readUInt16(&extraSize, stream) ||
						readExtraBytes + extraSize > header.extraFieldLength)
					{
						DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
						errno = EFORMAT;
						return false;
					}

					readExtraBytes += extraSize;
					if (extraSignature == zip64ExtraInfoSignature)
					{
						hasZip64 = true;
						size_t expectedExtraSize = 0;
						if (header.uncompressedSize == 0xFFFFFFFF)
							expectedExtraSize += sizeof(uint64_t);
						if (header.compressedSize == 0xFFFFFFFF)
							expectedExtraSize += sizeof(uint64_t);
						if (header.localHeaderOffset == 0xFFFFFFFF)
							expectedExtraSize += sizeof(uint64_t);
						if (header.startDisk == 0xFFFF)
							expectedExtraSize += sizeof(uint32_t);

						uint64_t uncompressedSize = header.uncompressedSize;
						uint64_t compressedSize = header.compressedSize;
						uint64_t localHeaderOffset = header.localHeaderOffset;
						uint32_t startDisk = header.startDisk;
						if (expectedExtraSize != extraSize ||
							(header.uncompressedSize == 0xFFFFFFFF &&
								!readUInt64(&uncompressedSize, stream)) ||
							(header.compressedSize == 0xFFFFFFFF &&
								!readUInt64(&compressedSize, stream)) ||
							(header.localHeaderOffset == 0xFFFFFFFF &&
								!readUInt64(&localHeaderOffset, stream)) ||
							(header.startDisk == 0xFFFF && !readUInt32(&startDisk, stream)))
						{
							DS_LOG_ERROR_F(DS_CORE_LOG_TAG,
								"File '%s' is not a valid .zip file.", path);
							errno = EFORMAT;
							return false;
						}

						if (startDisk != 0)
						{
							DS_LOG_ERROR_F(DS_CORE_LOG_TAG,
								"Multi-disk .zip file '%s' not supported.", path);
							errno = EFORMAT;
							return false;
						}

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
						entry->compressedSize = compressedSize;
						entry->unompressedSize = uncompressedSize;
						entry->offset = localHeaderOffset;
#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

						// No need to read any more extensions.
						break;
					}
					else if (!dsStream_seek(stream, extraSize, dsStreamSeekWay_Current))
					{
						DS_LOG_ERROR_F(DS_CORE_LOG_TAG,
							"File '%s' is not a valid .zip file.", path);
						errno = EFORMAT;
						return false;
					}
				}

				skipSize += header.extraFieldLength - readExtraBytes;
			}
			else
				skipSize += header.extraFieldLength;

			if (!hasZip64)
			{
				entry->compressedSize = header.compressedSize;
				entry->unompressedSize = header.uncompressedSize;
				entry->offset = header.localHeaderOffset;
			}
		}

		if (!dsStream_seek(stream, skipSize, dsStreamSeekWay_Current))
		{
			DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
			errno = EFORMAT;
			return false;
		}
	}

	// Second pass: read the local file headers to get the final offset to the file.
	for (size_t i = 0; i < entryCount; ++i)
	{
		FileEntry* entry = entries + i;
		// Skip directories, which have an invalid offset.
		if (entry->offset == DS_STREAM_INVALID_POS)
			continue;

		LocalFileHeader header;
		if (!dsStream_seek(stream, entry->offset, dsStreamSeekWay_Beginning) ||
			!readUInt32(&header.signature, stream) ||
			header.signature != localFileHeaderSignature ||
			!readUInt16(&header.requiredVersion, stream) ||
			!readUInt16(&header.generalPurposeFlags, stream) ||
			!readUInt16(&header.compressionMethod, stream) ||
			!readUInt16(&header.lastModFileTime, stream) ||
			!readUInt16(&header.lastModFileDate, stream) ||
			!readUInt32(&header.crc32, stream) ||
			!readUInt32(&header.compressedSize, stream) ||
			!readUInt32(&header.uncompressedSize, stream) ||
			!readUInt16(&header.fileNameLength, stream) ||
			!readUInt16(&header.extraFieldLength, stream) ||
			(entry->offset = dsStream_tell(stream)) == DS_STREAM_INVALID_POS)
		{
			DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
			errno = EFORMAT;
			return false;
		}

		entry->offset += header.fileNameLength + header.extraFieldLength;
	}

	return true;
}

static int compareEntries(const void* left, const void* right)
{
	const FileEntry* leftEntry = (const FileEntry*)left;
	const FileEntry* rightEntry = (const FileEntry*)right;
	return strcmp(leftEntry->fileName, rightEntry->fileName);
}

static dsZipArchive* openZipImpl(dsAllocator* allocator, dsFileResourceType type, const char* path,
	dsStream* stream, size_t decompressBufferSize)
{
	uint64_t fullSize;
	uint64_t endOfCentralDirOffset = findEndOfCentralDirectoryRecord(&fullSize, stream);
	if (endOfCentralDirOffset == DS_STREAM_INVALID_POS)
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "File '%s' is not a valid .zip file.", path);
		errno = EFORMAT;
		return NULL;
	}

	uint64_t firstDirRecordOffset;
	size_t entryCount;
	if (!readEndOfCentralDirectoryRecord(
			&firstDirRecordOffset, &entryCount, stream, path, endOfCentralDirOffset, fullSize))
	{
		return NULL;
	}

	size_t fullAllocSize = getFullAllocSize(stream, path, firstDirRecordOffset, entryCount);
	if (fullAllocSize == 0)
		return NULL;

	void* buffer = dsAllocator_alloc(allocator, fullAllocSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullAllocSize));

	dsZipArchive* archive = DS_ALLOCATE_OBJECT(&bufferAlloc, dsZipArchive);
	DS_ASSERT(archive);
	archive->allocator = dsAllocator_keepPointer(allocator);
	archive->resourceType = type;

	size_t pathLen = strlen(path) + 1;
	char* pathCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, pathLen);
	DS_ASSERT(pathCopy);
	memcpy(pathCopy, path, pathLen);
	archive->path = pathCopy;

	archive->entries = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, FileEntry, entryCount);
	DS_ASSERT(archive->entries);

	archive->entryCount = entryCount;
	archive->decompressBufferSize = decompressBufferSize > 0 ?
		decompressBufferSize : DS_DEFAULT_DECOMPRESS_BUFFER_SIZE;

	if (!readFileEntries(&bufferAlloc, stream, path, firstDirRecordOffset, archive->entries,
			archive->entryCount))
	{
		return false;
	}

	dsFileArchive* baseArchive = (dsFileArchive*)archive;
	baseArchive->getPathStatusFunc = (dsGetFileArchivePathStatusFunction)&dsZipArchive_pathStatus;
	baseArchive->openDirectoryFunc =
		(dsOpenFileArchiveDirectoryFunction)&dsZipArchive_openDirectory;
	baseArchive->nextDirectoryEntryFunc =
		(dsNextFileArchiveDirectoryEntryFunction)&dsZipArchive_nextDirectoryEntry;
	baseArchive->closeDirectoryFunc =
		(dsCloseFileArchiveDirectoryFunction)&dsZipArchive_closeDirectory;
	baseArchive->openFileFunc = (dsOpenFileArchiveFileFunction)&dsZipArchive_openFile;
	baseArchive->closeFileFunc = (dsCloseFileArchiveFileFunction)&dsZipArchive_closeFile;

	// Sort entries for binary sort.
	qsort(archive->entries, archive->entryCount, sizeof(FileEntry), &compareEntries);

	return archive;
}

static int comparePathPrefixWithEntry(const void* left, const void* right, void* context)
{
	const char* path = (const char*)left;
	const FileEntry* entry = (const FileEntry*)right;
	size_t length = (size_t)context;
	return strncmp(path, entry->fileName, length);
}

/*static int comparePathWithEntry(const void* left, const void* right, void* context)
{
	DS_UNUSED(context);
	const char* path = (const char*)left;
	const FileEntry* entry = (const FileEntry*)right;
	return strcmp(path, entry->fileName);
}*/

#if DS_NEEDS_PATH_SEPARATOR_FIXUP
static const char* fixupPathSeparators(
	char* finalPath, size_t finalPathLen, const char* path, size_t pathLen)
{
	if (pathLen >= finalPathLen)
	{
		errno = ESIZE;
		return false;
	}

	for (size_t i = 0; i < pathLen; ++i)
	{
		char c = path[i];
		if (c == DS_PATH_SEPARATOR || c == DS_PATH_ALT_SEPARATOR)
			finalPath[i] = '/';
		else
			finalPath[i] = c;
	}
	finalPath[pathLen] = 0;
	return finalPath;
}
#endif

static size_t removeEndingSlash(const char* path, size_t pathLen)
{
	while (pathLen > 0 && path[pathLen - 1] == '/')
		--pathLen;
	return pathLen;
}

static const char* removeLeadingDotDir(const char* path, size_t* pathLen)
{
	if (*pathLen == 0 || path[0] != '.')
		return path;

	unsigned int offset = 0;
	if (*pathLen == 1)
		offset = 1;
	else if (path[1] == '/')
		offset = 2;

	path += offset;
	*pathLen -= offset;
	return path;
}

dsZipArchive* dsZipArchive_open(
	dsAllocator* allocator, const char* path, size_t decompressBufferSize)
{
	if (!allocator || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_CORE_LOG_TAG, "Zip archive allocator musst support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, path, "rb"))
		return NULL;

	dsZipArchive* archive = openZipImpl(
		allocator, (dsFileResourceType)-1, path, (dsStream*)&stream, decompressBufferSize);
	DS_VERIFY(dsFileStream_close(&stream));
	return archive;
}

dsZipArchive* dsZipArchive_openResource(
	dsAllocator* allocator, dsFileResourceType type, const char* path, size_t decompressBufferSize)
{
	if (!allocator || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_CORE_LOG_TAG, "Zip archive allocator musst support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, path, "rb"))
		return NULL;

	dsZipArchive* archive = openZipImpl(
		allocator, type, path, (dsStream*)&stream, decompressBufferSize);
	DS_VERIFY(dsResourceStream_close(&stream));
	return archive;
}

dsPathStatus dsZipArchive_pathStatus(const dsZipArchive* archive, const char* path)
{
	if (!archive || !path || *path == 0)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

	size_t pathLen = strlen(path);

	// Always use / for path separator.
#if DS_NEEDS_PATH_SEPARATOR_FIXUP
	char finalPath[DS_PATH_MAX];
	path = fixupPathSeparators(finalPath, sizeof(finalPath), path, pathLen);
	if (!path)
		return dsPathStatus_Error;
#endif

	// Perform directory check by looking for / immediately after path, so strip / if present.
	pathLen = removeEndingSlash(path, pathLen);
	if (pathLen == 0)
		return dsPathStatus_Missing;

	// Allow for leading ./. If empty after removing, the root directory was referenced.
	path = removeLeadingDotDir(path, &pathLen);
	if (pathLen == 0)
		return dsPathStatus_ExistsDirectory;

	const FileEntry* endEntry = archive->entries + archive->entryCount;
	const FileEntry* curEntry = dsBinarySearchLowerBound(path, archive->entries,
		archive->entryCount, sizeof(FileEntry), &comparePathPrefixWithEntry, (void*)pathLen);
	if (!curEntry)
		return dsPathStatus_Missing;

	for (; curEntry != endEntry; ++curEntry)
	{
		if (strncmp(curEntry->fileName, path, pathLen) != 0)
			break;
		else if (curEntry->fileName[pathLen] == '/')
			return dsPathStatus_ExistsDirectory;
		else if (curEntry->fileName[pathLen] == 0)
			return dsPathStatus_ExistsFile;
	}

	return dsPathStatus_Missing;
}

dsDirectoryIterator dsZipArchive_openDirectory(const dsZipArchive* archive, const char* path)
{
	if (!archive || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t pathLen = strlen(path);

	// Always use / for path separator.
#if DS_NEEDS_PATH_SEPARATOR_FIXUP
	char finalPath[DS_PATH_MAX];
	path = fixupPathSeparators(finalPath, sizeof(finalPath), path, pathLen);
	if (!path)
		return NULL;
#endif

	// Perform directory check by looking for / immediately after path, so strip / if present.
	pathLen = removeEndingSlash(path, pathLen);
	if (pathLen == 0)
	{
		errno = ENOENT;
		return NULL;
	}

	// Allow for leading ./. If empty after removing, the root directory was referenced.
	path = removeLeadingDotDir(path, &pathLen);

	// Find the start for the iterator.
	const FileEntry* curEntry;
	const FileEntry* endEntry = archive->entries + archive->entryCount;
	if (pathLen == 0)
		curEntry = archive->entries;
	else
	{
		curEntry = dsBinarySearchLowerBound(path, archive->entries, archive->entryCount,
			sizeof(FileEntry), &comparePathPrefixWithEntry, (void*)pathLen);
		if (!curEntry)
		{
			errno = ENOENT;
			return NULL;
		}

		for (; curEntry != endEntry; ++curEntry)
		{
			if (strncmp(curEntry->fileName, path, pathLen) != 0)
			{
				errno = ENOENT;
				return NULL;
			}
			else if (curEntry->fileName[pathLen] == '/')
				break;
			else if (curEntry->fileName[pathLen] == 0)
			{
				errno = ENOTDIR;
				return NULL;
			}
		}

		if (curEntry == endEntry)
		{
			errno = ENOENT;
			return NULL;
		}

		// First entry might be for the directory itself.
		if (curEntry->fileName[pathLen + 1] == 0)
		{
			++curEntry;
			if (curEntry != endEntry && (strncmp(curEntry->fileName, path, pathLen) != 0 ||
					curEntry->fileName[pathLen] != '/'))
			{
				curEntry = endEntry;
			}
		}
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsDirectoryIteratorInfo));
	if (pathLen > 0)
		fullSize += DS_ALIGNED_SIZE(pathLen + 1);

	void* buffer = dsAllocator_alloc(archive->allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsDirectoryIteratorInfo* iterator = DS_ALLOCATE_OBJECT(&bufferAlloc, dsDirectoryIteratorInfo);
	DS_ASSERT(iterator);

	if (pathLen > 0)
	{
		char* prefix = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, pathLen + 1);
		DS_ASSERT(prefix);
		memcpy(prefix, path, pathLen);
		prefix[pathLen] = '/';
		iterator->prefix = prefix;
		iterator->prefixLen = pathLen + 1;
	}
	else
	{
		iterator->prefix = NULL;
		iterator->prefixLen = 0;
	}

	iterator->curEntry= curEntry;
	iterator->endEntry = endEntry;
	return iterator;
}

dsPathStatus dsZipArchive_nextDirectoryEntry(
	char* result, size_t resultSize, const dsZipArchive* archive, dsDirectoryIterator iterator)
{
	if (!result || resultSize == 0 || !archive || !iterator)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

	dsDirectoryIteratorInfo* iteratorInfo = (dsDirectoryIteratorInfo*)iterator;
	if (iteratorInfo->curEntry == iteratorInfo->endEntry)
		return dsPathStatus_Missing;

	const char* fileName = iteratorInfo->curEntry->fileName + iteratorInfo->prefixLen;
	size_t nameLen = 0;
	for (const char* c = fileName; *c && *c != '/'; ++c, ++nameLen)
		/* empty */;
	if (nameLen > resultSize)
	{
		errno = ESIZE;
		return dsPathStatus_Error;
	}

	dsPathStatus status = fileName[nameLen] == '/' ?
		dsPathStatus_ExistsDirectory : dsPathStatus_ExistsFile;
	memcpy(result, fileName, nameLen);
	result[nameLen] = 0;

	// Find the next entry.
	if (status == dsPathStatus_ExistsFile)
		++iteratorInfo->curEntry;
	else
	{
		const char* dirPrefix = iteratorInfo->curEntry->fileName;
		size_t dirPrefixLen = iteratorInfo->prefixLen + nameLen + 1;
		DS_ASSERT(dirPrefix[dirPrefixLen - 1] == '/');
		do
		{
			++iteratorInfo->curEntry;
		} while (iteratorInfo->curEntry != iteratorInfo->endEntry &&
			strncmp(dirPrefix, iteratorInfo->curEntry->fileName, dirPrefixLen) == 0);
	}

	// Check if we reached the end.
	if (iteratorInfo->curEntry != iteratorInfo->endEntry && iteratorInfo->prefix &&
		strncmp(iteratorInfo->prefix, iteratorInfo->curEntry->fileName,
			iteratorInfo->prefixLen) != 0)
	{
		iteratorInfo->curEntry = iteratorInfo->endEntry;
	}

	return status;
}

bool dsZipArchive_closeDirectory(const dsZipArchive* archive, dsDirectoryIterator iterator)
{
	if (!archive|| !iterator)
	{
		errno = EINVAL;
		return false;
	}

	return dsAllocator_free(archive->allocator, iterator);
}

dsStream* dsZipArchive_openFile(const dsZipArchive* archive, const char* path)
{
	if (!archive || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	return NULL;
}

bool dsZipArchive_closeFile(const dsZipArchive* archive, dsStream* stream)
{
	if (!archive || !stream)
	{
		errno = EINVAL;
		return false;
	}

	return true;
}

void dsZipArchive_close(dsZipArchive* archive)
{
	if (archive)
		DS_VERIFY(dsAllocator_free(archive->allocator, archive));
}
