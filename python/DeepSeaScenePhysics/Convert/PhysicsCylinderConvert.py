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
from .Helpers import readFloat
from DeepSeaPhysics.Axis import Axis
from DeepSeaPhysics.ShapeUnion import ShapeUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Cylinder
from DeepSeaPhysics import Shape

def convertPhysicsCylinderOffset(convertContext, data, builder):
	try:
		halfHeight = readFloat(data['halfHeight'], 0, 'half height')
		radius = readFloat(data['radius'], 0, 'radius')

		axisStr = str(data['axis'])
		try:
			axis = getattr(Axis, axisStr)
		except AttributeError:
			raise Exception('Invalid axis "' + axisStr + '".')

		convexRadius = readFloat(data.get('convexRadius', -1), 'convex radius')
	except (TypeError, ValueError):
		raise Exception('PhysicsCylinder data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsCylinder data doesn\'t contain element ' + str(e) + '.')

	Cylinder.Start(builder)
	Cylinder.AddHalfHeight(builder, halfHeight)
	Cylinder.AddRadius(builder, radius)
	Cylinder.AddAxis(builder, axis)
	Cylinder.AddConvexRadius(builder, convexRadius)
	cylinderOffset = Cylinder.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.Cylinder)
	Shape.AddShape(builder, cylinderOffset)
	return Shape.End(builder)

def convertPhysicsCylinder(convertContext, data, outputDir):
	"""
	Converts a PhysicsCylinder. The data map is expected to contain the following elements:
	- halfHeight: half the height of the cylinder.
	- radius: the radius of the cylinder.
	- axis: the axis of the cylinder. Valid values or X, Y, and Z.
	- convexRadius: the convex radius for collision checks. If unset or a value < 0 the physics
	  system's default will be used.
	"""
	builder = flatbuffers.Builder(0)
	builder.Finish(convertPhysicsCylinderOffset(convertContext, data, builder))
	return builder.Output()
