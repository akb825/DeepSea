/*
 * Copyright 2020-2024 Aaron Barany
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

#include <DeepSea/SceneLighting/InstanceForwardLightData.h>

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <string.h>

static dsShaderVariableElement baseElements[] =
{
	{"positionAndType", dsMaterialType_Vec4, 0},
	{"directionAndLinearFalloff", dsMaterialType_Vec4, 0},
	{"colorAndQuadraticFalloff", dsMaterialType_Vec4, 0},
	{"spotCosAngles", dsMaterialType_Vec2, 0},
	{"ambientColorHasMain", dsMaterialType_Vec4, 0}
};

static bool isLightDescValid(const dsShaderVariableGroupDesc* lightDesc)
{
	if (lightDesc->elementCount != DS_ARRAY_SIZE(baseElements))
		return false;

	uint32_t lightCount = lightDesc->elements[0].count;
	if (lightCount == 0)
		return false;

	for (uint32_t i = 0; i < lightDesc->elementCount; ++i)
	{
		const dsShaderVariableElement* baseElement = baseElements + i;
		const dsShaderVariableElement* element = lightDesc->elements + i;
		if (strcmp(element->name, baseElement->name) != 0 || element->type != baseElement->type ||
			(i != lightDesc->elementCount - 1 && element->count != lightCount))
		{
			return false;
		}
	}

	return true;
}

const char* const dsInstanceForwardLightData_typeName = "InstanceForwardLightData";

dsShaderVariableGroupDesc* dsInstanceForwardLightData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator, uint32_t lightCount)
{
	if (!resourceManager || lightCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	const uint32_t elementCount = DS_ARRAY_SIZE(baseElements);
	dsShaderVariableElement elements[DS_ARRAY_SIZE(baseElements)];
	for (uint32_t i = 0; i < elementCount; ++i)
	{
		elements[i] = baseElements[i];
		if (i < elementCount - 1)
			elements[i].count = lightCount;
	}

	return dsShaderVariableGroupDesc_create(resourceManager, allocator, elements, elementCount);
}

static void dsInstanceForwardLightData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_ASSERT(dataDesc->elementCount == DS_ARRAY_SIZE(baseElements));
	const dsSceneLightSet* lightSet = (const dsSceneLightSet*)userData;
	uint32_t lightCount = dataDesc->elements[0].count;
	DS_ASSERT(lightCount > 0);

	size_t size = 0;
	size_t positionAndTypeOffset = dsMaterialType_addElementBlockSize(&size, dsMaterialType_Vec4,
		lightCount);
	size_t directionAndLinearFalloffOffset = dsMaterialType_addElementBlockSize(&size,
		dsMaterialType_Vec4, lightCount);
	size_t colorAndQuadraticFalloffOffset = dsMaterialType_addElementBlockSize(&size,
		dsMaterialType_Vec4, lightCount);
	size_t spotCosAnglesOffset = dsMaterialType_addElementBlockSize(&size, dsMaterialType_Vec2,
		lightCount);
	size_t ambientColorHasMainOffset = dsMaterialType_addElementBlockSize(&size,
		dsMaterialType_Vec4, 0);
	DS_ASSERT(size <= stride);

	dsColor3f ambient;
	DS_VERIFY(dsSceneLightSet_getAmbient(&ambient, lightSet));

	const dsSceneLight** brightestLights =
		DS_ALLOCATE_STACK_OBJECT_ARRAY(const dsSceneLight*, lightCount);
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* transform = &instances[i]->transform;
		// Make sure any unset lights are 0 initialized.
		memset(data, 0, stride);
		dsVector4f* positionAndType = (dsVector4f*)(data + positionAndTypeOffset);
		dsVector4f* directionAndLinearFalloff =
			(dsVector4f*)(data + directionAndLinearFalloffOffset);
		dsVector4f* colorAndQuadraticFalloff = (dsVector4f*)(data + colorAndQuadraticFalloffOffset);
		// Due to padding, spotCosAngles has a stride of vec4.
		dsVector4f* spotCosAngles = (dsVector4f*)(data + spotCosAnglesOffset);
		dsColor4f* ambientColorHasMain = (dsColor4f*)(data + ambientColorHasMainOffset);

		const dsVector3f* position = (const dsVector3f*)(transform->columns + 3);
		bool hasMainLight = false;
		uint32_t brightestLightCount = dsSceneLightSet_findBrightestLights(brightestLights,
			lightCount, &hasMainLight, lightSet, position);
		for (uint32_t j = 0; j < brightestLightCount; ++j)
		{
			const dsSceneLight* light = brightestLights[j];
			DS_ASSERT(light);
			dsVector4f tempVec = {{light->position.x, light->position.y, light->position.z, 1.0}};
			dsMatrix44f_transform(positionAndType + j, &view->viewMatrix, &tempVec);
			positionAndType[j].w = (float)(light->type + 1);

			tempVec.x = -light->direction.x;
			tempVec.y = -light->direction.y;
			tempVec.z = -light->direction.z;
			tempVec.w = 0.0f;
			dsMatrix44f_transform(directionAndLinearFalloff + j, &view->viewMatrix, &tempVec);
			directionAndLinearFalloff[j].w = light->linearFalloff;

			colorAndQuadraticFalloff[j].r = light->color.r*light->intensity;
			colorAndQuadraticFalloff[j].g = light->color.g*light->intensity;
			colorAndQuadraticFalloff[j].b = light->color.b*light->intensity;
			colorAndQuadraticFalloff[j].w = light->quadraticFalloff;

			spotCosAngles[j].x = light->innerSpotCosAngle;
			spotCosAngles[j].y = light->outerSpotCosAngle;
			spotCosAngles[j].z = 0.0f;
			spotCosAngles[j].w = 0.0f;
		}

		*(dsColor3f*)ambientColorHasMain = ambient;
		ambientColorHasMain->a = hasMainLight;
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

dsSceneInstanceData* dsInstanceForwardLightData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* lightDesc, const dsSceneLightSet* lightSet)
{
	if (!allocator || !lightDesc || !lightSet)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!isLightDescValid(lightDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Instance forward light data's shader variable group description must have been "
			"created with dsInstanceTransformData_createShaderVariableGroupDesc().");
		return NULL;
	}

	return dsSceneInstanceVariables_create(allocator, resourceAllocator, resourceManager, lightDesc,
		dsUniqueNameID_create(dsInstanceForwardLightData_typeName),
		&dsInstanceForwardLightData_populateData, (void*)lightSet, NULL);
}
