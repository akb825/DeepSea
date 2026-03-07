/*
 * Copyright 2020-2026 Aaron Barany
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

#include <DeepSea/SceneVectorDraw/SceneVectorImage.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/VectorDraw/VectorImage.h>

const char* const dsSceneVectorImage_typeName = "VectorImage";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneVectorImage_type(void)
{
	return &resourceType;
}

dsSceneVectorImage* dsSceneVectorImage_create(
	dsAllocator* allocator, dsVectorImage* image, const dsVectorShaders* shaders)
{
	if (!allocator || !image || !shaders)
	{
		dsVectorImage_destroy(image);
		errno = EINVAL;
		return NULL;
	}

	dsSceneVectorImage* sceneImage = DS_ALLOCATE_OBJECT(allocator, dsSceneVectorImage);
	if (!sceneImage)
	{
		dsVectorImage_destroy(image);
		return NULL;
	}

	sceneImage->allocator = dsAllocator_keepPointer(allocator);
	sceneImage->image = image;
	sceneImage->shaders = shaders;
	return sceneImage;
}

bool dsSceneVectorImage_destroy(dsSceneVectorImage* image)
{
	if (!image)
		return true;

	if (!dsVectorImage_destroy(image->image))
		return false;

	if (image->allocator)
		DS_VERIFY(dsAllocator_free(image->allocator, image));
	return true;
}

dsCustomSceneResource* dsSceneVectorImage_createResource(
	dsAllocator* allocator, dsSceneVectorImage* image)
{
	if (!allocator || !image)
	{
		dsSceneVectorImage_destroy(image);
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
	{
		dsSceneVectorImage_destroy(image);
		return NULL;
	}

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = image;
	customResource->destroyFunc = (dsDestroyCustomSceneResourceFunction)&dsSceneVectorImage_destroy;
	return customResource;
}
