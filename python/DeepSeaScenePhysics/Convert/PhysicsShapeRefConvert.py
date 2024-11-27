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
from DeepSeaPhysics import ShapeRef
from DeepSeaPhysics import Shape

def convertPhysicsShapeRef(convertContext, data):
	"""
	Converts a PhysicsShapeRef. The data map is expected to contain the following elements:
	- shape: the name of the referenced shape.
	"""
	try:
		shapeName = str(data['shape'])
	except (TypeError, ValueError):
		raise Exception('PhysicsShapeRef data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsShapeRef data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)
	
	shapeNameOffset = builder.CreateString(shapeName)

	ShapeRef.Start(builder)
	ShapeRef.AddName(builder, shapeNameOffset)
	shapeRefOffset = ShapeRef.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.ShapeRef)
	Shape.AddShape(builder, shapeRefOffset)
	builder.Finish(Shape.End(builder))
	return builder.Output()
