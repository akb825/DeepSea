/*
 * Copyright 2023-2024 Aaron Barany
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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>
#include <DeepSea/Physics/RigidBodyInit.h>

#define MAX_STACK_MASS_PROPERTIES 256

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
	dsAllocator* scratchAllocator = rigidBody->engine->allocator;
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
		massPropertiesPtrs[i] = shapeMassProperties + i;
		const dsPhysicsShapeInstance* shape = rigidBody->shapes + i;
		if (!dsPhysicsShape_getMassProperties(
				shapeMassProperties + i, shape->shape, shape->density))
		{
			if (heapMassProperties)
			{
				DS_VERIFY(dsAllocator_free(scratchAllocator, shapeMassProperties));
				DS_VERIFY(dsAllocator_free(scratchAllocator, (void*)massPropertiesPtrs));
			}
			return false;
		}
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

dsRigidBody* dsRigidBody_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsRigidBodyInit* initParams)
{
	if (!engine || !engine->createRigidBodyFunc || !engine->destroyRigidBodyFunc || !allocator ||
		!dsRigidBodyInit_isValid(initParams))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Rigid body allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	return engine->createRigidBodyFunc(engine, allocator, initParams);
}

uint32_t dsRigidBody_addShape(dsRigidBody* rigidBody, dsPhysicsShape* shape,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale,
	float density)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->addRigidBodyShapeFunc || !shape ||
		!shape->type || (scale && (scale->x == 0.0f || scale->y == 0.0f || scale->z == 0.0f)) ||
		(density <= 0 && hasMassProperties(rigidBody)))
	{
		errno = EINVAL;
		return DS_NO_PHYSICS_SHAPE_ID;
	}

	if (rigidBody->shapesFinalized && !(rigidBody->flags & dsRigidBodyFlags_MutableShape))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot add a shape to a rigid body with finalized shapes "
			"unless mutable shape flag is set.");
		errno = EPERM;
		return DS_NO_PHYSICS_SHAPE_ID;
	}

	if (shape->type->staticBodiesOnly && (rigidBody->motionType != dsPhysicsMotionType_Static ||
		(rigidBody->flags & dsRigidBodyFlags_MutableMotionType)))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot add static-only shape to a rigid body that isn't "
			"static or has the mutable motion type flag set.");
		errno = EPERM;
		return DS_NO_PHYSICS_SHAPE_ID;
	}

	if (scale && shape->type->uniformScaleOnly && (scale->x != scale->y || scale->x != scale->z))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Attempting to set non-uniform scale a shape that "
			"requires uniform scaling.");
		errno = EPERM;
		return DS_NO_PHYSICS_SHAPE_ID;
	}

	if (rotate &&
		(rigidBody->scale.x != rigidBody->scale.y || rigidBody->scale.x != rigidBody->scale.z))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Attempting to set rotation for a shape on a rigid body with non-uniform scale.");
		errno = EPERM;
		return DS_NO_PHYSICS_SHAPE_ID;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	uint32_t shapeID = engine->addRigidBodyShapeFunc(engine, rigidBody, shape, translate, rotate,
		scale, density);
	if (shapeID != DS_NO_PHYSICS_SHAPE_ID)
		rigidBody->shapesFinalized = false;
	return shapeID;
}

bool dsRigidBody_setShapeTransformID(dsRigidBody* rigidBody, uint32_t shapeID,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyShapeTransformFunc ||
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

	dsPhysicsEngine* engine = rigidBody->engine;
	bool success = engine->setRigidBodyShapeTransformFunc(
		engine, rigidBody, index, translate, rotate, scale);
	if (success)
		rigidBody->shapesFinalized = false;
	return success;
}

bool dsRigidBody_setShapeTransformIndex(dsRigidBody* rigidBody, uint32_t shapeIndex,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyShapeTransformFunc ||
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

	dsPhysicsEngine* engine = rigidBody->engine;
	bool success = engine->setRigidBodyShapeTransformFunc(
		engine, rigidBody, shapeIndex, translate, rotate, scale);
	if (success)
		rigidBody->shapesFinalized = false;
	return success;
}

bool dsRigidBody_removeShapeID(dsRigidBody* rigidBody, uint32_t shapeID)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->removeRigidBodyShapeFunc)
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

	uint32_t index = DS_NO_PHYSICS_SHAPE_ID;
	for (uint32_t i = 0; i < rigidBody->shapeCount; ++i)
	{
		if (rigidBody->shapes[i].id == shapeID)
		{
			index = i;
			break;
		}
	}

	if (index == DS_NO_PHYSICS_SHAPE_ID)
	{
		errno = ENOTFOUND;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	bool success = engine->removeRigidBodyShapeFunc(engine, rigidBody, index);
	if (success)
		rigidBody->shapesFinalized = false;
	return success;
}

bool dsRigidBody_removeShapeIndex(dsRigidBody* rigidBody, uint32_t shapeIndex)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->removeRigidBodyShapeFunc)
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

	dsPhysicsEngine* engine = rigidBody->engine;
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
			"body that isn't dynamic motion type or with the mutable motion type flag set.");
		errno = EPERM;
		return false;
	}

	return computeDefaultMassProperties(outMassProperties, rigidBody);
}

bool dsRigidBody_finalizeShapes(dsRigidBody* rigidBody, const float* mass,
	const dsVector3f* rotationPointShift)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->finalizeRigidBodyShapesFunc ||
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

	dsPhysicsEngine* engine = rigidBody->engine;
	bool success = engine->finalizeRigidBodyShapesFunc(engine, rigidBody, &massProperties);
	if (success)
		rigidBody->shapesFinalized = true;
	return success;
}

bool dsRigidBody_finalizeShapesCustomMassProperties(dsRigidBody* rigidBody,
	const dsPhysicsMassProperties* massProperties)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->finalizeRigidBodyShapesFunc ||
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

	dsPhysicsEngine* engine = rigidBody->engine;
	bool success = engine->finalizeRigidBodyShapesFunc(engine, rigidBody, massProperties);
	if (success)
		rigidBody->shapesFinalized = true;
	return success;
}

bool dsRigidBody_addFlags(dsRigidBody* rigidBody, dsRigidBodyFlags flags)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyFlagsFunc)
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

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyFlagsFunc(engine, rigidBody, rigidBody->flags | flags);
}

bool dsRigidBody_removeFlags(dsRigidBody* rigidBody, dsRigidBodyFlags flags)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyFlagsFunc)
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

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyFlagsFunc(engine, rigidBody, rigidBody->flags & (~flags));
}

bool dsRigidBody_setMotionType(dsRigidBody* rigidBody, dsPhysicsMotionType motionType)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyMotionTypeFunc)
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

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyMotionTypeFunc(engine, rigidBody, motionType);
}

bool dsRigidBody_setDOFMask(dsRigidBody* rigidBody, dsPhysicsDOFMask dofMask)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyDOFMaskFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyDOFMaskFunc(engine, rigidBody, dofMask);
}

bool dsRigidBody_setCollisionGroup(dsRigidBody* rigidBody, uint64_t collisionGroup)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyCollisionGroupFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyCollisionGroupFunc(engine, rigidBody, collisionGroup);
}

bool dsRigidBody_setCanCollisionGroupsCollideFunction(dsRigidBody* rigidBody,
	dsCanCollisionGroupsCollideFunction canCollideFunc)
{
	if (!rigidBody || !rigidBody->engine ||
		!rigidBody->engine->setRigidBodyCanCollisionGroupsCollideFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyCanCollisionGroupsCollideFunc(engine, rigidBody, canCollideFunc);
}

bool dsRigidBody_setTransform(dsRigidBody* rigidBody, const dsVector3f* position,
	const dsQuaternion4f* orientation, const dsVector3f* scale)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyTransformFunc ||
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

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyTransformFunc(engine, rigidBody, position, orientation, scale);
}

bool dsRigidBody_getTransformMatrix(dsMatrix44f* outTransform, const dsRigidBody* rigidBody)
{
	if (outTransform || !rigidBody)
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix44f translate;
	dsMatrix44f_makeTranslate(&translate, rigidBody->position.x, rigidBody->position.y,
		rigidBody->position.z);

	dsMatrix44f rotateScale;
	if (rigidBody->flags & dsRigidBodyFlags_Scalable)
	{
		dsMatrix44f rotate, scale;
		dsMatrix44f_makeScale(&scale, rigidBody->scale.x, rigidBody->scale.y, rigidBody->scale.z);
		dsQuaternion4f_toMatrix44(&rotate, &rigidBody->orientation);
		dsMatrix44f_affineMul(&rotateScale, &rotate, &scale);
	}
	else
		dsQuaternion4f_toMatrix44(&rotateScale, &rigidBody->orientation);
	dsMatrix44f_affineMul(outTransform, &translate, &rotateScale);
	return true;
}

bool dsRigidBody_setTransformMatrix(dsRigidBody* rigidBody, const dsMatrix44f* transform)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyTransformFunc ||
		!transform)
	{
		errno = EINVAL;
		return false;
	}

	dsVector3f scale;
	scale.x = dsVector3f_len((const dsVector3f*)transform->columns);
	scale.y = dsVector3f_len((const dsVector3f*)(transform->columns + 1));
	scale.z = dsVector3f_len((const dsVector3f*)(transform->columns + 2));

	const float scaleEpsilon = 1e-5f;
	dsVector3f one = {{1.0f, 1.0f, 1.0f}};
	dsVector3f* scalePtr = NULL;
	bool unitScale = dsVector3f_epsilonEqual(&scale, &one, scaleEpsilon);
	bool scalable = (rigidBody->flags & dsRigidBodyFlags_Scalable) != 0;
	dsQuaternion4f orientation;
	if (unitScale)
	{
		if (scalable && !dsVector3_equal(one, rigidBody->scale))
			scalePtr = &one; // Avoid unit scales that are slightly off.
		dsQuaternion4f_fromMatrix44(&orientation, transform);
	}
	else
	{
		if (!scalable)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Rigid body must have scalable flag set to modify the scale.");
			errno = EPERM;
			return false;
		}

		if (dsEpsilonEqualf(scale.x, scale.y, scaleEpsilon) &&
			dsEpsilonEqualf(scale.x, scale.z, scaleEpsilon))
		{
			// Avoid uniform scales that are slightly off.
			scale.y = scale.z = scale.x;
		}
		else
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

		// Only change the scale if it's different from the previous, since this may be more
		// expensive than changing the position and rotation.
		if (!dsVector3f_epsilonEqual(&scale, &rigidBody->scale, scaleEpsilon))
			scalePtr = &scale;

		dsVector3f invScale;
		dsVector3_div(invScale, one, scale);
		dsMatrix33f rotationMatrix;
		dsVector3_scale(rotationMatrix.columns[0], transform->columns[0], invScale.x);
		dsVector3_scale(rotationMatrix.columns[1], transform->columns[1], invScale.y);
		dsVector3_scale(rotationMatrix.columns[2], transform->columns[2], invScale.z);
		dsQuaternion4f_fromMatrix33(&orientation, &rotationMatrix);
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyTransformFunc(engine, rigidBody,
		(const dsVector3f*)(transform->columns + 3), &orientation, scalePtr);
}

bool dsRigidBody_setMass(dsRigidBody* rigidBody, float mass)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyMassFunc || mass < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyMassFunc(engine, rigidBody, mass);
}

bool dsRigidBody_setFriction(dsRigidBody* rigidBody, float friction)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyFrictionFunc ||
		friction < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyFrictionFunc(engine, rigidBody, friction);
}

bool dsRigidBody_setRestitution(dsRigidBody* rigidBody, float restitution)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyRestitutionFunc ||
		restitution < 0 || restitution > 1)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyRestitutionFunc(engine, rigidBody, restitution);
}

bool dsRigidBody_setLinearDamping(dsRigidBody* rigidBody, float linearDamping)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyLinearDampingFunc ||
		linearDamping < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyLinearDampingFunc(engine, rigidBody, linearDamping);
}

bool dsRigidBody_setAngularDamping(dsRigidBody* rigidBody, float angularDamping)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyAngularDampingFunc ||
		angularDamping < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyAngularDampingFunc(engine, rigidBody, angularDamping);
}

bool dsRigidBody_setMaxLinearVelocity(dsRigidBody* rigidBody, float maxLinearVelocity)
{
	if (!rigidBody || !rigidBody->engine || !rigidBody->engine->setRigidBodyMaxLinearVelocityFunc ||
		maxLinearVelocity < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyMaxLinearVelocityFunc(engine, rigidBody, maxLinearVelocity);
}

bool dsRigidBody_setMaxAngularVelocity(dsRigidBody* rigidBody, float maxAngularVelocity)
{
	if (!rigidBody || !rigidBody->engine ||
		!rigidBody->engine->setRigidBodyMaxAngularVelocityFunc || maxAngularVelocity < 0)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = rigidBody->engine;
	return engine->setRigidBodyMaxAngularVelocityFunc(engine, rigidBody, maxAngularVelocity);
}

bool dsRigidBody_destroy(dsRigidBody* rigidBody)
{
	if (!rigidBody)
		return true;

	dsPhysicsEngine* engine = rigidBody->engine;
	if (!engine || !engine->destroyRigidBodyFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroyRigidBodyFunc(engine, rigidBody);
}
