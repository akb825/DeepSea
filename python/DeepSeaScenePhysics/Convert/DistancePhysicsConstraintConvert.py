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
from .Helpers import eulerToQuaternion, readFloat
from DeepSeaPhysics.ConstraintUnion import ConstraintUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import DistanceConstraint
from DeepSeaPhysics import Constraint

def convertDistancePhysicsConstraint(convertContext, data, outputDir):
	"""
	Converts a DistancePhysicsConstraint. The data map is expected to contain the following
	elements:
	- firstActor: the name of the first actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- firstPosition: array of 3 floats for the position of the constraint relative to the first
	  actor.
	- secondActor: the name of the second actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- secondPosition: array of 3 floats for the position of the constraint relative to the second
	  actor.
	- minDistance: the minimum distance between reference points.
	- maxDistance: the maximum distance between reference points.
	- limitStiffness: the stiffness for the spring to keep within the distance range.
	- limitDamping: the damping in the range [0, 1] to keep within the distance range.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstPositionData = data['firstPosition']
		if not isinstance(firstPositionData, list) or len(firstPositionData) != 3:
			raise Exception(
				'DistancePhysicsConstraint "firstPosition" must be an array of three floats.')
		firstPosition = (readFloat(value, 'first position') for value in firstPositionData)

		secondActor = str(data.get('secondActor', ''))

		secondPositionData = data['secondPosition']
		if not isinstance(secondPositionData, list) or len(secondPositionData) != 3:
			raise Exception(
				'DistancePhysicsConstraint "secondPosition" must be an array of three floats.')
		secondPosition = (readFloat(value, 'second position') for value in secondPositionData)

		minDistance = readFloat(data['minDistance'], 'min distance', 0)
		maxDistance = readFloat(data['maxDistance'], 'max distance', 0)
		limitStiffness = readFloat(data['limitStiffness'], 'limit stiffness', 0)
		limitDamping = readFloat(data['limitDamping'], 'limit damping', 0, 1)
	except (TypeError, ValueError):
		raise Exception('DistancePhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('DistancePhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0

	DistanceConstraint.Start(builder)
	DistanceConstraint.AddFirstActor(builder, firstActorOffset)
	DistanceConstraint.AddFirstPosition(builder, CreateVector3f(builder, *firstPosition))
	DistanceConstraint.AddSecondActor(builder, secondActorOffset)
	DistanceConstraint.AddSecondPosition(builder, CreateVector3f(builder, *secondPosition))
	DistanceConstraint.AddMinDistance(builder, minDistance)
	DistanceConstraint.AddMaxDistance(builder, maxDistance)
	DistanceConstraint.AddLimitStiffness(builder, limitStiffness)
	DistanceConstraint.AddLimitDamping(builder, limitDamping)
	constraintOffset = DistanceConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.DistanceConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
