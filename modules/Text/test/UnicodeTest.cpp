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
#include <gtest/gtest.h>
#include <string.h>

TEST(Unicode, IsCodepointValid)
{
	EXPECT_TRUE(dsIsUnicodeCodepointValid(0));
	EXPECT_TRUE(dsIsUnicodeCodepointValid(0xD7FF));
	EXPECT_FALSE(dsIsUnicodeCodepointValid(0xD800));
	EXPECT_FALSE(dsIsUnicodeCodepointValid(0xDA00));
	EXPECT_FALSE(dsIsUnicodeCodepointValid(0xDFFF));
	EXPECT_TRUE(dsIsUnicodeCodepointValid(0xE000));
	EXPECT_TRUE(dsIsUnicodeCodepointValid(0x10FFFF));
	EXPECT_FALSE(dsIsUnicodeCodepointValid(0x110000));
}

TEST(Unicode, UTF8)
{
	uint8_t utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0xF0, 0x90, 0x8D, 0x88, 0};
	size_t index = 0;
	EXPECT_EQ(4U, dsUTF8_codepointCount((const char*)utf8));
	EXPECT_EQ(10U, dsUTF8_length((const char*)utf8));
	EXPECT_EQ(0x24U, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0xA2U, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0x20ACU, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0x10348U, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF8_nextCodepoint((const char*)utf8, &index));

	char buffer[DS_ARRAY_SIZE(utf8)];
	size_t offset = 0;
	offset = dsUTF8_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x24);
	EXPECT_EQ(1U, offset);
	offset = dsUTF8_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0xA2);
	EXPECT_EQ(3U, offset);
	offset = dsUTF8_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x20AC);
	EXPECT_EQ(6U, offset);
	offset = dsUTF8_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x10348);
	EXPECT_EQ(10U, offset);
	offset = dsUTF8_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0);
	EXPECT_EQ(11U, offset);
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF8_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0));
	EXPECT_EQ(0, memcmp(utf8, buffer, sizeof(utf8)));
}

TEST(Unicode, UTF8Invalid)
{
	uint8_t utf8[5];
	// 0 encoded as a 2-byte sequence.
	utf8[0] = 0xC0;
	utf8[1] = 0x80;
	utf8[2] = 0;
	size_t index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Max overlong value for 2-byte sequence. (0x7F)
	utf8[0] = 0xC1;
	utf8[1] = 0xBF;
	utf8[2] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// First valid 2-byte sequence.
	utf8[0] = 0xC2;
	utf8[1] = 0x80;
	utf8[2] = 0;
	index = 0;
	EXPECT_EQ(0x80, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// 0 encoded as a 3-byte sequence.
	utf8[0] = 0xE0;
	utf8[1] = 0x80;
	utf8[2] = 0x80;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Max overlong value for 3-byte sequence. (0x7FF)
	utf8[0] = 0xE0;
	utf8[1] = 0x9F;
	utf8[2] = 0xBF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// First valid 3-byte sequence.
	utf8[0] = 0xE0;
	utf8[1] = 0xA0;
	utf8[2] = 0x80;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(0x800, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Valid 3-byte sequences between ones with limited range.
	utf8[0] = 0xE1;
	utf8[1] = 0x80;
	utf8[2] = 0x80;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(0x1000, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xEC;
	utf8[1] = 0xBF;
	utf8[2] = 0xBF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(0xCFFF, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// First value before UTF-16 surrogate pair range.
	utf8[0] = 0xED;
	utf8[1] = 0x9F;
	utf8[2] = 0xBF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(0xD7FF, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// First invalid value due to UTF-16 surrogate pair range. (0xD800)
	utf8[0] = 0xED;
	utf8[1] = 0xA0;
	utf8[2] = 0x80;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Last invalid value due to UTF-16 surrogate pair range. (0xDFFF)
	utf8[0] = 0xED;
	utf8[1] = 0xBF;
	utf8[2] = 0xBF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// First value after UTF-16 surrogate pair range.
	utf8[0] = 0xEE;
	utf8[1] = 0x80;
	utf8[2] = 0x80;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(0xE000, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Last valid 3-byte sequence.
	utf8[0] = 0xEF;
	utf8[1] = 0xBF;
	utf8[2] = 0xBF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(0xFFFF, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// 0 encoded as a 4-byte sequence.
	utf8[0] = 0xF0;
	utf8[1] = 0x80;
	utf8[2] = 0x80;
	utf8[3] = 0x80;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Max overlong value for 4-byte sequence. (0xFFFF)
	utf8[0] = 0xF0;
	utf8[1] = 0x8F;
	utf8[2] = 0xBF;
	utf8[3] = 0xBF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// First valid 4-byte sequence.
	utf8[0] = 0xF0;
	utf8[1] = 0x90;
	utf8[2] = 0x80;
	utf8[3] = 0x80;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(0x10000, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Valid 4-byte sequences between ones with limited range.
	utf8[0] = 0xF1;
	utf8[1] = 0x80;
	utf8[2] = 0x80;
	utf8[3] = 0x80;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(0x40000, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xF3;
	utf8[1] = 0xBF;
	utf8[2] = 0xBF;
	utf8[3] = 0xBF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(0xFFFFF, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Last valid 4-byte sequence.
	utf8[0] = 0xF4;
	utf8[1] = 0x8F;
	utf8[2] = 0xBF;
	utf8[3] = 0xBF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(0x10FFFF, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// First valid after last valid 4-byte sequence.
	utf8[0] = 0xF4;
	utf8[1] = 0x90;
	utf8[2] = 0x80;
	utf8[3] = 0x80;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Invalid continuation
	utf8[0] = 0xC2;
	utf8[1] = 0xF;
	utf8[2] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xC2;
	utf8[1] = 0xF0;
	utf8[2] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xEF;
	utf8[1] = 0xF;
	utf8[2] = 0xBF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xEF;
	utf8[1] = 0xF0;
	utf8[2] = 0xBF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xEF;
	utf8[1] = 0xBF;
	utf8[2] = 0xF;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xEF;
	utf8[1] = 0xBF;
	utf8[2] = 0xF0;
	utf8[3] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xF3;
	utf8[1] = 0xF;
	utf8[2] = 0xBF;
	utf8[3] = 0xBF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xF3;
	utf8[1] = 0xF0;
	utf8[2] = 0xBF;
	utf8[3] = 0xBF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xF3;
	utf8[1] = 0xBF;
	utf8[2] = 0xF;
	utf8[3] = 0xBF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xF3;
	utf8[1] = 0xBF;
	utf8[2] = 0xF0;
	utf8[3] = 0xBF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xF3;
	utf8[1] = 0xBF;
	utf8[2] = 0xBF;
	utf8[3] = 0xF;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	utf8[0] = 0xF3;
	utf8[1] = 0xBF;
	utf8[2] = 0xBF;
	utf8[3] = 0xF0;
	utf8[4] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_nextCodepoint((const char*)utf8, &index));

	// Check boundaries for adding codepoints.
	EXPECT_EQ(1U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0x7F));
	EXPECT_EQ(2U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0xFF));
	EXPECT_EQ(2U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0x7FF));
	EXPECT_EQ(3U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0x800));
	EXPECT_EQ(3U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0xD7FF));
	EXPECT_EQ(DS_UNICODE_INVALID_OFFSET,
		dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0xD800));
	EXPECT_EQ(DS_UNICODE_INVALID_OFFSET,
		dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0xDFFF));
	EXPECT_EQ(3U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0xE000));
	EXPECT_EQ(3U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0xFFFF));
	EXPECT_EQ(4U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0x80000));
	EXPECT_EQ(4U, dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0x10FFFF));
	EXPECT_EQ(DS_UNICODE_INVALID_OFFSET,
		dsUTF8_addCodepoint((char*)utf8, DS_ARRAY_SIZE(utf8), 0, 0x110000));
}

TEST(Unicode, UTF16)
{
	uint16_t utf16[] = {0x0024, 0x20AC, 0xD801, 0xDC37, 0xD852, 0xDF62, 0};
	size_t index = 0;
	EXPECT_EQ(4U, dsUTF16_codepointCount(utf16));
	EXPECT_EQ(6U, dsUTF16_length(utf16));
	EXPECT_EQ(0x24U, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x20ACU, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x10437U, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x24B62U, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF16_nextCodepoint(utf16, &index));

	uint16_t buffer[DS_ARRAY_SIZE(utf16)];
	size_t offset = 0;
	offset = dsUTF16_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x24);
	EXPECT_EQ(1U, offset);
	offset = dsUTF16_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x20AC);
	EXPECT_EQ(2U, offset);
	offset = dsUTF16_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x10437);
	EXPECT_EQ(4U, offset);
	offset = dsUTF16_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x24B62);
	EXPECT_EQ(6U, offset);
	offset = dsUTF16_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0);
	EXPECT_EQ(7U, offset);
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF16_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0));
	EXPECT_EQ(0, memcmp(utf16, buffer, sizeof(utf16)));
}

TEST(Unicode, UTF16Invalid)
{
	uint16_t utf16[3];

	// Last valid surrogate pair.
	utf16[0] = 0xDBFF;
	utf16[1] = 0xDFFF;
	utf16[2] = 0;
	size_t index = 0;
	EXPECT_EQ(0x10FFFF, dsUTF16_nextCodepoint(utf16, &index));

	// First invalid encoded value.
	utf16[0] = 0xDC00;
	utf16[1] = 0xDC00;
	utf16[2] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF16_nextCodepoint(utf16, &index));

	// Check boundaries for adding codepoints.
	EXPECT_EQ(1U, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0xD7FF));
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0xD800));
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0xDFFF));
	EXPECT_EQ(1U, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0xE000));
	EXPECT_EQ(1U, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0xFFFF));
	EXPECT_EQ(2U, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0x10000));
	EXPECT_EQ(2U, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0x10FFFF));
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF16_addCodepoint(utf16, DS_ARRAY_SIZE(utf16), 0, 0x110000));
}

TEST(Unicode, UTF32)
{
	uint32_t utf32[] = {0x0024, 0x20AC, 0x10437, 0x24B62, 0};
	size_t index = 0;
	EXPECT_EQ(4U, dsUTF32_codepointCount(utf32));
	EXPECT_EQ(4U, dsUTF32_length(utf32));
	EXPECT_EQ(0x24U, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(0x20ACU, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(0x10437U, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(0x24B62U, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF32_nextCodepoint(utf32, &index));

	uint32_t buffer[DS_ARRAY_SIZE(utf32)];
	size_t offset = 0;
	offset = dsUTF32_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x24);
	EXPECT_EQ(1U, offset);
	offset = dsUTF32_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x20AC);
	EXPECT_EQ(2U, offset);
	offset = dsUTF32_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x10437);
	EXPECT_EQ(3U, offset);
	offset = dsUTF32_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0x24B62);
	EXPECT_EQ(4U, offset);
	offset = dsUTF32_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0);
	EXPECT_EQ(5U, offset);
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF32_addCodepoint(buffer, DS_ARRAY_SIZE(buffer), offset, 0));
	EXPECT_EQ(0, memcmp(utf32, buffer, sizeof(utf32)));
}

TEST(Unicode, UTF32Invalid)
{
	uint32_t utf32[2];

	// Just need to check valid Unicode ranges.
	utf32[0] = 0xD7FF;
	utf32[1] = 0;
	size_t index = 0;
	EXPECT_EQ(0xD7FF, dsUTF32_nextCodepoint(utf32, &index));

	utf32[0] = 0xD800;
	utf32[1] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF32_nextCodepoint(utf32, &index));

	utf32[0] = 0xDFFF;
	utf32[1] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF32_nextCodepoint(utf32, &index));

	utf32[0] = 0xE0000;
	utf32[1] = 0;
	index = 0;
	EXPECT_EQ(0xE0000, dsUTF32_nextCodepoint(utf32, &index));

	utf32[0] = 0x10FFFF;
	utf32[1] = 0;
	index = 0;
	EXPECT_EQ(0x10FFFF, dsUTF32_nextCodepoint(utf32, &index));

	utf32[0] = 0x110000;
	utf32[1] = 0;
	index = 0;
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF32_nextCodepoint(utf32, &index));

	// Check boundaries for adding codepoints.
	EXPECT_EQ(1U, dsUTF32_addCodepoint(utf32, DS_ARRAY_SIZE(utf32), 0, 0xD7FF));
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF32_addCodepoint(utf32, DS_ARRAY_SIZE(utf32), 0, 0xD800));
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF32_addCodepoint(utf32, DS_ARRAY_SIZE(utf32), 0, 0xDFFF));
	EXPECT_EQ(1U, dsUTF32_addCodepoint(utf32, DS_ARRAY_SIZE(utf32), 0, 0x10FFFF));
	EXPECT_EQ(
		DS_UNICODE_INVALID_OFFSET, dsUTF32_addCodepoint(utf32, DS_ARRAY_SIZE(utf32), 0, 0x110000));
}
