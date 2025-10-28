/*
 * Copyright 2018-2023 Aaron Barany
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

#include <DeepSea/VectorDraw/VectorResources.h>

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Streams/MemoryStream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <algorithm>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/VectorResources_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

static void printFlatbufferError(const char* name)
{
	if (name)
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Invalid vector resources flatbuffer format for '%s'.", name);
	}
	else
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Invalid vector resources flatbuffer format.");
}

extern "C"
dsVectorResources* dsVectorResources_loadImpl(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsResourceManager* resourceManager, const void* data, size_t size,
	void* loadUserData, dsLoadVectorResourcesTextureFunction loadTextureFunc,
	dsLoadVectorResourcesFontFaceFunction loadFontFaceFunc, const dsTextQuality* qualityRemap,
	const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaVectorDraw::VerifyVectorResourcesBuffer(verifier))
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	auto resourceSet = DeepSeaVectorDraw::GetVectorResources(data);
	auto textures = resourceSet->textures();
	auto faceGroups = resourceSet->faceGroups();
	auto fonts = resourceSet->fonts();

	uint32_t textureCount = 0, faceGroupCount = 0, fontCount = 0;
	if (textures)
		textureCount = textures->size();
	if (faceGroups)
		faceGroupCount = faceGroups->size();
	if (fonts)
		fontCount = fonts->size();

	dsVectorResources* resources = dsVectorReosurces_create(allocator, textureCount, faceGroupCount,
		fontCount);
	if (!resources)
		return NULL;

	for (uint32_t i = 0; i < textureCount; ++i)
	{
		auto textureRef = (*textures)[i];
		if (!textureRef)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		dsTextureUsage usage = dsTextureUsage_Texture;
		dsGfxMemory memoryHints = dsGfxMemory_Static | dsGfxMemory_GPUOnly;
		dsTexture* texture;
		if (auto fileRef = textureRef->data_as_FileReference())
		{
			texture = loadTextureFunc(loadUserData, resourceManager, allocator,
				scratchAllocator, fileRef->path()->c_str(), usage, memoryHints);
		}
		else if (auto rawData = textureRef->data_as_RawData())
		{
			auto data = rawData->data();
			dsMemoryStream stream;
			DS_VERIFY(dsMemoryStream_open(&stream, (void*)data->data(), data->size()));
			texture = dsTextureData_loadStreamToTexture(resourceManager, allocator,
				scratchAllocator, reinterpret_cast<dsStream*>(&stream), nullptr, usage,
				memoryHints);
			DS_VERIFY(dsMemoryStream_close(&stream));
		}
		else
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			return nullptr;
		}

		if (!texture)
		{
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		DS_VERIFY(dsVectorResources_addTexture(resources, textureRef->name()->c_str(), texture,
			true));
	}

	for (uint32_t i = 0; i < faceGroupCount; ++i)
	{
		auto faceGroupRef = (*faceGroups)[i];
		if (!faceGroupRef || faceGroupRef->faces()->size() == 0)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		auto faces = faceGroupRef->faces();
		uint32_t faceCount = faces->size();

		dsFaceGroup* faceGroup = dsFaceGroup_create(allocator, allocator, faceCount);
		if (!faceGroup)
		{
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		DS_VERIFY(dsVectorResources_addFaceGroup(resources, faceGroupRef->name()->c_str(),
			faceGroup, true));

		for (uint32_t j = 0; j < faceCount; ++j)
		{
			auto faceRef = (*faces)[j];
			if (!faceRef)
			{
				errno = EFORMAT;
				printFlatbufferError(name);
				DS_VERIFY(dsVectorResources_destroy(resources));
				return nullptr;
			}

			bool success;
			if (auto fileRef = faceRef->data_as_FileReference())
			{
				success = loadFontFaceFunc(loadUserData, faceGroup, fileRef->path()->c_str(),
					faceRef->name()->c_str());
			}
			else if (auto rawData = faceRef->data_as_RawData())
			{
				auto data = rawData->data();
				success = dsFaceGroup_loadFaceData(faceGroup, allocator, data->data(),
					data->size(), faceRef->name()->c_str());
			}
			else
			{
				errno = EFORMAT;
				printFlatbufferError(name);
				return nullptr;
			}

			if (!success)
			{
				DS_VERIFY(dsVectorResources_destroy(resources));
				return nullptr;
			}
		}
	}

	uint32_t maxFaces = 0;
	for (uint32_t i = 0; i < fontCount; ++i)
	{
		auto fontRef = (*fonts)[i];
		if (!fontRef || fontRef->faces()->size() == 0)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		maxFaces = std::max(maxFaces, fontRef->faces()->size());
	}
	const char** faceList = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, maxFaces);

	for (uint32_t i = 0; i < fontCount; ++i)
	{
		auto fontRef = (*fonts)[i];
		dsFaceGroup* faceGroup = dsVectorResources_findFaceGroup(resources,
			fontRef->faceGroup()->c_str());
		if (!faceGroup)
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
					"Face group '%s' isn't present in vector resources '%s'.",
					fontRef->faceGroup()->c_str(), name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
					"Face group '%s' isn't present in vector resources.",
					fontRef->faceGroup()->c_str());
			}
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		auto faces = fontRef->faces();
		uint32_t faceCount = faces->size();
		DS_ASSERT(faceCount <= maxFaces);
		for (uint32_t j = 0; j < faceCount; ++j)
		{
			auto faceRef = (*faces)[j];
			if (!faceRef)
			{
				errno = EFORMAT;
				printFlatbufferError(name);
				DS_VERIFY(dsVectorResources_destroy(resources));
				return nullptr;
			}

			faceList[j] = faceRef->c_str();
		}

		dsTextQuality quality = static_cast<dsTextQuality>(fontRef->quality());
		if (quality < dsTextQuality_Low || quality > dsTextQuality_VeryHigh)
			quality = dsTextQuality_Medium;
		if (qualityRemap)
			quality = qualityRemap[quality];

		dsFont* font = dsFont_create(faceGroup, resourceManager, allocator, faceList, faceCount,
			nullptr, quality, static_cast<dsTextCache>(fontRef->cacheSize()));
		if (!font)
		{
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		DS_VERIFY(dsVectorResources_addFont(resources, fontRef->name()->c_str(), font, true));
	}

	return resources;
}
