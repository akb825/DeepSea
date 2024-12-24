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
from .Helpers import eulerToQuaternion, readFloat
from .PhysicsBoxConvert import convertPhysicsBoxOffset
from .PhysicsCapsuleConvert import convertPhysicsCapsuleOffset
from .PhysicsConeConvert import convertPhysicsConeOffset
from .PhysicsConvexHullConvert import convertPhysicsConvexHullOffset
from .PhysicsCylinderConvert import convertPhysicsCylinderOffset
from .PhysicsMeshConvert import convertPhysicsMeshOffset
from .PhysicsShapeRefConvert import convertPhysicsShapeRefOffset
from .PhysicsSphereConvert import convertPhysicsSphereOffset
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.ShapePartMaterial import CreateShapePartMaterial
from DeepSeaPhysics.ShapeUnion import ShapeUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import ShapeInstance

shapeTypeToConvertFunc = {
	ShapeUnion.Box: convertPhysicsBoxOffset,
	ShapeUnion.Capsule: convertPhysicsCapsuleOffset,
	ShapeUnion.Cone: convertPhysicsConeOffset,
	ShapeUnion.ConvexHull: convertPhysicsConvexHullOffset,
	ShapeUnion.Cylinder: convertPhysicsCylinderOffset,
	ShapeUnion.Mesh: convertPhysicsMeshOffset,
	ShapeUnion.ShapeRef: convertPhysicsShapeRefOffset,
	ShapeUnion.Sphere: convertPhysicsSphereOffset
}

def convertShapeInstance(convertContext, data, builder):
	shapeType = str(data['type'])
	try:
		shapeConvertFunc = shapeTypeToConvertFunc[getattr(ShapeUnion, shapeType)]
	except AttributeError:
		raise Exception('Invalid shape type "' + shapeType + '".')
	shapeOffset = shapeConvertFunc(convertContext, data, builder)

	density = readFloat(data['density'], 'density', 0)

	translateData = data.get('translate')
	if translateData:
		if not isinstance(translateData, list) or len(translateData) != 3:
			raise Exception(
				'PhysicsShapeInstance translate must ben an array of three floats.')
		translate = (readFloat(translateData[0], 'translate'),
			readFloat(translateData[1], 'translate'),
			readFloat(translateData[2], 'translate'))
	else:
		translate = None

	rotateData = data.get('rotate')
	if rotateData:
		if not isinstance(rotateData, list) or len(rotateData) != 3:
			raise Exception(
				'PhysicsShapeInstance rotate must ben an array of three floats.')
		rotate = eulerToQuaternion(readFloat(rotateData[0], 'rotate'),
			readFloat(rotateData[1], 'rotate'),
			readFloat(rotateData[2], 'rotate'))
	else:
		rotate = None

	scaleData = data.get('scale')
	if scaleData:
		if not isinstance(scaleData, list) or len(scaleData) != 3:
			raise Exception(
				'PhysicsShapeInstance scale must ben an array of three floats.')
		scale = (readFloat(scaleData[0], 'scale'),
			readFloat(scaleData[1], 'scale'),
			readFloat(scaleData[2], 'scale'))
	else:
		scale = None

	materialData = data.get('material')
	if materialData:
		try:
			material = (readFloat(materialData['friction'], 'friction', 0),
				readFloat(materialData['restitution'], 'restitution', 0),
				readFloat(materialData['hardness'], 'hardness', 0))
		except (TypeError, ValueError):
			raise Exception('PhysicsShapeInstance material data must be an object.')
		except KeyError as e:
			raise Exception(
				'PhysicsShapeInstance material data doesn\'t contain element ' + str(e) + '.')
	else:
		material = None

	ShapeInstance.Start(builder)
	ShapeInstance.AddShape(builder, shapeOffset)
	ShapeInstance.AddDensity(builder, density)
	ShapeInstance.AddTranslate(builder, CreateVector3f(builder, *translate) if translate else 0)
	ShapeInstance.AddRotate(builder, CreateQuaternion4f(builder, *rotate) if rotate else 0)
	ShapeInstance.AddScale(builder, CreateVector3f(builder, *scale) if scale else 0)
	ShapeInstance.AddMaterial(builder,
		CreateShapePartMaterial(builder, *material) if material else 0)
	return ShapeInstance.End(builder)
