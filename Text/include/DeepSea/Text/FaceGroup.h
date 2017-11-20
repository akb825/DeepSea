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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Text/Export.h>
#include <DeepSea/Text/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating face groups.
 *
 * At least one face group will be needed to create fonts with. Faces used across multiple fonts
 * inside the same group will be shared.
 *
 * @remark Opertions on dsFaceGroup are thread-safe and mutex protected with other font operations.
 *
 * @see dsFaceGroup
 */

/**
 * @brief The default maximum font faces for a face group.
 */
#define DS_DEFAULT_MAX_FACES 100U

/**
 * @brief The maximum lenght of a face name, including the null terminator.
 */
#define DS_MAX_FACE_NAME_LENGTH 100U

/**
 * @brief Gets the full allocation size fo a face group.
 * @param maxFaces The maximum number of faces.
 * @return The full allocation size.
 */
DS_TEXT_EXPORT size_t dsFaceGroup_fullAllocSize(uint32_t maxFaces);

/**
 * @brief Creates a face group.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the face group with.
 * @param scratchAllocator The allocator used for temporary memory. It is also used for the system
 *     font allocations that support a custom allocator. If NULL it will use allocator, and must
 *     support freeing memory.
 * @param maxFaces The maximum number of font faces to be used with the face group.
 * @param quality The quality of the rendered text.
 * @return The face group, or NULL if it couldn't be created.
 */
DS_TEXT_EXPORT dsFaceGroup* dsFaceGroup_create(dsAllocator* allocator,
	dsAllocator* scratchAllocator, uint32_t maxFaces, dsTextQuality quality);

/**
 * @brief Gets the allocator for a face group.
 * @param group The face group.
 * @return The allocator.
 */
DS_TEXT_EXPORT dsAllocator* dsFaceGroup_getAllocator(const dsFaceGroup* group);

/**
 * @brief Gets the number of remaining faces that can be loaded.
 * @param group The face group.
 * @return The number of remaining faces.
 */
DS_TEXT_EXPORT uint32_t dsFaceGroup_getRemainingFaces(const dsFaceGroup* group);

/**
 * @brief Gets whether or not a face is present in the font group.
 * @param group The font group.
 * @param name The name of the face.
 * @return True if the face has been loaded.
 */
DS_TEXT_EXPORT bool dsFaceGroup_hasFace(const dsFaceGroup* group, const char* name);

/**
 * @brief Loads a font face from file.
 * @remark errno will be set on failure.
 * @param group The face group.
 * @param fileName The name of the font file to load the face from.
 * @param name The name of the font face. The length, including null terminator, must not exceed
 *     DS_MAX_FACE_NAME_LENGTH.
 * @return True if the face was loaded.
 */
DS_TEXT_EXPORT bool dsFaceGroup_loadFaceFile(dsFaceGroup* group, const char* fileName,
	const char* name);

/**
 * @brief Loads a font face from a memory buffer.
 * @remark errno will be set on failure.
 * @param group The face group.
 * @param allocator The allocator to create a copy of the buffer. If NULL, a copy will not be
 *     created, and the caller must keep the buffer alive as long as the face group is alive.
 * @param buffer The buffer containing the font data.
 * @param size The size of the buffer.
 * @param name The name of the font face. The length, including null terminator, must not exceed
 *     DS_MAX_FACE_NAME_LENGTH.
 * @return True if the face was loaded.
 */
DS_TEXT_EXPORT bool dsFaceGroup_loadFaceBuffer(dsFaceGroup* group, dsAllocator* allocator,
	const void* buffer, size_t size, const char* name);

/**
 * @brief Gets the text rendering quality of a face group.
 * @param group The face group.
 */
DS_TEXT_EXPORT dsTextQuality dsFaceGroup_getTextQuality(const dsFaceGroup* group);

/**
 * @brief Destroys a face group.
 * @param faceGroup The face group to destroy.
 */
DS_TEXT_EXPORT void dsFaceGroup_destroy(dsFaceGroup* faceGroup);

#ifdef __cplusplus
}
#endif
