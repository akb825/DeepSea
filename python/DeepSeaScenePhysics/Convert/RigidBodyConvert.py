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
from .CustomMassPropertiesConvert import convertCustomMassProperties
from .Helpers import eulerToQuaternion, readFloat, readInt
from .ShapeInstanceConvert import convertShapeInstance
from DeepSeaPhysics import ShapeInstance
from DeepSeaPhysics.DOFMask import DOFMask
from DeepSeaPhysics.MotionType import MotionType
from DeepSeaPhysics.PhysicsLayer import PhysicsLayer
from DeepSeaPhysics.RigidBodyFlags import RigidBodyFlags
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import RigidBody

def convertRigidBody(convertContext, data):
	"""
	Converts a RigidBody. The data map is expected to contain the following elements:
	- group: the name of the rigid body group, or unset if not part of a group.
	- flags: list of flags control the behavior of the rigid body. See the dsRigidBodyFlags enum
	  for the valid values, omitting the type prefix.
	- motionType: the type of motion for the rigid body. See the dsPhysicsMotionType enum for valid
	  values, omitting the type prefix.
	- dofMask: list of DOF mask values to apply. See the dsPhysicsDOFMask enum for valid values,
	  omitting the type prefix. Defaults to ["All"].
	- layer: the physics layer the rigid body is a member of. See the dsPhysicsLayer enum for
	  valid values, omitting the type prefix.
	- collisionGroup: integer ID for the collision group. Defaults to 0 if not provided.
	- customMassProperties: either a shifted mass or mass properties to customize the mass and
	  inertia. If unset, the mass properties will be computed based on the shapes in the rigid
	  body. If set, it is expected to contain the following elements based on the type of mass
	  properties:
	  Shafted mass:
	  - orientationPointShift: array of 3 floats for the offset to shift the orientation point. A value of
	    all zeros will be the center of mass, while non-zero values will adjust the point of
	    orientation when the object is in free-fall. For example, to make the rigid body top or bottom
	    heavy.
	  - mass: the mass for the rigid body. If unset or a value < 0, the mass will be computed by the
	    shapes.
	  Full mass properties:
	  - centeredInertia: 2D array of floats for the moment of inertia as a 3x3 tensor matrix,
	    centered around the center of mass. Each inner array corresponds to a column of the matrix.
	    The matrix should be symmetrical, such that the transpose is the same.
	  - centerOfMass: array of 3 floats for the center of mass relative to the local space of the
	    rigid body.
	  - mass: the mass of the rigid body.
	  - inertiaTranslate: array of 3 floats for the translation for the frame of reference of the
	    inertia tensor. This will be the point around which the object will rotate when in
	    free-fall. If unset, the center of mass will be used.
	  - inertiaRotate: array with x, y, z Euler angles in degrees for the orientation of the frame of
	    reference of the inertia tensor. If unset, the identity orientation will be used.
	- position: array of 3 floats for the position of the body in world space. If unset, the origin
	  will be used.
	- orientation: array with x, y, z Euler angles for the orientation of the body in world space. If
	  unset, the identity orientation will be used.
	- scale: array of 3 floats for the scale of the body. If unset, a scale of 1 will be used.
	- linearVelocity: array of 3 floats for the initial linear velocity of the body. If unset, the
	  body will have no linear motion.
	- angularVelocity: array of 3 floats for the initial angular velocity of the body along each
	  axis. If unset, the body will have no linear motion.
	- friction: the coefficient of friction, with 0 meaning no friction and increasing values having
	  higher friction
	- restitution: the restitution value, where 0 is fully inelastic and 1 is fully elastic.
	- hardness: the hardness value, where 0 indicates to use this body's restitution on collision
	  and 1 indicates to use the other body's restitution.
	- linearDamping: linear damping factor in the range [0, 1] to reduce the velocity over time.
	  If unset or a value < 0 the default will be used.
	- angularDamping: angular damping factor in the range [0, 1] to reduce the velocity over time.
	  If unset or a value < 0 the default will be used.
	- maxLinearVelocity: the maximum linear velocity. If unset or a value < 0 the default will be
	  used.
	- maxAngularVelocity: the maximum angular velocity. If unset or a value < 0 the default will be
	  used.
	- shapes: array of objects for the shapes to use with the rigid body. Each element of the array
	  is expected to have the following members:
	  - type: the type of the shape. See the different shape resource types, removing the "Physics"
	    prefix. (e.g. instead of "PhysicsSphere" use just "Sphere") The members for the
	    corresponding shape resource type should be used as well.
	  - density: the density of the shape for computing its mass.
	  - translate: array of 3 floats for the translation of the shape. If unset, the shape will
	    never have a translation applied beyond the transform of the body itself.
	  - rotate: array x, y, z Euler angles in degrees for the orientation of the shape. If unset, the
	    shape will never have a orientation applied beyond the transform of the body itself.
	  - scale: array of 3 floats for the scale of the shape. If unset, the shape will never have a
	    scale applied beyond the transform of the body itself.
	  - material: material to apply to the shape. If unset, the material values for the body will
	    be used. If set, it is expected to be an object with the following elements:
	    - friction: the coefficient of friction, with 0 meaning no friction and increasing values
	      having higher friction
	    - restitution: the restitution value, where 0 is fully inelastic and 1 is fully elastic.
	    - hardness: the hardness value, where 0 indicates to use this body's restitution on
	      collision and 1 indicates to use the other body's restitution. 
	"""
	builder = flatbuffers.Builder(0)
	try:
		group = str(data.get('group', ''))

		flags = 0
		flagsData = data.get('flags')
		if flagsData:
			if not isinstance(flagsData, list):
				raise Exception('RigidBody flags must be an array of strings.')
			for flag in flagsData:
				flagStr = str(flag)
				try:
					flags = flags | getattr(RigidBodyFlags, flagStr)
				except AttributeError:
					raise Exception('Invalid rigid body flag "' + flagStr + '".')

		motionTypeStr = str(data['motionType'])
		try:
			motionType = getattr(MotionType, motionTypeStr)
		except AttributeError:
			raise Exception('Invalid motion type "' + motionTypeStr + '".')

		dofMaskData = data.get('dofMask')
		if dofMaskData is None:
			dofMask = DOFMask.All
		else:
			dofMask = 0
			if not isinstance(dofMaskData, list):
				raise Exception('RigidBody DOF mask must be an array of strings.')
			for dof in dofMaskData:
				dofStr = str(dof)
				try:
					dofMask = dofMask | getattr(DOFMask, dofStr)
				except AttributeError:
					raise Exception('Invalid DOF mask "' + dofStr + '".')

		layerStr = str(data['layer'])
		try:
			layer = getattr(PhysicsLayer, layerStr)
		except AttributeError:
			raise Exception('Invalid physics layer "' + layerStr + '".')

		collisionGroup = readInt(data.get('collisionGroup', 0), 'collision group', 0)

		try:
			massPropertiesType, massPropertiesOffset = convertCustomMassProperties(convertContext,
				data.get('customMassProperties'), builder)
		except (TypeError, ValueError):
			raise Exception('RigidBody custom mass properties must be an object.')
		except KeyError as e:
			raise Exception(
				'RigidBody custom mass properties doesn\'t contain element ' + str(e) + '.')

		positionData = data.get('position')
		if positionData:
			if not isinstance(positionData, list) or len(positionData) != 3:
				raise Exception(
					'RigidBody position must ben an array of three floats.')
			position = (readFloat(value, 'position') for value in positionData)
		else:
			position = None

		orientationData = data.get('orientation')
		if orientationData:
			if not isinstance(orientationData, list) or len(orientationData) != 3:
				raise Exception(
					'RigidBody orientation must ben an array of three floats.')
			orientation = eulerToQuaternion(*(readFloat(value, 'orientation')
				for value in orientationData))
		else:
			orientation = None

		scaleData = data.get('scale')
		if scaleData:
			if not isinstance(scaleData, list) or len(scaleData) != 3:
				raise Exception(
					'RigidBody scale must ben an array of three floats.')
			scale = (readFloat(value, 'scale') for value in scaleData)
		else:
			scale = None

		linearVelocityData = data.get('linearVelocity')
		if linearVelocityData:
			if not isinstance(linearVelocityData, list) or len(linearVelocityData) != 3:
				raise Exception(
					'RigidBody linear velocity must ben an array of three floats.')
			linearVelocity = (readFloat(value, 'linear velocity') for value in linearVelocityData)
		else:
			linearVelocity = None

		angularVelocityData = data.get('angularVelocity')
		if angularVelocityData:
			if not isinstance(angularVelocityData, list) or len(angularVelocityData) != 3:
				raise Exception(
					'RigidBody angular velocity must ben an array of three floats.')
			angularVelocity = (readFloat(value, 'angular velocity')
				for value in angularVelocityData)
		else:
			angularVelocity = None

		friction = readFloat(data['friction'], 'friction', 0)
		restitution = readFloat(data['restitution'], 'restitution', 0, 1)
		hardness = readFloat(data['hardness'], 'hardness', 0, 1)
		linearDamping = readFloat(data.get('linearDamping', -1), 'linear damping', 0, 1)
		angularDamping = readFloat(data.get('angularDamping', -1), 'angular damping', 0, 1)
		maxLinearVelocity = readFloat(data.get('maxLinearVelocity', -1), 'max linear velocity')
		maxAngularVelocity = readFloat(data.get('maxAngularVelocity', -1), 'max angular velocity')

		shapesData = data['shapes']
		shapes = []
		try:
			for shapeData in shapesData:
				shapes.append(convertShapeInstance(convertContext, shapeData, builder))
		except (AttributeError, TypeError, ValueError):
			raise Exception('RigidBody shape instances must be an array of objects.')
		except KeyError as e:
			raise Exception('RigidBody shape instance doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('RigidBody data must be an object.')
	except KeyError as e:
		raise Exception('RigidBody data doesn\'t contain element ' + str(e) + '.')

	groupOffset = builder.CreateString(group) if group else 0

	RigidBody.StartShapesVector(builder, len(shapes))
	for shape in reversed(shapes):
		builder.PrependUOffsetTRelative(shape)
	shapesOffset = builder.EndVector()

	RigidBody.Start(builder)
	RigidBody.AddGroup(builder, groupOffset)
	RigidBody.AddFlags(builder, flags)
	RigidBody.AddMotionType(builder, motionType)
	RigidBody.AddDofMask(builder, dofMask)
	RigidBody.AddLayer(builder, layer)
	RigidBody.AddCollisionGroup(builder, collisionGroup)
	RigidBody.AddCustomMassPropertiesType(builder, massPropertiesType)
	RigidBody.AddCustomMassProperties(builder, massPropertiesOffset)
	RigidBody.AddPosition(builder, CreateVector3f(builder, *position) if position else None)
	RigidBody.AddOrientation(builder, CreateQuaternion4f(builder, *orientation) if orientation else None)
	RigidBody.AddScale(builder, CreateVector3f(builder, *scale) if scale else None)
	RigidBody.AddLinearVelocity(builder,
		CreateVector3f(builder, *linearVelocity) if linearVelocity else None)
	RigidBody.AddAngularVelocity(builder,
		CreateVector3f(builder, *angularVelocity) if angularVelocity else None)
	RigidBody.AddFriction(builder, friction)
	RigidBody.AddRestitution(builder, restitution)
	RigidBody.AddHardness(builder, hardness)
	RigidBody.AddLinearDamping(builder, linearDamping)
	RigidBody.AddAngularDamping(builder, angularDamping)
	RigidBody.AddMaxLinearVelocity(builder, maxLinearVelocity)
	RigidBody.AddMaxAngularVelocity(builder, maxAngularVelocity)
	RigidBody.AddShapes(builder, shapesOffset)
	builder.Finish(RigidBody.End(builder))
	return builder.Output()
