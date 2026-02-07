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

import os
import struct

import flatbuffers
from .Helpers import readFloat, readInt
from DeepSeaPhysics.ShapePartMaterial import CreateShapePartMaterial
from DeepSeaPhysics.ShapeUnion import ShapeUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Mesh
from DeepSeaPhysics import Shape
from DeepSeaScene.Convert.ModelConvert import VertexAttrib, convertModelGeometry, loadModel, \
	modelVertexAttribEnum, readVertexAttrib

def convertPhysicsMeshOffset(convertContext, data, inputDir, builder):
	try:
		vertexData = data.get('vertices')
		vertices = []
		indices = []
		triangleMaterials = []
		materialIndices = []
		if vertexData:
			if not isinstance(vertexData, list) or len(vertexData) % 3 != 0:
				raise Exception(
					'PhysicsMesh "vertices" must be a list of floats with a length divisible by 3.')
			for f in vertexData:
				vertices.append(readFloat(f, 'vertex'))

			indexData = data['indices']
			if not isinstance(indexData, list) or len(indexData) % 3 != 0:
				raise Exception(
					'PhysicsMesh "indices" must be a list of ints with a length divisible by 3.')
			for i in indexData:
				indices.append(readInt(i, 'index', 0))

			triangleMaterialData = data.get('triangleMaterials', [])
			if not isinstance(triangleMaterialData, list) or len(triangleMaterialData) % 3 != 0:
				raise Exception('PhysicsMesh "triangleMaterials" must be a list of floats with a '
					'length divisible by 3.')
			for f in triangleMaterialData:
				triangleMaterials.append(readFloat(f, 'triangle material', 0))

			materialIndexData = data.get('materialIndices', [])
			if not isinstance(materialIndexData, list):
				raise Exception('PhysicsMesh "materialIndices" must be a list of ints.')
			for i in materialIndexData:
				materialIndices.append(readInt(i, 'material index', 0))
		else:
			path = str(data['path'])
			modelType = str(data.get('modelType', ''))
			if not modelType:
				modelType = os.path.splitext(path)[1]
				if modelType:
					modelType = modelType[1:]
				if not modelType:
					raise Exception('PhysicsMesh geometry has no known model type.')
			component = str(data.get('component'))

			finalPath = os.path.join(inputDir, path)
			geometryDataList = loadModel(convertContext, modelType, finalPath)
			modelBytes = bytearray()
			modelGeometry = convertModelGeometry(convertContext, geometryDataList, [component],
				[[VertexAttrib(modelVertexAttribEnum['Position'], 'X32Y32Z32', 'Float')]], 4,
				None, modelBytes, modelType = modelType, path = finalPath)[component]
			if not modelGeometry.primitiveType.startsWith('TriangleList'):
				raise Exception('PhysicsMesh model "' + path + '" component "' + component +
					'" must be a triangle list.')
			offset = modelGeometry.vertexBuffers[0].offset
			for i in range(modelGeometry.vertexCount/3):
				vertices.extend(struct.unpack_from('fff', modelBytes, offset + i*12))
			offset = modelGeometry.indexBuffers[0].offset
			for i in range(modelGeometry.indexBuffers[0].indexCount/3):
				indices.extend(struct.unpack_from('III', modelBytes, offset + i*12))

			triangleMeshAttribData = data.get('triangleMeshAttrib')
			if triangleMeshAttribData is not None:
				triangleMeshAttrib = readVertexAttrib(triangleMeshAttrib)
				frictionScale = readFloat(data.get('frictionScale', 1), 'friction scale')
				modelBytes = bytearray()
				convertModelGeometry(convertContext, geometryDataList, [component],
					[[VertexAttrib(triangleMeshAttrib, 'X32Y32Z32', 'Float')]], None, None,
					modelBytes, modelType = modelType, path = path)[component]
				uniqueMaterials = dict()
				for i in range(len(modelBytes)/36):
					vertexMaterialValues = struct.unpack_from('fffffffff', modelBytes, i*36)
					material0 = vertexMaterialValues[0:3]
					material1 = vertexMaterialValues[3:6]
					material1 = vertexMaterialValues[6:9]
					triangleMaterial = (
						(material0[0] + material1[0] + material1[0])*frictionScale/3,
						(material0[1] + material1[1] + material1[1])/3,
						(material0[2] + material1[2] + material1[2])/3)
					materialIndex = uniqueMaterials.get(triangleMaterial)
					if materialIndex is None:
						materialIndex = len(triangleMaterials)
						uniqueMaterials[triangleMaterial] = materialIndex
						triangleMaterials.extend(triangleMaterial)
					materialIndices.append(materialIndex)

		if bool(triangleMaterials) != bool(materialIndices):
			raise Exception('PhysicsMesh triangle materials and material indices are mismatched.')
		if materialIndices and len(materialIndices) != len(indices)/3:
			raise Exception('PhysicsMesh indices and material indices are mismatched.')

		cacheName = str(data.get('cacheName', ''))
	except (TypeError, ValueError):
		raise Exception('PhysicsMesh data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsMesh data doesn\'t contain element ' + str(e) + '.')

	Mesh.StartVerticesVector(builder, len(vertices))
	for f in reversed(vertices):
		builder.PrependFloat32(f)
	verticesOffset = builder.EndVector()

	UINT16_MAX = 65535

	if len(vertices) <= UINT16_MAX:
		Mesh.StartIndices16Vector(builder, len(indices))
		for i in reversed(indices):
			builder.PrependUint16(i)
		indices16Offset = builder.EndVector()
		indices32Offset = 0
	else:
		indices16Offset = 0
		Mesh.StartIndices32Vector(builder, len(indices))
		for i in reversed(indices):
			builder.PrependUint32(i)
		indices32Offset = builder.EndVector()

	if triangleMaterials and materialIndices:
		materialCount = len(triangleMaterials)/3
		Mesh.StartTriangleMaterialsVector(builder, materialCount)
		for i in reversed(range(0, len(triangleMaterials), 3)):
			offset = CreateShapePartMaterial(builder, triangleMaterials[i],
				triangleMaterials[i + 1], triangleMaterials[i + 2])
			builder.PrependUOffsetTRelative(offset)
		triangleMaterialsOffset = builder.EndVector()

		if materialCount <= UINT16_MAX:
			Mesh.StartMaterialIndices16Vector(builder, len(materialIndices))
			for i in reversed(materialIndices):
				builder.PrependUint16(i)
			materialIndices16Offset = builder.EndVector()
			materialIndices32Offset = 0
		else:
			materialIndices16Offset = 0
			Mesh.StartMaterialIndices32Vector(builder, len(materialIndices))
			for i in reversed(materialIndices):
				builder.PrependUint32(i)
			materialIndices32Offset = builder.EndVector()
	else:
		triangleMaterialsOffset = 0
		materialIndices16Offset = 0
		materialIndices32Offset = 0

	if cacheName:
		cacheNameOffset = builder.CreateString(cacheName)
	else:
		cacheNameOffset = 0

	Mesh.Start(builder)
	Mesh.AddVertices(builder, verticesOffset)
	Mesh.AddIndices16(builder, indices16Offset)
	Mesh.AddIndices32(builder, indices32Offset)
	Mesh.AddTriangleMaterials(builder, triangleMaterialsOffset)
	Mesh.AddMaterialIndices16(builder, materialIndices16Offset)
	Mesh.AddMaterialIndices32(builder, materialIndices32Offset)
	Mesh.AddCacheName(builder, cacheNameOffset)
	meshOffset = Mesh.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.Mesh)
	Shape.AddShape(builder, meshOffset)
	return Shape.End(builder)

def convertPhysicsMesh(convertContext, data, inputDir, outputDir):
	"""
	Converts a PhysicsMesh. The data map is expected to contain the following elements:
	- The following elements are used when providing data from a model:
	  - modelType: the name of the model type, such as "obj" or "gltf". If omitted, the type is
	    inferred from the path extension.
	  - path: the path to the geometry.
	  - component: the name of the component of the model. The component must use a triangle list.
	  - triangleMaterialAttrib: the attribute to gather per-triangle material values from. The
	    attribute must have three values per vertex, corresponding to friction, restitution, and
	    hardness, respectively. The values for for each vertex that comprises a triangle will be
		averaged. Most commonly the color attribute will be used, as it is the easiest to set in
	    modeling programs. If not set, no per-trinagle materials will be used.
	  - frictionScale: scale value to apply to the friction for the per-triangle material
	    attributes. This can be used to allow for friction values > 1 when used with normalized
	    attributes, such as colors. Defaults to 1.
	- The following elements are used when providing data directly:
	  - vertices: array of floats for the raw vertex data. This must be divisible by 3, with each
	    vertex having three values.
	  - indices: array of ints for the indices for each triangle. Must be divisible by 3, with each
	    triangle having three values
	  - triangleMaterials: array of floats for the raw per-triangle material. This must be divisible
	    by 3, with each triangle having three values for the friction, restitution, and hardness,
	    respectively. Defaults to no per-triangle materials.
	  - materialIndices: array of ints for which material each triangle uses.
	- cacheName: name used for caching pre-computed data. If not set, the pre-computed data will
	  not be cached.
	"""
	builder = flatbuffers.Builder(0)
	builder.Finish(convertPhysicsMeshOffset(convertContext, data, inputDir, builder))
	return builder.Output()
