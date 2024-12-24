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
from DeepSeaPhysics import Capsule
from DeepSeaPhysics import Shape

def convertPhysicsCapsuleOffset(convertContext, data, builder):
	try:
		halfHeight = readFloat(data['halfHeight'], 0, 'half height')
		radius = readFloat(data['radius'], 0, 'radius')

		axisStr = str(data['axis'])
		try:
			axis = getattr(Axis, axisStr)
		except AttributeError:
			raise Exception('Invalid axis "' + axisStr + '".')
	except (TypeError, ValueError):
		raise Exception('PhysicsCapsule data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsCapsule data doesn\'t contain element ' + str(e) + '.')

	Capsule.Start(builder)
	Capsule.AddHalfHeight(builder, halfHeight)
	Capsule.AddRadius(builder, radius)
	Capsule.AddAxis(builder, axis)
	capsuleOffset = Capsule.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.Capsule)
	Shape.AddShape(builder, capsuleOffset)
	return Shape.End(builder)

def convertPhysicsCapsule(convertContext, data):
	"""
	Converts a PhysicsCapsule. The data map is expected to contain the following elements:
	- halfHeight: half the height of the capsule.
	- radius: the radius of the capsule.
	- axis: the axis of the capsule. Valid values are X, Y, and Z.
	"""
	builder = flatbuffers.Builder(0)
	builder.Finish(convertPhysicsCapsuleOffset(convertContext, data, builder))
	return builder.Output()
