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

from DeepSeaPhysics.Axis import Axis
from DeepSeaPhysics.ShapeUnion import ShapeUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Sphere
from DeepSeaPhysics import Shape

def convertPhysicsSphere(convertContext, data):
	"""
	Converts a PhysicsSphere. The data map is expected to contain the following elements:
	- radius: the radius of the sphere.
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
		radius = readFloat(data['radius'], 0, 'radius')
	except (TypeError, ValueError):
		raise Exception('PhysicsSphere data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsSphere data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	Sphere.Start(builder)
	Sphere.AddRadius(builder, radius)
	sphereOffset = Sphere.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.Sphere)
	Shape.AddShape(builder, sphereOffset)
	builder.Finish(Shape.End(builder))
	return builder.Output()
