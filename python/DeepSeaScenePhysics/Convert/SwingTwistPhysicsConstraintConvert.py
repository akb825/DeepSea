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

import math

import flatbuffers
from .Helpers import eulerToQuaternion, readFloat
from DeepSeaPhysics.ConstraintMotorType import ConstraintMotorType
from DeepSeaPhysics.ConstraintUnion import ConstraintUnion
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import SwingTwistConstraint
from DeepSeaPhysics import Constraint

def convertSwingTwistPhysicsConstraint(convertContext, data, outputDir):
	"""
	Converts a SwingTwistPhysicsConstraint. The data map is expected to contain the following
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
	- maxSwingXAngle: the maximum angle in degrees of the constraint along the X axis.
	- maxSwingYAngle: the maximum angle in degrees of the constraint along the Y axis.
	- maxTwistZAngle: the maximum angle in degrees of the constraint along the Z axis.
	- motorType: the type of the motor to apply to the constraint. See the
	  dsPhysicsConstraintMotorType enum for valid values, omitting the type prefix. Velocity is not
	  supported. Defaults to Disabled.
	- motorTargetOrientation: array of x, y, z Euler angles in degrees for the target orientation of
	  the motor relative to the second actor. Defaults to the identity rotation.
	- maxMotorTorque: the maximum torque of the motor to reach the target orientation. If the motor
	  is disabled, this will be the toque used to apply to stop motion. Defaults to 0.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstPositionData = data['firstPosition']
		if not isinstance(firstPositionData, list) or len(firstPositionData) != 3:
			raise Exception(
				'SwingTwistPhysicsConstraint "firstPosition" must be an array of three floats.')
		firstPosition = (readFloat(value, 'first position') for value in firstPositionData)

		firstOrientationData = data['firstOrientation']
		if not isinstance(firstOrientationData, list) or len(firstOrientationData) != 3:
			raise Exception(
				'SwingTwistPhysicsConstraint "firstOrientation" must be an array of three floats.')
		firstOrientation = eulerToQuaternion(*(readFloat(value, 'first orientation')
			for value in firstOrientationData))

		secondActor = str(data.get('secondActor', ''))

		secondPositionData = data['secondPosition']
		if not isinstance(secondPositionData, list) or len(secondPositionData) != 3:
			raise Exception(
				'SwingTwistPhysicsConstraint "secondPosition" must be an array of three floats.')
		secondPosition = (readFloat(value, 'second position') for value in secondPositionData)

		secondOrientationData = data['secondOrientation']
		if not isinstance(secondOrientationData, list) or len(secondOrientationData) != 3:
			raise Exception(
				'SwingTwistPhysicsConstraint "secondOrientation" must be an array of three floats.')
		secondOrientation = eulerToQuaternion(*(readFloat(value, 'second orientation')
			for value in secondOrientationData))

		maxSwingXAngle = math.radians(
			readFloat(data['maxSwingXAngle'], 'max swing X angle', 0, 180))
		maxSwingYAngle = math.radians(
			readFloat(data['maxSwingYAngle'], 'max swing Y angle', 0, 180))
		maxTwistZAngle = math.radians(
			readFloat(data['maxTwistZAngle'], 'max twist Z angle', 0, 180))

		motorTypeStr = str(data.get('motorType', 'Disabled'))
		try:
			motorType = getattr(ConstraintMotorType, motorTypeStr)
		except AttributeError:
			raise Exception('Invalid motor type "' + motorTypeStr + '".')
		if motorType == ConstraintMotorType.Velocity:
			raise Exception(
				'SwingTwistPhysicsConstraint doesn\'t support motor type of "Velocity".')

		motorTargetOrientationData = data.get('motorTargetOrientation')
		if motorTargetOrientationData:
			if (not isinstance(motorTargetOrientationData, list) or
					len(motorTargetOrientationData) != 3):
				raise Exception('SwingTwistPhysicsConstraint "motorTargetOrientationData" must be '
					'an array of three floats.')
			motorTargetOrientation = eulerToQuaternion(*(
				readFloat(value, 'motor target orientation')
				for value in motorTargetOrientationData))
		else:
			motorTargetOrientation = None

		maxMotorTorque = readFloat(data.get('maxMotorTorque', 0), 'max motor torque', 0)
	except (TypeError, ValueError):
		raise Exception('SwingTwistPhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('SwingTwistPhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0

	SwingTwistConstraint.Start(builder)
	SwingTwistConstraint.AddFirstActor(builder, firstActorOffset)
	SwingTwistConstraint.AddFirstPosition(builder, CreateVector3f(builder, *firstPosition))
	SwingTwistConstraint.AddFirstOrientation(builder, CreateQuaternion4f(builder, *firstOrientation))
	SwingTwistConstraint.AddSecondActor(builder, secondActorOffset)
	SwingTwistConstraint.AddSecondPosition(builder, CreateVector3f(builder, *secondPosition))
	SwingTwistConstraint.AddSecondOrientation(builder, CreateQuaternion4f(builder, *secondOrientation))
	SwingTwistConstraint.AddMaxSwingXangle(builder, maxSwingXAngle)
	SwingTwistConstraint.AddMaxSwingYangle(builder, maxSwingYAngle)
	SwingTwistConstraint.AddMaxTwistZangle(builder, maxTwistZAngle)
	SwingTwistConstraint.AddMotorType(builder, motorType)
	SwingTwistConstraint.AddMotorTargetOrientation(builder,
		CreateQuaternion4f(builder, *motorTargetOrientation) if motorTargetOrientation else 0)
	SwingTwistConstraint.AddMaxMotorTorque(builder, maxMotorTorque)
	constraintOffset = SwingTwistConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.SwingTwistConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
