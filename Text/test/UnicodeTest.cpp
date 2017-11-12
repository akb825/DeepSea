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

TEST(Unicode, UTF8)
{
	uint8_t utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0xF0, 0x90, 0x8D, 0x88, 0};
	uint32_t index = 0;
	EXPECT_EQ(4U, dsUTF8_codepointCount((const char*)utf8));
	EXPECT_EQ(0x24, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0xA2, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0x20AC, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(0x10348, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF8_nextCodepoint((const char*)utf8, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF8_nextCodepoint((const char*)utf8, &index));
}

TEST(Unicode, UTF16)
{
	uint16_t utf16[] = {0x0024, 0x20AC, 0xD801, 0xDC37, 0xD852, 0xDF62, 0};
	uint32_t index = 0;
	EXPECT_EQ(4U, dsUTF16_codepointCount(utf16));
	EXPECT_EQ(0x24, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x20AC, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x10437, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(0x24B62, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF16_nextCodepoint(utf16, &index));
	EXPECT_EQ(DS_UNICODE_END, dsUTF16_nextCodepoint(utf16, &index));
}
