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

bool dsPhysicsActorContactManifold_getContactPoint(dsPhysicsActorContactPoint* outPoint,
	const dsPhysicsActorContactManifold* manifold, uint32_t index)
{
	if (!outPoint || !manifold || !manifold->engine ||
		!manifold->engine->getPhysicsActorContactPointFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (index >= manifold->pointCount)
	{
		errno = EINDEX;
		return false;
	}

	dsPhysicsEngine* engine = manifold->engine;
	return engine->getPhysicsActorContactPointFunc(outPoint, engine, manifold, index);
}

bool dsPhysicsActorContactManifold_getContactProperties(
	dsPhysicsActorContactProperties* outProperties, const dsPhysicsActorContactManifold* manifold,
	uint32_t index)
{
	if (!outProperties || !manifold || !manifold->engine ||
		!manifold->engine->getPhysicsActorContactPropertiesFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (index >= manifold->contactPropertiesCount)
	{
		errno = EINDEX;
		return false;
	}

	dsPhysicsEngine* engine = manifold->engine;
	return engine->getPhysicsActorContactPropertiesFunc(outProperties, engine, manifold, index);
}

bool dsPhysicsActorContactManifold_setContactSetting(dsPhysicsActorContactManifold* manifold,
	uint32_t index, const dsPhysicsActorContactProperties* properties)
{
	if (!manifold || !manifold->engine || !manifold->engine->setPhysicsActorContactPropertiesFunc ||
		!properties || properties->combinedFriction < 0.0f ||
		properties->combinedRestitution < 0.0f || properties->combinedRestitution > 1.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = manifold->engine;
	return engine->setPhysicsActorContactPropertiesFunc(engine, manifold, index, properties);
}
