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
from DeepSeaPhysics import Constraint
from DeepSeaPhysics import PointConstraint

def convertPointPhysicsConstraint(convertContext, data, outputDir):
	"""
	Converts a PointPhysicsConstraint. The data map is expected to contain the following elements:
	- firstActor: the name of the first actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- firstPosition: array of 3 floats for the position of the constraint relative to the first
	  actor.
	- secondActor: the name of the second actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- secondPosition: array of 3 floats for the position of the constraint relative to the second
	  actor.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstPositionData = data['firstPosition']
		if not isinstance(firstPositionData, list) or len(firstPositionData) != 3:
			raise Exception(
				'PointPhysicsConstraint "firstPosition" must be an array of three floats.')
		firstPosition = (readFloat(value, 'first position') for value in firstPositionData)

		secondActor = str(data.get('secondActor', ''))

		secondPositionData = data['secondPosition']
		if not isinstance(secondPositionData, list) or len(secondPositionData) != 3:
			raise Exception(
				'PointPhysicsConstraint "secondPosition" must be an array of three floats.')
		secondPosition = (readFloat(value, 'second position') for value in secondPositionData)
	except (TypeError, ValueError):
		raise Exception('PointPhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('PointPhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0

	PointConstraint.Start(builder)
	PointConstraint.AddFirstActor(builder, firstActorOffset)
	PointConstraint.AddFirstPosition(builder, CreateVector3f(builder, *firstPosition))
	PointConstraint.AddSecondActor(builder, secondActorOffset)
	PointConstraint.AddSecondPosition(builder, CreateVector3f(builder, *secondPosition))
	constraintOffset = PointConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.PointConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
