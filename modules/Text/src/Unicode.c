/*
 * Copyright 2017-2026 Aaron Barany
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

#include <DeepSea/Text/Unicode.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

bool dsIsUnicodeCodepointValid(uint32_t codepoint)
{
	// Values up to 0x10FFFF, except for UTF-16 surrogate pair values.
	return codepoint < 0xD800 || (codepoint > 0xDFFF && codepoint <= 0x10FFFF);
}

uint32_t dsUTF8_nextCodepoint(const char* string, size_t* index)
{
	if (!string || !index)
		return DS_UNICODE_INVALID;

	uint32_t character = 0;
	uint8_t c = string[*index];
	if (!c)
		return DS_UNICODE_END;

	++*index;

	// See https://datatracker.ietf.org/doc/html/rfc3629#section-4; this checks against overlong
	// encodings (e.g. encoding the value 0 with the byte sequence 0xC0 0x80) and disallowed values
	// that match UTF-16 surrogate pairs, which can have security implications.

	// ASCII values.
	if ((c & 0x80) == 0)
		return c;

	// Range for valid 2 byte sequences.
	if (c >= 0xC2 && c <= 0xDF)
	{
		character = c & 0x1F;
		c = string[*index];
		if ((c & 0xC0) != 0x80)
			return DS_UNICODE_INVALID;

		++*index;
		return (character << 6) | (c & 0x3F);
	}

	// Range for valid 3 byte sequences.
	if (c >= 0xE0 && c <= 0xEF)
	{
		uint8_t prevC = c;
		character = c & 0x0F;
		c = string[*index];
		// Also check for overlong encodings and disallowed values matching UTF-16 surrogate pairs.
		if ((c & 0xC0) != 0x80 || (prevC == 0xE0 && (c < 0xA0 || c > 0xBF)) ||
			(prevC == 0xED && c > 0x9F))
		{
			return DS_UNICODE_INVALID;
		}

		++*index;
		character = (character << 6) | (c & 0x3F);

		c = string[*index];
		if ((c & 0xC0) != 0x80)
			return DS_UNICODE_INVALID;

		++*index;
		return (character << 6) | (c & 0x3F);
	}

	// Range for valid 4 byte sequences.
	if (c >= 0xF0 && c <= 0xF4)
	{
		uint8_t prevC = c;
		character = c & 0x07;
		c = string[*index];
		// Also check for overlong encodings.
		if ((c & 0xC0) != 0x80 || (prevC == 0xF0 && c < 0x90) || (prevC == 0xF4 && c > 0x8F))
			return DS_UNICODE_INVALID;

		++*index;
		character = (character << 6) | (c & 0x3F);

		c = string[*index];
		if ((c & 0xC0) != 0x80)
			return DS_UNICODE_INVALID;

		++*index;
		character = (character << 6) | (c & 0x3F);

		c = string[*index];
		if ((c & 0xC0) != 0x80)
			return DS_UNICODE_INVALID;

		++*index;
		return (character << 6) | (c & 0x3F);
	}

	return DS_UNICODE_INVALID;
}

size_t dsUTF8_codepointCount(const char* string)
{
	if (!string)
		return 0;

	size_t length = 0;
	size_t index = 0;
	do
	{
		uint32_t codepoint = dsUTF8_nextCodepoint(string, &index);
		if (codepoint == DS_UNICODE_END)
			return length;
		else if (codepoint == DS_UNICODE_INVALID)
			return DS_UNICODE_INVALID;

		++length;
	} while (true);
}

size_t dsUTF8_length(const char* string)
{
	if (!string)
		return 0;

	return strlen(string);
}

uint32_t dsUTF8_codepointSize(uint32_t codepoint)
{
	if (!dsIsUnicodeCodepointValid(codepoint))
		return DS_UNICODE_INVALID;

	if (codepoint <= 0x7F)
		return 1;
	if (codepoint <= 0x7FF)
		return 2;
	if (codepoint <= 0xFFFF)
		return 3;
	DS_ASSERT(codepoint <= 0x10FFFF);
	return 4;
}

size_t dsUTF8_addCodepoint(char* string, size_t length, size_t offset, uint32_t codepoint)
{
	uint32_t codepointSize = dsUTF8_codepointSize(codepoint);
	if (!string || codepointSize == DS_UNICODE_INVALID)
	{
		errno = EINVAL;
		return DS_UNICODE_INVALID_OFFSET;
	}

	if (!DS_IS_BUFFER_RANGE_VALID(offset, codepointSize, length))
	{
		errno = ESIZE;
		return DS_UNICODE_INVALID_OFFSET;
	}

	switch (codepointSize)
	{
		case 1:
			string[offset++] = (char)codepoint;
			break;
		case 2:
			string[offset++] = (char)(0xC0 | ((codepoint >> 6) & 0x1F));
			string[offset++] = (char)(0x80 | (codepoint & 0x3F));
			break;
		case 3:
			string[offset++] = (char)(0xE0 | ((codepoint >> 12) & 0xF));
			string[offset++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
			string[offset++] = (char)(0x80 | (codepoint & 0x3F));
			break;
		case 4:
			string[offset++] = (char)(0xF0 | ((codepoint >> 18) & 0x7));
			string[offset++] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
			string[offset++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
			string[offset++] = (char)(0x80 | (codepoint & 0x3F));
			break;
	}

	return offset;
}

uint32_t dsUTF16_nextCodepoint(const uint16_t* string, size_t* index)
{
	if (!string || !index)
		return DS_UNICODE_INVALID;

	// See https://datatracker.ietf.org/doc/html/rfc2781#section-2.2
	uint16_t c = string[*index];
	if (!c)
		return DS_UNICODE_END;

	++*index;
	if (c < 0xD800 || c > 0xDFFF)
		return c;

	if (c > 0xDBFF)
		return DS_UNICODE_INVALID;

	uint16_t c2 = string[*index];
	if ((c2 & 0xDC00) != 0xDC00)
		return DS_UNICODE_INVALID;

	++*index;
	return 0x10000 + (((c & 0x3FF) << 10) | (c2 & 0x3FF));
}

size_t dsUTF16_codepointCount(const uint16_t* string)
{
	if (!string)
		return 0;

	size_t length = 0;
	size_t index = 0;
	do
	{
		uint32_t codepoint = dsUTF16_nextCodepoint(string, &index);
		if (codepoint == DS_UNICODE_END)
			return length;
		else if (codepoint == DS_UNICODE_INVALID)
			return DS_UNICODE_INVALID;

		++length;
	} while (true);
}

size_t dsUTF16_length(const uint16_t* string)
{
	if (!string)
		return 0;

	size_t count = 0;
	for (; *string; ++string, ++count)
		/* empty */;
	return count;
}

uint32_t dsUTF16_codepointSize(uint32_t codepoint)
{
	if (!dsIsUnicodeCodepointValid(codepoint))
		return DS_UNICODE_INVALID;

	if (codepoint <= 0xFFFF)
		return 1;
	DS_ASSERT(codepoint <= 0x10FFFF);
	return 2;
}

size_t dsUTF16_addCodepoint(uint16_t* string, size_t length, size_t offset, uint32_t codepoint)
{
	if (!string)
		return DS_UNICODE_INVALID_OFFSET;

	uint32_t codepointSize = dsUTF16_codepointSize(codepoint);
	if (codepointSize == DS_UNICODE_INVALID)
		return DS_UNICODE_INVALID_OFFSET;

	if (!DS_IS_BUFFER_RANGE_VALID(offset, codepointSize, length))
		return DS_UNICODE_INVALID_OFFSET;

	switch (codepointSize)
	{
		case 1:
			string[offset++] = (uint16_t)codepoint;
			break;
		case 2:
			codepoint -= 0x010000;
			string[offset++] = (uint16_t)(0xD800 | ((codepoint >> 10) & 0x3FF));
			string[offset++] = (uint16_t)(0xDC00 | (codepoint & 0x3FF));
			break;
	}

	return offset;
}

uint32_t dsUTF32_nextCodepoint(const uint32_t* string, size_t* index)
{
	if (!string || !index)
		return DS_UNICODE_INVALID;

	uint32_t c = string[*index];
	if (c == 0)
		return DS_UNICODE_END;
	else if (!dsIsUnicodeCodepointValid(c))
		return DS_UNICODE_INVALID;

	++*index;
	return c;
}

size_t dsUTF32_codepointCount(const uint32_t* string)
{
	return dsUTF32_length(string);
}

size_t dsUTF32_length(const uint32_t* string)
{
	if (!string)
		return 0;

	size_t count = 0;
	for (; *string; ++string, ++count)
		/* empty */;
	return count;
}

uint32_t dsUTF32_codepointSize(uint32_t codepoint)
{
	if (!dsIsUnicodeCodepointValid(codepoint))
		return DS_UNICODE_INVALID;

	return 1;
}

size_t dsUTF32_addCodepoint(uint32_t* string, size_t length, size_t offset, uint32_t codepoint)
{
	if (!string || !dsIsUnicodeCodepointValid(codepoint))
	{
		errno = EINVAL;
		return DS_UNICODE_INVALID_OFFSET;
	}

	if (offset >= length)
	{
		errno = ESIZE;
		return DS_UNICODE_INVALID_OFFSET;
	}

	string[offset++] = codepoint;
	return offset;
}
