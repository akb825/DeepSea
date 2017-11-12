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

uint32_t dsUTF16_nextCodepoint(const uint16_t* string, uint32_t* index)
{
	if (!string || !index)
		return DS_UNICODE_INVALID;

	uint16_t c = string[*index];
	if (!c)
		return DS_UNICODE_END;
	++*index;

	if (c < 0xD800)
		return c;

	if ((c & 0xD800) == 0xD800)
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
