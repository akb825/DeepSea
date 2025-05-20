/*
 * Copyright 2019-2025 Aaron Barany
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
#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>
#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include <DeepSea/Scene/Flatbuffers/SceneCommon_generated.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

/**
 * @file
 * @brief Helper functions for working with Scene flatbuffer types.
 */

namespace DeepSeaScene
{

/**
 * @brief Converts from a flatbuffer file resource type to a dsFileResourceType.
 * @param resourceType The file resource type.
 * @return The converted resource type.
 */
inline dsFileResourceType convert(FileResourceType resourceType)
{
	return static_cast<dsFileResourceType>(resourceType);
}

/**
 * @brief Converts from a flatbuffer texture format to a dsGfxFormat.
 * @param renderer The renderer.
 * @param format The texture format.
 * @param decoration The decoration for the texture format.
 * @return The converted format.
 */
DS_SCENE_EXPORT dsGfxFormat convert(const dsRenderer* renderer, TextureFormat format,
	FormatDecoration decoration);

/**
 * @brief Converts from a flatbuffer texture dimension to a dsTextureDim.
 * @param textureDim The texture dimension.
 * @return The converted texture dimension.
 */
inline dsTextureDim convert(TextureDim textureDim)
{
	return static_cast<dsTextureDim>(textureDim);
}

/**
 * @brief Converts from a flatbuffer cube face to a dsCubeFace.
 * @param cubeFace The cube face.
 * @return The converted cube face.
 */
inline dsCubeFace convert(CubeFace cubeFace)
{
	return static_cast<dsCubeFace>(cubeFace);
}

/**
 * @brief Converts from a flatbuffer material type to a dsMaterialType.
 * @param materialType The material type.
 * @return The converted material type.
 */
inline dsMaterialType convert(MaterialType materialType)
{
	return static_cast<dsMaterialType>(materialType);
}

/**
 * @brief Converts from a flatbuffer material binding to a dsMaterialBinding.
 * @param materialBinding The material binding.
 * @return The converted material binding.
 */
inline dsMaterialBinding convert(MaterialBinding materialBinding)
{
	return static_cast<dsMaterialBinding>(materialBinding);
}

/**
 * @brief Converts from a flatbuffer vertex element format to a dsGfxFormat.
 * @param format The vertex format.
 * @param decoration The decoration for the vertex format.
 * @return The converted format.
 */
DS_SCENE_EXPORT dsGfxFormat convert(VertexElementFormat format, FormatDecoration decoration);

/**
 * @brief Converts from a flatbuffer Vector2f to a dsVector2f.
 * @param vector The vector to convert.
 * @return The converted vector.
 */
inline const dsVector2f& convert(const Vector2f& vector)
{
	return reinterpret_cast<const dsVector2f&>(vector);
}

/**
 * @brief Converts from a flatbuffer Vector3f to a dsVector3f.
 * @param vector The vector to convert.
 * @return The converted vector.
 */
inline const dsVector3f& convert(const Vector3f& vector)
{
	return reinterpret_cast<const dsVector3f&>(vector);
}

/**
 * @brief Converts from a flatbuffer Vector3d to a dsVector3d.
 * @param vector The vector to convert.
 * @return The converted vector.
 */
inline const dsVector3d& convert(const Vector3d& vector)
{
	return reinterpret_cast<const dsVector3d&>(vector);
}

/**
 * @brief Converts from a flatbuffer Vector4f to a dsVector4f.
 * @param vector The vector to convert.
 * @return The converted vector.
 */
inline dsVector4f convert(const Vector4f& vector)
{
	// Avoid unaligned access.
	dsVector4f value = {{vector.x(), vector.y(), vector.z(), vector.z()}};
	return value;
}

/**
 * @brief Converts from a flatbuffer Color3f to a dsColor3f.
 * @param color The color to convert.
 * @return The converted color.
 */
inline const dsColor3f& convert(const Color3f& color)
{
	return reinterpret_cast<const dsColor3f&>(color);
}

/**
 * @brief Converts from a flatbuffer Color4f to a dsColor4f.
 * @param color The color to convert.
 * @return The converted color.
 */
inline dsColor4f convert(const Color4f& color)
{
	// Avoid unaligned access.
	dsColor4f value = {{color.red(), color.green(), color.blue(), color.alpha()}};
	return value;
}

/**
 * @brief Converts from a flatbuffer AlignedBox3f to a dsAlignedBox3f.
 * @param box The box to convert.
 * @return The converted box.
 */
inline const dsAlignedBox3f& convert(const AlignedBox3f& box)
{
	return reinterpret_cast<const dsAlignedBox3f&>(box);
}

/**
 * @brief Converts from a flatbuffer Matrix33f to a dsMatrix33f.
 * @param matrix The matrix to convert.
 * @return The converted matrix.
 */
inline const dsMatrix33f& convert(const Matrix33f& matrix)
{
	return reinterpret_cast<const dsMatrix33f&>(matrix);
}

/**
 * @brief Converts from a flatbuffer Matrix44f to a dsMatrix44f.
 * @param matrix The matrix to convert.
 * @return The converted matrix.
 */
inline dsMatrix44f convert(const Matrix44f& matrix)
{
	// Avoid unaligned access.
	dsMatrix44f value;
	std::memcpy(&value, &matrix, sizeof(dsMatrix44f));
	return value;
}

/**
 * @brief Converts from a flatbuffer OrientedBox3f to a dsOrientedBox3f.
 * @param box The box to convert.
 * @return The converted box.
 */
inline const dsOrientedBox3f& convert(const OrientedBox3f& box)
{
	return reinterpret_cast<const dsOrientedBox3f&>(box);
}

/**
 * @brief Converts from a flatbuffer DynamicRenderStates to a dsDynamicRenderStates.
 * @param renderStates The render states to convert.
 * @return The converted render states.
 */
DS_SCENE_EXPORT dsDynamicRenderStates convert(const DynamicRenderStates& renderStates);

} // namespace DeepSeaScene
