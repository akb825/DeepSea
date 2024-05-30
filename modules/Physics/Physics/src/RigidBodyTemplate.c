/*
 * Copyright 2024 Aaron Barany
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

#include <DeepSea/Physics/RigidBodyTemplate.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Quaternion.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>
#include <DeepSea/Physics/RigidBody.h>
#include <DeepSea/Physics/Types.h>

#include <string.h>

#define MAX_STACK_MASS_PROPERTIES 256

static bool hasMassProperties(const dsRigidBodyTemplate* rigidBodyTemplate)
{
	return rigidBodyTemplate->motionType == dsPhysicsMotionType_Dynamic ||
		(rigidBodyTemplate->flags & dsRigidBodyFlags_MutableMotionType);
}

// For some reason GCC (at least with 13.2) complains that massPropertiesPtrs is maybe
// uninitialized, despite very clearly being assigned in all code paths, even if explicitly
// initializing to NULL on declaration. Only option is to disable the warning for the function.
#if DS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

static bool computeDefaultMassProperties(dsRigidBodyTemplate* rigidBodyTemplate)
{
	dsAllocator* scratchAllocator = rigidBodyTemplate->engine->allocator;
	bool heapMassProperties = rigidBodyTemplate->shapeCount > MAX_STACK_MASS_PROPERTIES;
	dsPhysicsMassProperties* shapeMassProperties;
	const dsPhysicsMassProperties** massPropertiesPtrs;
	if (heapMassProperties)
	{
		shapeMassProperties = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsPhysicsMassProperties,
			rigidBodyTemplate->shapeCount);
		if (!shapeMassProperties)
			return false;

		massPropertiesPtrs = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator,
			const dsPhysicsMassProperties*,  rigidBodyTemplate->shapeCount);
		if (!massPropertiesPtrs)
		{
			DS_VERIFY(dsAllocator_free(scratchAllocator, (void*)massPropertiesPtrs));
			return false;
		}
	}
	else
	{
		shapeMassProperties =
			DS_ALLOCATE_STACK_OBJECT_ARRAY(dsPhysicsMassProperties, rigidBodyTemplate->shapeCount);
		massPropertiesPtrs = DS_ALLOCATE_STACK_OBJECT_ARRAY(const dsPhysicsMassProperties*,
			rigidBodyTemplate->shapeCount);
	}

	for (uint32_t i = 0; i < rigidBodyTemplate->shapeCount; ++i)
	{
		dsPhysicsMassProperties* thisMassProperties = shapeMassProperties + 1;
		massPropertiesPtrs[i] = thisMassProperties;
		const dsPhysicsShapeInstance* shape = rigidBodyTemplate->shapes + i;
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

	DS_VERIFY(dsPhysicsMassProperties_initializeCombined(&rigidBodyTemplate->massProperties,
		massPropertiesPtrs, rigidBodyTemplate->shapeCount));

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

dsRigidBodyTemplate* dsRigidBodyTemplate_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsRigidBodyFlags flags, dsPhysicsMotionType motionType, dsPhysicsLayer layer,
	float friction, float restitution, float hardness, uint32_t shapeCount)
{
	if (!engine || friction < 0 || restitution < 0 || restitution > 1 || hardness < 0 ||
		hardness > 1)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Rigid body allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	dsRigidBodyTemplate* rigidBodyTemplate = DS_ALLOCATE_OBJECT(allocator, dsRigidBodyTemplate);
	if (!rigidBodyTemplate)
		return NULL;

	rigidBodyTemplate->engine = engine;
	rigidBodyTemplate->allocator = dsAllocator_keepPointer(allocator);
	rigidBodyTemplate->flags = flags;
	rigidBodyTemplate->motionType = motionType;
	rigidBodyTemplate->dofMask = dsPhysicsDOFMask_All;
	rigidBodyTemplate->layer = layer;
	rigidBodyTemplate->collisionGroup = 0;
	rigidBodyTemplate->canCollisionGroupsCollideFunc = NULL;
	DS_VERIFY(dsPhysicsMassProperties_initializeEmpty(&rigidBodyTemplate->massProperties));
	rigidBodyTemplate->friction = friction;
	rigidBodyTemplate->restitution = restitution;
	rigidBodyTemplate->hardness = hardness;
	rigidBodyTemplate->linearDamping = DS_DEFAULT_PHYSICS_DAMPING;
	rigidBodyTemplate->angularDamping = DS_DEFAULT_PHYSICS_DAMPING;
	rigidBodyTemplate->maxLinearVelocity = DS_DEFAULT_PHYSICS_MAX_LINEAR_VELOCITY;
	rigidBodyTemplate->maxAngularVelocity = DS_DEFAULT_PHYSICS_MAX_ANGULAR_VELOCITY;
	rigidBodyTemplate->shapes = NULL;
	rigidBodyTemplate->shapeCount = 0;
	rigidBodyTemplate->maxShapes = shapeCount;

	if (shapeCount > 0)
	{
		rigidBodyTemplate->shapes = DS_ALLOCATE_OBJECT_ARRAY(
			rigidBodyTemplate->allocator, dsPhysicsShapeInstance, shapeCount);
		if (!rigidBodyTemplate->shapes)
			DS_VERIFY(dsAllocator_free(allocator, rigidBodyTemplate));
		return NULL;
	}

	return rigidBodyTemplate;
}

bool dsRigidBodyTemplate_addShape(dsRigidBodyTemplate* rigidBodyTemplate, dsPhysicsShape* shape,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale,
	float density, const dsPhysicsShapePartMaterial* material)
{
	if (!rigidBodyTemplate || !shape || !shape->type || (scale && (scale->x == 0.0f ||
			scale->y == 0.0f || scale->z == 0.0f)) ||
		(density <= 0 && hasMassProperties(rigidBodyTemplate)) ||
		(material && (material->friction < 0.0f || material->restitution < 0.0f ||
			material->restitution > 1.0f || material->hardness < 0.0f ||
			material->hardness > 1.0f)))
	{
		errno = EINVAL;
		return false;
	}

	if (shape->type->staticBodiesOnly &&
		(rigidBodyTemplate->motionType != dsPhysicsMotionType_Static ||
		(rigidBodyTemplate->flags & dsRigidBodyFlags_MutableMotionType)))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot add static-only shape to a rigid body that isn't "
			"static or has the mutable motion type flag set.");
		errno = EPERM;
		return false;
	}

	if (scale && shape->type->uniformScaleOnly && (scale->x != scale->y || scale->x != scale->z))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Attempting to set non-uniform scale a shape that "
			"requires uniform scaling.");
		errno = EPERM;
		return false;
	}

	uint32_t index = rigidBodyTemplate->shapeCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(rigidBodyTemplate->allocator, rigidBodyTemplate->shapes,
			rigidBodyTemplate->shapeCount, rigidBodyTemplate->maxShapes, 1))
	{
		return false;
	}

	dsPhysicsShapeInstance* instance = rigidBodyTemplate->shapes + index;
	instance->shape = dsPhysicsShape_addRef(shape);
	instance->id = 0;
	instance->density = density;

	if (translate)
	{
		instance->hasTranslate = true;
		instance->translate = *translate;
	}
	else
	{
		instance->hasTranslate = false;
		memset(&instance->translate, 0, sizeof(instance->translate));
	}

	if (rotate)
	{
		instance->hasRotate = true;
		instance->rotate = *rotate;
	}
	else
	{
		instance->hasRotate = false;
		dsQuaternion4_identityRotation(instance->rotate);
	}

	if (scale)
	{
		instance->hasScale = true;
		instance->scale = *scale;
	}
	else
	{
		instance->hasScale = false;
		memset(&instance->scale, 0, sizeof(instance->scale));
	}

	if (material)
	{
		instance->hasMaterial = true;
		instance->material = *material;
	}
	else
	{
		instance->hasMaterial = false;
		instance->material.friction = instance->material.restitution =
			instance->material.hardness = 0.0f;
	}

	return true;
}

bool dsRigidBodyTemplate_finalizeShapes(dsRigidBodyTemplate* rigidBodyTemplate, const float* mass,
	const dsVector3f* rotationPointShift)
{
	if (!rigidBodyTemplate)
	{
		errno = EINVAL;
		return false;
	}

	if (rigidBodyTemplate->shapeCount == 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Rigid body template must have at least one shape added before finalizing the shapes.");
		errno = EPERM;
		return false;
	}

	if (hasMassProperties(rigidBodyTemplate))
	{
		if (!computeDefaultMassProperties(rigidBodyTemplate))
			return false;

		if (mass)
			DS_VERIFY(dsPhysicsMassProperties_setMass(&rigidBodyTemplate->massProperties, *mass));
		if (rotationPointShift)
		{
			DS_VERIFY(dsPhysicsMassProperties_shift(&rigidBodyTemplate->massProperties,
				rotationPointShift, NULL));
		}
	}
	else
		DS_VERIFY(dsPhysicsMassProperties_initializeEmpty(&rigidBodyTemplate->massProperties));
	return true;
}

dsRigidBody* dsRigidBodyTemplate_instantiate(const dsRigidBodyTemplate* rigidBodyTemplate,
	dsAllocator* allocator, dsRigidBodyGroup* group, const dsVector3f* position,
	dsQuaternion4f* orientation, const dsVector3f* scale, const dsVector3f* linearVelocity,
	const dsVector3f* angularVelocity, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!rigidBodyTemplate || !rigidBodyTemplate->engine)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = rigidBodyTemplate->allocator;

	dsRigidBodyInit init;
	init.userData = userData;
	init.destroyUserDataFunc = destroyUserDataFunc;
	init.group = group;
	init.flags = rigidBodyTemplate->flags;
	init.motionType = rigidBodyTemplate->motionType;
	init.dofMask = rigidBodyTemplate->dofMask;
	init.layer = rigidBodyTemplate->layer;
	init.collisionGroup = rigidBodyTemplate->collisionGroup;
	init.canCollisionGroupsCollideFunc = rigidBodyTemplate->canCollisionGroupsCollideFunc;
	if (position)
		init.position = *position;
	else
		memset(&init.position, 0, sizeof(init.position));
	if (orientation)
		init.orientation = *orientation;
	else
		dsQuaternion4_identityRotation(init.orientation);
	if (scale)
		init.scale = *scale;
	else
		memset(&init.scale, 0, sizeof(init.scale));
	if (linearVelocity)
		init.linearVelocity = *linearVelocity;
	else
		memset(&init.linearVelocity, 0, sizeof(init.linearVelocity));
	if (angularVelocity)
		init.angularVelocity = *angularVelocity;
	else
		memset(&init.angularVelocity, 0, sizeof(init.angularVelocity));
	init.friction = rigidBodyTemplate->friction;
	init.restitution = rigidBodyTemplate->restitution;
	init.hardness = rigidBodyTemplate->hardness;
	init.linearDamping = rigidBodyTemplate->linearDamping;
	init.angularDamping = rigidBodyTemplate->angularDamping;
	init.maxLinearVelocity = rigidBodyTemplate->maxLinearVelocity;
	init.maxAngularVelocity = rigidBodyTemplate->maxAngularVelocity;
	init.shapeCount = rigidBodyTemplate->shapeCount;

	dsRigidBody* rigidBody = dsRigidBody_create(rigidBodyTemplate->engine, allocator, &init);
	if (!rigidBody)
		return NULL;

	for (uint32_t i = 0; i < rigidBodyTemplate->shapeCount; ++i)
	{
		const dsPhysicsShapeInstance* instance = rigidBodyTemplate->shapes + i;
		uint32_t instanceID = dsRigidBody_addShape(rigidBody, instance->shape,
			instance->hasTranslate ? &instance->translate : NULL,
			instance->hasRotate ? &instance->rotate : NULL,
			instance->hasScale ? &instance->scale : NULL, instance->density,
			instance->hasMaterial ? &instance->material : NULL);
		if (instanceID == DS_INVALID_PHYSICS_ID)
		{
			dsRigidBody_destroy(rigidBody);
			return NULL;
		}
	}

	if (rigidBodyTemplate->shapeCount > 0 && !dsRigidBody_finalizeShapesCustomMassProperties(
			rigidBody, &rigidBodyTemplate->massProperties))
	{
		dsRigidBody_destroy(rigidBody);
		return NULL;
	}

	return rigidBody;
}

void dsRigidBodyTemplate_destroy(dsRigidBodyTemplate* rigidBodyTemplate)
{
	if (!rigidBodyTemplate)
		return;

	for (uint32_t i = 0; i < rigidBodyTemplate->shapeCount; ++i)
	{
		const dsPhysicsShapeInstance* instance = rigidBodyTemplate->shapes + i;
		dsPhysicsShape_freeRef(instance->shape);
	}
	DS_VERIFY(dsAllocator_free(rigidBodyTemplate->allocator, rigidBodyTemplate->shapes));
	DS_VERIFY(dsAllocator_free(rigidBodyTemplate->allocator, rigidBodyTemplate));
}
