# Copyright 2024-2026 Aaron Barany
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

import math

import flatbuffers
from .Helpers import eulerToQuaternion, readFloat
from DeepSeaPhysics.ConstraintMotorType import ConstraintMotorType
from DeepSeaPhysics.ConstraintUnion import ConstraintUnion
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import RevoluteConstraint
from DeepSeaPhysics import Constraint

def convertRevolutePhysicsConstraint(convertContext, data, inputDir, outputDir):
	"""
	Converts a RevolutePhysicsConstraint. The data map is expected to contain the following
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
	- minAngle: the minimum angle in degrees in the range [-180, 0] when the limit is enabled.
	  Defaults to -180 degrees.
	- maxAngle: the maximum angle in degrees in the range [0, 180] when the limit is enabled.
	  Defaults to 180 degrees.
	- limitStiffness: the spring stiffness applied when limiting the angle. Defaults to 100.
	- limitDamping: the spring damping in the range [0, 1] applied when limiting the angle.
	  Defaults to 1.
	- motorType: the type of the motor to apply to the constraint. See the
	  dsPhysicsConstraintMotorType enum for valid values, omitting the type prefix. Defaults to
	  Disabled.
	- motorTarget: the target for the motor. This will be an angle in degrees if motorType is
	  Position or an angular velocity (typically degrees/second) if motorType is Velocity. Defaults
	  to 0.
	- maxMotorTorque: the maximum torque of the motor to reach the target. If the motor is disabled,
	  this will be the toque used to apply to stop motion. Defaults to 0.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstPositionData = data['firstPosition']
		if not isinstance(firstPositionData, list) or len(firstPositionData) != 3:
			raise Exception(
				'RevolutePhysicsConstraint "firstPosition" must be an array of three floats.')
		firstPosition = (readFloat(value, 'first position') for value in firstPositionData)

		firstOrientationData = data['firstOrientation']
		if not isinstance(firstOrientationData, list) or len(firstOrientationData) != 3:
			raise Exception(
				'RevolutePhysicsConstraint "firstOrientation" must be an array of three floats.')
		firstOrientation = eulerToQuaternion(*(readFloat(value, 'first orientation')
			for value in firstOrientationData))

		secondActor = str(data.get('secondActor', ''))

		secondPositionData = data['secondPosition']
		if not isinstance(secondPositionData, list) or len(secondPositionData) != 3:
			raise Exception(
				'RevolutePhysicsConstraint "secondPosition" must be an array of three floats.')
		secondPosition = (readFloat(value, 'second position') for value in secondPositionData)

		secondOrientationData = data['secondOrientation']
		if not isinstance(secondOrientationData, list) or len(secondOrientationData) != 3:
			raise Exception(
				'RevolutePhysicsConstraint "secondOrientation" must be an array of three floats.')
		secondOrientation = eulerToQuaternion(*(readFloat(value, 'second orientation')
			for value in secondOrientationData))

		limitEnabled = bool(data.get('limitEnabled'))
		minAngle = math.radians(readFloat(data.get('minAngle', -180), 'min angle', -180, 0))
		maxAngle = math.radians(readFloat(data.get('maxAngle', 180), 'max angle', 0, 180))
		limitStiffness = readFloat(data.get('limitStiffness', 100), 'limit stiffness', 0)
		limitDamping = readFloat(data.get('limitDamping', 1), 'limit damping', 0, 1)

		motorTypeStr = str(data.get('motorType', 'Disabled'))
		try:
			motorType = getattr(ConstraintMotorType, motorTypeStr)
		except AttributeError:
			raise Exception('Invalid motor type "' + motorTypeStr + '".')

		motorTarget = math.radians(readFloat(data.get('motorTarget', 0), 'motor target'))
		maxMotorTorque = readFloat(data.get('maxMotorTorque', 0), 'max motor torque', 0)
	except (TypeError, ValueError):
		raise Exception('RevolutePhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('RevolutePhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0

	RevoluteConstraint.Start(builder)
	RevoluteConstraint.AddFirstActor(builder, firstActorOffset)
	RevoluteConstraint.AddFirstPosition(builder, CreateVector3f(builder, *firstPosition))
	RevoluteConstraint.AddFirstOrientation(builder, CreateQuaternion4f(builder, *firstOrientation))
	RevoluteConstraint.AddSecondActor(builder, secondActorOffset)
	RevoluteConstraint.AddSecondPosition(builder, CreateVector3f(builder, *secondPosition))
	RevoluteConstraint.AddSecondOrientation(builder, CreateQuaternion4f(builder, *secondOrientation))
	RevoluteConstraint.AddLimitEnabled(builder, limitEnabled)
	RevoluteConstraint.AddMinAngle(builder, minAngle)
	RevoluteConstraint.AddMaxAngle(builder, maxAngle)
	RevoluteConstraint.AddLimitStiffness(builder, limitStiffness)
	RevoluteConstraint.AddLimitDamping(builder, limitDamping)
	RevoluteConstraint.AddMotorType(builder, motorType)
	RevoluteConstraint.AddMotorTarget(builder, motorTarget)
	RevoluteConstraint.AddMaxMotorTorque(builder, maxMotorTorque)
	constraintOffset = RevoluteConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.RevoluteConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
