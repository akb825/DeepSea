/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/Particle/Types.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/SceneParticle library.
 */

/**
 * @brief Log tag used by the scene particle library.
 */
#define DS_SCENE_PARTICLE_LOG_TAG "scene-particle"

/**
 * @brief Struct describing a node for a scene that draws particles.
 *
 * This node will create a dsParticleEmitter for each instance within the scene graph.
 *
 * @see SceneParticleNode.h
 */
typedef struct dsSceneParticleNode dsSceneParticleNode;

/**
 * @brief Function to create a particle emitter from a particle node.
 * @param particleNode The particle node to create the emitter for.
 * @param allocator The allocator to create the emitter with.
 * @param userData User data to aid in creating the emitter.
 * @return The particle emitter.
 */
typedef dsParticleEmitter* (*dsCreateSceneParticleNodeEmitterFunction)(
	const dsSceneParticleNode* particleNode, dsAllocator* allocator, void* userData);

#ifdef __cplusplus
}
#endif
