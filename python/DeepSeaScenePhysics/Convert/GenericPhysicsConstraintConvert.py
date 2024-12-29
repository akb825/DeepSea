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
from DeepSeaPhysics.ConstraintLimitType import ConstraintLimitType
from DeepSeaPhysics.ConstraintUnion import ConstraintUnion
from DeepSeaPhysics.DOF import DOF
from DeepSeaPhysics.GenericConstraintLimit import CreateGenericConstraintLimit
from DeepSeaPhysics.GenericConstraintMotor import CreateGenericConstraintMotor
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Constraint
from DeepSeaPhysics import GenericConstraint

class Object:
	pass

def convertGenericPhysicsConstraint(convertContext, data):
	"""
	Converts a GenericPhysicsConstraint. The data map is expected to contain the following
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
	- limits: the limits for the degrees of freedom. Missing DOfs will have their limits implicitly
	  set to Free. Each array element is expected to have the following members:
	  - dof: the degree of freedom. See the dsPhysicsConstraintDOF enum for valid values, omitting
	    the type prefix.
	  - limitType: the type of limit. See the dsPhysicsConstraintLimitType enum for valid values,
	    omitting the type prefix.
	  - minValue: the minimum value. For the rotation DOFs the value will be in degrees.
	  - maxValue: the maximum value. For the rotation DOFs the value will be in degrees.
	  - stiffness: the stiffness of the spring to limit the value.
	  - damping: the damping in the range of [0, 1] for the spring to limit the value.
	- motors: the motors for the degrees of freedom. Missing DOFs will have their motors implicitly
	  disabled. Each array element is expected to have the following members:
	  - dof: the degree of freedom. See the dsPhysicsConstraintDOF enum for valid values, omitting
	    the type prefix.
	  - motorType: the type of the motor to apply. See the dsPhysicsConstraintMotorType enum for
	    valid values, omitting the type prefix. Defaults to Disabled.
	  - target: the target of the motor, as either a position or velocity. Rotation DOFs have the
	    target in degrees or degrees/second.
	  - maxForce: the maximum force or torque of the motor. If the motor is disabled this is the
	    maximum amount of force to apply to stop motion.
	- combineSwingTwistMotors: whether the swing and twist motors are combined. Defaults to false.
	"""
	try:
		firstActor = str(data.get('firstActor', ''))

		firstPositionData = data['firstPosition']
		if not isinstance(firstPositionData, list) or len(firstPositionData) != 3:
			raise Exception(
				'GenericPhysicsConstraint "firstPosition" must be an array of three floats.')
		firstPosition = (readFloat(value, 'first position') for value in firstPositionData)

		firstOrientationData = data['firstOrientation']
		if not isinstance(firstOrientationData, list) or len(firstOrientationData) != 3:
			raise Exception(
				'GenericPhysicsConstraint "firstOrientation" must be an array of three floats.')
		firstOrientation = eulerToQuaternion(*(readFloat(value, 'first orientation')
			for value in firstOrientationData))

		secondActor = str(data.get('secondActor', ''))

		secondPositionData = data['secondPosition']
		if not isinstance(secondPositionData, list) or len(secondPositionData) != 3:
			raise Exception(
				'GenericPhysicsConstraint "secondPosition" must be an array of three floats.')
		secondPosition = (readFloat(value, 'second position') for value in secondPositionData)

		secondOrientationData = data['secondOrientation']
		if not isinstance(secondOrientationData, list) or len(secondOrientationData) != 3:
			raise Exception(
				'GenericPhysicsConstraint "secondOrientation" must be an array of three floats.')
		secondOrientation = eulerToQuaternion(*(readFloat(value, 'second orientation')
			for value in secondOrientationData))

		limitsData = data.get('limits')
		if limitsData:
			limits = []
			try:
				for limitData in limitsData:
					limit = Object()

					dofStr = str(limitData['dof'])
					try:
						limit.dof = getattr(DOF, dofStr)
					except AttributeError:
						raise Exception('Invalid DOF "' + dofStr + '".')

					limitTypeStr = str(limitData['limitType'])
					try:
						limit.limitType = getattr(ConstraintLimitType, limitTypeStr)
					except AttributeError:
						raise Exception('Invalid limit type "' + limitTypeStr + '".')

					limit.minValue = readFloat(limitData['minValue'], 'min value')
					limit.maxValue = readFloat(limitData['maxValue'], 'max value')
					if limit.dof in (DOF.RotateX, DOF.RotateY, DOF.RotateZ):
						limit.minValue = math.radians(limit.minValue)
						limit.maxValue = math.radians(limit.maxValue)
					limit.stiffness = readFloat(limitData['stiffness'], 'stiffness', 0)
					limit.damping = readFloat(limitData['damping'], 'damping', 0, 1)

					limits.append(limit)
			except (AttributeError, TypeError, ValueError):
				raise Exception('GenericPhysicsConstraint "limits" must be an array of objects.')
			except KeyError as e:
				raise Exception('GenericPhysicsConstraint "limits" doesn\'t contain element ' +
					str(e) + '.')

		motorsData = data.get('motors')
		if motorsData:
			motors = []
			try:
				for motorData in motorsData:
					motor = Object()

					dofStr = str(motorData['dof'])
					try:
						motor.dof = getattr(DOF, dofStr)
					except AttributeError:
						raise Exception('Invalid DOF "' + dofStr + '".')

					motorTypeStr = str(motorData['motorType'])
					try:
						motor.motorType = getattr(ConstraintMotorType, motorTypeStr)
					except AttributeError:
						raise Exception('Invalid motor type "' + motorTypeStr + '".')

					motor.target = readFloat(motorData['target'], 'target')
					if motor.dof in (DOF.RotateX, DOF.RotateY, DOF.RotateZ):
						motor.target = math.radians(motor.target)
					motor.maxForce = readFloat(motorData['maxForce'], 'max force', 0)

					motors.append(motor)
			except (AttributeError, TypeError, ValueError):
				raise Exception('GenericPhysicsConstraint "motors" must be an array of objects.')
			except KeyError as e:
				raise Exception('GenericPhysicsConstraint "motors" doesn\'t contain element ' +
					str(e) + '.')
		else:
			motors = None

		combineSwingTwistMotors = bool(data.get('combineSwingTwistMotors'))
	except (TypeError, ValueError):
		raise Exception('GenericPhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception('GenericPhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	firstActorOffset = builder.CreateString(firstActor) if firstActor else 0
	secondActorOffset = builder.CreateString(secondActor) if secondActor else 0

	if limits:
		GenericConstraint.StartLimitsVector(builder, len(limits))
		for limit in reversed(limits):
			builder.PrependUOffsetTRelative(CreateGenericConstraintLimit(builder, limit.dof,
				limit.limitType, limit.minValue, limit.maxValue, limit.stiffness, limit.damping))
		limitsOffset = builder.EndVector()
	else:
		limitsOffset = 0

	if motors:
		GenericConstraint.StartMotorsVector(builder, len(motors))
		for motor in reversed(motors):
			builder.PrependUOffsetTRelative(CreateGenericConstraintMotor(builder, motor.dof,
				motor.motorType, motor.target, motor.maxForce))
		motorsOffset = builder.EndVector()
	else:
		motorsOffset = 0

	GenericConstraint.Start(builder)
	GenericConstraint.AddFirstActor(builder, firstActorOffset)
	GenericConstraint.AddFirstPosition(builder, CreateVector3f(builder, *firstPosition))
	GenericConstraint.AddFirstOrientation(builder, CreateQuaternion4f(builder, *firstOrientation))
	GenericConstraint.AddSecondActor(builder, secondActorOffset)
	GenericConstraint.AddSecondPosition(builder, CreateVector3f(builder, *secondPosition))
	GenericConstraint.AddSecondOrientation(builder, CreateQuaternion4f(builder, *secondOrientation))
	GenericConstraint.AddLimits(builder, limitsOffset)
	GenericConstraint.AddMotors(builder, motorsOffset)
	GenericConstraint.AddCombineSwingTwistMotors(builder, combineSwingTwistMotors)
	constraintOffset = GenericConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.GenericConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
