/*
 * Copyright 2023-2025 Aaron Barany
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

#include <DeepSea/Physics/RigidBody.h>

#include "RigidBodyLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>
#include <DeepSea/Physics/RigidBodyInit.h>

#include <string.h>

#define MAX_STACK_MASS_PROPERTIES 256
#define SCALE_EPSILON 1e-5f

static bool hasMassProperties(const dsRigidBody* rigidBody)
{
	return rigidBody->motionType == dsPhysicsMotionType_Dynamic ||
		(rigidBody->flags & dsRigidBodyFlags_MutableMotionType);
}

// For some reason GCC (at least with 13.2) complains that massPropertiesPtrs is maybe
// uninitialized, despite very clearly being assigned in all code paths, even if explicitly
// initializing to NULL on declaration. Only option is to disable the warning for the function.
#if DS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

static bool computeDefaultMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsRigidBody* rigidBody)
{
	dsAllocator* scratchAllocator = ((const dsPhysicsActor*)rigidBody)->engine->allocator;
	bool heapMassProperties = rigidBody->shapeCount > MAX_STACK_MASS_PROPERTIES;
	dsPhysicsMassProperties* shapeMassProperties;
	const dsPhysicsMassProperties** massPropertiesPtrs;
	if (heapMassProperties)
	{
		shapeMassProperties = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsPhysicsMassProperties,
			rigidBody->shapeCount);
		if (!shapeMassProperties)
			return false;

		massPropertiesPtrs = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator,
			const dsPhysicsMassProperties*,  rigidBody->shapeCount);
		if (!massPropertiesPtrs)
		{
			DS_VERIFY(dsAllocator_free(scratchAllocator, (void*)massPropertiesPtrs));
			return false;
		}
	}
	else
	{
		shapeMassProperties =
			DS_ALLOCATE_STACK_OBJECT_ARRAY(dsPhysicsMassProperties, rigidBody->shapeCount);
		massPropertiesPtrs =
			DS_ALLOCATE_STACK_OBJECT_ARRAY(const dsPhysicsMassProperties*, rigidBody->shapeCount);
	}

	for (uint32_t i = 0; i < rigidBody->shapeCount; ++i)
	{
		dsPhysicsMassProperties* thisMassProperties = shapeMassProperties + 1;
		massPropertiesPtrs[i] = thisMassProperties;
		const dsPhysicsShapeInstance* shape = rigidBody->shapes + i;
		if (!dsPhysicsShape_getMassProperties(thisMassProperties, shape->shape, shape->density))
		{
			if (heapMassProperties)
			{
				DS_VERIFY(dsAllocator_free(scratchAllocator, shapeMassProperties));
				DS_VERIFY(dsAllocator_free(scratchAllocator, (void*)massPropertiesPtrs));
			}
			return false;
		}

		DS_VERIFY(dsPhysicsMassProperties_transform(thisMassProperties,
			shape->hasTranslate ? &shape->translate : NULL,
			shape->hasRotate ? &shape->rotate : NULL, shape->hasScale ? &shape->scale : NULL));
	}

	DS_VERIFY(dsPhysicsMassProperties_initializeCombined(outMassProperties, massPropertiesPtrs,
		rigidBody->shapeCount));

	if (heapMassProperties)
	{
		DS_VERIFY(dsAllocator_free(scratchAllocator, shapeMassProperties));
		DS_VERIFY(dsAllocator_free(scratchAllocator, (void*)massPropertiesPtrs));
	}
	return true;
}

#if DS_GCC
#pragma GCC diagnostic pop
#endif

inline static bool getShapeMaterial(dsPhysicsShapePartMaterial* outMaterial,
	const dsRigidBody* rigidBody, const dsPhysicsShapeInstance* shapeInstance, uint32_t faceIndex)
{
	const dsPhysicsShape* shape = shapeInstance->shape;
	const dsPhysicsShapeType* type = shape->type;
	if (type->getMassPropertiesFunc)
	{
		if (type->getMaterialFunc(outMaterial, shape, faceIndex))
			return true;
		else if (errno != EPERM)
			return false;
	}

	if (shapeInstance->hasMaterial)
		memcpy(outMaterial, &shapeInstance->material, sizeof(dsPhysicsShapePartMaterial));
	else
	{
		outMaterial->friction = rigidBody->friction;
		outMaterial->restitution = rigidBody->restitution;
		outMaterial->hardness = rigidBody->hardness;
	}
	return true;
}

bool dsRigidBody_extractTransformFromMatrix(dsVector3f* outPosition, dsQuaternion4f* outOrientation,
	dsVector3f* outScale, bool* outHasScale, const dsMatrix44f* transform, dsRigidBodyFlags flags,
	const dsPhysicsShapeInstance* shapes, uint32_t shapeCount)
{
	if (!outPosition || !outOrientation || !outScale || !outHasScale || !transform ||
		(!shapes && shapeCount > 0))
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix44f_decomposeTransform(outPosition, outOrientation, outScale, transform);

	dsVector3f one = {{1.0f, 1.0f, 1.0f}};
	bool unitScale = dsVector3f_epsilonEqual(outScale, &one, SCALE_EPSILON);
	bool scalable = (flags & dsRigidBodyFlags_Scalable) != 0;
	if (unitScale)
		*outScale = one; // Avoid unit scales that are slightly off.
	else
	{
		if (!scalable)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Rigid body must have scalable flag set to modify the scale.");
			errno = EPERM;
			return false;
		}

		if (dsEpsilonEqualf(outScale->x, outScale->y, SCALE_EPSILON) &&
			dsEpsilonEqualf(outScale->x, outScale->z, SCALE_EPSILON))
		{
			// Avoid uniform scales that are slightly off.
			outScale->y = outScale->z = outScale->x;
		}
		else
		{
			for (uint32_t i = 0; i < shapeCount; ++i)
			{
				const dsPhysicsShapeInstance* shapeInstance = shapes + i;
				if (shapeInstance->hasRotate)
				{
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
						"Attempting to set non-uniform scale on rigid body with a rotated shape.");
					errno = EPERM;
					return false;
				}

				if (shapeInstance->shape->type->uniformScaleOnly)
				{
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
						"Attempting to set non-uniform scale on rigid body with a shape that "
						"requires uniform scales.");
					errno = EPERM;
					return false;
				}
			}
		}
	}
	return true;
}

dsRigidBody* dsRigidBody_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsRigidBodyInit* initParams)
{
	if (!engine || !engine->createRigidBodyFunc || !engine->destroyRigidBodyFunc ||
		!dsRigidBodyInit_isValid(initParams))
	{
		if (initParams && initParams->destroyUserDataFunc)
			initParams->destroyUserDataFunc(initParams->userData);
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	if (!allocator->freeFunc)
	{
		if (initParams && initParams->destroyUserDataFunc)
			initParams->destroyUserDataFunc(initParams->userData);
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Rigid body allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	return engine->createRigidBodyFunc(engine, allocator, initParams);
}

dsRigidBody* dsRigidBody_loadFile(dsPhysicsEngine* engine, dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindRigidBodyGroupFunction findRigidBodyGroupFunc, void* findRigidBodyGroupUserData,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const char* filePath)
{
	if (!engine || !filePath)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open rigid body file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, engine->allocator);
	dsFileStream_close(&stream);
	if (!buffer)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsRigidBody* rigidBody = dsRigidBody_loadImpl(engine, allocator, userData, destroyUserDataFunc,
		canCollisionGroupsCollideFunc, findRigidBodyGroupFunc, findRigidBodyGroupUserData,
		findShapeFunc, findShapeUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return rigidBody;
}

dsRigidBody* dsRigidBody_loadResource(dsPhysicsEngine* engine, dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindRigidBodyGroupFunction findRigidBodyGroupFunc, void* findRigidBodyGroupUserData,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, dsFileResourceType type,
	const char* filePath)
{
	if (!engine || !filePath)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open rigid body file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, engine->allocator);
	dsResourceStream_close(&stream);
	if (!buffer)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsRigidBody* rigidBody = dsRigidBody_loadImpl(engine, allocator, userData, destroyUserDataFunc,
		canCollisionGroupsCollideFunc, findRigidBodyGroupFunc, findRigidBodyGroupUserData,
		findShapeFunc, findShapeUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return rigidBody;
}

dsRigidBody* dsRigidBody_loadArchive(dsPhysicsEngine* engine, dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindRigidBodyGroupFunction findRigidBodyGroupFunc, void* findRigidBodyGroupUserData,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const dsFileArchive* archive,
	const char* filePath)
{
	if (!engine ||!archive || !filePath)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open rigid body file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, engine->allocator);
	dsStream_close(stream);
	if (!buffer)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsRigidBody* rigidBody = dsRigidBody_loadImpl(engine, allocator, userData, destroyUserDataFunc,
		canCollisionGroupsCollideFunc, findRigidBodyGroupFunc, findRigidBodyGroupUserData,
		findShapeFunc, findShapeUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return rigidBody;
}

dsRigidBody* dsRigidBody_loadStream(dsPhysicsEngine* engine, dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindRigidBodyGroupFunction findRigidBodyGroupFunc, void* findRigidBodyGroupUserData,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, dsStream* stream)
{
	if (!engine || !stream)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, engine->allocator);
	if (!buffer)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsRigidBody* rigidBody = dsRigidBody_loadImpl(engine, allocator, userData, destroyUserDataFunc,
		canCollisionGroupsCollideFunc, findRigidBodyGroupFunc, findRigidBodyGroupUserData,
		findShapeFunc, findShapeUserData, buffer, size, NULL);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return rigidBody;
}

dsRigidBody* dsRigidBody_loadData(dsPhysicsEngine* engine, dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindRigidBodyGroupFunction findRigidBodyGroupFunc, void* findRigidBodyGroupUserData,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const void* data,
	size_t size)
{
	if (!engine || !data || size == 0)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	dsRigidBody* rigidBody = dsRigidBody_loadImpl(engine, allocator, userData, destroyUserDataFunc,
		canCollisionGroupsCollideFunc, findRigidBodyGroupUserData, findRigidBodyGroupUserData,
		findShapeFunc, findShapeUserData, data, size, NULL);
	return rigidBody;
}

uint32_t dsRigidBody_addShape(dsRigidBody* rigidBody, dsPhysicsShape* shape,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale,
	float density, const dsPhysicsShapePartMaterial* material)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->addRigidBodyShapeFunc || !shape ||
		!shape->type || (scale && (scale->x == 0.0f || scale->y == 0.0f || scale->z == 0.0f)) ||
		(density <= 0 && hasMassProperties(rigidBody)) || (material && (material->friction < 0.0f ||
			material->restitution < 0.0f || material->restitution > 1.0f ||
			material->hardness < 0.0f || material->hardness > 1.0f)))
	{
		errno = EINVAL;
		return DS_INVALID_PHYSICS_ID;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot add a shape to a rigid body with finalized shapes "
			"unless mutable shape flag is set.");
		errno = EPERM;
		return DS_INVALID_PHYSICS_ID;
	}

	if (shape->type->staticBodiesOnly && (rigidBody->motionType != dsPhysicsMotionType_Static ||
		(rigidBody->flags & dsRigidBodyFlags_MutableMotionType)))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot add static-only shape to a rigid body that isn't "
			"static or has the mutable motion type flag set.");
		errno = EPERM;
		return DS_INVALID_PHYSICS_ID;
	}

	if (scale && shape->type->uniformScaleOnly && (scale->x != scale->y || scale->x != scale->z))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Attempting to set non-uniform scale a shape that "
			"requires uniform scaling.");
		errno = EPERM;
		return DS_INVALID_PHYSICS_ID;
	}

	if (rotate &&
		(rigidBody->scale.x != rigidBody->scale.y || rigidBody->scale.x != rigidBody->scale.z))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Attempting to set rotation for a shape on a rigid body with non-uniform scale.");
		errno = EPERM;
		return DS_INVALID_PHYSICS_ID;
	}

	dsPhysicsEngine* engine = actor->engine;
	uint32_t shapeID = engine->addRigidBodyShapeFunc(engine, rigidBody, shape, translate, rotate,
		scale, density, material);
	if (shapeID != DS_INVALID_PHYSICS_ID)
		rigidBody->shapesFinalized = false;
	return shapeID;
}

bool dsRigidBody_setShapeTransformID(dsRigidBody* rigidBody, uint32_t shapeID,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyShapeTransformFunc ||
		(scale && (scale->x == 0.0f || scale->y == 0.0f || scale->z == 0.0f)))
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot set a shape transform on a rigid body with "
			"finalized shapes unless mutable shape flag is set.");
		errno = EPERM;
		return false;
	}

	uint32_t index = 0;
	const dsPhysicsShapeInstance* shape = NULL;
	for (uint32_t i = 0; i < rigidBody->shapeCount; ++i)
	{
		if (rigidBody->shapes[i].id == shapeID)
		{
			index = i;
			shape = rigidBody->shapes + i;
			break;
		}
	}

	if (!shape)
	{
		errno = ENOTFOUND;
		return false;
	}

	if ((!shape->hasTranslate && translate) || (!shape->hasRotate && rotate) ||
		(!shape->hasScale && scale))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot set a shape transform element that was previously "
			"NULL when adding to the rigid body.");
		errno = EPERM;
		return false;
	}

	if (scale && shape->shape->type->uniformScaleOnly &&
		(scale->x != scale->y || scale->x != scale->z))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Attempting to set non-uniform scale a shape that "
			"requires uniform scaling.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	bool success = engine->setRigidBodyShapeTransformFunc(
		engine, rigidBody, index, translate, rotate, scale);
	if (success)
		rigidBody->shapesFinalized = false;
	return success;
}

bool dsRigidBody_setShapeTransformIndex(dsRigidBody* rigidBody, uint32_t shapeIndex,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyShapeTransformFunc ||
		(scale && (scale->x == 0.0f || scale->y == 0.0f || scale->z == 0.0f)))
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot set a shape transform on a rigid body with "
			"finalized shapes unless mutable shape flag is set.");
		errno = EPERM;
		return false;
	}

	if (shapeIndex >= rigidBody->shapeCount)
	{
		errno = EINDEX;
		return false;
	}

	const dsPhysicsShapeInstance* shape = rigidBody->shapes + shapeIndex;
	if ((!shape->hasTranslate && translate) || (!shape->hasRotate && rotate) ||
		(!shape->hasScale && scale))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot set a shape transform element that was previously "
			"NULL when adding to the rigid body.");
		errno = EPERM;
		return false;
	}

	if (scale && shape->shape->type->uniformScaleOnly &&
		(scale->x != scale->y || scale->x != scale->z))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Attempting to set non-uniform scale a shape that "
			"requires uniform scaling.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	bool success = engine->setRigidBodyShapeTransformFunc(
		engine, rigidBody, shapeIndex, translate, rotate, scale);
	if (success)
		rigidBody->shapesFinalized = false;
	return success;
}

bool dsRigidBody_setShapeMaterialID(dsRigidBody* rigidBody, uint32_t shapeID,
	const dsPhysicsShapePartMaterial* material)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyShapeMaterialFunc ||
		(material && (material->friction < 0.0f || material->restitution < 0.0f ||
			material->restitution > 1.0f || material->hardness < 0.0f ||
			material->hardness > 1.0f)))
	{
		errno = EINVAL;
		return false;
	}

	uint32_t index = 0;
	const dsPhysicsShapeInstance* shape = NULL;
	for (uint32_t i = 0; i < rigidBody->shapeCount; ++i)
	{
		if (rigidBody->shapes[i].id == shapeID)
		{
			index = i;
			shape = rigidBody->shapes + i;
			break;
		}
	}

	if (!shape)
	{
		errno = ENOTFOUND;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyShapeMaterialFunc(engine, rigidBody, index, material);
}

bool dsRigidBody_setShapeMaterialIndex(dsRigidBody* rigidBody, uint32_t shapeIndex,
	const dsPhysicsShapePartMaterial* material)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyShapeMaterialFunc ||
		(material && (material->friction < 0.0f || material->restitution < 0.0f ||
			material->restitution > 1.0f || material->hardness < 0.0f ||
			material->hardness > 1.0f)))
	{
		errno = EINVAL;
		return false;
	}

	if (shapeIndex >= rigidBody->shapeCount)
	{
		errno = EINDEX;
		return false;
	}
	{
		errno = ENOTFOUND;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyShapeMaterialFunc(engine, rigidBody, shapeIndex, material);
}

bool dsRigidBody_removeShapeID(dsRigidBody* rigidBody, uint32_t shapeID)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->removeRigidBodyShapeFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot remove a shape from a rigid body with finalized "
			"shapes unless mutable shape flag is set.");
		errno = EPERM;
		return false;
	}

	uint32_t index = DS_INVALID_PHYSICS_ID;
	for (uint32_t i = 0; i < rigidBody->shapeCount; ++i)
	{
		if (rigidBody->shapes[i].id == shapeID)
		{
			index = i;
			break;
		}
	}

	if (index == DS_INVALID_PHYSICS_ID)
	{
		errno = ENOTFOUND;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	bool success = engine->removeRigidBodyShapeFunc(engine, rigidBody, index);
	if (success)
		rigidBody->shapesFinalized = false;
	return success;
}

bool dsRigidBody_removeShapeIndex(dsRigidBody* rigidBody, uint32_t shapeIndex)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->removeRigidBodyShapeFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot remove a shape from a rigid body with finalized "
			"shapes unless mutable shape flag is set.");
		errno = EPERM;
		return false;
	}

	if (shapeIndex >= rigidBody->shapeCount)
	{
		errno = ENOTFOUND;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	bool success = engine->removeRigidBodyShapeFunc(engine, rigidBody, shapeIndex);
	if (success)
		rigidBody->shapesFinalized = false;
	return success;
}

bool dsRigidBody_computeDefaultMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsRigidBody* rigidBody)
{
	if (!outMassProperties || !rigidBody)
	{
		errno = EINVAL;
		return false;
	}

	if (!hasMassProperties(rigidBody))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot compute the default mass properties for a rigid "
			"body that isn't dynamic or have the mutable motion type flag set.");
		errno = EPERM;
		return false;
	}

	return computeDefaultMassProperties(outMassProperties, rigidBody);
}

bool dsRigidBody_finalizeShapes(dsRigidBody* rigidBody, const float* mass,
	const dsVector3f* rotationPointShift)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->finalizeRigidBodyShapesFunc ||
		(mass && mass <= 0))
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot finalize shapes on a rigid body with already "
			"finalized shapes unless mutable shape flag is set.");
		errno = EPERM;
		return false;
	}

	if (rigidBody->shapeCount == 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Rigid body must have at least one shape added before finalizing the shapes.");
		errno = EPERM;
		return false;
	}

	dsPhysicsMassProperties massProperties;
	if (hasMassProperties(rigidBody))
	{
		if (!computeDefaultMassProperties(&massProperties, rigidBody))
			return false;

		if (mass)
			DS_VERIFY(dsPhysicsMassProperties_setMass(&massProperties, *mass));
		if (rotationPointShift)
			DS_VERIFY(dsPhysicsMassProperties_shift(&massProperties, rotationPointShift, NULL));
	}
	else
		DS_VERIFY(dsPhysicsMassProperties_initializeEmpty(&massProperties));

	dsPhysicsEngine* engine = actor->engine;
	bool success = engine->finalizeRigidBodyShapesFunc(engine, rigidBody, &massProperties);
	if (success)
		rigidBody->shapesFinalized = true;
	return success;
}

bool dsRigidBody_finalizeShapesCustomMassProperties(dsRigidBody* rigidBody,
	const dsPhysicsMassProperties* massProperties)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->finalizeRigidBodyShapesFunc ||
		!massProperties)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot finalize shapes on a rigid body with already "
			"finalized shapes unless mutable shape flag is set.");
		errno = EPERM;
		return false;
	}

	if (rigidBody->shapeCount == 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Rigid body must have at least one shape added before finalizing the shapes.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	bool success = engine->finalizeRigidBodyShapesFunc(engine, rigidBody, massProperties);
	if (success)
		rigidBody->shapesFinalized = true;
	return success;
}

bool dsRigidBody_getShapeMaterialID(dsPhysicsShapePartMaterial* outMaterial,
	const dsRigidBody* rigidBody, uint32_t shapeID, uint32_t faceIndex)
{
	if (!outMaterial || !rigidBody)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < rigidBody->shapeCount; ++i)
	{
		const dsPhysicsShapeInstance* shape = rigidBody->shapes + i;
		if (shape->id == shapeID)
			return getShapeMaterial(outMaterial, rigidBody, shape, faceIndex);
	}

	errno = ENOTFOUND;
	return false;
}

bool dsRigidBody_getShapeMaterialIndex(dsPhysicsShapePartMaterial* outMaterial,
	const dsRigidBody* rigidBody, uint32_t shapeIndex, uint32_t faceIndex)
{
	if (!outMaterial || !rigidBody)
	{
		errno = EINVAL;
		return false;
	}

	if (faceIndex >= rigidBody->shapeCount)
	{
		errno = EINDEX;
		return false;
	}

	return getShapeMaterial(outMaterial, rigidBody, rigidBody->shapes + shapeIndex, faceIndex);
}

bool dsRigidBody_addFlags(dsRigidBody* rigidBody, dsRigidBodyFlags flags)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyFlagsFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (flags & dsRigidBodyFlags_MutableMotionType)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Rigid body mutable motion type flag may not be changed after creation.");
		errno = EPERM;
		return false;
	}

	if (flags & dsRigidBodyFlags_MutableShape)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Rigid body mutable shape flag may not be changed after creation.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyFlagsFunc(engine, rigidBody, rigidBody->flags | flags);
}

bool dsRigidBody_removeFlags(dsRigidBody* rigidBody, dsRigidBodyFlags flags)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyFlagsFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (flags & dsRigidBodyFlags_MutableMotionType)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Rigid body mutable motion type flag may not be changed after creation.");
		errno = EPERM;
		return false;
	}

	if (flags & dsRigidBodyFlags_MutableShape)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Rigid body mutable shape flag may not be changed after creation.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyFlagsFunc(engine, rigidBody, rigidBody->flags & (~flags));
}

bool dsRigidBody_setMotionType(dsRigidBody* rigidBody, dsPhysicsMotionType motionType)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyMotionTypeFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!(rigidBody->flags & dsRigidBodyFlags_MutableMotionType))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Can't set rigid body motion type without the mutable motion type flag set.");
		errno = EPERM;
		return false;
	}

	if (rigidBody->motionType == motionType)
		return true;

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyMotionTypeFunc(engine, rigidBody, motionType);
}

bool dsRigidBody_setDOFMask(dsRigidBody* rigidBody, dsPhysicsDOFMask dofMask)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyDOFMaskFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyDOFMaskFunc(engine, rigidBody, dofMask);
}

bool dsRigidBody_setCollisionGroup(dsRigidBody* rigidBody, uint64_t collisionGroup)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyCollisionGroupFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyCollisionGroupFunc(engine, rigidBody, collisionGroup);
}

bool dsRigidBody_setCanCollisionGroupsCollideFunction(dsRigidBody* rigidBody,
	dsCanCollisionGroupsCollideFunction canCollideFunc)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyCanCollisionGroupsCollideFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyCanCollisionGroupsCollideFunc(engine, rigidBody, canCollideFunc);
}

bool dsRigidBody_setTransform(dsRigidBody* rigidBody, const dsVector3f* position,
	const dsQuaternion4f* orientation, const dsVector3f* scale, bool activate)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyTransformFunc ||
		(scale && (scale->x == 0.0f || scale->y == 0.0f || scale->z == 0.0f)))
	{
		errno = EINVAL;
		return false;
	}

	if (scale)
	{
		if (!(rigidBody->flags & dsRigidBodyFlags_Scalable))
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Rigid body must have scalable flag set to modify the scale.");
			errno = EPERM;
			return false;
		}

		if (scale->y != scale->x || scale->z != scale->x)
		{
			for (uint32_t i = 0; i < rigidBody->shapeCount; ++i)
			{
				const dsPhysicsShapeInstance* shapeInstance = rigidBody->shapes + i;
				if (shapeInstance->hasRotate)
				{
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
						"Attempting to set non-uniform scale on rigid body with a rotated shape.");
					errno = EPERM;
					return false;
				}

				if (shapeInstance->shape->type->uniformScaleOnly)
				{
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
						"Attempting to set non-uniform scale on rigid body with a shape that "
						"requires uniform scales.");
					errno = EPERM;
					return false;
				}
			}
		}
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyTransformFunc(
		engine, rigidBody, position, orientation, scale, activate);
}

bool dsRigidBody_getTransformMatrix(dsMatrix44f* outTransform, const dsRigidBody* rigidBody)
{
	if (outTransform || !rigidBody)
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix44f translate;
	dsMatrix44f_composeTransform(
		&translate, &rigidBody->position, &rigidBody->orientation, &rigidBody->scale);
	return true;
}

bool dsRigidBody_setTransformMatrix(dsRigidBody* rigidBody, const dsMatrix44f* transform,
	bool activate)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyTransformFunc || !transform)
	{
		errno = EINVAL;
		return false;
	}

	dsVector3f position, scale;
	dsQuaternion4f orientation;
	bool hasScale;
	if (!dsRigidBody_extractTransformFromMatrix(&position, &orientation, &scale, &hasScale,
			transform, rigidBody->flags, rigidBody->shapes, rigidBody->shapeCount))
	{
		return false;
	}

	// Check if scale has changed rather than any scale at all.
	hasScale = !dsVector3f_epsilonEqual(&scale, &rigidBody->scale, SCALE_EPSILON);
	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyTransformFunc(engine, rigidBody, &position, &orientation,
		hasScale ? &scale : NULL, activate);
}

bool dsRigidBody_setKinematicTarget(dsRigidBody* rigidBody, float time, const dsVector3f* position,
	const dsQuaternion4f* orientation)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyKinematicTargetFunc ||
		time < 0)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Kinematic || !actor->scene)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Kinematic target may only be set on a rigid body that's part of a physics scene.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyKinematicTargetFunc(engine, rigidBody, time, position, orientation);
}

bool dsRigidBody_setKinematicTargetMatrix(dsRigidBody* rigidBody, float time,
	const dsMatrix44f* transform)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyKinematicTargetFunc ||
		!actor->engine->setRigidBodyTransformFunc || time < 0 || !transform)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Kinematic || !actor->scene)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Kinematic target may only be set on a rigid body that's part of a physics scene.");
		errno = EPERM;
		return false;
	}

	dsVector3f position, scale;
	dsQuaternion4f orientation;
	bool hasScale;
	if (!dsRigidBody_extractTransformFromMatrix(&position, &orientation, &scale, &hasScale,
			transform, rigidBody->flags, rigidBody->shapes, rigidBody->shapeCount))
	{
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	if (!dsVector3f_epsilonEqual(&scale, &rigidBody->scale, SCALE_EPSILON))
		engine->setRigidBodyTransformFunc(engine, rigidBody, NULL, NULL, &scale, false);
	return engine->setRigidBodyKinematicTargetFunc(
		engine, rigidBody, time, &position, &orientation);
}

bool dsRigidBody_getWorldRotationPosition(dsVector3f* outPosition, const dsRigidBody* rigidBody)
{
	if (!outPosition || !rigidBody)
	{
		errno = EINVAL;
		return false;
	}

	if (!hasMassProperties(rigidBody))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot get the rotation position on a rigid body that "
			"isn't dynamic or have the mutable motion type set.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot rotation position on a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsVector3_mul(*outPosition, rigidBody->massProperties.inertiaTranslate, rigidBody->scale);
	dsQuaternion4f_rotate(outPosition, &rigidBody->orientation, outPosition);
	dsVector3_add(*outPosition, *outPosition, rigidBody->position);
	return true;
}

bool dsRigidBody_setMass(dsRigidBody* rigidBody, float mass)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyMassFunc || mass <= 0)
	{
		errno = EINVAL;
		return false;
	}

	if (!hasMassProperties(rigidBody))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot set the mass on a rigid body that isn't dynamic "
			"or have the mutable motion type set.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot set the mass on a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	bool success = engine->setRigidBodyMassFunc(engine, rigidBody, mass);
	if (success)
		DS_VERIFY(dsPhysicsMassProperties_setMass(&rigidBody->massProperties, mass));
	return success;
}

bool dsRigidBody_setFriction(dsRigidBody* rigidBody, float friction)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyFrictionFunc ||
		friction < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyFrictionFunc(engine, rigidBody, friction);
}

bool dsRigidBody_setRestitution(dsRigidBody* rigidBody, float restitution)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyRestitutionFunc ||
		restitution < 0 || restitution > 1)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyRestitutionFunc(engine, rigidBody, restitution);
}

bool dsRigidBody_setHardness(dsRigidBody* rigidBody, float hardness)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyHardnessFunc ||
		hardness < 0 || hardness > 1)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyHardnessFunc(engine, rigidBody, hardness);
}

bool dsRigidBody_setLinearDamping(dsRigidBody* rigidBody, float linearDamping)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyLinearDampingFunc ||
		linearDamping < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyLinearDampingFunc(engine, rigidBody, linearDamping);
}

bool dsRigidBody_setAngularDamping(dsRigidBody* rigidBody, float angularDamping)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyAngularDampingFunc ||
		angularDamping < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyAngularDampingFunc(engine, rigidBody, angularDamping);
}

bool dsRigidBody_setMaxLinearVelocity(dsRigidBody* rigidBody, float maxLinearVelocity)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyMaxLinearVelocityFunc ||
		maxLinearVelocity < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyMaxLinearVelocityFunc(engine, rigidBody, maxLinearVelocity);
}

bool dsRigidBody_setMaxAngularVelocity(dsRigidBody* rigidBody, float maxAngularVelocity)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyMaxAngularVelocityFunc ||
		maxAngularVelocity < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyMaxAngularVelocityFunc(engine, rigidBody, maxAngularVelocity);
}

bool dsRigidBody_getLinearVelocity(dsVector3f* outVelocity, const dsRigidBody* rigidBody)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!outVelocity || !rigidBody || !actor->engine ||
		!actor->engine->getRigidBodyLinearVelocityFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Velocities are only valid on a rigid body that is dynamic.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->getRigidBodyLinearVelocityFunc(outVelocity, engine, rigidBody);
}

bool dsRigidBody_setLinearVelocity(dsRigidBody* rigidBody, const dsVector3f* velocity)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyLinearVelocityFunc ||
		!velocity)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply velocities to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyLinearVelocityFunc(engine, rigidBody, velocity);
}

bool dsRigidBody_getAngularVelocity(dsVector3f* outVelocity, const dsRigidBody* rigidBody)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!outVelocity || !rigidBody || !actor->engine ||
		!actor->engine->getRigidBodyAngularVelocityFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Velocities are only valid on a rigid body that is dynamic.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->getRigidBodyAngularVelocityFunc(outVelocity, engine, rigidBody);
}

bool dsRigidBody_setAngularVelocity(dsRigidBody* rigidBody, const dsVector3f* velocity)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyAngularVelocityFunc ||
		!velocity)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply velocities to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyAngularVelocityFunc(engine, rigidBody, velocity);
}

bool dsRigidBody_addForce(dsRigidBody* rigidBody, const dsVector3f* force)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->addRigidBodyForceFunc || !force)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->addRigidBodyForceFunc(engine, rigidBody, force);
}

bool dsRigidBody_addForceAtPoint(dsRigidBody* rigidBody, const dsVector3f* force,
	const dsVector3f* point)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->addRigidBodyForceFunc ||
		!actor->engine->addRigidBodyTorqueFunc || !force || !point)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsVector3f rotationPos;
	dsVector3_mul(rotationPos, rigidBody->massProperties.inertiaTranslate, rigidBody->scale);
	dsQuaternion4f_rotate(&rotationPos, &rigidBody->orientation, &rotationPos);
	dsVector3_add(rotationPos, rotationPos, rigidBody->position);

	dsVector3f relativePos, torque;
	dsVector3_sub(relativePos, *point, rotationPos);
	dsVector3_cross(torque, relativePos, *force);

	dsPhysicsEngine* engine = actor->engine;
	return engine->addRigidBodyForceFunc(engine, rigidBody, force) &&
		engine->addRigidBodyTorqueFunc(engine, rigidBody, &torque);
}

bool dsRigidBody_clearForce(dsRigidBody* rigidBody)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->clearRigidBodyForceFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->clearRigidBodyForceFunc(engine, rigidBody);
}

bool dsRigidBody_addTorque(dsRigidBody* rigidBody, const dsVector3f* torque)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->addRigidBodyTorqueFunc || !torque)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->addRigidBodyTorqueFunc(engine, rigidBody, torque);
}

bool dsRigidBody_clearTorque(dsRigidBody* rigidBody)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->clearRigidBodyTorqueFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->clearRigidBodyTorqueFunc(engine, rigidBody);
}

bool dsRigidBody_addLinearImpulse(dsRigidBody* rigidBody, const dsVector3f* impulse)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->addRigidBodyLinearImpulseFunc || !impulse)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->addRigidBodyLinearImpulseFunc(engine, rigidBody, impulse);
}

bool dsRigidBody_clearLinearImpulse(dsRigidBody* rigidBody)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->clearRigidBodyLinearImpulseFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->clearRigidBodyLinearImpulseFunc(engine, rigidBody);
}

bool dsRigidBody_addAngularImpulse(dsRigidBody* rigidBody, const dsVector3f* impulse)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->addRigidBodyAngularImpulseFunc || !impulse)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->addRigidBodyAngularImpulseFunc(engine, rigidBody, impulse);
}

bool dsRigidBody_clearAngularImpulse(dsRigidBody* rigidBody)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->clearRigidBodyAngularImpulseFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBody->motionType != dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot apply forces to a rigid body that isn't dynamic.");
		errno = EPERM;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot apply forces to a rigid body that hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->clearRigidBodyAngularImpulseFunc(engine, rigidBody);
}

bool dsRigidBody_setActive(dsRigidBody* rigidBody, bool active)
{
	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	if (!rigidBody || !actor->engine || !actor->engine->setRigidBodyActiveFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!rigidBody->shapesFinalized)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot change the active state for a rigid body that "
			"hasn't had its shapes finalized.");
		errno = EPERM;
		return false;
	}

	dsPhysicsEngine* engine = actor->engine;
	return engine->setRigidBodyActiveFunc(engine, rigidBody, active);
}

bool dsRigidBody_destroy(dsRigidBody* rigidBody)
{
	if (!rigidBody)
		return true;

	dsPhysicsEngine* engine = ((dsPhysicsActor*)rigidBody)->engine;
	if (!engine || !engine->destroyRigidBodyFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroyRigidBodyFunc(engine, rigidBody);
}

void dsRigidBody_initialize(dsRigidBody* rigidBody, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsRigidBodyInit* initParams)
{
	DS_ASSERT(rigidBody);
	DS_ASSERT(engine);
	DS_ASSERT(allocator);
	DS_ASSERT(initParams);

	dsPhysicsActor* actor = (dsPhysicsActor*)rigidBody;
	actor->engine = engine;
	actor->allocator = allocator;
	actor->destroyUserDataFunc = initParams->destroyUserDataFunc;
	actor->scene = NULL;
	actor->type = dsPhysicsActorType_RigidBody;
	actor->layer = initParams->layer;
	actor->collisionGroup = initParams->collisionGroup;
	actor->canCollisionGroupsCollideFunc = initParams->canCollisionGroupsCollideFunc;
	actor->userData = initParams->userData;

	rigidBody->active = false;
	rigidBody->position = initParams->position;
	rigidBody->orientation = initParams->orientation;
	rigidBody->scale = initParams->scale;
	rigidBody->flags = initParams->flags;
	rigidBody->motionType = initParams->motionType;
	rigidBody->dofMask = initParams->dofMask;
	DS_VERIFY(dsPhysicsMassProperties_initializeEmpty(&rigidBody->massProperties));
	rigidBody->friction = initParams->friction;
	rigidBody->restitution = initParams->restitution;
	rigidBody->hardness = initParams->hardness;
	rigidBody->linearDamping = initParams->linearDamping;
	rigidBody->angularDamping = initParams->angularDamping;
	rigidBody->maxLinearVelocity = initParams->maxLinearVelocity;
	rigidBody->maxAngularVelocity = initParams->maxAngularVelocity;
	rigidBody->group = initParams->group;
	if (initParams->shapeCount > 0)
	{
		rigidBody->shapes =
			DS_ALLOCATE_OBJECT_ARRAY(allocator, dsPhysicsShapeInstance, initParams->shapeCount);
		if (rigidBody->shapes)
			rigidBody->maxShapes = initParams->shapeCount;
		else
			rigidBody->maxShapes = 0;
	}
	else
	{
		rigidBody->shapes = NULL;
		rigidBody->maxShapes = 0;
	}
	rigidBody->shapeCount = 0;
	rigidBody->shapesFinalized = false;
}
