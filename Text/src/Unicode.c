/*
 * Copyright 2017 Aaron Barany
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
#include <string.h>

uint32_t dsUTF8_nextCodepoint(const char* string, uint32_t* index)
{
	if (!string || !index)
		return DS_UNICODE_INVALID;

	uint32_t character = 0;
	uint8_t c = string[*index];
	if (!c)
		return DS_UNICODE_END;
	++*index;

	if ((c & 0x80) == 0)
		return c;
	else if ((c & 0xE0) == 0xC0)
	{
		character = c & 0x1F;
		c = string[*index];
		if ((c & 0xC0) != 0x80)
			return DS_UNICODE_INVALID;
		++*index;

		return (character << 6) | (c & 0x3F);
	}
	else if ((c & 0xF0) == 0xE0)
	{
		character = c & 0x0F;
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
	else if ((c & 0xF8) == 0xF0)
	{
		character = c & 0x07;
		c = string[*index];
		if ((c & 0xC0) != 0x80)
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

uint32_t dsUTF8_codepointCount(const char* string)
{
	if (!string)
		return 0;

	uint32_t length = 0;
	uint32_t index = 0;
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

uint32_t dsUTF8_length(const char* string)
{
	if (!string)
		return 0;

	return (uint32_t)strlen(string);
}

uint32_t dsUTF8_codepointSize(uint32_t codepoint)
{
	if (codepoint <= 0x7F)
		return 1;
	else if (codepoint <= 0x7FF)
		return 2;
	else if (codepoint <= 0xFFFF)
		return 3;
	else if (codepoint <= 0x1FFFFF)
		return 4;
	return DS_UNICODE_INVALID;
}

uint32_t dsUTF8_addCodepoint(char* string, uint32_t length, uint32_t offset, uint32_t codepoint)
{
	if (!string)
		return DS_UNICODE_INVALID;

	uint32_t codepointSize = dsUTF8_codepointSize(codepoint);
	if (codepointSize == DS_UNICODE_INVALID)
		return DS_UNICODE_INVALID;

	if (!DS_IS_BUFFER_RANGE_VALID(offset, codepointSize, length))
		return DS_UNICODE_INVALID;

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

uint32_t dsUTF16_nextCodepoint(const uint16_t* string, uint32_t* index)
{
	if (!string || !index)
		return DS_UNICODE_INVALID;

	uint16_t c = string[*index];
	if (!c)
		return DS_UNICODE_END;
	++*index;

	if (c < 0xD800 || c > 0xDFFF)
		return c;

	if ((c & 0xDC00) == 0xD800)
	{
		uint16_t c2 = string[*index];
		if ((c2 & 0xDC00) != 0xDC00)
			return DS_UNICODE_INVALID;
		++*index;
		return 0x010000 + (((c & 0x3FF) << 10) | (c2 & 0x3FF));
	}
	return DS_UNICODE_INVALID;
}

uint32_t dsUTF16_codepointCount(const uint16_t* string)
{
	if (!string)
		return 0;

	uint32_t length = 0;
	uint32_t index = 0;
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

uint32_t dsUTF16_length(const uint16_t* string)
{
	if (!string)
		return 0;

	uint32_t count = 0;
	for (; *string; ++string, ++count)
		/* empty */;
	return count;
}

uint32_t dsUTF16_codepointSize(uint32_t codepoint)
{
	if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
		return DS_UNICODE_INVALID;
	if (codepoint <= 0xFFFF)
		return 1;
	else if (codepoint <= 0x10FFFF)
		return 2;
	return DS_UNICODE_INVALID;
}

uint32_t dsUTF16_addCodepoint(uint16_t* string, uint32_t length, uint32_t offset,
	uint32_t codepoint)
{
	if (!string)
		return DS_UNICODE_INVALID;

	uint32_t codepointSize = dsUTF16_codepointSize(codepoint);
	if (codepointSize == DS_UNICODE_INVALID)
		return DS_UNICODE_INVALID;

	if (!DS_IS_BUFFER_RANGE_VALID(offset, codepointSize, length))
		return DS_UNICODE_INVALID;

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

uint32_t dsUTF32_nextCodepoint(const uint32_t* string, uint32_t* index)
{
	if (!string || !index)
		return DS_UNICODE_INVALID;

	uint32_t c = string[*index];
	if (c == 0)
		return DS_UNICODE_END;

	++*index;
	return c;
}

uint32_t dsUTF32_codepointCount(const uint32_t* string)
{
	return dsUTF32_length(string);
}

uint32_t dsUTF32_length(const uint32_t* string)
{
	if (!string)
		return 0;

	uint32_t count = 0;
	for (; *string; ++string, ++count)
		/* empty */;
	return count;
}

uint32_t dsUTF32_codepointSize(uint32_t codepoint)
{
	DS_UNUSED(codepoint);
	return 1;
}

uint32_t dsUTF32_addCodepoint(uint32_t* string, uint32_t length, uint32_t offset,
		uint32_t codepoint)
{
	if (!string)
		return DS_UNICODE_INVALID;

	if (offset >= length)
		return DS_UNICODE_INVALID;

	string[offset++] = codepoint;
	return offset;
}
