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
from DeepSeaPhysics.ConstraintMotorType import ConstraintMotorType
from DeepSeaPhysics.ConstraintUnion import ConstraintUnion
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import SliderConstraint
from DeepSeaPhysics import Constraint

def convertSliderPhysicsConstraint(convertContext, data, outputDir):
	"""
	Converts a SliderPhysicsConstraint. The data map is expected to contain the following
	elements:
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
	- limitEnabled: whether the limit is enabled. Defaults to false.
	- minDistance: the minimum distance when the limit is enabled. Defaults to 0.
	- maxDistance: the maximum distance when the limit is enabled. Defaults to 100.
	- limitStiffness: the spring stiffness applied when limiting the angle. Defaults to 100.
	- limitDamping: the spring damping in the range [0, 1] applied when limiting the angle.
	  Defaults to 1.
	- motorType: the type of the motor to apply to the constraint. See the
	  dsPhysicsConstraintMotorType enum for valid values, omitting the type prefix. Defaults to
	  Disabled.
	- motorTarget: the target for the motor. This will be a distance if motorType is Position or
	  an velocity if motorType is Velocity. Defaults to 0.
	- maxMotorForce: the maximum force of the motor to reach the target. If the motor is disabled,
	  this will be the force used to apply to stop motion. Defaults to 0.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstPositionData = data['firstPosition']
		if not isinstance(firstPositionData, list) or len(firstPositionData) != 3:
			raise Exception(
				'SliderPhysicsConstraint "firstPosition" must be an array of three floats.')
		firstPosition = (readFloat(value, 'first position') for value in firstPositionData)

		firstOrientationData = data['firstOrientation']
		if not isinstance(firstOrientationData, list) or len(firstOrientationData) != 3:
			raise Exception(
				'SliderPhysicsConstraint "firstOrientation" must be an array of three floats.')
		firstOrientation = eulerToQuaternion(*(readFloat(value, 'first orientation')
			for value in firstOrientationData))

		secondActor = str(data.get('secondActor', ''))

		secondPositionData = data['secondPosition']
		if not isinstance(secondPositionData, list) or len(secondPositionData) != 3:
			raise Exception(
				'SliderPhysicsConstraint "secondPosition" must be an array of three floats.')
		secondPosition = (readFloat(value, 'second position') for value in secondPositionData)

		secondOrientationData = data['secondOrientation']
		if not isinstance(secondOrientationData, list) or len(secondOrientationData) != 3:
			raise Exception(
				'SliderPhysicsConstraint "secondOrientation" must be an array of three floats.')
		secondOrientation = eulerToQuaternion(*(readFloat(value, 'second orientation')
			for value in secondOrientationData))

		limitEnabled = bool(data.get('limitEnabled'))
		minDistance = readFloat(data.get('minDistance', 0), 'min distance', 0)
		maxDistance = readFloat(data.get('maxDistance', 100), 'max distance', 0)
		limitStiffness = readFloat(data.get('limitStiffness', 100), 'limit stiffness', 0)
		limitDamping = readFloat(data.get('limitDamping', 1), 'limit damping', 0, 1)

		motorTypeStr = str(data.get('motorType', 'Disabled'))
		try:
			motorType = getattr(ConstraintMotorType, motorTypeStr)
		except AttributeError:
			raise Exception('Invalid motor type "' + motorTypeStr + '".')

		motorTarget = readFloat(data.get('motorTarget', 0), 'motor target')
		maxMotorForce = readFloat(data.get('maxMotorForce', 0), 'max motor force', 0)
	except (TypeError, ValueError):
		raise Exception('SliderPhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('SliderPhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0

	SliderConstraint.Start(builder)
	SliderConstraint.AddFirstActor(builder, firstActorOffset)
	SliderConstraint.AddFirstPosition(builder, CreateVector3f(builder, *firstPosition))
	SliderConstraint.AddFirstOrientation(builder, CreateQuaternion4f(builder, *firstOrientation))
	SliderConstraint.AddSecondActor(builder, secondActorOffset)
	SliderConstraint.AddSecondPosition(builder, CreateVector3f(builder, *secondPosition))
	SliderConstraint.AddSecondOrientation(builder, CreateQuaternion4f(builder, *secondOrientation))
	SliderConstraint.AddLimitEnabled(builder, limitEnabled)
	SliderConstraint.AddMinDistance(builder, minDistance)
	SliderConstraint.AddMaxDistance(builder, maxDistance)
	SliderConstraint.AddLimitStiffness(builder, limitStiffness)
	SliderConstraint.AddLimitDamping(builder, limitDamping)
	SliderConstraint.AddMotorType(builder, motorType)
	SliderConstraint.AddMotorTarget(builder, motorTarget)
	SliderConstraint.AddMaxMotorForce(builder, maxMotorForce)
	constraintOffset = SliderConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.SliderConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
