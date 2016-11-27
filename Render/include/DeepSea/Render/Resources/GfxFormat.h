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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Utility functions for dealing with graphics resources.
 */

/**
 * @brief Checks whether or not a graphics format is valid.
 * @param format The format to check.
 * @return True if format is valid.
 */
DS_RENDER_EXPORT bool dsGfxFormat_isValid(dsGfxFormat format);

/**
 * @brief Gets the index of a standard format.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param format The format.
 * @return The standard format index, or 0 if not a standard format. The first valid format will
 *     be index 1.
 */
inline unsigned int dsGfxFormat_standardIndex(dsGfxFormat format);

/**
 * @brief Gets the enum of a standard format index.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param index The index of the format.
 * @return The standard format enum.
 */
inline dsGfxFormat dsGfxFormat_standardEnum(unsigned int index);

/**
 * @brief Gets the index of a special format.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param format The format.
 * @return The special format index, or 0 if not a special format. The first valid format will be
 *     index 1.
 */
inline unsigned int dsGfxFormat_specialIndex(dsGfxFormat format);

/**
 * @brief Gets the enum of a special format index.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param index The index of the format.
 * @return The special format enum.
 */
inline dsGfxFormat dsGfxFormat_specialEnum(unsigned int index);

/**
 * @brief Gets the index of a compressed format.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param format The format.
 * @return The compressed format index, or 0 if not a compressed format. The first valid format will
 *     be index 1.
 */
inline unsigned int dsGfxFormat_compressedIndex(dsGfxFormat format);

/**
 * @brief Gets the enum of a compressed format index.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param index The index of the format.
 * @return The compressed format enum.
 */
inline dsGfxFormat dsGfxFormat_compressedEnum(unsigned int index);

/**
 * @brief Gets the index of a decorator.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param format The format.
 * @return The decoration index, or 0 if not decorated.
 */
inline unsigned int dsGfxFormat_decoratorIndex(dsGfxFormat format);

/**
 * @brief Gets the enum of a decorator index.
 *
 * This is useful when indexing into an array by the format.
 *
 * @param index The index of the format.
 * @return The decorator format enum.
 */
inline dsGfxFormat dsGfxFormat_decoratorEnum(unsigned int index);

/**
 * @brief Decorates a graphics format.
 * @param format The format to decorate.
 * @param decorator The decorator to apply.
 * @return The decorated format.
 */
inline dsGfxFormat dsGfxFormat_decorate(dsGfxFormat format, dsGfxFormat decorator);

inline unsigned int dsGfxFormat_standardIndex(dsGfxFormat format)
{
	return format & dsGfxFormat_StandardMask;
}

inline dsGfxFormat dsGfxFormat_standardEnum(unsigned int index)
{
	if (index >= (unsigned int)dsGfxFormat_StandardCount)
		return dsGfxFormat_Unknown;

	return (dsGfxFormat)index;
}

inline unsigned int dsGfxFormat_specialIndex(dsGfxFormat format)
{
	return (format & dsGfxFormat_SpecialMask) >> 8;
}

inline dsGfxFormat dsGfxFormat_specialEnum(unsigned int index)
{
	if (index >= (unsigned int)dsGfxFormat_SpecialCount)
		return dsGfxFormat_Unknown;

	return (dsGfxFormat)(index << 8);
}

inline unsigned int dsGfxFormat_compressedIndex(dsGfxFormat format)
{
	return (format & dsGfxFormat_CompressedMask) >> 12;
}

inline dsGfxFormat dsGfxFormat_compressedEnum(unsigned int index)
{
	if (index >= (unsigned int)dsGfxFormat_CompressedCount)
		return dsGfxFormat_Unknown;

	return (dsGfxFormat)(index << 12);
}

inline unsigned int dsGfxFormat_decoratorIndex(dsGfxFormat format)
{
	return (format & dsGfxFormat_DecoratorMask) >> 20;
}

inline dsGfxFormat dsGfxFormat_decoratorEnum(unsigned int index)
{
	if (index >= (unsigned int)dsGfxFormat_DecoratorCount)
		return dsGfxFormat_Unknown;

	return (dsGfxFormat)(index << 20);
}

inline dsGfxFormat dsGfxFormat_decorate(dsGfxFormat format, dsGfxFormat decorator)
{
	return (dsGfxFormat)(format | decorator);
}

#ifdef __cplusplus
}
#endif
