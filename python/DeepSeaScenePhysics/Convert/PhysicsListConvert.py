# Copyright 2024 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import flatbuffers
from .Helpers import readFloat, readInt
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaScenePhysics import PhysicsList

def convertPhysicsList(convertContext, data):
	"""
	Converts a PhysicsList. The data map is expected to contain the following elements:
	- maxStaticBodies: the maximum number of bodies that are only used for collision and not
	  affected by physics.
	- maxDynamicBodies: the maximum number of bodies that are affected by physics.
	- maxConstrainedBodyGroups: the maximum number of groups of bodies that are connected through
	  constraints.
	- maxStaticShapes: the maximum number of shapes used by static bodies. If 0
	  maxStaticBodies will be used. Defaults to 0.
	- maxDynamicShapes: the maximum number of shapes used by dynamic bodies. If 0 maxDynamicBodies
	  will be used. Defaults to 0.
	- maxConstraints: The maximum number of constraints.
	- maxBodyCollisionPairs: the maximum number of pairs of bodies that may collide. The
	  implementation is only guaranteed to process this many pairs of potentially colliding bodies.
	  If it is exceeded, further collisions may be ignored. This should be much larger than the
	  maximum number of contact points as the collision pairs may not actually touch.
	- maxContactPoints: the maximum number of contact points between colliding bodies. The
	  implementation is only guaranteed to process this many contacts between bodies. If it is
	  exceeded, further contacts may be discarded.
	- gravity: array of 3 floats for the the initial gravity of the scene.
	- multiThreadedModifications: whether modifications may be made across threads. When false, the
	  locking functions will become NOPs that only enforce that the proper locking functions are
	  used. This can reduce overhead when locking isn't required. Defaults to false.
	- targetStepTime: the step time that is desired when updating the physics list. This will keep
	  each step as close to this time as possible. Defaults to 1/60 s.
	"""
	try:
		maxStaticBodies = readInt(data['maxStaticBodies'], 'max static bodies', 0)
		maxDynamicBodies = readInt(data['maxDynamicBodies'], 'max dynamic bodies', 0)
		maxConstrainedBodyGroups = readInt(
			data['maxConstrainedBodyGroups'], 'max constrained body groups', 0)
		maxStaticShapes = readInt(data.get('maxStaticShapes', 0), 'max static shapes', 0)
		maxDynamicShapes = readInt(data.get('maxDynamicShapes', 0), 'max dynamic shapes', 0)
		maxConstraints = readInt(data['maxConstraints'], 'max constraints', 0)
		maxBodyCollisionPairs = readInt(
			data['maxBodyCollisionPairs'], 'max body collision pairs', 0)
		maxContactPoints = readInt(data['maxContactPoints'], 'max contact points', 0)

		gravityData = data['gravity']
		if not isinstance(gravityData, list) or len(gravityData) != 3:
			raise Exception('PhysicsList data array of three floats.')
		gravity = (readFloat(gravityData[0], 'gravity'), readFloat(gravityData[1], 'gravity'),
			readFloat(gravityData[2], 'gravity'))

		targetStepTime = readFloat(data.get('targetStepTime', 1/60), 'target step time', 0)
	except (TypeError, ValueError):
		raise Exception('PhysicsList data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsList data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)
	PhysicsList.Start(builder)
	PhysicsList.AddMaxStaticBodies(builder, maxStaticBodies)
	PhysicsList.AddMaxDynamicBodies(builder, maxDynamicBodies)
	PhysicsList.AddMaxConstrainedBodyGroups(builder, maxConstrainedBodyGroups)
	PhysicsList.AddMaxStaticShapes(builder, maxStaticShapes)
	PhysicsList.AddMaxDynamicShapes(builder, maxDynamicShapes)
	PhysicsList.AddMaxConstraints(builder, maxConstraints)
	PhysicsList.AddMaxBodyCollisionPairs(builder, maxBodyCollisionPairs)
	PhysicsList.AddMaxContactPoints(builder, maxContactPoints)
	PhysicsList.AddGravity(builder, CreateVector3f(builder, *gravity))
	PhysicsList.AddTargetStepTime(builder, targetStepTime)
	builder.Finish(PhysicsList.End(builder))
	return builder.Output()
