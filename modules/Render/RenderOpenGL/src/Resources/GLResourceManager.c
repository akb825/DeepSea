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

#include "Resources/GLResourceManager.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Platform/GLPlatform.h"
#include "Resources/GLDrawGeometry.h"
#include "Resources/GLFramebuffer.h"
#include "Resources/GLGfxBuffer.h"
#include "Resources/GLGfxFence.h"
#include "Resources/GLGfxQueryPool.h"
#include "Resources/GLMaterialDesc.h"
#include "Resources/GLRenderbuffer.h"
#include "Resources/GLShader.h"
#include "Resources/GLShaderModule.h"
#include "Resources/GLShaderVariableGroupDesc.h"
#include "Resources/GLTexture.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <stdint.h>
#include <string.h>

enum FormatBit
{
	FormatBit_Vertex = 0x1,
	FormatBit_Texture = 0x2,
	FormatBit_Offscreen = 0x4,
	FormatBit_TextureBuffer = 0x8
};

static size_t dsGLResourceManager_fullAllocSize(const dsRendererOptions* options)
{
	return DS_ALIGNED_SIZE(sizeof(dsGLResourceManager)) +
		DS_ALIGNED_SIZE(options->maxResourceThreads*sizeof(dsResourceContext)) +
		dsMutex_fullAllocSize();
}

static void glGetSizeT(GLenum pname, size_t* value)
{
	if (ANYGL_SUPPORTED(glGetInteger64v))
	{
		GLint64 temp = 0;
		glGetInteger64v(pname, &temp);
		*value = (size_t)temp;
	}
	else
	{
		GLint temp = 0;
		glGetIntegerv(pname, &temp);
		*value = temp;
	}
}

static dsGfxBufferUsage getSupportedBuffers(uint32_t shaderVersion)
{
	dsGfxBufferUsage supportedBuffers = (dsGfxBufferUsage)(dsGfxBufferUsage_Vertex |
		dsGfxBufferUsage_Index | dsGfxBufferUsage_CopyTo | dsGfxBufferUsage_CopyFrom);
	if (AnyGL_atLeastVersion(4, 0, false) || AnyGL_atLeastVersion(3, 1, true) ||
		AnyGL_ARB_draw_indirect)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_IndirectDraw);
	}

	if (AnyGL_atLeastVersion(4, 3, false) || AnyGL_atLeastVersion(3, 1, true) ||
		AnyGL_ARB_compute_shader)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_IndirectDispatch);
	}

	if (AnyGL_atLeastVersion(3, 1, false) || AnyGL_atLeastVersion(3, 2, true) ||
		AnyGL_ARB_texture_buffer_object || AnyGL_EXT_texture_buffer_object)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_Image);
	}

	if (AnyGL_atLeastVersion(4, 3, false) || AnyGL_atLeastVersion(3, 2, true))
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_MutableImage);

	// Use shader version to determine if uniform blocks are enabled. MSL requires named uniform
	// blocks, and it's possible that the extension is supported but the shaders loaded wouldn't
	// use uniform blocks.
	if ((ANYGL_GLES && shaderVersion >= DS_ENCODE_VERSION(3, 0, 0)) ||
		(!ANYGL_GLES && shaderVersion >= DS_ENCODE_VERSION(1, 5, 0)))
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_UniformBlock);
	}

	if (AnyGL_atLeastVersion(4, 3, false) || AnyGL_atLeastVersion(3, 1, true) ||
		AnyGL_ARB_shader_storage_buffer_object)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_UniformBuffer);
	}

	return supportedBuffers;
}

static dsGfxBufferMapSupport getBufferMapSupport(void)
{
	if (!ANYGL_SUPPORTED(glMapBuffer))
		return dsGfxBufferMapSupport_None;
	else if (!ANYGL_SUPPORTED(glMapBufferRange))
		return dsGfxBufferMapSupport_Full;
	else if (AnyGL_atLeastVersion(4, 4, true) || AnyGL_ARB_buffer_storage)
		return dsGfxBufferMapSupport_Persistent;

	return dsGfxBufferMapSupport_Range;
}

static void setStandardVertexFormat(dsGLResourceManager* resourceManager, dsGfxFormat format,
	dsGfxFormat decorator, GLenum glFormat, GLint elements)
{
	unsigned int index = dsGfxFormat_standardIndex(format);
	unsigned int decoratorIndex = dsGfxFormat_decoratorIndex(decorator);
	resourceManager->standardFormats[index][decoratorIndex] |= (uint8_t)FormatBit_Vertex;
	resourceManager->standardVertexFormats[index][decoratorIndex] = glFormat;
	resourceManager->standardVertexElements[index][decoratorIndex] = elements;
}

static void setSpecialVertexFormat(dsGLResourceManager* resourceManager, dsGfxFormat format,
	GLenum glFormat, GLint elements)
{
	unsigned int index = dsGfxFormat_specialIndex(format);
	resourceManager->specialFormats[index] |= (uint8_t)FormatBit_Vertex;
	resourceManager->specialVertexFormats[index] = glFormat;
	resourceManager->specialVertexElements[index] = elements;
}

static void setStandardFormat(dsGLResourceManager* resourceManager, dsGfxFormat format,
	dsGfxFormat decorator, int bits, GLenum internalFormat, GLenum glFormat, GLenum type)
{
	unsigned int index = dsGfxFormat_standardIndex(format);
	unsigned int decoratorIndex = dsGfxFormat_decoratorIndex(decorator);
	resourceManager->standardFormats[index][decoratorIndex] |= (uint8_t)bits;
	resourceManager->standardInternalFormats[index][decoratorIndex] = internalFormat;
	resourceManager->standardGlFormats[index][decoratorIndex] = glFormat;
	resourceManager->standardTypes[index][decoratorIndex] = type;
}

static void setSpecialFormat(dsGLResourceManager* resourceManager, dsGfxFormat format,
	int bits, GLenum internalFormat, GLenum glFormat, GLenum type)
{
	unsigned int index = dsGfxFormat_specialIndex(format);
	resourceManager->specialFormats[index] |= (uint8_t)bits;
	resourceManager->specialInternalFormats[index] = internalFormat;
	resourceManager->specialGlFormats[index] = glFormat;
	resourceManager->specialTypes[index] = type;
}

static void setCompressedFormat(dsGLResourceManager* resourceManager, dsGfxFormat format,
	dsGfxFormat decorator, int bits, GLenum internalFormat, GLenum glFormat)
{
	unsigned int index = dsGfxFormat_compressedIndex(format);
	unsigned int decoratorIndex = dsGfxFormat_decoratorIndex(decorator);
	resourceManager->compressedFormats[index][decoratorIndex] |= (uint8_t)bits;
	resourceManager->compressedInternalFormats[index][decoratorIndex] = internalFormat;
	resourceManager->compressedGlFormats[index][decoratorIndex] = glFormat;
}

static void cacheVertexFormats(dsGLResourceManager* resourceManager)
{
	// Standard vertex formats
	// UNorm
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8, dsGfxFormat_UNorm, GL_UNSIGNED_BYTE,
		1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8, dsGfxFormat_UNorm, GL_UNSIGNED_BYTE,
		2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8, dsGfxFormat_UNorm,
		GL_UNSIGNED_BYTE, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8W8, dsGfxFormat_UNorm,
		GL_UNSIGNED_BYTE, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X16, dsGfxFormat_UNorm, GL_UNSIGNED_SHORT,
		1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16, dsGfxFormat_UNorm,
		GL_UNSIGNED_SHORT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16, dsGfxFormat_UNorm,
		GL_UNSIGNED_SHORT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16W16, dsGfxFormat_UNorm,
		GL_UNSIGNED_SHORT, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X32, dsGfxFormat_UNorm, GL_UNSIGNED_INT,
		1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32, dsGfxFormat_UNorm,
		GL_UNSIGNED_INT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32, dsGfxFormat_UNorm,
		GL_UNSIGNED_INT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32W32, dsGfxFormat_UNorm,
		GL_UNSIGNED_INT, 4);

	bool packedInt = AnyGL_atLeastVersion(3, 0, false) || AnyGL_atLeastVersion(3, 0, false);
	bool d3dPackedInt = AnyGL_atLeastVersion(3, 2, false) || AnyGL_ARB_vertex_array_bgra;
	if (packedInt)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_UNorm,
			GL_UNSIGNED_INT_2_10_10_10_REV, 4);
	}
	if (d3dPackedInt)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_UNorm,
			GL_UNSIGNED_INT_2_10_10_10_REV, GL_RGBA);
	}

	// SNorm
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8, dsGfxFormat_SNorm, GL_BYTE, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8, dsGfxFormat_SNorm, GL_BYTE, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8, dsGfxFormat_SNorm, GL_BYTE, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8W8, dsGfxFormat_SNorm, GL_BYTE, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X16, dsGfxFormat_SNorm, GL_SHORT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16, dsGfxFormat_SNorm, GL_SHORT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16, dsGfxFormat_SNorm, GL_SHORT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SNorm, GL_SHORT,
		4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X32, dsGfxFormat_SNorm, GL_INT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32, dsGfxFormat_SNorm, GL_INT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32, dsGfxFormat_SNorm, GL_INT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32W32, dsGfxFormat_SNorm, GL_INT,
		4);

	if (packedInt || AnyGL_OES_vertex_type_10_10_10_2)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_SNorm,
			GL_INT_2_10_10_10_REV, 4);
	}
	if (d3dPackedInt)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_SNorm,
			GL_UNSIGNED_INT_2_10_10_10_REV, GL_RGBA);
	}

	// UScaled
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8, dsGfxFormat_UScaled, GL_UNSIGNED_BYTE,
		1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8, dsGfxFormat_UScaled,
		GL_UNSIGNED_BYTE, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8, dsGfxFormat_UScaled,
		GL_UNSIGNED_BYTE, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8W8, dsGfxFormat_UScaled,
		GL_UNSIGNED_BYTE, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X16, dsGfxFormat_UScaled,
		GL_UNSIGNED_SHORT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16, dsGfxFormat_UScaled,
		GL_UNSIGNED_SHORT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16, dsGfxFormat_UScaled,
		GL_UNSIGNED_SHORT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16W16, dsGfxFormat_UScaled,
		GL_UNSIGNED_SHORT, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X32, dsGfxFormat_UScaled, GL_UNSIGNED_INT,
		1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32, dsGfxFormat_UScaled,
		GL_UNSIGNED_INT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32, dsGfxFormat_UScaled,
		GL_UNSIGNED_INT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32W32, dsGfxFormat_UScaled,
		GL_UNSIGNED_INT, 4);

	if (packedInt)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_UScaled,
			GL_UNSIGNED_INT_2_10_10_10_REV, 4);
	}

	// SScaled
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8, dsGfxFormat_SScaled, GL_BYTE, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8, dsGfxFormat_SScaled, GL_BYTE, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8, dsGfxFormat_SScaled, GL_BYTE, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8W8, dsGfxFormat_SScaled, GL_BYTE, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X16, dsGfxFormat_SScaled, GL_SHORT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16, dsGfxFormat_SScaled, GL_SHORT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16, dsGfxFormat_SScaled, GL_SHORT,
		3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SScaled,
		GL_SHORT, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X32, dsGfxFormat_SScaled, GL_INT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32, dsGfxFormat_SScaled, GL_INT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32, dsGfxFormat_SScaled, GL_INT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32W32, dsGfxFormat_SScaled, GL_INT,
		4);

	if (packedInt)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_SScaled,
			GL_INT_2_10_10_10_REV, 4);
	}

	// UInt
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8, dsGfxFormat_UInt, GL_UNSIGNED_BYTE,
		1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8, dsGfxFormat_UInt,
		GL_UNSIGNED_BYTE, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8, dsGfxFormat_UInt,
		GL_UNSIGNED_BYTE, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8W8, dsGfxFormat_UInt,
		GL_UNSIGNED_BYTE, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X16, dsGfxFormat_UInt,
		GL_UNSIGNED_SHORT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16, dsGfxFormat_UInt,
		GL_UNSIGNED_SHORT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16, dsGfxFormat_UInt,
		GL_UNSIGNED_SHORT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16W16, dsGfxFormat_UInt,
		GL_UNSIGNED_SHORT, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X32, dsGfxFormat_UInt, GL_UNSIGNED_INT,
		1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32, dsGfxFormat_UInt,
		GL_UNSIGNED_INT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32, dsGfxFormat_UInt,
		GL_UNSIGNED_INT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32W32, dsGfxFormat_UInt,
		GL_UNSIGNED_INT, 4);

	if (packedInt)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_UInt,
			GL_UNSIGNED_INT_2_10_10_10_REV, 4);
	}

	// SInt
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8, dsGfxFormat_SInt, GL_BYTE, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8, dsGfxFormat_SInt, GL_BYTE, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8, dsGfxFormat_SInt, GL_BYTE, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X8Y8Z8W8, dsGfxFormat_SInt, GL_BYTE, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X16, dsGfxFormat_SInt, GL_SHORT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16, dsGfxFormat_SInt, GL_SHORT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16, dsGfxFormat_SInt, GL_SHORT,
		3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SInt,
		GL_SHORT, 4);

	setStandardVertexFormat(resourceManager, dsGfxFormat_X32, dsGfxFormat_SInt, GL_INT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32, dsGfxFormat_SInt, GL_INT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32, dsGfxFormat_SInt, GL_INT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32W32, dsGfxFormat_SInt, GL_INT,
		4);

	if (packedInt)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_W2Z10Y10X10, dsGfxFormat_SInt,
			GL_INT_2_10_10_10_REV, 4);
	}

	// Float
	if (AnyGL_atLeastVersion(3, 0, false) || AnyGL_atLeastVersion(3, 0, true) ||
		AnyGL_OES_vertex_half_float)
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_X16, dsGfxFormat_Float, AnyGL_HALF_FLOAT,
			1);
		setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16, dsGfxFormat_Float,
			AnyGL_HALF_FLOAT, 2);
		setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16, dsGfxFormat_Float,
			AnyGL_HALF_FLOAT, 3);
		setStandardVertexFormat(resourceManager, dsGfxFormat_X16Y16Z16W16, dsGfxFormat_Float,
			AnyGL_HALF_FLOAT, 4);
	}

	setStandardVertexFormat(resourceManager, dsGfxFormat_X32, dsGfxFormat_Float, GL_FLOAT, 1);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32, dsGfxFormat_Float, GL_FLOAT, 2);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32, dsGfxFormat_Float, GL_FLOAT, 3);
	setStandardVertexFormat(resourceManager, dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float, GL_FLOAT,
		4);

	if (ANYGL_SUPPORTED(glVertexAttribLPointer))
	{
		setStandardVertexFormat(resourceManager, dsGfxFormat_X64, dsGfxFormat_Float, GL_DOUBLE, 1);
		setStandardVertexFormat(resourceManager, dsGfxFormat_X64Y64, dsGfxFormat_Float, GL_DOUBLE,
			2);
		setStandardVertexFormat(resourceManager, dsGfxFormat_X64Y64Z64, dsGfxFormat_Float,
			GL_DOUBLE, 3);
		setStandardVertexFormat(resourceManager, dsGfxFormat_X64Y64Z64W64, dsGfxFormat_Float,
			GL_DOUBLE, 4);
	}

	// Special formats
	if (AnyGL_atLeastVersion(4, 4, false) || AnyGL_ARB_vertex_type_10f_11f_11f_rev)
	{
		setSpecialVertexFormat(resourceManager, dsGfxFormat_Z10Y11X11_UFloat,
			GL_UNSIGNED_INT_10F_11F_11F_REV, 3);
	}
}

static void cacheTextureFormats(dsGLResourceManager* resourceManager)
{
	unsigned int intOffscreen = FormatBit_Offscreen;
	unsigned int float16Offscreen = FormatBit_Offscreen;
	unsigned int floatOffscreen = FormatBit_Offscreen;
	if (ANYGL_GLES)
	{
		intOffscreen = 0;
		if (!AnyGL_EXT_color_buffer_float)
			floatOffscreen = 0;
		if (!AnyGL_EXT_color_buffer_half_float && !AnyGL_EXT_color_buffer_float)
			float16Offscreen = 0;
	}
	if (AnyGL_atLeastVersion(3, 0, false) || AnyGL_atLeastVersion(3, 0, true))
	{
		// Standard formats
		// UNorm
		setStandardFormat(resourceManager, dsGfxFormat_R4G4B4A4, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
		setStandardFormat(resourceManager, dsGfxFormat_B4G4R4A4, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGBA4, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4);
		setStandardFormat(resourceManager, dsGfxFormat_R5G6B5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
		setStandardFormat(resourceManager, dsGfxFormat_B5G6R5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV);
		setStandardFormat(resourceManager, dsGfxFormat_R5G5B5A1, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
		setStandardFormat(resourceManager, dsGfxFormat_B5G5R5A1, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_5_5_5_1);
		setStandardFormat(resourceManager, dsGfxFormat_A1R5G5B5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV);
		setStandardFormat(resourceManager, dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen, GL_RGB10_A2, GL_RGBA,
			GL_UNSIGNED_INT_2_10_10_10_REV);
		setStandardFormat(resourceManager, dsGfxFormat_A2R10G10B10, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen, GL_RGB10_A2, GL_BGRA,
			GL_UNSIGNED_INT_2_10_10_10_REV);

		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RGBA8, GL_RGBA,
			GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RGBA8, GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8);
		setStandardFormat(resourceManager, dsGfxFormat_A8B8G8R8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RGBA8, GL_RGBA,
			GL_UNSIGNED_INT_8_8_8_8_REV);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RG8, GL_RG,
			GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_R8, GL_RED,
			GL_UNSIGNED_BYTE);

		if (!ANYGL_GLES)
		{
			setStandardFormat(resourceManager, dsGfxFormat_R16G16B16A16, dsGfxFormat_UNorm,
				FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RGBA16,
				GL_RGBA, GL_UNSIGNED_SHORT);
			setStandardFormat(resourceManager, dsGfxFormat_R16G16B16, dsGfxFormat_UNorm,
				FormatBit_Texture, GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT);
			setStandardFormat(resourceManager, dsGfxFormat_R16G16, dsGfxFormat_UNorm,
				FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RG16, GL_RG,
				GL_UNSIGNED_SHORT);
			setStandardFormat(resourceManager, dsGfxFormat_R16, dsGfxFormat_UNorm,
				FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_R16, GL_RED,
				GL_UNSIGNED_SHORT);
		}

		// SNorm
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8A8, dsGfxFormat_SNorm,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA8_SNORM, GL_RGBA, GL_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_B8G8R8A8, dsGfxFormat_SNorm,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA8_SNORM, GL_BGRA,
				GL_UNSIGNED_INT_8_8_8_8);
		setStandardFormat(resourceManager, dsGfxFormat_A8B8G8R8, dsGfxFormat_SNorm,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA8_SNORM, GL_RGBA,
				GL_UNSIGNED_INT_8_8_8_8_REV);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8, dsGfxFormat_SNorm,
			FormatBit_Texture, GL_RGB8_SNORM, GL_RGB, GL_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8, dsGfxFormat_SNorm,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RG8_SNORM, GL_RG, GL_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8, dsGfxFormat_SNorm,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_R8_SNORM, GL_RED, GL_BYTE);

		if (!ANYGL_GLES)
		{
			setStandardFormat(resourceManager, dsGfxFormat_R16G16B16A16, dsGfxFormat_SNorm,
				FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA16_SNORM, GL_RGBA, GL_SHORT);
			setStandardFormat(resourceManager, dsGfxFormat_R16G16B16, dsGfxFormat_SNorm,
				FormatBit_Texture, GL_RGB16_SNORM, GL_RGB, GL_SHORT);
			setStandardFormat(resourceManager, dsGfxFormat_R16G16, dsGfxFormat_SNorm,
				FormatBit_Texture | FormatBit_TextureBuffer, GL_RG16_SNORM, GL_RG, GL_SHORT);
			setStandardFormat(resourceManager, dsGfxFormat_R16, dsGfxFormat_SNorm,
				FormatBit_Texture | FormatBit_TextureBuffer, GL_R16_SNORM, GL_RED, GL_SHORT);
		}

		// UInt
		setStandardFormat(resourceManager, dsGfxFormat_A2B10G10R10, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen, GL_RGB10_A2UI, GL_RGBA_INTEGER,
			GL_UNSIGNED_INT_2_10_10_10_REV);
		setStandardFormat(resourceManager, dsGfxFormat_A2R10G10B10, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen, GL_RGB10_A2UI, GL_BGRA_INTEGER,
			GL_UNSIGNED_INT_2_10_10_10_REV);

		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8A8, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RGBA8UI,
			GL_RGBA_INTEGER, GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_B8G8R8A8, dsGfxFormat_UInt,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA8UI, GL_BGRA_INTEGER,
				GL_UNSIGNED_INT_8_8_8_8);
		setStandardFormat(resourceManager, dsGfxFormat_A8B8G8R8, dsGfxFormat_UInt,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA8UI, GL_RGBA_INTEGER,
				GL_UNSIGNED_INT_8_8_8_8_REV);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8, dsGfxFormat_UInt,
			FormatBit_Texture, GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RG8UI,
			GL_RG_INTEGER, GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_R8UI,
			GL_RED_INTEGER, GL_UNSIGNED_BYTE);

		setStandardFormat(resourceManager, dsGfxFormat_R16G16B16A16, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RGBA16UI,
			GL_RGBA_INTEGER, GL_UNSIGNED_SHORT);
		setStandardFormat(resourceManager, dsGfxFormat_R16G16B16, dsGfxFormat_UInt,
			FormatBit_Texture, GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT);
		setStandardFormat(resourceManager, dsGfxFormat_R16G16, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RG16UI,
			GL_RG_INTEGER, GL_UNSIGNED_SHORT);
		setStandardFormat(resourceManager, dsGfxFormat_R16, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_R16UI,
			GL_RED_INTEGER, GL_UNSIGNED_SHORT);

		setStandardFormat(resourceManager, dsGfxFormat_R32G32B32A32, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RGBA32UI,
			GL_RGBA_INTEGER, GL_UNSIGNED_INT);
		setStandardFormat(resourceManager, dsGfxFormat_R32G32B32, dsGfxFormat_UInt,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGB32UI, GL_RGB_INTEGER,
			GL_UNSIGNED_INT);
		setStandardFormat(resourceManager, dsGfxFormat_R32G32, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RG32UI,
			GL_RG_INTEGER, GL_UNSIGNED_INT);
		setStandardFormat(resourceManager, dsGfxFormat_R32, dsGfxFormat_UInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_R32UI,
			GL_RED_INTEGER, GL_UNSIGNED_INT);

		// SInt
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8A8, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RGBA8I,
			GL_RGBA_INTEGER, GL_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_B8G8R8A8, dsGfxFormat_UInt,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA8I, GL_BGRA_INTEGER,
			GL_UNSIGNED_INT_8_8_8_8);
		setStandardFormat(resourceManager, dsGfxFormat_A8B8G8R8, dsGfxFormat_UInt,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGBA8I, GL_RGBA_INTEGER,
			GL_UNSIGNED_INT_8_8_8_8_REV);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8, dsGfxFormat_SInt,
			FormatBit_Texture, GL_RGB8I, GL_RGB_INTEGER, GL_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RG8I,
			GL_RG_INTEGER, GL_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_R8I,
			GL_RED_INTEGER, GL_BYTE);

		setStandardFormat(resourceManager, dsGfxFormat_R16G16B16A16, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RGBA16I,
			GL_RGBA_INTEGER, GL_SHORT);
		setStandardFormat(resourceManager, dsGfxFormat_R16G16B16, dsGfxFormat_SInt,
			FormatBit_Texture, GL_RGB16I, GL_RGB_INTEGER, GL_SHORT);
		setStandardFormat(resourceManager, dsGfxFormat_R16G16, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RG16I,
			GL_RG_INTEGER, GL_SHORT);
		setStandardFormat(resourceManager, dsGfxFormat_R16, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_R16I,
			GL_RED_INTEGER, GL_SHORT);

		setStandardFormat(resourceManager, dsGfxFormat_R32G32B32A32, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RGBA32I,
			GL_RGBA_INTEGER, GL_INT);
		setStandardFormat(resourceManager, dsGfxFormat_R32G32B32, dsGfxFormat_SInt,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGB32I, GL_RGB_INTEGER, GL_INT);
		setStandardFormat(resourceManager, dsGfxFormat_R32G32, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_RG32I,
			GL_RG_INTEGER, GL_INT);
		setStandardFormat(resourceManager, dsGfxFormat_R32, dsGfxFormat_SInt,
			FormatBit_Texture | intOffscreen | FormatBit_TextureBuffer, GL_R32I,
			GL_RED_INTEGER, GL_INT);

		// Float
		setStandardFormat(resourceManager, dsGfxFormat_R16G16B16A16, dsGfxFormat_Float,
			FormatBit_Texture | float16Offscreen | FormatBit_TextureBuffer, GL_RGBA16F, GL_RGBA,
			AnyGL_HALF_FLOAT);
		setStandardFormat(resourceManager, dsGfxFormat_R16G16B16, dsGfxFormat_Float,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGB16F, GL_RGB, AnyGL_HALF_FLOAT);
		setStandardFormat(resourceManager, dsGfxFormat_R16G16, dsGfxFormat_Float,
			FormatBit_Texture | float16Offscreen | FormatBit_TextureBuffer, GL_RG16F, GL_RG,
			AnyGL_HALF_FLOAT);
		setStandardFormat(resourceManager, dsGfxFormat_R16, dsGfxFormat_Float,
			FormatBit_Texture | float16Offscreen | FormatBit_TextureBuffer, GL_R16F, GL_RED,
			AnyGL_HALF_FLOAT);

		setStandardFormat(resourceManager, dsGfxFormat_R32G32B32A32, dsGfxFormat_Float,
			FormatBit_Texture | floatOffscreen | FormatBit_TextureBuffer, GL_RGBA32F, GL_RGBA,
			GL_FLOAT);
		setStandardFormat(resourceManager, dsGfxFormat_R32G32B32, dsGfxFormat_Float,
			FormatBit_Texture | FormatBit_TextureBuffer, GL_RGB32F, GL_RGB, GL_FLOAT);
		setStandardFormat(resourceManager, dsGfxFormat_R32G32, dsGfxFormat_Float,
			FormatBit_Texture | floatOffscreen | FormatBit_TextureBuffer, GL_RG32F, GL_RG,
			GL_FLOAT);
		setStandardFormat(resourceManager, dsGfxFormat_R32, dsGfxFormat_Float,
			FormatBit_Texture | floatOffscreen | FormatBit_TextureBuffer, GL_R32F, GL_RED,
			GL_FLOAT);

		// SRGB
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB,
			FormatBit_Texture | FormatBit_Offscreen, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_B8G8R8A8, dsGfxFormat_SRGB,
			FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_SRGB8_ALPHA8,
			GL_BGRA, GL_UNSIGNED_INT_8_8_8_8);
		setStandardFormat(resourceManager, dsGfxFormat_A8B8G8R8, dsGfxFormat_SRGB,
			FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_SRGB8_ALPHA8,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE);

		// Special formats
		setSpecialFormat(resourceManager, dsGfxFormat_B10G11R11_UFloat,
			FormatBit_Texture | floatOffscreen, GL_R11F_G11F_B10F, GL_RGB,
			GL_UNSIGNED_INT_10F_11F_11F_REV);
		setSpecialFormat(resourceManager, dsGfxFormat_E5B9G9R9_UFloat, FormatBit_Texture,
			GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV);

		setSpecialFormat(resourceManager, dsGfxFormat_D16, FormatBit_Texture | FormatBit_Offscreen,
			GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT);
		setSpecialFormat(resourceManager, dsGfxFormat_X8D24,
			FormatBit_Texture | FormatBit_Offscreen, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT,
			GL_UNSIGNED_INT);
		setSpecialFormat(resourceManager, dsGfxFormat_S8, FormatBit_Texture | FormatBit_Offscreen,
			GL_STENCIL_INDEX8, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE);
		setSpecialFormat(resourceManager, dsGfxFormat_D24S8,
			FormatBit_Texture | FormatBit_Offscreen, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
			GL_UNSIGNED_INT_24_8);
		setSpecialFormat(resourceManager, dsGfxFormat_D32S8_Float,
			FormatBit_Texture | FormatBit_Offscreen, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL,
			GL_FLOAT_32_UNSIGNED_INT_24_8_REV);
	}
	else
	{
		// UNorm
		setStandardFormat(resourceManager, dsGfxFormat_R4G4B4A4, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
		setStandardFormat(resourceManager, dsGfxFormat_R5G6B5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
		setStandardFormat(resourceManager, dsGfxFormat_R5G5B5A1, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
		if (AnyGL_atLeastVersion(1, 2, false))
		{
			setStandardFormat(resourceManager, dsGfxFormat_B5G6R5, dsGfxFormat_UNorm,
				FormatBit_Texture, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV);
			setStandardFormat(resourceManager, dsGfxFormat_A1R5G5B5, dsGfxFormat_UNorm,
				FormatBit_Texture, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV);
			setStandardFormat(resourceManager, dsGfxFormat_B4G4R4A4, dsGfxFormat_UNorm,
				FormatBit_Texture, GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4);
			setStandardFormat(resourceManager, dsGfxFormat_B5G5R5A1, dsGfxFormat_UNorm,
				FormatBit_Texture, GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_5_5_5_1);
			setStandardFormat(resourceManager, dsGfxFormat_A1R5G5B5, dsGfxFormat_UNorm,
				FormatBit_Texture, GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV);
		}

		if (AnyGL_EXT_texture_type_2_10_10_10_REV)
		{
			setStandardFormat(resourceManager, dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm,
				FormatBit_Texture | FormatBit_Offscreen, GL_RGBA, GL_RGBA,
				GL_UNSIGNED_INT_2_10_10_10_REV);
			if (AnyGL_atLeastVersion(1, 2, false))
			{
				setStandardFormat(resourceManager, dsGfxFormat_A2R10G10B10, dsGfxFormat_UNorm,
					FormatBit_Texture | FormatBit_Offscreen, GL_BGRA, GL_RGBA,
					GL_UNSIGNED_INT_2_10_10_10_REV);
			}
		}

		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
		if (AnyGL_atLeastVersion(1, 2, false))
		{
			setStandardFormat(resourceManager, dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm,
				FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RGBA, GL_BGRA,
				GL_UNSIGNED_INT_8_8_8_8);
			setStandardFormat(resourceManager, dsGfxFormat_A8B8G8R8, dsGfxFormat_UNorm,
				FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_RGBA, GL_RGBA,
				GL_UNSIGNED_INT_8_8_8_8_REV);
		}
		setStandardFormat(resourceManager, dsGfxFormat_R8G8B8, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8G8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA,
			GL_UNSIGNED_BYTE);
		setStandardFormat(resourceManager, dsGfxFormat_R8, dsGfxFormat_UNorm,
			FormatBit_Texture | FormatBit_Offscreen, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE);

		// Float
		if (AnyGL_ARB_texture_float || AnyGL_OES_texture_float)
		{
			setStandardFormat(resourceManager, dsGfxFormat_R32G32B32A32, dsGfxFormat_Float,
				FormatBit_Texture | floatOffscreen, GL_RGBA, GL_RGBA, GL_FLOAT);
			setStandardFormat(resourceManager, dsGfxFormat_R32G32B32, dsGfxFormat_Float,
				FormatBit_Texture, GL_RGB, GL_RGB, GL_FLOAT);
			setStandardFormat(resourceManager, dsGfxFormat_R32G32, dsGfxFormat_Float,
				FormatBit_Texture | floatOffscreen, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA,
				GL_FLOAT);
			setStandardFormat(resourceManager, dsGfxFormat_R32, dsGfxFormat_Float,
				FormatBit_Texture | floatOffscreen, GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT);

			if (AnyGL_ARB_half_float_pixel || AnyGL_OES_texture_half_float)
			{
				setStandardFormat(resourceManager, dsGfxFormat_R16G16B16A16, dsGfxFormat_Float,
					FormatBit_Texture | float16Offscreen, GL_RGBA, GL_RGBA, AnyGL_HALF_FLOAT);
				setStandardFormat(resourceManager, dsGfxFormat_R16G16B16, dsGfxFormat_Float,
					FormatBit_Texture, GL_RGB, GL_RGB, AnyGL_HALF_FLOAT);
				setStandardFormat(resourceManager, dsGfxFormat_R16G16, dsGfxFormat_Float,
					FormatBit_Texture | float16Offscreen, GL_LUMINANCE_ALPHA,
					GL_LUMINANCE_ALPHA, AnyGL_HALF_FLOAT);
				setStandardFormat(resourceManager, dsGfxFormat_R16, dsGfxFormat_Float,
					FormatBit_Texture | float16Offscreen, GL_LUMINANCE, GL_LUMINANCE,
					AnyGL_HALF_FLOAT);
			}
		}

		// SRGB
		if (AnyGL_atLeastVersion(2, 1, false) || AnyGL_EXT_texture_sRGB || AnyGL_EXT_sRGB)
		{
			setStandardFormat(resourceManager, dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB,
				FormatBit_Texture | FormatBit_Offscreen, GL_SRGB8_ALPHA8, GL_RGBA,
				GL_UNSIGNED_BYTE);
			if (AnyGL_atLeastVersion(1, 2, false))
			{
				setStandardFormat(resourceManager, dsGfxFormat_B8G8R8A8, dsGfxFormat_SRGB,
					FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer, GL_SRGB8_ALPHA8,
					GL_BGRA, GL_UNSIGNED_INT_8_8_8_8);
				setStandardFormat(resourceManager, dsGfxFormat_A8B8G8R8, dsGfxFormat_SRGB,
					FormatBit_Texture | FormatBit_Offscreen | FormatBit_TextureBuffer,
					GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV);
			}
			setStandardFormat(resourceManager, dsGfxFormat_R8G8B8, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE);
		}

		// Special formats
		if (AnyGL_EXT_packed_float)
		{
			setSpecialFormat(resourceManager, dsGfxFormat_B10G11R11_UFloat,
				FormatBit_Texture | floatOffscreen, GL_RGB, GL_RGB,
				GL_UNSIGNED_INT_10F_11F_11F_REV);
			setSpecialFormat(resourceManager, dsGfxFormat_E5B9G9R9_UFloat, FormatBit_Texture,
				GL_RGB, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV);
		}

		if (AnyGL_atLeastVersion(2, 0, false) || AnyGL_OES_depth_texture)
		{
			setSpecialFormat(resourceManager, dsGfxFormat_D16,
				FormatBit_Texture | FormatBit_Offscreen, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT,
				GL_UNSIGNED_SHORT);
			setSpecialFormat(resourceManager, dsGfxFormat_X8D24,
				FormatBit_Texture | FormatBit_Offscreen, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT,
				GL_UNSIGNED_INT);
		}

		if (AnyGL_atLeastVersion(2, 0, false) || AnyGL_OES_texture_stencil8)
		{
			setSpecialFormat(resourceManager, dsGfxFormat_S8,
				FormatBit_Texture | FormatBit_Offscreen, GL_STENCIL_INDEX, GL_STENCIL_INDEX,
				GL_UNSIGNED_BYTE);
		}

		if (AnyGL_EXT_packed_depth_stencil || AnyGL_OES_packed_depth_stencil)
		{
			setSpecialFormat(resourceManager, dsGfxFormat_D24S8,
				FormatBit_Texture | FormatBit_Offscreen, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL,
				GL_UNSIGNED_INT_24_8);
		}
	}

	// Compressed formats
	if (AnyGL_EXT_texture_compression_s3tc || AnyGL_EXT_texture_compression_dxt1)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_BC1_RGB, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_RGB);
		setCompressedFormat(resourceManager, dsGfxFormat_BC1_RGBA, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_RGBA);
		if (AnyGL_EXT_texture_sRGB)
		{
			setCompressedFormat(resourceManager, dsGfxFormat_BC1_RGB, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, GL_RGB);
			setCompressedFormat(resourceManager, dsGfxFormat_BC1_RGBA, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_RGBA);
		}
	}

	if (AnyGL_EXT_texture_compression_s3tc)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_BC2, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_BC3, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_RGBA);
		if (AnyGL_EXT_texture_sRGB)
		{
			setCompressedFormat(resourceManager, dsGfxFormat_BC2, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_RGB);
			setCompressedFormat(resourceManager, dsGfxFormat_BC3, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, GL_RGB);
		}
	}

	if (AnyGL_atLeastVersion(3, 0, false) || AnyGL_EXT_texture_compression_rgtc)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_BC4, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RED_RGTC1, GL_RED);
		setCompressedFormat(resourceManager, dsGfxFormat_BC4, dsGfxFormat_SNorm,
			FormatBit_Texture, GL_COMPRESSED_SIGNED_RED_RGTC1, GL_RED);
		setCompressedFormat(resourceManager, dsGfxFormat_BC5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RG_RGTC2, GL_RG);
		setCompressedFormat(resourceManager, dsGfxFormat_BC5, dsGfxFormat_SNorm,
			FormatBit_Texture, GL_COMPRESSED_SIGNED_RG_RGTC2, GL_RED);
	}

	if (AnyGL_atLeastVersion(4, 2, false) || AnyGL_ARB_texture_compression_bptc)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_BC6H, dsGfxFormat_Float,
			FormatBit_Texture, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_RGB);
		setCompressedFormat(resourceManager, dsGfxFormat_BC6H, dsGfxFormat_UFloat,
			FormatBit_Texture, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, GL_RGB);
		setCompressedFormat(resourceManager, dsGfxFormat_BC7, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_BPTC_UNORM, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_BC7, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, GL_RGBA);
	}

	if (AnyGL_OES_compressed_ETC1_RGB8_texture)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_ETC1, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_ETC1_RGB8_OES, GL_RGB);
	}

	if (AnyGL_atLeastVersion(3, 0, true) || AnyGL_ARB_ES3_compatibility)
	{
		if (!AnyGL_OES_compressed_ETC1_RGB8_texture)
		{
			setCompressedFormat(resourceManager, dsGfxFormat_ETC2_R8G8B8, dsGfxFormat_UNorm,
				FormatBit_Texture, GL_COMPRESSED_RGB8_ETC2, GL_RGB);
		}

		setCompressedFormat(resourceManager, dsGfxFormat_ETC2_R8G8B8, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGB8_ETC2, GL_RGB);
		setCompressedFormat(resourceManager, dsGfxFormat_ETC2_R8G8B8A1, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ETC2_R8G8B8A8, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA8_ETC2_EAC, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_EAC_R11, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_R11_EAC, GL_RED);
		setCompressedFormat(resourceManager, dsGfxFormat_EAC_R11G11, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RG11_EAC, GL_RG);

		setCompressedFormat(resourceManager, dsGfxFormat_ETC2_R8G8B8, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ETC2, GL_RGB);
		setCompressedFormat(resourceManager, dsGfxFormat_ETC2_R8G8B8A1, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ETC2_R8G8B8A8, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, GL_RGBA);

		setCompressedFormat(resourceManager, dsGfxFormat_EAC_R11, dsGfxFormat_SNorm,
			FormatBit_Texture, GL_COMPRESSED_SIGNED_R11_EAC, GL_RED);
		setCompressedFormat(resourceManager, dsGfxFormat_EAC_R11G11, dsGfxFormat_SNorm,
			FormatBit_Texture, GL_COMPRESSED_SIGNED_RG11_EAC, GL_RG);
	}

	if (AnyGL_OES_texture_compression_astc)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_4x4, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_4x4_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_5x4, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_5x4_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_5x5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_5x5_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_6x5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_6x5_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_6x6, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_6x6_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_8x5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_8x5_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_8x6, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_8x6_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_8x8, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_8x8_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x5, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_10x5_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x6, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_10x6_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x8, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_10x8_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x10, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_10x10_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_12x10, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_12x10_KHR, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_12x12, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_ASTC_12x12_KHR, GL_RGBA);

		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_4x4, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_5x4, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_5x5, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_6x5, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_6x6, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_8x5, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_8x6, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_8x8, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x5, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x6, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x8, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_10x10, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_12x10, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, GL_SRGB8_ALPHA8);
		setCompressedFormat(resourceManager, dsGfxFormat_ASTC_12x12, dsGfxFormat_SRGB,
			FormatBit_Texture, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, GL_SRGB8_ALPHA8);
	}

	if (AnyGL_IMG_texture_compression_pvrtc)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGB_2BPP, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, GL_RGB);
		setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGBA_2BPP, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGB_4BPP, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, GL_RGB);
		setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGBA_4BPP, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, GL_RGBA);

		if (AnyGL_EXT_pvrtc_sRGB)
		{
			setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGB_2BPP, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT, GL_RGB);
			setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGBA_2BPP, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, GL_RGBA);
			setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGB_4BPP, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT, GL_RGB);
			setCompressedFormat(resourceManager, dsGfxFormat_PVRTC1_RGBA_4BPP, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, GL_RGBA);
		}
	}

	if (AnyGL_IMG_texture_compression_pvrtc2)
	{
		setCompressedFormat(resourceManager, dsGfxFormat_PVRTC2_RGBA_2BPP, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG, GL_RGBA);
		setCompressedFormat(resourceManager, dsGfxFormat_PVRTC2_RGBA_4BPP, dsGfxFormat_UNorm,
			FormatBit_Texture, GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG, GL_RGBA);

		if (AnyGL_EXT_pvrtc_sRGB)
		{
			setCompressedFormat(resourceManager, dsGfxFormat_PVRTC2_RGBA_2BPP, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG, GL_RGBA);
			setCompressedFormat(resourceManager, dsGfxFormat_PVRTC2_RGBA_4BPP, dsGfxFormat_SRGB,
				FormatBit_Texture, GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG, GL_RGBA);
		}
	}
}

static bool formatSupported(const dsGLResourceManager* resourceManager, dsGfxFormat format, int bit)
{
	unsigned int standardIndex = dsGfxFormat_standardIndex(format);
	unsigned int decoratorIndex = dsGfxFormat_decoratorIndex(format);
	if (standardIndex > 0)
		return (resourceManager->standardFormats[standardIndex][decoratorIndex] & bit) != 0;

	unsigned int specialIndex = dsGfxFormat_specialIndex(format);
	if (specialIndex > 0)
		return (resourceManager->specialFormats[specialIndex] & bit) != 0;

	unsigned int compressedIndex = dsGfxFormat_compressedIndex(format);
	if (compressedIndex > 0)
		return (resourceManager->compressedFormats[compressedIndex][decoratorIndex] & bit) != 0;

	return false;
}

bool dsGLResourceManager_vertexFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_ASSERT(resourceManager);

	const dsGLResourceManager* glResourceManager = (const dsGLResourceManager*)resourceManager;
	return formatSupported(glResourceManager, format, FormatBit_Vertex);
}

bool dsGLResourceManager_textureFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_ASSERT(resourceManager);

	const dsGLResourceManager* glResourceManager = (const dsGLResourceManager*)resourceManager;
	return formatSupported(glResourceManager, format, FormatBit_Texture);
}

bool dsGLResourceManager_offscreenFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_ASSERT(resourceManager);

	const dsGLResourceManager* glResourceManager = (const dsGLResourceManager*)resourceManager;
	return formatSupported(glResourceManager, format, FormatBit_Offscreen);
}

bool dsGLResourceManager_textureBufferFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_ASSERT(resourceManager);

	const dsGLResourceManager* glResourceManager = (const dsGLResourceManager*)resourceManager;
	return formatSupported(glResourceManager, format, FormatBit_TextureBuffer);
}

bool dsGLResourceManager_surfaceBlitFormatsSupported(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat, dsBlitFilter filter)
{
	DS_UNUSED(resourceManager);
	if (!ANYGL_SUPPORTED(glBlitFramebuffer))
		return false;

	return dsGfxFormat_standardSurfaceBlitSupported(srcFormat, dstFormat, filter);
}

bool dsGLResourceManager_textureCopyFormatsSupported(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat)
{
	if (!ANYGL_SUPPORTED(glCopyImageSubData))
	{
		return dsGLResourceManager_surfaceBlitFormatsSupported(resourceManager, srcFormat,
			dstFormat, dsBlitFilter_Nearest);
	}

	if (!dsGLResourceManager_textureFormatSupported(resourceManager, srcFormat) ||
		!dsGLResourceManager_textureFormatSupported(resourceManager, dstFormat))
	{
		return false;
	}

	return dsGfxFormat_size(srcFormat) == dsGfxFormat_size(srcFormat);
}

dsResourceContext* dsGLResourceManager_createResourceContext(dsResourceManager* resourceManager)
{
	DS_ASSERT(resourceManager);

	dsGLResourceManager* glResourceManager = (dsGLResourceManager*)resourceManager;
	DS_VERIFY(dsMutex_lock(glResourceManager->mutex));
	dsResourceContext* context = NULL;
	for (uint32_t i = 0; i < resourceManager->maxResourceContexts; ++i)
	{
		if (!glResourceManager->resourceContexts[i].claimed)
		{
			context = glResourceManager->resourceContexts + i;
			context->claimed = true;
			break;
		}
	}
	DS_VERIFY(dsMutex_unlock(glResourceManager->mutex));

	// This should only be null in case of a bug or somebody manually messing with the members.
	DS_ASSERT(context);
	const dsRendererOptions* options = &((dsGLRenderer*)resourceManager->renderer)->options;
	DS_VERIFY(dsBindGLContext(options->display, context->context, context->dummySurface));
	return context;
}

bool dsGLResourceManager_destroyResourceContext(dsResourceManager* resourceManager,
	dsResourceContext* context)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(context);

	const dsRendererOptions* options = &((dsGLRenderer*)resourceManager->renderer)->options;
	DS_VERIFY(dsBindGLContext(options->display, NULL, NULL));

	dsGLResourceManager* glResourceManager = (dsGLResourceManager*)resourceManager;
	DS_VERIFY(dsMutex_lock(glResourceManager->mutex));
	context->claimed = false;
	DS_VERIFY(dsMutex_unlock(glResourceManager->mutex));

	return true;
}

dsGLResourceManager* dsGLResourceManager_create(dsAllocator* allocator, dsGLRenderer* renderer)
{
	DS_ASSERT(allocator);
	DS_ASSERT(renderer);

	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	const dsRendererOptions* options = &renderer->options;
	size_t bufferSize = dsGLResourceManager_fullAllocSize(options);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));

	dsGLResourceManager* resourceManager = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsGLResourceManager);
	DS_ASSERT(resourceManager);
	dsResourceManager* baseResourceManager = (dsResourceManager*)resourceManager;
	DS_VERIFY(dsResourceManager_initialize(baseResourceManager));

	if (options->maxResourceThreads > 0)
	{
		resourceManager->resourceContexts = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsResourceContext, options->maxResourceThreads);
		DS_ASSERT(resourceManager->resourceContexts);
		memset(resourceManager->resourceContexts, 0,
			sizeof(dsResourceContext)*options->maxResourceThreads);
	}
	else
		resourceManager->resourceContexts = NULL;

	resourceManager->mutex = dsMutex_create((dsAllocator*)&bufferAlloc, "Resource Manager");
	DS_ASSERT(resourceManager->mutex);

	baseResourceManager->renderer = baseRenderer;
	baseResourceManager->allocator = dsAllocator_keepPointer(allocator);
	baseResourceManager->maxResourceContexts = options->maxResourceThreads;

	for (uint8_t i = 0; i < options->maxResourceThreads; ++i)
	{
		dsResourceContext* resourceContext = resourceManager->resourceContexts + i;
		resourceContext->context = dsCreateGLContext(allocator, options->display,
			renderer->sharedConfig, renderer->sharedContext);
		if (!resourceContext->context)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create GL context.");
			dsGLResourceManager_destroy(resourceManager);
			return NULL;
		}

		resourceContext->dummySurface = dsCreateDummyGLSurface(allocator, options->display,
			renderer->sharedConfig, &resourceManager->resourceContexts[i].dummyOsSurface);
		if (!resourceContext->dummySurface)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create dummy GL surface.");
			dsGLResourceManager_destroy(resourceManager);
			return NULL;
		}
	}

	// Formats
	cacheVertexFormats(resourceManager);
	cacheTextureFormats(resourceManager);
	baseResourceManager->vertexFormatSupportedFunc = &dsGLResourceManager_vertexFormatSupported;
	baseResourceManager->textureFormatSupportedFunc = &dsGLResourceManager_textureFormatSupported;
	baseResourceManager->offscreenFormatSupportedFunc =
		&dsGLResourceManager_offscreenFormatSupported;
	baseResourceManager->textureBufferFormatSupportedFunc =
		&dsGLResourceManager_textureBufferFormatSupported;
	if (ANYGL_SUPPORTED(glGenerateMipmap))
	{
		baseResourceManager->generateMipmapFormatSupportedFunc =
			&dsGLResourceManager_offscreenFormatSupported;
	}
	baseResourceManager->textureCopyFormatsSupportedFunc =
		&dsGLResourceManager_textureCopyFormatsSupported;
	baseResourceManager->surfaceBlitFormatsSupportedFunc =
		&dsGLResourceManager_surfaceBlitFormatsSupported;

	// Resource contexts
	baseResourceManager->createResourceContextFunc = &dsGLResourceManager_createResourceContext;
	baseResourceManager->destroyResourceContextFunc = &dsGLResourceManager_destroyResourceContext;

	// Buffers
	baseResourceManager->supportedBuffers = getSupportedBuffers(baseRenderer->shaderVersion);
	baseResourceManager->bufferMapSupport = getBufferMapSupport();
	baseResourceManager->canCopyBuffers = ANYGL_SUPPORTED(glCopyBufferSubData);
	baseResourceManager->hasTextureBufferSubrange = ANYGL_SUPPORTED(glTexBufferRange);
	if (baseResourceManager->hasTextureBufferSubrange)
	{
		glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT,
			(GLint*)&baseResourceManager->minTextureBufferAlignment);
	}

	if (AnyGL_atLeastVersion(1, 0, false) || AnyGL_atLeastVersion(3, 0, true) ||
		AnyGL_OES_element_index_uint)
	{
		baseResourceManager->maxIndexSize = (uint32_t)sizeof(uint32_t);
	}
	else
		baseResourceManager->maxIndexSize = (uint32_t)sizeof(uint16_t);

	if (baseResourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock)
		glGetSizeT(GL_MAX_UNIFORM_BLOCK_SIZE, &baseResourceManager->maxUniformBlockSize);
	if (baseResourceManager->supportedBuffers & dsGfxBufferUsage_Image)
		glGetSizeT(GL_MAX_TEXTURE_BUFFER_SIZE, &baseResourceManager->maxTextureBufferElements);

	baseResourceManager->createBufferFunc = &dsGLGfxBuffer_create;
	baseResourceManager->destroyBufferFunc = &dsGLGfxBuffer_destroy;
	if (baseResourceManager->bufferMapSupport != dsGfxBufferMapSupport_None)
	{
		baseResourceManager->mapBufferFunc = &dsGLGfxBuffer_map;
		baseResourceManager->unmapBufferFunc = &dsGLGfxBuffer_unmap;
		if (baseResourceManager->bufferMapSupport == dsGfxBufferMapSupport_Persistent)
		{
			baseResourceManager->flushBufferFunc = &dsGLGfxBuffer_flush;
			baseResourceManager->invalidateBufferFunc = &dsGLGfxBuffer_invalidate;
		}
	}
	baseResourceManager->copyBufferDataFunc = &dsGLGfxBuffer_copyData;
	baseResourceManager->copyBufferFunc = &dsGLGfxBuffer_copy;

	// Draw geometry
	GLint maxVertexAttribs = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	baseResourceManager->maxVertexAttribs = dsMin(maxVertexAttribs, DS_MAX_ALLOWED_VERTEX_ATTRIBS);
	baseResourceManager->createGeometryFunc = &dsGLDrawGeometry_create;
	baseResourceManager->destroyGeometryFunc = &dsGLDrawGeometry_destroy;

	// Textures
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&baseResourceManager->maxTextureSize);
	if (ANYGL_SUPPORTED(glTexImage3D))
	{
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE,
			(GLint*)&baseResourceManager->maxTextureArrayLevels);
	}
	if (AnyGL_atLeastVersion(3, 0, false) || AnyGL_atLeastVersion(3, 0, true) ||
		AnyGL_EXT_texture_array)
	{
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS,
			(GLint*)&baseResourceManager->maxTextureArrayLevels);
	}
	baseResourceManager->hasArbitraryMipmapping = AnyGL_atLeastVersion(1, 2, false) ||
		AnyGL_atLeastVersion(3, 0, true);
	baseResourceManager->hasCubeArrays = AnyGL_atLeastVersion(4, 0, false) ||
		AnyGL_ARB_texture_cube_map_array;
	GLint maxSamplers;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxSamplers);
	baseResourceManager->maxSamplers = maxSamplers;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxSamplers);
	baseResourceManager->maxVertexSamplers = maxSamplers;
	if (ANYGL_SUPPORTED(glTexStorage2DMultisample))
		glGetIntegerv(GL_MAX_SAMPLES, (GLint*)&baseResourceManager->maxTextureSamples);
	else
		baseResourceManager->maxTextureSamples = 1;
	baseResourceManager->texturesReadable = ANYGL_SUPPORTED(glGetTexImage);
	baseResourceManager->createTextureFunc = &dsGLTexture_create;
	baseResourceManager->createOffscreenFunc = &dsGLTexture_createOffscreen;
	baseResourceManager->destroyTextureFunc = &dsGLTexture_destroy;
	baseResourceManager->copyTextureDataFunc = &dsGLTexture_copyData;
	baseResourceManager->copyTextureFunc = &dsGLTexture_copy;
	baseResourceManager->generateTextureMipmapsFunc = &dsGLTexture_generateMipmaps;
	baseResourceManager->getTextureDataFunc = &dsGLTexture_getData;

	// Renderbuffers
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, (GLint*)&baseResourceManager->maxRenderbufferSize);
	baseResourceManager->createRenderbufferFunc = &dsGLRenderbuffer_create;
	baseResourceManager->destroyRenderbufferFunc = &dsGLRenderbuffer_destroy;

	// Framebuffers
	if (ANYGL_SUPPORTED(glFramebufferParameteri))
	{
		glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS,
			(GLint*)&baseResourceManager->maxFramebufferLayers);
	}
	else
		baseResourceManager->maxFramebufferLayers = 1;
	baseResourceManager->requiresColorBuffer = ANYGL_SUPPORTED(glDrawBuffer) ||
		ANYGL_SUPPORTED(glDrawBuffers);
	baseResourceManager->requiresAnySurface = !AnyGL_atLeastVersion(4, 3, false) &&
		(!ANYGL_SUPPORTED(glFramebufferParameteri) || !AnyGL_ARB_framebuffer_no_attachments);
	baseResourceManager->canMixWithRenderSurface = false;
	baseResourceManager->hasVertexPipelineWrites = AnyGL_atLeastVersion(4, 2, false) ||
		AnyGL_atLeastVersion(3, 1, true);
	baseResourceManager->hasFragmentWrites = baseResourceManager->hasVertexPipelineWrites;
	baseResourceManager->createFramebufferFunc = &dsGLFramebuffer_create;
	baseResourceManager->destroyFramebufferFunc = &dsGLFramebuffer_destroy;

	// Fences
	baseResourceManager->hasFences = ANYGL_SUPPORTED(glFenceSync);
	baseResourceManager->createFenceFunc = &dsGLGfxFence_create;
	baseResourceManager->destroyFenceFunc = &dsGLGfxFence_destroy;
	baseResourceManager->setFencesFunc = &dsGLGfxFence_set;
	baseResourceManager->waitFenceFunc = &dsGLGfxFence_wait;
	baseResourceManager->resetFenceFunc = &dsGLGfxFence_reset;

	// Queries
	baseResourceManager->hasQueries = ANYGL_SUPPORTED(glGenQueries);
	baseResourceManager->has64BitQueries = ANYGL_SUPPORTED(glGetQueryObjectui64v);
	baseResourceManager->hasQueryBuffers = AnyGL_atLeastVersion(4, 4, false) ||
		AnyGL_ARB_query_buffer_object;
	if (AnyGL_atLeastVersion(3, 3, false) || AnyGL_ARB_timer_query || AnyGL_EXT_timer_query ||
		AnyGL_EXT_disjoint_timer_query)
	{
		DS_ASSERT(baseResourceManager->has64BitQueries);
		baseResourceManager->timestampPeriod = 1.0f;
	}
	baseResourceManager->createQueryPoolFunc = &dsGLGfxQueryPool_create;
	baseResourceManager->destroyQueryPoolFunc = &dsGLGfxQueryPool_destroy;
	baseResourceManager->resetQueryPoolFunc = &dsGLGfxQueryPool_reset;
	baseResourceManager->beginQueryFunc = &dsGLGfxQueryPool_beginQuery;
	baseResourceManager->endQueryFunc = &dsGLGfxQueryPool_endQuery;
	baseResourceManager->queryTimestampFunc = &dsGLGfxQueryPool_queryTimestamp;
	baseResourceManager->getQueryValuesFunc = &dsGLGfxQueryPool_getValues;
	baseResourceManager->copyQueryValuesFunc = &dsGLGfxQueryPool_copyValues;

	// Shaders and materials
	baseResourceManager->createShaderModuleFunc = &dsGLShaderModule_create;
	baseResourceManager->destroyShaderModuleFunc = &dsGLShaderModule_destroy;
	baseResourceManager->isShaderUniformInternalFunc = &dsGLShader_isUniformInternal;
	baseResourceManager->createMaterialDescFunc = &dsGLMaterialDesc_create;
	baseResourceManager->destroyMaterialDescFunc = &dsGLMaterialDesc_destroy;
	baseResourceManager->createShaderVariableGroupDescFunc = &dsGLShaderVariableGroupDesc_create;
	baseResourceManager->destroyShaderVariableGroupDescFunc = &dsGLShaderVariableGroupDesc_destroy;
	baseResourceManager->createShaderFunc = &dsGLShader_create;
	baseResourceManager->destroyShaderFunc = &dsGLShader_destroy;
	baseResourceManager->bindShaderFunc = &dsGLShader_bind;
	baseResourceManager->updateShaderVolatileValuesFunc = &dsGLShader_updateVolatileValues;
	baseResourceManager->unbindShaderFunc = &dsGLShader_unbind;
	baseResourceManager->bindComputeShaderFunc = &dsGLShader_bindCompute;
	baseResourceManager->updateComputeShaderVolatileValuesFunc =
		&dsGLShader_updateComputeVolatileValues;
	baseResourceManager->unbindComputeShaderFunc = &dsGLShader_unbindCompute;

	return resourceManager;
}

bool dsGLResourceManager_getVertexFormatInfo(GLenum* outFormat, GLint* outElements,
	bool* outNormalized, const dsResourceManager* resourceManager, dsGfxFormat format)
{
	const dsGLResourceManager* glResourceManager = (const dsGLResourceManager*)resourceManager;
	unsigned int standardIndex = dsGfxFormat_standardIndex(format);
	unsigned int decoratorIndex = dsGfxFormat_decoratorIndex(format);
	if (standardIndex > 0)
	{
		if (!glResourceManager->standardVertexFormats[standardIndex][decoratorIndex])
			return false;

		if (outFormat)
			*outFormat = glResourceManager->standardVertexFormats[standardIndex][decoratorIndex];
		if (outElements)
			*outElements = glResourceManager->standardVertexElements[standardIndex][decoratorIndex];
		if (outNormalized)
		{
			int decorator = format & dsGfxFormat_DecoratorMask;
			*outNormalized = decorator == dsGfxFormat_UNorm || decorator == dsGfxFormat_SNorm;
		}
		return true;
	}

	unsigned int specialIndex = dsGfxFormat_specialIndex(format);
	if (specialIndex > 0)
	{
		if (!glResourceManager->specialVertexFormats[specialIndex])
			return false;

		if (outFormat)
			*outFormat = glResourceManager->specialVertexFormats[specialIndex];
		if (outElements)
			*outElements = glResourceManager->specialVertexElements[specialIndex];
		if (outNormalized)
			*outNormalized = false;
		return true;
	}

	return false;
}

bool dsGLResourceManager_getTextureFormatInfo(GLenum* outInternalFormat, GLenum* outFormat,
	GLenum* outType, const dsResourceManager* resourceManager, dsGfxFormat format)
{
	const dsGLResourceManager* glResourceManager = (const dsGLResourceManager*)resourceManager;
	unsigned int standardIndex = dsGfxFormat_standardIndex(format);
	unsigned int decoratorIndex = dsGfxFormat_decoratorIndex(format);
	if (standardIndex > 0)
	{
		if (!glResourceManager->standardInternalFormats[standardIndex][decoratorIndex])
			return false;

		if (outInternalFormat)
		{
			*outInternalFormat =
				glResourceManager->standardInternalFormats[standardIndex][decoratorIndex];
		}
		if (outFormat)
			*outFormat = glResourceManager->standardGlFormats[standardIndex][decoratorIndex];
		if (outType)
			*outType = glResourceManager->standardTypes[standardIndex][decoratorIndex];
		return true;
	}

	unsigned int specialIndex = dsGfxFormat_specialIndex(format);
	if (specialIndex > 0)
	{
		if (!glResourceManager->specialInternalFormats[specialIndex])
			return false;

		if (outInternalFormat)
		{
			*outInternalFormat =
				glResourceManager->specialInternalFormats[specialIndex];
		}
		if (outFormat)
			*outFormat = glResourceManager->specialGlFormats[specialIndex];
		if (outType)
			*outType = glResourceManager->specialTypes[specialIndex];
		return true;
	}

	unsigned int compressedIndex = dsGfxFormat_compressedIndex(format);
	if (compressedIndex > 0)
	{
		if (!glResourceManager->compressedInternalFormats[compressedIndex][decoratorIndex])
			return false;

		if (outInternalFormat)
		{
			*outInternalFormat =
				glResourceManager->compressedInternalFormats[compressedIndex][decoratorIndex];
		}
		if (outFormat)
			*outFormat = glResourceManager->compressedGlFormats[compressedIndex][decoratorIndex];
		if (outType)
			*outType = 0;
		return true;
	}

	return false;
}

void dsGLResourceManager_destroy(dsGLResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

	dsResourceManager* baseResourceManager = (dsResourceManager*)resourceManager;
	const dsRendererOptions* options = &((dsGLRenderer*)baseResourceManager->renderer)->options;
	for (uint8_t i = 0; i < ((dsResourceManager*)resourceManager)->maxResourceContexts; ++i)
	{
		dsResourceContext* resourceContext = resourceManager->resourceContexts + i;
		dsDestroyGLContext(options->display, resourceContext->context);
		dsDestroyDummyGLSurface(options->display, resourceContext->dummySurface,
			resourceContext->dummyOsSurface);
	}

	dsMutex_destroy(resourceManager->mutex);
	dsResourceManager_shutdown((dsResourceManager*)resourceManager);
	if (((dsResourceManager*)resourceManager)->allocator)
		dsAllocator_free(((dsResourceManager*)resourceManager)->allocator, resourceManager);
}
