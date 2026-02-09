/*
 * Copyright 2018-2026 Aaron Barany
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

#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/Resources/TextureData.h>

#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/TextIcons.h>
#include <DeepSea/Text/TextureTextIcons.h>

#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDraw/VectorTextIcons.h>

#include <limits>

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

static dsTexture* loadTexture(const DeepSeaVectorDraw::TextureResource& fbTexture,
	void* relativePathUserData, dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	dsResourceManager* resourceManager, dsAllocator* allocator, dsAllocator* scratchAllocator,
	const char* name)
{
	dsTextureUsage usage = dsTextureUsage_Texture;
	dsGfxMemory memoryHints = dsGfxMemory_Static | dsGfxMemory_GPUOnly;
	if (auto fileRef = fbTexture.data_as_FileReference())
	{
		dsStream* stream = openRelativePathStreamFunc(
			relativePathUserData, fileRef->path()->c_str(), "rb");
		if (!stream)
			return nullptr;

		dsTexture* texture = dsTextureData_loadStreamToTexture(resourceManager, allocator,
			scratchAllocator, stream, nullptr, usage, memoryHints);
		closeRelativePathStreamFunc(relativePathUserData, stream);
		return texture;
	}
	else if (auto rawData = fbTexture.data_as_RawData())
	{
		auto data = rawData->data();
		dsMemoryStream stream;
		DS_VERIFY(dsMemoryStream_open(&stream, (void*)data->data(), data->size()));
		dsTexture* texture = dsTextureData_loadStreamToTexture(resourceManager, allocator,
			scratchAllocator, reinterpret_cast<dsStream*>(&stream), nullptr, usage,
			memoryHints);
		DS_VERIFY(dsMemoryStream_close(&stream));
		return texture;
	}

	errno = EFORMAT;
	printFlatbufferError(name);
	return nullptr;
}

static dsVectorImage* loadVectorImage(const DeepSeaVectorDraw::VectorImageResource& fbVectorImage,
	void* relativePathUserData, dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, float pixelSize, const char* name)
{
	const dsVector2f* size = reinterpret_cast<const dsVector2f*>(fbVectorImage.targetSize());
	if (auto fileRef = fbVectorImage.data_as_FileReference())
	{
		dsStream* stream = openRelativePathStreamFunc(
			relativePathUserData, fileRef->path()->c_str(), "rb");
		if (!stream)
			return nullptr;

		dsVectorImage* image = dsVectorImage_loadStream(allocator, resourceAllocator,
			initResources, stream, pixelSize, size);
		closeRelativePathStreamFunc(relativePathUserData, stream);
		return image;
	}
	else if (auto rawData = fbVectorImage.data_as_RawData())
	{
		auto data = rawData->data();
		return dsVectorImage_loadData(allocator, resourceAllocator, initResources,
			data->data(), data->size(), pixelSize, size);
	}

	errno = EFORMAT;
	printFlatbufferError(name);
	return nullptr;
}

static dsTextIcons* loadTextIcons(const DeepSeaVectorDraw::TextIcons& fbTextIcons,
	const dsVectorResources* resources, dsAllocator* allocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, const dsVectorShaders* vectorIconShaders,
	const dsShader* textureIconShader, const dsMaterial* textureIconMaterial, const char* name)
{
	uint32_t iconCount = 0;
	auto fbIcons = fbTextIcons.icons();
	uint32_t codepointRangeCount = fbIcons->size();
	if (codepointRangeCount == 0)
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	dsIndexRange* codepointRanges =
		DS_ALLOCATE_STACK_OBJECT_ARRAY(dsIndexRange, codepointRangeCount);
	for (uint32_t i = 0; i < codepointRangeCount; ++i)
	{
		uint32_t minCodepoint = std::numeric_limits<uint32_t>::max();
		uint32_t maxCodepoint = 0;
		auto fbIconGroup = (*fbIcons)[i];
		if (!fbIconGroup)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			return nullptr;
		}

		auto fbIconRange = fbIconGroup->icons();
		uint32_t rangeCount = fbIconRange->size();
		if (rangeCount == 0)
		{
			printFlatbufferError(name);
			return nullptr;
		}

		iconCount += rangeCount;
		for (uint32_t j = 0; j < rangeCount; ++j)
		{
			auto fbIcon = (*fbIconRange)[j];
			if (!fbIcon)
			{
				errno = EFORMAT;
				printFlatbufferError(name);
				return nullptr;
			}

			uint32_t codepoint = fbIcon->codepoint();
			minCodepoint = dsMin(minCodepoint, codepoint);
			maxCodepoint = dsMax(maxCodepoint, codepoint);
		}

		dsIndexRange* codepointRange = codepointRanges + i;
		codepointRange->start = minCodepoint;
		codepointRange->count = maxCodepoint - minCodepoint + 1;
	}

	switch (fbTextIcons.type())
	{
		case DeepSeaVectorDraw::IconType::Texture:
		{
			if (!textureIconShader)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
					"Must provide texture icon shader to vector resources load.");
				return nullptr;
			}

			dsTextIcons* textIcons = dsTextureTextIcons_create(allocator, resourceManager,
				resourceAllocator, textureIconShader, textureIconMaterial, codepointRanges,
				codepointRangeCount, iconCount);
			if (!textIcons)
				return nullptr;

			for (uint32_t i = 0; i < codepointRangeCount; ++i)
			{
				auto fbIconRange = (*fbIcons)[i]->icons();
				size_t rangeCount = fbIconRange->size();
				for (uint32_t j = 0; j < rangeCount; ++j)
				{
					auto fbIcon = (*fbIconRange)[j];
					const char* iconName = fbIcon->icon()->c_str();
					dsVectorResourceType resourceType;
					dsTexture* texture;
					if (!dsVectorResources_findResource(
							&resourceType, (void**)&texture, resources, iconName) ||
						resourceType != dsVectorResourceType_Texture)
					{
						errno = ENOTFOUND;
						if (name)
						{
							DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Couldn't find texture '%s' for "
								"text icons in vector resources '%s'.", iconName, name);
						}
						else
						{
							DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Couldn't find texture '%s' for "
								"text icons in vector resources.", iconName);
						}
						return nullptr;
					}

					auto fbBoundsMin = fbIcon->boundsMin();
					auto fbBoundsMax = fbIcon->boundsMax();
					dsAlignedBox2f bounds = {{{fbBoundsMin->x(), fbBoundsMin->y()}},
						{{fbBoundsMax->x(), fbBoundsMax->y()}}};
					if (!dsTextureTextIcons_addIcon(textIcons, fbIcon->codepoint(),
							fbIcon->advance(), &bounds, texture, false))
					{
						dsTextIcons_destroy(textIcons);
						return nullptr;
					}
				}
			}
			return textIcons;
		}
		case DeepSeaVectorDraw::IconType::VectorImage:
		{
			if (!vectorIconShaders)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
					"Must provide vector icon shaders to vector resources load.");
				return nullptr;
			}

			dsTextIcons* textIcons = dsVectorTextIcons_create(allocator, resourceManager,
				vectorIconShaders, codepointRanges, codepointRangeCount, iconCount);
			if (!textIcons)
				return nullptr;

			for (uint32_t i = 0; i < codepointRangeCount; ++i)
			{
				auto fbIconRange = (*fbIcons)[i]->icons();
				size_t rangeCount = fbIconRange->size();
				for (uint32_t j = 0; j < rangeCount; ++j)
				{
					auto fbIcon = (*fbIconRange)[j];
					const char* iconName = fbIcon->icon()->c_str();
					dsVectorResourceType resourceType;
					dsVectorImage* image;
					if (!dsVectorResources_findResource(
							&resourceType, (void**)&image, resources, iconName) ||
						resourceType != dsVectorResourceType_VectorImage)
					{
						errno = ENOTFOUND;
						if (name)
						{
							DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Couldn't find vector image "
								"'%s' for text icons in vector resources '%s'.", iconName, name);
						}
						else
						{
							DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Couldn't find vector image "
								"'%s' for text icons in vector resources.", iconName);
						}
						return nullptr;
					}

					auto fbBoundsMin = fbIcon->boundsMin();
					auto fbBoundsMax = fbIcon->boundsMax();
					dsAlignedBox2f bounds = {{{fbBoundsMin->x(), fbBoundsMin->y()}},
						{{fbBoundsMax->x(), fbBoundsMax->y()}}};
					if (!dsVectorTextIcons_addIcon(textIcons, fbIcon->codepoint(),
							fbIcon->advance(), &bounds, image, false))
					{
						dsTextIcons_destroy(textIcons);
						return nullptr;
					}
				}
			}
			return textIcons;
		}
	}

	errno = EFORMAT;
	printFlatbufferError(name);
	return nullptr;
}

static dsFaceGroup* loadFaceGroup(const DeepSeaVectorDraw::FaceGroup& fbFaceGroup,
	dsAllocator* allocator, dsAllocator* scratchAllocator, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc, const char* name)
{
	auto faces = fbFaceGroup.faces();
	uint32_t faceCount = faces->size();

	dsFaceGroup* faceGroup = dsFaceGroup_create(allocator, scratchAllocator, faceCount);
	if (!faceGroup)
		return nullptr;

	for (uint32_t i = 0; i < faceCount; ++i)
	{
		auto faceRef = (*faces)[i];
		if (!faceRef)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			dsFaceGroup_destroy(faceGroup);
			return nullptr;
		}

		bool success;
		if (auto fileRef = faceRef->data_as_FileReference())
		{
			dsStream* stream = openRelativePathStreamFunc(
				relativePathUserData, fileRef->path()->c_str(), "rb");
			if (!stream)
				return nullptr;

			success = dsFaceGroup_loadFaceStream(
				faceGroup, allocator, stream, faceRef->name()->c_str());
			closeRelativePathStreamFunc(relativePathUserData, stream);
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
			dsFaceGroup_destroy(faceGroup);
			return nullptr;
		}

		if (!success)
		{
			dsFaceGroup_destroy(faceGroup);
			return nullptr;
		}
	}

	return faceGroup;
}

static dsFont* loadFont(const DeepSeaVectorDraw::Font& fbFont, const dsVectorResources* resources,
	dsAllocator* allocator, dsResourceManager* resourceManager, const dsTextQuality* qualityRemap,
	const char* name)
{
	const char* faceGroupName = fbFont.faceGroup()->c_str();
	dsVectorResourceType resourceType;
	dsFaceGroup* faceGroup;
	if (!dsVectorResources_findResource(
			&resourceType, (void**)&faceGroup, resources, faceGroupName) ||
		resourceType != dsVectorResourceType_FaceGroup)
	{
		errno = ENOTFOUND;
		if (name)
		{
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
				"Couldn't find face group '%s' for font in vector resources '%s'.", faceGroupName,
				name);
		}
		else
		{
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
				"Couldn't find face group '%s' for font in vector resources.", faceGroupName);
		}
		return nullptr;
	}

	auto fbFaces = fbFont.faces();
	const char** faces = nullptr;
	uint32_t faceCount = 0;
	if (fbFaces && !fbFaces->empty())
	{
		faceCount = fbFaces->size();
		faces = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, faceCount);
		for (uint32_t i = 0; i < faceCount; ++i)
		{
			auto fbFace = (*fbFaces)[i];
			if (!fbFace)
			{
				errno = EFORMAT;
				printFlatbufferError(name);
				return nullptr;
			}

			faces[i] = fbFace->c_str();
		}
	}

	dsTextIcons* textIcons = nullptr;
	auto fbIcons = fbFont.icons();
	if (fbIcons)
	{
		const char* iconsName = fbIcons->c_str();
		if (!dsVectorResources_findResource(
				&resourceType, (void**)&textIcons, resources, iconsName) ||
			resourceType != dsVectorResourceType_TextIcons)
		{
			errno = ENOTFOUND;
			if (name)
			{
				DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
					"Couldn't find text icons '%s' for font in vector resources '%s'.", iconsName,
					name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
					"Couldn't find text icons '%s' for font in vector resources.", iconsName);
			}
			return nullptr;
		}
	}

	dsTextQuality quality = static_cast<dsTextQuality>(fbFont.quality());
	if (quality < dsTextQuality_Low || quality > dsTextQuality_VeryHigh)
		quality = dsTextQuality_Medium;
	if (qualityRemap)
		quality = qualityRemap[quality];

	return dsFont_create(faceGroup, resourceManager, allocator, faces, faceCount, textIcons,
		quality, static_cast<dsTextCache>(fbFont.cacheSize()));
}

extern "C"
dsVectorResources* dsVectorResources_loadImpl(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager, const void* data,
	size_t size, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaVectorDraw::VerifyVectorResourcesBuffer(verifier))
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;
	if (!resourceAllocator)
		resourceAllocator = allocator;

	auto resourceSet = DeepSeaVectorDraw::GetVectorResources(data);
	auto fbResources = resourceSet->resources();
	uint32_t resourceCount = fbResources->size();
	if (resourceCount == 0)
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	dsVectorResources* resources = dsVectorReosurces_create(allocator, resourceCount);
	if (!resources)
		return nullptr;

	dsVectorImageInitResources initResourcesWithThis;
	if (initResources)
	{
		initResourcesWithThis = *initResources;

		// Add these resources to the list of vector resources for initialization.
		dsVectorResources** allResources = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsVectorResources*,
			initResources->resourceCount + 1);
		memcpy(allResources, initResources->resources,
			sizeof(dsVectorResources*)*initResources->resourceCount);
		allResources[initResources->resourceCount] = resources;
		initResourcesWithThis.resources = allResources;
		++initResourcesWithThis.resourceCount;
	}

	for (uint32_t i = 0; i < resourceCount; ++i)
	{
		auto fbResource = (*fbResources)[i];
		if (!fbResource)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			DS_VERIFY(dsVectorResources_destroy(resources));
			return nullptr;
		}

		const char* resourceName = fbResource->name()->c_str();
		dsVectorResourceType resourceType;
		void* resource;
		switch (fbResource->resource_type())
		{
			case DeepSeaVectorDraw::VectorResourceUnion::TextureResource:
			{
				resourceType = dsVectorResourceType_Texture;
				resource = loadTexture(*fbResource->resource_as_TextureResource(),
					relativePathUserData, openRelativePathStreamFunc, closeRelativePathStreamFunc,
					resourceManager, resourceAllocator, scratchAllocator, name);
				break;
			}
			case DeepSeaVectorDraw::VectorResourceUnion::VectorImageResource:
			{
				if (!initResources)
				{
					errno = EINVAL;
					DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
						"Must provide init resources to vector resources load.");
					return nullptr;
				}

				resourceType = dsVectorResourceType_VectorImage;
				resource = loadVectorImage(*fbResource->resource_as_VectorImageResource(),
					relativePathUserData, openRelativePathStreamFunc, closeRelativePathStreamFunc,
					allocator, resourceAllocator, &initResourcesWithThis, pixelSize, name);
				break;
			}
			case DeepSeaVectorDraw::VectorResourceUnion::TextIcons:
			{
				resourceType = dsVectorResourceType_TextIcons;
				resource = loadTextIcons(*fbResource->resource_as_TextIcons(), resources, allocator,
					resourceAllocator, resourceManager, vectorIconShaders, textureIconShader,
					textureIconMaterial, name);
				break;
			}
			case DeepSeaVectorDraw::VectorResourceUnion::FaceGroup:
			{
				resourceType = dsVectorResourceType_FaceGroup;
				resource = loadFaceGroup(*fbResource->resource_as_FaceGroup(), allocator,
					scratchAllocator, relativePathUserData, openRelativePathStreamFunc,
					closeRelativePathStreamFunc, name);
				break;
			}
			case DeepSeaVectorDraw::VectorResourceUnion::Font:
			{
				resourceType = dsVectorResourceType_Font;
				resource = loadFont(*fbResource->resource_as_Font(), resources, allocator,
					resourceManager, qualityRemap, name);
				break;
			}
			default:
				errno = EFORMAT;
				printFlatbufferError(name);
				dsVectorResources_destroy(resources);
				return nullptr;
		}

		if (!resource)
		{
			dsVectorResources_destroy(resources);
			return nullptr;
		}

		if (!dsVectorResources_addResource(resources, resourceName, resourceType, resource, true))
		{
			if (name)
			{
				DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
					"Couldn't resource '%s' for font in vector resources '%s'.", resourceName, name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
					"Couldn't resource '%s' for font in vector resources.", resourceName);
			}
			dsVectorResources_destroy(resources);
			return nullptr;
		}
	}

	return resources;
}
