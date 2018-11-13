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
#include <gtest/gtest.h>
#include <string.h>

TEST(Unicode, UTF8)
{
	uint8_t utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0xF0, 0x90, 0x8D, 0x88, 0};
	uint32_t index = 0;
	EXPECT_EQ(4U, dsUTF8_codepointCount((const char*)utf8));
	EXPECT_EQ(10U, dsUTF8_length((const char*)utf8));
	EXPECT_EQ(0x24U, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0xA2U, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0x20ACU, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0x10348U, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF8_nextCodepoint((const char*)utf8, &index));

	char buffer[DS_ARRAY_SIZE(utf8)];
	uint32_t offset = 0;
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
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF8_addCodepoint(buffer, DS_ARRAY_SIZE(buffer),
		offset, 0));
	EXPECT_EQ(0, memcmp(utf8, buffer, sizeof(utf8)));
}

TEST(Unicode, UTF16)
{
	uint16_t utf16[] = {0x0024, 0x20AC, 0xD801, 0xDC37, 0xD852, 0xDF62, 0};
	uint32_t index = 0;
	EXPECT_EQ(4U, dsUTF16_codepointCount(utf16));
	EXPECT_EQ(6U, dsUTF16_length(utf16));
	EXPECT_EQ(0x24U, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x20ACU, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x10437U, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x24B62U, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF16_nextCodepoint(utf16, &index));

	uint16_t buffer[DS_ARRAY_SIZE(utf16)];
	uint32_t offset = 0;
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
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF16_addCodepoint(buffer, DS_ARRAY_SIZE(buffer),
		offset, 0));
	EXPECT_EQ(0, memcmp(utf16, buffer, sizeof(utf16)));
}

TEST(Unicode, UTF32)
{
	uint32_t utf32[] = {0x0024, 0x20AC, 0x10437, 0x24B62, 0};
	uint32_t index = 0;
	EXPECT_EQ(4U, dsUTF32_codepointCount(utf32));
	EXPECT_EQ(4U, dsUTF32_length(utf32));
	EXPECT_EQ(0x24U, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(0x20ACU, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(0x10437U, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(0x24B62U, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF32_nextCodepoint(utf32, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF32_nextCodepoint(utf32, &index));

	uint32_t buffer[DS_ARRAY_SIZE(utf32)];
	uint32_t offset = 0;
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
	EXPECT_EQ(DS_UNICODE_INVALID, dsUTF32_addCodepoint(buffer, DS_ARRAY_SIZE(buffer),
		offset, 0));
	EXPECT_EQ(0, memcmp(utf32, buffer, sizeof(utf32)));
}
