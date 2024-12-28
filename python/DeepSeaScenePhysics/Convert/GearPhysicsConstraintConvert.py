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
from DeepSeaPhysics import GearConstraint

def convertGearPhysicsConstraint(convertContext, data):
	"""
	Converts a GearPhysicsConstraint. The data map is expected to contain the following elements:
	- firstActor: the name of the first actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- firstAxis: array of 3 floats for the axis of rotation of the first actor.
	- firstConstraint: the name of the revolute constraint for the first actor. This may be unset if
	  the constraint won't be set or will be provided later.
	- firstToothCount: the number of teeth for the first actor's gear to compute the gear ratio.
	  This may be negative if it is flipped.
	- secondActor: the name of the second actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- secondAxis: array of 3 floats for the axis of rotation of the second actor.
	- secondConstraint: the name of the revolute constraint for the second actor. This may be unset
	  if the constraint won't be set or will be provided later.
	- secondToothCount: the number of teeth for the second actor's gear to compute the gear ratio.
	  This may be negative if it is flipped.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstAxisData = data['firstAxis']
		if not isinstance(firstAxisData, list) or len(firstAxisData) != 3:
			raise Exception(
				'GearPhysicsConstraint firstAxis must be an array of three floats.')
		firstAxis = (readFloat(value, 'first position') for value in firstAxisData)

		firstConstraint = str(data.get('firstConstraint', ''))
		firstToothCount = readFloat(data['firstToothCount'], 'first tooth count')

		secondActor = str(data.get('secondActor', ''))

		secondAxisData = data['secondAxis']
		if not isinstance(secondAxisData, list) or len(secondAxisData) != 3:
			raise Exception(
				'GearPhysicsConstraint secondAxis must be an array of three floats.')
		secondAxis = (readFloat(value, 'second position') for value in secondAxisData)

		secondConstraint = str(data.get('secondConstraint', ''))
		secondToothCount = readFloat(data['secondToothCount'], 'second tooth count')
	except (TypeError, ValueError):
		raise Exception('GearPhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('GearPhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	firstConstraintOffset = builder.CreateString(firstConstraint) if firstConstraint else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0
	secondConstraintOffset = builder.CreateString(secondConstraint) if secondConstraint else 0

	GearConstraint.Start(builder)
	GearConstraint.AddFirstActor(builder, firstActorOffset)
	GearConstraint.AddFirstAxis(builder, CreateVector3f(builder, *firstAxis))
	GearConstraint.AddFirstConstraint(builder, firstConstraintOffset)
	GearConstraint.AddSecondActor(builder, secondActorOffset)
	GearConstraint.AddSecondAxis(builder, CreateVector3f(builder, *secondAxis))
	GearConstraint.AddSecondConstraint(builder, secondConstraintOffset)
	GearConstraint.AddRatio(builder, firstToothCount/secondToothCount)
	constraintOffset = GearConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.GearConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
