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

import flatbuffers
from .Helpers import readFloat
from DeepSeaPhysics.Axis import Axis
from DeepSeaPhysics.ShapeUnion import ShapeUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Cone
from DeepSeaPhysics import Shape

def convertPhysicsConeOffset(convertContext, data, inputDir, builder):
	try:
		height = readFloat(data['height'], 0, 'height')
		radius = readFloat(data['radius'], 0, 'radius')

		axisStr = str(data['axis'])
		try:
			axis = getattr(Axis, axisStr)
		except AttributeError:
			raise Exception('Invalid axis "' + axisStr + '".')

		convexRadius = readFloat(data.get('convexRadius', -1), 'convex radius')
	except (TypeError, ValueError):
		raise Exception('PhysicsCone data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsCone data doesn\'t contain element ' + str(e) + '.')

	Cone.Start(builder)
	Cone.AddHeight(builder, height)
	Cone.AddRadius(builder, radius)
	Cone.AddAxis(builder, axis)
	Cone.AddConvexRadius(builder, convexRadius)
	coneOffset = Cone.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.Cone)
	Shape.AddShape(builder, coneOffset)
	return Shape.End(builder)

def convertPhysicsCone(convertContext, data, inputDir, outputDir):
	"""
	Converts a PhysicsCone. The data map is expected to contain the following elements:
	- height: the height of the cone.
	- radius: the radius of the cone.
	- axis: the axis of the cone. Valid values are X, Y, and Z.
	- convexRadius: the convex radius for collision checks. If unset or a value < 0 the physics
	  system's default will be used.
	"""
	builder = flatbuffers.Builder(0)
	builder.Finish(convertPhysicsConeOffset(convertContext, data, inputDir, builder))
	return builder.Output()
