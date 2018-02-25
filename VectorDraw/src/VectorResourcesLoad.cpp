/*
 * Copyright 2018 Aaron Barany
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

#include "Flatbuffers/VectorResources_generated.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <algorithm>

#if DS_WINDOWS
#include <malloc.h>
#else
#include <alloca.h>
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
	dsLoadVectorResourcesFontFaceFunction loadFontFaceFunc, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaVectorDraw::VerifyResourceSetBuffer(verifier))
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	auto resourceSet = DeepSeaVectorDraw::GetResourceSet(data);
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

		dsTexture* texture = loadTextureFunc(loadUserData, resourceManager, allocator,
			scratchAllocator, textureRef->path()->c_str(), dsTextureUsage_Texture,
			dsGfxMemory_Static | dsGfxMemory_GpuOnly);
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
		if (!faceGroupRef || !faceGroupRef->faces() || faceGroupRef->faces()->size() == 0)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		auto faces = faceGroupRef->faces();
		uint32_t faceCount = faces->size();
		dsTextQuality quality = static_cast<dsTextQuality>(faceGroupRef->quality());
		dsFaceGroup* faceGroup = dsFaceGroup_create(allocator, allocator, faceCount, quality);
		if (!faceGroup)
		{
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		DS_VERIFY(dsVectorResources_addFaceGroup(resources, faceGroupRef->name()->c_str(),
			faceGroup, true));

		for (uint32_t j = 0; j < faceCount; ++j)
		{
			auto faceRef = (*faces)[i];
			if (!faceRef)
			{
				errno = EFORMAT;
				printFlatbufferError(name);
				DS_VERIFY(dsVectorResources_destroy(resources));
				return nullptr;
			}

			if (!loadFontFaceFunc(loadUserData, faceGroup, faceRef->path()->c_str(),
				faceRef->name()->c_str()))
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
		if (!fontRef || !fontRef->faces() || fontRef->faces()->size() == 0)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		maxFaces = std::max(maxFaces, fontRef->faces()->size());
	}
	const char** faceList = (const char**)alloca(maxFaces*sizeof(const char*));

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
					fontRef->faceGroup()->c_str(),  name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
					"Face group '%s' isn't present in vector resources '%s'.",
					fontRef->faceGroup()->c_str(),  name);
			}
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		auto faces = fontRef->faces();
		uint32_t faceCount = faces->size();
		DS_ASSERT(faceCount <= maxFaces);
		for (uint32_t j = 0; j < faceCount; ++j)
		{
			auto faceRef = (*faces)[i];
			if (!faceRef)
			{
				errno = EFORMAT;
				printFlatbufferError(name);
				DS_VERIFY(dsVectorResources_destroy(resources));
				return nullptr;
			}

			faceList[j] = faceRef->c_str();
		}

		dsFont* font = dsFont_create(faceGroup, resourceManager, allocator, faceList, faceCount);
		if (!font)
		{
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		DS_VERIFY(dsVectorResources_addFont(resources, fontRef->name()->c_str(), font, true));
	}

	return resources;
}
