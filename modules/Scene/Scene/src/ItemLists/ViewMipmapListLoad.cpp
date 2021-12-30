/*
 * Copyright 2021 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/ViewMipmapList.h>

#include "SceneLoadContextInternal.h"
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/ViewMipmapList_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

extern "C"
dsSceneItemList* dsViewMipmapList_load(const dsSceneLoadContext*, dsSceneLoadScratchData*,
	dsAllocator* allocator, dsAllocator*, void*, const char* name, const uint8_t* data,
	size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyViewMipmapListBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid view mipmap list data flatbuffer format.");
		return nullptr;
	}

	auto fbMipmapList = DeepSeaScene::GetViewMipmapList(data);
	auto fbTextureList = fbMipmapList->textures();

	uint32_t fullCount = fbTextureList->size();
	const char** textureNames = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, fullCount);
	uint32_t textureCount = 0;
	for (uint32_t i = 0; i < fullCount; ++i)
	{
		auto fbTexture = (*fbTextureList)[i];
		if (!fbTexture)
			continue;

		textureNames[textureCount++] = fbTexture->c_str();
	}

	if (textureCount == 0)
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "View mipmap list contains no valid texture names.");
	}

	return dsViewMipmapList_create(allocator, name, textureNames, textureCount);
}
