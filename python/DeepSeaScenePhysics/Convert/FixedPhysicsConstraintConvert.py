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
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Constraint
from DeepSeaPhysics import FixedConstraint

def convertFixedPhysicsConstraint(convertContext, data):
	"""
	Converts a FixedPhysicsConstraint. The data map is expected to contain the following elements:
	- firstActor: the name of the first actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- firstPosition: array of 3 floats for the position of the constraint relative to the first
	  actor.
	- firstOrientation: array of x, y, z Euler angles in degrees for the orientation of the
	  constraint relative to the first actor.
	- secondActor: the name of the second actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- secondPosition: array of 3 floats for the position of the constraint relative to the second
	  actor.
	- secondOrientation: array of x, y, z Euler angles in degrees for the orientation of the
	  constraint relative to the second actor.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstPositionData = data['firstPosition']
		if not isinstance(firstPositionData, list) or len(firstPositionData) != 3:
			raise Exception(
				'FixedPhysicsConstraint "firstPosition" must be an array of three floats.')
		firstPosition = (readFloat(value, 'first position') for value in firstPositionData)

		firstOrientationData = data['firstOrientation']
		if not isinstance(firstOrientationData, list) or len(firstOrientationData) != 3:
			raise Exception(
				'FixedPhysicsConstraint "firstOrientation" must be an array of three floats.')
		firstOrientation = eulerToQuaternion(*(readFloat(value, 'first orientation')
			for value in firstOrientationData))

		secondActor = str(data.get('secondActor', ''))

		secondPositionData = data['secondPosition']
		if not isinstance(secondPositionData, list) or len(secondPositionData) != 3:
			raise Exception(
				'FixedPhysicsConstraint "secondPosition" must be an array of three floats.')
		secondPosition = (readFloat(value, 'second position') for value in secondPositionData)

		secondOrientationData = data['secondOrientation']
		if not isinstance(secondOrientationData, list) or len(secondOrientationData) != 3:
			raise Exception(
				'FixedPhysicsConstraint "secondOrientation" must be an array of three floats.')
		secondOrientation = eulerToQuaternion(*(readFloat(value, 'second orientation')
			for value in secondOrientationData))
	except (TypeError, ValueError):
		raise Exception('FixedPhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('FixedPhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0

	FixedConstraint.Start(builder)
	FixedConstraint.AddFirstActor(builder, firstActorOffset)
	FixedConstraint.AddFirstPosition(builder, CreateVector3f(builder, *firstPosition))
	FixedConstraint.AddFirstOrientation(builder, CreateQuaternion4f(builder, *firstOrientation))
	FixedConstraint.AddSecondActor(builder, secondActorOffset)
	FixedConstraint.AddSecondPosition(builder, CreateVector3f(builder, *secondPosition))
	FixedConstraint.AddSecondOrientation(builder, CreateQuaternion4f(builder, *secondOrientation))
	constraintOffset = FixedConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.FixedConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
