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

from DeepSeaPhysics.ShapeUnion import ShapeUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Box
from DeepSeaPhysics import Shape

def convertPhysicsBox(convertContext, data):
	"""
	Converts a PhysicsBox. The data map is expected to contain the following elements:
	- halfExtents: array of 3 floats for the half extents of the box. The full box geometry ranges
	  from -halfExtents to +halfExtents.
	- convexRadius: the convex radius for collision checks. If unset or a value < 0 the physics
	  system's default will be used.
	"""
	def readFloat(value, name, minValue = None):
		try:
			floatVal = float(value)
			if minValue is not None and value < minValue:
				raise Exception()
			return floatVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	try:
		halfExtentsData = data['halfExtents']
		if not isinstance(halfExtentsData, list) or len(halfExtentsData) != 3:
			raise Exception('PhysicsBox halfExtents must be an array of three floats.')
		halfExtents = (readFloat(value, 'halfExtents', 0) for value in halfExtentsData)

		convexRadius = readFloat(data.get('convexRadius', -1), 'convexRadius')
	except (TypeError, ValueError):
		raise Exception('PhysicsBox data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsBox data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	Box.Start(builder)
	Box.AddHalfExtents(builder, CreateVector3f(builder, *halfExtents))
	Box.AddConvexRadius(builder, convexRadius)
	boxOffset = Box.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.Box)
	Shape.AddShape(builder, boxOffset)
	builder.Finish(Shape.End(builder))
	return builder.Output()
