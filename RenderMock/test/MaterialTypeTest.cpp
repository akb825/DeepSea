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
#include <DeepSea/Render/Types.h>
#include <gtest/gtest.h>

struct StructCpu
{
	float vec3Mem[3];
	float vec2Mem[2];
	float floatMem;
	int intMem;
	unsigned int uintMem;
	double doubleMem;
	float matrix3x4Mem[3][4];
	double doubleMatrix2x3Mem[3][2];
	float floatArrayMem[5];
};

struct StructUniformBlock
{
	float vec3Mem[3];
	float padding1;
	float vec2Mem[2];
	float floatMem;
	int intMem;
	unsigned int uintMem;
	float padding2;
	double doubleMem;
	float matrix3x4Mem[3][4];
	double doubleMatrix2x3Mem[2][4];
	float floatArrayMem[5][4];
};

struct StructStorageBuffer
{
	float vec3Mem[3];
	float padding1;
	float vec2Mem[2];
	float floatMem;
	int intMem;
	unsigned int uintMem;
	float padding2;
	double doubleMem;
	float matrix3x4Mem[3][4];
	double doubleMatrix2x3Mem[2][4];
	float floatArrayMem[5];
};

TEST(MaterialTypeTest, CpuOffsets)
{
	size_t curSize = 0;
	EXPECT_EQ(offsetof(StructCpu, vec3Mem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_Vec3, 0));
	EXPECT_EQ(offsetof(StructCpu, vec2Mem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_Vec2, 0));
	EXPECT_EQ(offsetof(StructCpu, floatMem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_Float, 0));
	EXPECT_EQ(offsetof(StructCpu, intMem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_Int, 0));
	EXPECT_EQ(offsetof(StructCpu, uintMem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_UInt, 0));
	EXPECT_EQ(offsetof(StructCpu, doubleMem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_Double, 0));
	EXPECT_EQ(offsetof(StructCpu, matrix3x4Mem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_Mat3x4, 0));
	EXPECT_EQ(offsetof(StructCpu, doubleMatrix2x3Mem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_DMat2x3, 0));
	EXPECT_EQ(offsetof(StructCpu, floatArrayMem), dsMaterialType_addElementCpuSize(&curSize,
		dsMaterialType_Float, 5));
	// Extra padding at the end
	EXPECT_EQ(sizeof(StructCpu), curSize + sizeof(float));
}

TEST(MaterialTypeTest, UniformBlockOffsets)
{
	size_t curSize = 0;
	EXPECT_EQ(offsetof(StructUniformBlock, vec3Mem), dsMaterialType_addElementBlockSize(&curSize,
		dsMaterialType_Vec3, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, vec2Mem), dsMaterialType_addElementBlockSize(&curSize,
		dsMaterialType_Vec2, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, floatMem), dsMaterialType_addElementBlockSize(&curSize,
		dsMaterialType_Float, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, intMem), dsMaterialType_addElementBlockSize(&curSize,
		dsMaterialType_Int, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, uintMem), dsMaterialType_addElementBlockSize(&curSize,
		dsMaterialType_UInt, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, doubleMem), dsMaterialType_addElementBlockSize(&curSize,
		dsMaterialType_Double, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, matrix3x4Mem), dsMaterialType_addElementBlockSize(
		&curSize, dsMaterialType_Mat3x4, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, doubleMatrix2x3Mem), dsMaterialType_addElementBlockSize(
		&curSize, dsMaterialType_DMat2x3, 0));
	EXPECT_EQ(offsetof(StructUniformBlock, floatArrayMem), dsMaterialType_addElementBlockSize(
		&curSize, dsMaterialType_Float, 5));
	EXPECT_EQ(sizeof(StructUniformBlock), curSize);
}

TEST(MaterialTypeTest, StorageBufferOffsets)
{
	size_t curSize = 0;
	EXPECT_EQ(offsetof(StructStorageBuffer, vec3Mem), dsMaterialType_addElementBufferSize(&curSize,
		dsMaterialType_Vec3, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, vec2Mem), dsMaterialType_addElementBufferSize(&curSize,
		dsMaterialType_Vec2, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, floatMem), dsMaterialType_addElementBufferSize(&curSize,
		dsMaterialType_Float, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, intMem), dsMaterialType_addElementBufferSize(&curSize,
		dsMaterialType_Int, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, uintMem), dsMaterialType_addElementBufferSize(&curSize,
		dsMaterialType_UInt, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, doubleMem), dsMaterialType_addElementBufferSize(
		&curSize, dsMaterialType_Double, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, matrix3x4Mem), dsMaterialType_addElementBufferSize(
		&curSize, dsMaterialType_Mat3x4, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, doubleMatrix2x3Mem),
		dsMaterialType_addElementBufferSize(&curSize, dsMaterialType_DMat2x3, 0));
	EXPECT_EQ(offsetof(StructStorageBuffer, floatArrayMem), dsMaterialType_addElementBufferSize(
		&curSize, dsMaterialType_Float, 5));
	// Extra padding at the end
	EXPECT_EQ(sizeof(StructStorageBuffer), curSize + sizeof(float));
}
