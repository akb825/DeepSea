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
from DeepSeaPhysics import RigidBodyTemplate

def convertRigidBodyTemplate(convertContext, data):
	"""
	Converts a RigidBodyTemplate. The data map is expected to contain the following elements:
	- flags: list of flags control the behavior of the rigid body. See the dsRigidBodyFlags enum
	  for the valid values, omitting the type prefix.
	- motionType: the type of motion for the rigid body. See the dsPhysicsMotionType enum for valid
	  values, omitting the type prefix.
	- dofMask: list of DOF mask values to apply. See the dsPhysicsDOFMask enum for valid values,
	  omitting the type prefix.
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
		flags = 0
		flagsData = data.get('flags')
		if flagsData:
			if not isinstance(flagsData, list):
				raise Exception('RigidBodyTemplate flags must be an array of strings.')
			for flag in flagsData:
				flagStr = str(flag)
				if not hasattr(RigidBodyFlags, flagStr):
					raise Exception('Invalid rigid body flag "' + flagStr + '".')
				flags = flags | getattr(RigidBodyFlags, flagStr)

		motionTypeStr = str(data['motionType'])
		if not hasattr(MotionType, motionTypeStr):
			raise Exception('Invalid motion type "' + motionTypeStr + '".')
		motionType = getattr(MotionType, motionTypeStr)

		dofMask = 0
		dofMaskData = data['dofMask']
		if not isinstance(dofMaskData, list):
			raise Exception('RigidBodyTemplate DOF mask must be an array of strings.')
		for dof in dofMaskData:
			dofStr = str(dof)
			if dofStr == 'TransAll':
				dofMask = dofMask | DOFMask.TransX | DOFMask.TransY | DOFMask.TransY
			elif dofMask == 'RotAll':
				dofMask = dofMask | DOFMask.RotX | DOFMask.RotY | DOFMask.RotY
			elif dofMask == 'All':
				dofMask = (dofMask | DOFMask.TransX | DOFMask.TransY | DOFMask.TransY |
					DOFMask.RotX | DOFMask.RotY | DOFMask.RotY)
			else:
				if not hasattr(DOFMask, dofStr):
					raise Exception('Invalid DOF mask "' + dofStr + '".')
				dofMask = dofMask | getattr(DOFMask, dofStr)

		layerStr = str(data['layer'])
		if not hasattr(PhysicsLayer, layerStr):
			raise Exception('Invalid physics layer "' + layerStr + '".')
		layer = getattr(PhysicsLayer, layerStr)

		collisionGroup = readInt(data.get('collisionGroup', 0), 'collision group', 0)

		try:
			massPropertiesType, massPropertiesOffset = convertCustomMassProperties(convertContext,
				data.get('customMassProperties'), builder)
		except (TypeError, ValueError):
			raise Exception('RigidBodyTemplate custom mass properties must be an object.')
		except KeyError as e:
			raise Exception(
				'RigidBodyTemplate custom mass properties doesn\'t contain element ' + str(e) + '.')

		friction = readFloat(data['friction'], 'friction', 0)
		restitution = readFloat(data['restitution'], 'restitution', 0)
		hardness = readFloat(data['hardness'], 'hardness', 0)
		linearDamping = readFloat(data.get('linearDamping', -1), 'linear damping')
		angularDamping = readFloat(data.get('angularDamping', -1), 'angular damping')
		maxLinearVelocity = readFloat(data.get('maxLinearVelocity', -1), 'max linear velocity')
		maxAngularVelocity = readFloat(data.get('maxAngularVelocity', -1), 'max angular velocity')

		shapesData = data['shapes']
		shapes = []
		try:
			for shapeData in shapesData:
				shapes.append(convertShapeInstance(convertContext, shapeData, builder))
		except (AttributeError, TypeError, ValueError):
			raise Exception('RigidBodyTemplate shape instances must be an array of objects.')
		except KeyError as e:
			raise Exception('RigidBodyTemplate shape instance doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('RigidBodyTemplate data must be an object.')
	except KeyError as e:
		raise Exception('RigidBodyTemplate data doesn\'t contain element ' + str(e) + '.')

	RigidBodyTemplate.StartShapesVector(builder, len(shapes))
	for shape in reversed(shapes):
		builder.PrependUOffsetTRelative(shape)
	shapesOffset = builder.EndVector(builder)

	RigidBodyTemplate.Start(builder)
	RigidBodyTemplate.AddFlags(builder, flags)
	RigidBodyTemplate.AddMotionType(builder, motionType)
	RigidBodyTemplate.AddDofMask(builder, dofMask)
	RigidBodyTemplate.AddLayer(builder, layer)
	RigidBodyTemplate.AddCollisionGroup(builder, collisionGroup)
	RigidBodyTemplate.AddCustomMassPropertiesType(builder, massPropertiesType)
	RigidBodyTemplate.AddCustomMassProperties(builder, massPropertiesOffset)
	RigidBodyTemplate.AddFriction(builder, friction)
	RigidBodyTemplate.AddRestitution(builder, restitution)
	RigidBodyTemplate.AddHardness(builder, hardness)
	RigidBodyTemplate.AddLinearDamping(builder, linearDamping)
	RigidBodyTemplate.AddAngularDamping(builder, angularDamping)
	RigidBodyTemplate.AddMaxLinearVelocity(builder, maxLinearVelocity)
	RigidBodyTemplate.AddMaxAngularVelocity(builder, maxAngularVelocity)
	RigidBodyTemplate.AddShapes(builder, shapesOffset)
	builder.Finish(RigidBodyTemplate.End(builder))
	return builder.Output()
