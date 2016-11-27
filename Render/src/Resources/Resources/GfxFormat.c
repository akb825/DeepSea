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

#include <DeepSea/Render/Resources/GfxFormat.h>

bool dsGfxFormat_isValid(dsGfxFormat format)
{
	if (format & dsGfxFormat_StandardMask)
	{
		if (!(format & dsGfxFormat_DecoratorMask))
			return false;

		if ((format & dsGfxFormat_SpecialMask) || (format & dsGfxFormat_CompressedMask))
			return false;

		return true;
	}
	else if ((format & dsGfxFormat_SpecialMask))
	{
		if (format & dsGfxFormat_DecoratorMask)
			return false;

		if (format & dsGfxFormat_CompressedMask)
			return false;

		return true;
	}
	else if (format & dsGfxFormat_CompressedMask)
		return true;
	else
		return false;
}

DS_RENDER_EXPORT unsigned int dsGfxFormat_standardIndex(dsGfxFormat format);
DS_RENDER_EXPORT dsGfxFormat dsGfxFormat_standardEnum(unsigned int index);
DS_RENDER_EXPORT unsigned int dsGfxFormat_specialIndex(dsGfxFormat format);
DS_RENDER_EXPORT dsGfxFormat dsGfxFormat_specialEnum(unsigned int index);
DS_RENDER_EXPORT unsigned int dsGfxFormat_compressedIndex(dsGfxFormat format);
DS_RENDER_EXPORT dsGfxFormat dsGfxFormat_compressedEnum(unsigned int index);
DS_RENDER_EXPORT unsigned int dsGfxFormat_decoratorIndex(dsGfxFormat format);
DS_RENDER_EXPORT dsGfxFormat dsGfxFormat_decoratorEnum(unsigned int index);
DS_RENDER_EXPORT dsGfxFormat dsGfxFormat_decorate(dsGfxFormat format, dsGfxFormat decorator);
