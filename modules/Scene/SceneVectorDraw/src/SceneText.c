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

#include <DeepSea/SceneVectorDraw/SceneText.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Scene/SceneResources.h>

#include <DeepSea/Text/Text.h>
#include <DeepSea/Text/TextSubstitutionTable.h>

#include <string.h>

typedef struct SubstituteAllUserData
{
	const dsTextSubstitutionTable* substitutionTable;
	dsTextSubstitutionData* substitutionData;
	bool success;
} SubstituteAllUserData;

static bool destroySceneText(void* text)
{
	dsSceneText_destroy((dsSceneText*)text);
	return true;
}

static bool resubstituteTextVisitor(
	const char* name, void* resource, dsSceneResourceType type, void* userData)
{
	DS_UNUSED(name);
	SubstituteAllUserData* substituteData = (SubstituteAllUserData*)userData;
	if (type != dsSceneResourceType_Custom)
		return true;

	dsCustomSceneResource* customResource = (dsCustomSceneResource*)resource;
	if (customResource->type != dsSceneText_type())
		return true;

	dsSceneText* sceneText = (dsSceneText*)customResource->resource;
	if (!dsSceneText_resubstitute(
			sceneText, substituteData->substitutionTable, substituteData->substitutionData))
	{
		substituteData->success = false;
		return false;
	}
	return true;
}

bool dsSceneText_resubstituteAll(const dsSceneResources* resources,
	const dsTextSubstitutionTable* substitutionTable, dsTextSubstitutionData* substitutionData)
{
	if (!resources || !substitutionTable || !substitutionData)
	{
		errno = EINVAL;
		return false;
	}

	SubstituteAllUserData substituteData = {substitutionTable, substitutionData, true};
	return dsSceneResources_forEachResource(resources, &resubstituteTextVisitor, &substituteData) &&
		substituteData.success;
}

const char* const dsSceneText_typeName = "Text";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneText_type(void)
{
	return &resourceType;
}

dsSceneText* dsSceneText_create(dsAllocator* allocator, dsFont* font, const char* string,
	const dsTextStyle* styles, uint32_t styleCount,
	const dsTextSubstitutionTable* substitutionTable, dsTextSubstitutionData* substitutionData)
{
	if (!allocator || !font || !string || (!styles && styleCount > 0) ||
		(substitutionTable && !substitutionData))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(
			DS_SCENE_VECTOR_DRAW_LOG_TAG, "Scene text allocator must support freeing memory.");
		return NULL;
	}
	bool needsSubstitution = substitutionTable && dsTextSubstitutionTable_needsSubstitution(string);

	size_t stringSize = 0;
	size_t styleSize = sizeof(dsTextStyle)*styleCount;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneText)) + DS_ALIGNED_SIZE(styleSize);
	if (needsSubstitution)
	{
		stringSize = strlen(string) + 1;
		fullSize += DS_ALIGNED_SIZE(stringSize) + DS_ALIGNED_SIZE(styleSize);
	}
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneText* sceneText = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneText);
	DS_ASSERT(sceneText);

	sceneText->allocator = allocator;
	sceneText->font = font;

	if (styleCount > 0)
	{
		sceneText->styles = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsTextStyle, styleCount);
		DS_ASSERT(sceneText->styles);
		memcpy(sceneText->styles, styles, styleSize);
	}
	else
		sceneText->styles = NULL;
	sceneText->styleCount = styleCount;

	if (needsSubstitution)
	{
		// Keep original copies of the string and styles and perform substitution.
		char* stringCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, stringSize);
		DS_ASSERT(stringCopy);
		memcpy(stringCopy, string, stringSize);
		sceneText->originalString = stringCopy;

		if (styleCount > 0)
		{
			dsTextStyle* stylesCopy = DS_ALLOCATE_OBJECT_ARRAY(
				&bufferAlloc, dsTextStyle, styleCount);
			DS_ASSERT(stylesCopy);
			memcpy(stylesCopy, styles, sizeof(dsTextStyle)*styleCount);
			sceneText->originalStyles = stylesCopy;
		}
		else
			sceneText->originalStyles = NULL;

		string = dsTextSubstitutionTable_substitute(
			substitutionTable, substitutionData, string, sceneText->styles, sceneText->styleCount);
		if (!string)
		{
			DS_VERIFY(dsAllocator_free(allocator, sceneText));
			return NULL;
		}
	}
	else
	{
		sceneText->originalString = NULL;
		sceneText->originalStyles = NULL;
	}

	sceneText->text = dsText_create(font, allocator, string, dsUnicodeType_UTF8, false);
	if (!sceneText->text)
	{
		DS_VERIFY(dsAllocator_free(allocator, sceneText));
		return NULL;
	}

	sceneText->textVersion = 0;
	return sceneText;
}

bool dsSceneText_resubstitute(dsSceneText* text,
	const dsTextSubstitutionTable* substitutionTable, dsTextSubstitutionData* substitutionData)
{
	if (!text || !substitutionTable || !substitutionData)
	{
		errno = EINVAL;
		return false;
	}

	if (!text->originalString)
		return true;

	// Operate on a temporary stack array of styles to avoid corrupting the styles on failure.
	dsTextStyle* tempStyles = NULL;
	size_t styleSize = 0;
	if (text->originalStyles)
	{
		tempStyles = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsTextStyle, text->styleCount);
		styleSize = sizeof(dsTextStyle)*text->styleCount;
		memcpy(tempStyles, text->originalStyles, styleSize);
	}

	const char* string = dsTextSubstitutionTable_substitute(
		substitutionTable, substitutionData, text->originalString, tempStyles, text->styleCount);
	if (!string)
		return false;

	dsText* newText = dsText_create(text->font, text->allocator, string, dsUnicodeType_UTF8, false);
	if (!newText)
		return false;

	dsText_destroy(text->text);
	text->text = newText;
	if (tempStyles)
		memcpy(text->styles, tempStyles, styleSize);
	++text->textVersion;
	return true;
}

void dsSceneText_destroy(dsSceneText* text)
{
	if (!text)
		return;

	dsText_destroy(text->text);
	DS_VERIFY(dsAllocator_free(text->allocator, text));
}

dsCustomSceneResource* dsSceneText_createResource(dsAllocator* allocator, dsSceneText* text)
{
	if (!allocator || !text)
	{
		dsSceneText_destroy(text);
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
	{
		dsSceneText_destroy(text);
		return NULL;
	}

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = text;
	customResource->destroyFunc = &destroySceneText;
	return customResource;
}
