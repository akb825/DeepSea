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

#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Core/Assert.h>

static const uint16_t bufferAlignment[] =
{
	// Scalars and vectors
	sizeof(float),          // dsMaterialType_Float
	sizeof(float)*2,        // dsMaterialType_Vec2
	sizeof(float)*4,        // dsMaterialType_Vec3
	sizeof(float)*4,        // dsMaterialType_Vec4
	sizeof(double),         // dsMaterialType_Double
	sizeof(double)*2,       // dsMaterialType_DVec2
	sizeof(double)*4,       // dsMaterialType_DVec3
	sizeof(double)*4,       // dsMaterialType_DVec4
	sizeof(int),            // dsMaterialType_Int
	sizeof(int)*2,          // dsMaterialType_IVec2
	sizeof(int)*4,          // dsMaterialType_IVec3
	sizeof(int)*4,          // dsMaterialType_IVec4
	sizeof(unsigned int),   // dsMaterialType_UInt
	sizeof(unsigned int)*2, // dsMaterialType_UVec2
	sizeof(unsigned int)*4, // dsMaterialType_UVec3
	sizeof(unsigned int)*4, // dsMaterialType_UVec4
	sizeof(int),            // dsMaterialType_Bool
	sizeof(int)*2,          // dsMaterialType_BVec2
	sizeof(int)*4,          // dsMaterialType_BVec3
	sizeof(int)*4,          // dsMaterialType_BVec4
};
DS_STATIC_ASSERT(DS_ARRAY_SIZE(bufferAlignment) == dsMaterialType_Mat2,
	buffer_alignment_array_enum_mismatch);

unsigned int dsMaterialType_matrixRows(dsMaterialType type)
{
	if (type < dsMaterialType_Mat2 || type > dsMaterialType_DMat4x3)
		return 0;

	static const unsigned int rows[] =
	{
		2, // dsMaterialType_Mat2
		3, // dsMaterialType_Mat3
		4, // dsMaterialType_Mat4
		3, // dsMaterialType_Mat2x3
		4, // dsMaterialType_Mat2x4
		2, // dsMaterialType_Mat3x2
		4, // dsMaterialType_Mat3x4
		2, // dsMaterialType_Mat4x2
		3, // dsMaterialType_Mat4x3
		2, // dsMaterialType_DMat2
		3, // dsMaterialType_DMat3
		4, // dsMaterialType_DMat4
		3, // dsMaterialType_DMat2x3
		4, // dsMaterialType_DMat2x4
		2, // dsMaterialType_DMat3x2
		4, // dsMaterialType_DMat3x4
		2, // dsMaterialType_DMat4x2
		3, // dsMaterialType_DMat4x3
	};

	DS_ASSERT(type - dsMaterialType_Mat2 < DS_ARRAY_SIZE(rows));
	return rows[type - dsMaterialType_Mat2];
}

unsigned int dsMaterialType_matrixColumns(dsMaterialType type)
{
	if (type < dsMaterialType_Mat2 || type > dsMaterialType_DMat4x3)
		return 0;

	static const unsigned int columns[] =
	{
		2, // dsMaterialType_Mat2
		3, // dsMaterialType_Mat3
		4, // dsMaterialType_Mat4
		2, // dsMaterialType_Mat2x3
		2, // dsMaterialType_Mat2x4
		3, // dsMaterialType_Mat3x2
		3, // dsMaterialType_Mat3x4
		4, // dsMaterialType_Mat4x2
		4, // dsMaterialType_Mat4x3
		2, // dsMaterialType_DMat2
		3, // dsMaterialType_DMat3
		4, // dsMaterialType_DMat4
		2, // dsMaterialType_DMat2x3
		2, // dsMaterialType_DMat2x4
		3, // dsMaterialType_DMat3x2
		3, // dsMaterialType_DMat3x4
		4, // dsMaterialType_DMat4x2
		4, // dsMaterialType_DMat4x3
	};

	DS_ASSERT(type - dsMaterialType_Mat2 < DS_ARRAY_SIZE(columns));
	return columns[type - dsMaterialType_Mat2];
}

dsMaterialType dsMaterialType_matrixColumnType(dsMaterialType type)
{
	if (type < dsMaterialType_Mat2 || type > dsMaterialType_DMat4x3)
		return dsMaterialType_Count;

	static const dsMaterialType columnTypes[] =
	{
		dsMaterialType_Vec2,  // dsMaterialType_Mat2
		dsMaterialType_Vec3,  // dsMaterialType_Mat3
		dsMaterialType_Vec4,  // dsMaterialType_Mat4
		dsMaterialType_Vec3,  // dsMaterialType_Mat2x3
		dsMaterialType_Vec4,  // dsMaterialType_Mat2x4
		dsMaterialType_Vec2,  // dsMaterialType_Mat3x2
		dsMaterialType_Vec4,  // dsMaterialType_Mat3x4
		dsMaterialType_Vec2,  // dsMaterialType_Mat4x2
		dsMaterialType_Vec3,  // dsMaterialType_Mat4x3
		dsMaterialType_DVec2, // dsMaterialType_DMat2
		dsMaterialType_DVec3, // dsMaterialType_DMat3
		dsMaterialType_DVec4, // dsMaterialType_DMat4
		dsMaterialType_DVec3, // dsMaterialType_DMat2x3
		dsMaterialType_DVec4, // dsMaterialType_DMat2x4
		dsMaterialType_DVec2, // dsMaterialType_DMat3x2
		dsMaterialType_DVec4, // dsMaterialType_DMat3x4
		dsMaterialType_DVec2, // dsMaterialType_DMat4x2
		dsMaterialType_DVec3, // dsMaterialType_DMat4x3
	};

	DS_ASSERT(type - dsMaterialType_Mat2 < DS_ARRAY_SIZE(columnTypes));
	return columnTypes[type - dsMaterialType_Mat2];
}

dsMaterialType dsMaterialType_matrixRowType(dsMaterialType type)
{
	if (type < dsMaterialType_Mat2 || type > dsMaterialType_DMat4x3)
		return dsMaterialType_Count;

	static const dsMaterialType columnTypes[] =
	{
		dsMaterialType_Vec2,  // dsMaterialType_Mat2
		dsMaterialType_Vec3,  // dsMaterialType_Mat3
		dsMaterialType_Vec4,  // dsMaterialType_Mat4
		dsMaterialType_Vec2,  // dsMaterialType_Mat2x3
		dsMaterialType_Vec2,  // dsMaterialType_Mat2x4
		dsMaterialType_Vec3,  // dsMaterialType_Mat3x2
		dsMaterialType_Vec3,  // dsMaterialType_Mat3x4
		dsMaterialType_Vec4,  // dsMaterialType_Mat4x2
		dsMaterialType_Vec4,  // dsMaterialType_Mat4x3
		dsMaterialType_DVec2, // dsMaterialType_DMat2
		dsMaterialType_DVec3, // dsMaterialType_DMat3
		dsMaterialType_DVec4, // dsMaterialType_DMat4
		dsMaterialType_DVec2, // dsMaterialType_DMat2x3
		dsMaterialType_DVec2, // dsMaterialType_DMat2x4
		dsMaterialType_DVec3, // dsMaterialType_DMat3x2
		dsMaterialType_DVec3, // dsMaterialType_DMat3x4
		dsMaterialType_DVec4, // dsMaterialType_DMat4x2
		dsMaterialType_DVec4, // dsMaterialType_DMat4x3
	};

	DS_ASSERT(type - dsMaterialType_Mat2 < DS_ARRAY_SIZE(columnTypes));
	return columnTypes[type - dsMaterialType_Mat2];
}

uint16_t dsMaterialType_cpuSize(dsMaterialType type)
{
	static const uint16_t size[] =
	{
		// Scalars and vectors
		sizeof(float),        // dsMaterialType_Float
		sizeof(dsVector2f),   // dsMaterialType_Vec2
		sizeof(dsVector3f),   // dsMaterialType_Vec3
		sizeof(dsVector4f),   // dsMaterialType_Vec4
		sizeof(double),       // dsMaterialType_Double
		sizeof(dsVector2d),   // dsMaterialType_DVec2
		sizeof(dsVector3d),   // dsMaterialType_DVec3
		sizeof(dsVector4d),   // dsMaterialType_DVec4
		sizeof(int),          // dsMaterialType_Int
		sizeof(dsVector2i),   // dsMaterialType_IVec2
		sizeof(dsVector3i),   // dsMaterialType_IVec3
		sizeof(dsVector4i),   // dsMaterialType_IVec4
		sizeof(unsigned int), // dsMaterialType_UInt
		sizeof(dsVector2i),   // dsMaterialType_UVec2
		sizeof(dsVector3i),   // dsMaterialType_UVec3
		sizeof(dsVector4i),   // dsMaterialType_UVec4
		sizeof(int),          // dsMaterialType_Bool
		sizeof(dsVector2i),   // dsMaterialType_BVec2
		sizeof(dsVector3i),   // dsMaterialType_BVec3
		sizeof(dsVector4i),   // dsMaterialType_BVec4

		// Matrices
		sizeof(float)*2*2,  // dsMaterialType_Mat2
		sizeof(float)*3*3,  // dsMaterialType_Mat3
		sizeof(float)*4*4,  // dsMaterialType_Mat4
		sizeof(float)*2*3,  // dsMaterialType_Mat2x3
		sizeof(float)*2*4,  // dsMaterialType_Mat2x4
		sizeof(float)*3*2,  // dsMaterialType_Mat3x2
		sizeof(float)*3*4,  // dsMaterialType_Mat3x4
		sizeof(float)*4*2,  // dsMaterialType_Mat4x2
		sizeof(float)*4*3,  // dsMaterialType_Mat4x3
		sizeof(double)*2*2, // dsMaterialType_DMat2
		sizeof(double)*3*3, // dsMaterialType_DMat3
		sizeof(double)*4*4, // dsMaterialType_DMat4
		sizeof(double)*2*3, // dsMaterialType_DMat2x3
		sizeof(double)*2*4, // dsMaterialType_DMat2x4
		sizeof(double)*3*2, // dsMaterialType_DMat3x2
		sizeof(double)*3*4, // dsMaterialType_DMat3x4
		sizeof(double)*4*2, // dsMaterialType_DMat4x2
		sizeof(double)*4*3, // dsMaterialType_DMat4x3

		// Other types
		sizeof(void*), // dsMaterialType_Texture
		sizeof(void*), // dsMaterialType_Image
		sizeof(void*), // dsMaterialType_SubpassInput
		sizeof(void*), // dsMaterialType_TextureBuffer
		sizeof(void*), // dsMaterialType_MutableTextureBuffer
		sizeof(void*), // dsMaterialType_VariableGroup
		sizeof(void*), // dsMaterialType_UniformBlock
		sizeof(void*), // dsMaterialType_UniformBuffer
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(size) == dsMaterialType_Count, array_enum_mismatch);

	if ((unsigned int)type >= dsMaterialType_Count)
		return 0;

	return size[type];
}

uint16_t dsMaterialType_cpuAlignment(dsMaterialType type)
{
	static const uint16_t alignment[] =
	{
		// Scalars and vectors
		sizeof(float),        // dsMaterialType_Float
		sizeof(float),        // dsMaterialType_Vec2
		sizeof(float),        // dsMaterialType_Vec3
		sizeof(float),        // dsMaterialType_Vec4
		sizeof(double),       // dsMaterialType_Double
		sizeof(double),       // dsMaterialType_DVec2
		sizeof(double),       // dsMaterialType_DVec3
		sizeof(double),       // dsMaterialType_DVec4
		sizeof(int),          // dsMaterialType_Int
		sizeof(int),          // dsMaterialType_IVec2
		sizeof(int),          // dsMaterialType_IVec3
		sizeof(int),          // dsMaterialType_IVec4
		sizeof(unsigned int), // dsMaterialType_UInt
		sizeof(unsigned int), // dsMaterialType_UVec2
		sizeof(unsigned int), // dsMaterialType_UVec3
		sizeof(unsigned int), // dsMaterialType_UVec4
		sizeof(int),          // dsMaterialType_Bool
		sizeof(int),          // dsMaterialType_BVec2
		sizeof(int),          // dsMaterialType_BVec3
		sizeof(int),          // dsMaterialType_BVec4

		// Matrices
		sizeof(float),  // dsMaterialType_Mat2
		sizeof(float),  // dsMaterialType_Mat3
		sizeof(float),  // dsMaterialType_Mat4
		sizeof(float),  // dsMaterialType_Mat2x3
		sizeof(float),  // dsMaterialType_Mat2x4
		sizeof(float),  // dsMaterialType_Mat3x2
		sizeof(float),  // dsMaterialType_Mat3x4
		sizeof(float),  // dsMaterialType_Mat4x2
		sizeof(float),  // dsMaterialType_Mat4x3
		sizeof(double), // dsMaterialType_DMat2
		sizeof(double), // dsMaterialType_DMat3
		sizeof(double), // dsMaterialType_DMat4
		sizeof(double), // dsMaterialType_DMat2x3
		sizeof(double), // dsMaterialType_DMat2x4
		sizeof(double), // dsMaterialType_DMat3x2
		sizeof(double), // dsMaterialType_DMat3x4
		sizeof(double), // dsMaterialType_DMat4x2
		sizeof(double), // dsMaterialType_DMat4x3

		// Other types
		sizeof(void*), // dsMaterialType_Texture
		sizeof(void*), // dsMaterialType_Image
		sizeof(void*), // dsMaterialType_SubpassInput
		sizeof(void*), // dsMaterialType_TextureBuffer
		sizeof(void*), // dsMaterialType_MutableTextureBuffer
		sizeof(void*), // dsMaterialType_VariableGroup
		sizeof(void*), // dsMaterialType_UniformBlock
		sizeof(void*), // dsMaterialType_UniformBuffer
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(alignment) == dsMaterialType_Count, array_enum_mismatch);

	if ((unsigned int)type >= dsMaterialType_Count)
		return 0;

	return alignment[type];
}

size_t dsMaterialType_addElementCpuSize(size_t* curSize, dsMaterialType type, uint32_t count)
{
	if (!curSize)
		return DS_INVALID_MATERIAL_OFFSET;

	uint32_t alignment = dsMaterialType_cpuAlignment(type);
	if (alignment == 0)
		return DS_INVALID_MATERIAL_OFFSET;

	size_t offset = ((*curSize + alignment - 1)/alignment)*alignment;
	if (count == 0)
		count = 1;
	*curSize = offset + dsMaterialType_cpuSize(type)*count;
	return offset;
}

uint16_t dsMaterialType_blockSize(dsMaterialType type, bool isArray)
{
	uint16_t alignment = dsMaterialType_blockAlignment(type, isArray);
	if ((unsigned int)type >= dsMaterialType_Mat2)
		alignment = (uint16_t)(alignment*dsMaterialType_matrixColumns(type));
	return alignment;
}

uint16_t dsMaterialType_blockAlignment(dsMaterialType type, bool isArray)
{
	if ((unsigned int)type >= dsMaterialType_Texture)
		return 0;
	else if ((unsigned int)type >= dsMaterialType_Mat2)
	{
		// Treat matrices as an array of column vectors.
		type = dsMaterialType_matrixColumnType(type);
		isArray = true;
		DS_ASSERT((unsigned int)type < dsMaterialType_Mat2);
	}

	uint16_t typeAlignment = bufferAlignment[type];
	if (isArray)
	{
		const size_t vec4Size = sizeof(float)*4;
		typeAlignment = (uint16_t)(((typeAlignment + vec4Size - 1)/vec4Size)*vec4Size);
	}

	return typeAlignment;
}

size_t dsMaterialType_addElementBlockSize(size_t* curSize, dsMaterialType type, uint32_t count)
{
	if (!curSize)
		return DS_INVALID_MATERIAL_OFFSET;

	uint32_t alignment = dsMaterialType_blockAlignment(type, count > 0);
	if (alignment == 0)
		return DS_INVALID_MATERIAL_OFFSET;

	size_t offset = ((*curSize + alignment - 1)/alignment)*alignment;
	if (count == 0)
		count = 1;
	*curSize = offset + dsMaterialType_blockSize(type, count > 1)*count;
	return offset;
}

uint16_t dsMaterialType_bufferSize(dsMaterialType type)
{
	uint16_t alignment = dsMaterialType_bufferAlignment(type);
	if ((unsigned int)type >= dsMaterialType_Mat2)
		alignment = (uint16_t)(alignment*dsMaterialType_matrixColumns(type));
	return alignment;
}

uint16_t dsMaterialType_bufferAlignment(dsMaterialType type)
{
	if ((unsigned int)type >= dsMaterialType_Texture)
		return 0;
	else if ((unsigned int)type >= dsMaterialType_Mat2)
	{
		// Treat matrices as an array of column vectors.
		type = dsMaterialType_matrixColumnType(type);
		DS_ASSERT((unsigned int)type < dsMaterialType_Mat2);
	}

	return bufferAlignment[type];
}

size_t dsMaterialType_addElementBufferSize(size_t* curSize, dsMaterialType type, uint32_t count)
{
	if (!curSize)
		return DS_INVALID_MATERIAL_OFFSET;

	uint32_t alignment = dsMaterialType_bufferAlignment(type);
	if (alignment == 0)
		return DS_INVALID_MATERIAL_OFFSET;

	size_t offset = ((*curSize + alignment - 1)/alignment)*alignment;
	if (count == 0)
		count = 1;
	*curSize = offset + dsMaterialType_bufferSize(type)*count;
	return offset;
}
