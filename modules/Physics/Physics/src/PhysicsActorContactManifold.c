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

#include <DeepSea/Physics/PhysicsActorContactManifold.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Physics/RigidBody.h>

bool dsPhysicsActorContactManifold_getContactPoint(dsPhysicsActorContactPoint* outPoint,
	const dsPhysicsActorContactManifold* manifold, uint32_t index)
{
	if (!outPoint || !manifold || !manifold->scene || !manifold->scene->engine ||
		!manifold->scene->engine->getPhysicsActorContactPointFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (index >= manifold->pointCount)
	{
		errno = EINDEX;
		return false;
	}

	dsPhysicsEngine* engine = manifold->scene->engine;
	return engine->getPhysicsActorContactPointFunc(outPoint, engine, manifold, index);
}

bool dsPhysicsActorContactManifold_getDefaultContactProperties(
	dsPhysicsActorContactProperties* outProperties, const dsPhysicsActorContactManifold* manifold,
	const dsPhysicsActorContactPoint* point)
{
	if (!outProperties || !manifold || !manifold->scene || !manifold->scene->combineFrictionFunc ||
		!manifold->scene->combineRestitutionFunc || !manifold->actorA || !manifold->actorB ||
		!point)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsShapePartMaterial materialA;
	switch (manifold->actorA->type)
	{
		case dsPhysicsActorType_RigidBody:
			if (!dsRigidBody_getShapeMaterialIndex(&materialA, (const dsRigidBody*)manifold->actorA,
					point->shapeIndexA, point->faceIndexA))
			{
				return false;
			}
			break;
		default:
			errno = EPERM;
			return false;
	}

	dsPhysicsShapePartMaterial materialB;
	switch (manifold->actorB->type)
	{
		case dsPhysicsActorType_RigidBody:
			if (!dsRigidBody_getShapeMaterialIndex(&materialB, (const dsRigidBody*)manifold->actorB,
					point->shapeIndexB, point->faceIndexB))
			{
				return false;
			}
			break;
		default:
			errno = EPERM;
			return false;
	}

	dsPhysicsScene* scene = manifold->scene;
	outProperties->combinedFriction =
		scene->combineFrictionFunc(materialA.friction, materialB.friction);
	outProperties->combinedRestitution = scene->combineRestitutionFunc(materialA.restitution,
		materialA.hardness, materialB.friction, materialB.hardness);
	outProperties->targetVelocity.x = outProperties->targetVelocity.y =
		outProperties->targetVelocity.z = 0.0f;
	return true;
}

bool dsPhysicsActorContactManifold_setContactSetting(dsPhysicsActorContactManifold* manifold,
	uint32_t index, const dsPhysicsActorContactProperties* properties)
{
	if (!manifold || !manifold->scene || !manifold->scene->engine ||
		!manifold->scene->engine->setPhysicsActorContactPropertiesFunc || !properties ||
		properties->combinedFriction < 0.0f || properties->combinedRestitution < 0.0f ||
		properties->combinedRestitution > 1.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = manifold->scene->engine;
	return engine->setPhysicsActorContactPropertiesFunc(engine, manifold, index, properties);
}
