/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/VectorDrawScene/SceneText.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Text/Text.h>

#include <string.h>

const char* const dsSceneText_typeName = "Text";

static bool destroySceneText(void* text)
{
	dsSceneText_destroy((dsSceneText*)text);
	return true;
}

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneText_type(void)
{
	return &resourceType;
}

dsSceneText* dsSceneText_create(dsAllocator* allocator, dsText* text, void* userData,
	const dsTextStyle* styles, uint32_t styleCount)
{
	if (!allocator || !text || (!styles && styleCount > 0))
	{
		errno = EINVAL;
		dsText_destroy(text);
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneText)) +
		DS_ALIGNED_SIZE(sizeof(dsTextStyle)*styleCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		dsText_destroy(text);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneText* sceneText = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneText);
	DS_ASSERT(sceneText);

	sceneText->allocator = dsAllocator_keepPointer(allocator);
	sceneText->text = text;
	sceneText->userData = userData;

	if (styleCount > 0)
	{
		sceneText->styles = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsTextStyle, styleCount);
		DS_ASSERT(sceneText->styles);
		memcpy(sceneText->styles, styles, sizeof(dsTextStyle)*styleCount);
	}
	else
		sceneText->styles = NULL;
	sceneText->styleCount = styleCount;

	return sceneText;
}

dsCustomSceneResource* dsSceneText_createResource(dsAllocator* allocator, dsSceneText* text)
{
	if (!allocator || !text)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = text;
	customResource->destroyFunc = &destroySceneText;
	return customResource;
}

void dsSceneText_destroy(dsSceneText* text)
{
	if (!text)
		return;

	dsText_destroy(text->text);
	if (text->allocator)
		DS_VERIFY(dsAllocator_free(text->allocator, text));
}
