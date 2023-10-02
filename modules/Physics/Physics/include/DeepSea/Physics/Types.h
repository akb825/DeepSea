/*
 * Copyright 2023 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Physics library.
 */

/// @cond
typedef struct dsPhysicsEngine dsPhysicsEngine;
/// @endcond

/**
 * @brief Enum describing a layer of physics objects.
 */
typedef enum dsPhysicsLayer
{
	dsPhsyicsLayer_StaticWorld, ///< Static world collision that cannot collide with itself.
	dsPhysicsLayer_Objects,     ///< Standard physics objects that can collide with anything.
	dsPhysicsLayer_Projectiles  ///< Projectiles that can cannot collide with each-other.
} dsPhysicsLayer;

/**
 * @brief Struct describing limits for objects within a scene.
 *
 * Some implementations may view these as strict upper limits, others may use them as hints to
 * pre-allocate, while others may ignore them completely.
 *
 * @param dsPhysicsScene
 * @see PhysicsScene.h
 */
typedef struct dsPhysicsSceneLimits
{
	/**
	 * @brief The maximum number of bodies that are only used for collision and not affected by
	 * physics.
	 */
	uint32_t maxStaticBodies;

	/**
	 * @brief The maximum number of bodies that are affected by physics.
	 */
	uint32_t maxDynamicBodies;

	/**
	 * @brief The maximum number of groups of bodies that are connected through constraints.
	 */
	uint32_t maxConstrainedBodyGroups;

	/**
	 * @brief The maximum number of shapes used by static bodies.
	 *
	 * If 0 maxStaticBodies will be used.
	 */
	uint32_t maxStaticShapes;

	/**
	 * @brief The maximum number of shapes used by dynamic bodies.
	 *
	 * If 0 maxDynamicBodies will be used.
	 */
	uint32_t maxDynamicShapes;

	/**
	 * @brief The maximum number of constraints.
	 */
	uint32_t maxConstraints;

	/**
	 * @brief The maximum number of pairs of bodies that may collide.
	 *
	 * The implementation is only guaranteed to process this many pairs of potentially colliding
	 * bodies. If it is exceeded, further collisions may be ignored.
	 *
	 * This should be much larger than the maximum number of contact points as the collision pairs
	 * may not actually touch.
	 */
	uint32_t maxBodyCollisionPairs;

	/**
	 * @brief The maximum number of contact points between colliding bodies.
	 *
	 * The implementation is only guaranteed to process this many contacts between bodies. If it is
	 * exceeded, further contacts may be discarded.
	 */
	uint32_t maxContactPoints;
} dsPhysicsSceneLimits;

/**
 * @brief Struct defining a scene of objects in a physics simulation.
 * @see dsPhysicsSceneLimits
 * @see PhysicsScene.h
 */
typedef struct dsPhysicsScene
{
	/**
	 * @brief The physics engine the scene was created with.
	 */
	dsPhysicsEngine* engine;

	/**
	 * @brief The allocator the scene was created with.
	 */
	dsAllocator* allocator;
} dsPhysicsScene;

/**
 * @brief Function to create a physics scene.
 * @param engine The physics engine to create the scene with.
 * @param allocator The allocator to create the scene with.
 * @param limits The limits for the physics scene.
 * @return The created physics scene or NULL if it couldn't be created.
 */
typedef dsPhysicsScene* (*dsCreatePhysicsSceneFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneLimits* limits);

/**
 * @brief Function to destroy a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The physics scene to destroy.
 * @return False if the physics scene couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsSceneFunction)(dsPhysicsEngine* engine, dsPhysicsScene* scene);

/**
 * @brief Struct describing the core engine for managing physics.
 *
 * This is a base type for the physics engine, which is implemented to either integrate to a 3rd
 * party physics engine or with a custom engine. It contains function pointers to create and destroy
 * the various physics objects and any other central management.
 */
typedef struct dsPhysicsEngine
{
	/**
	 * @brief Allocator for the physics engine.
	 *
	 * When possible, this will be used for global allocations in the physics allocations. Depending
	 * on the level of control for the underlying implementation, this may also be used for some
	 * internal allocations for individual objects.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Function to create a physics scene.
	 */
	dsCreatePhysicsSceneFunction createSceneFunc;

	/**
	 * @brief Function to destroy a physics scene.
	 */
	dsDestroyPhysicsSceneFunction destroySceneFunc;
} dsPhysicsEngine;

#ifdef __cplusplus
}
#endif
