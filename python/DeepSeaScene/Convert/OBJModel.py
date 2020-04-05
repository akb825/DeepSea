# Copyright 2020 Aaron Barany
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

import struct
from .ModelNodeConvert import ModelNodeVertexStream, ModelNodeGeometryData, addModelType
from .SceneResourcesConvert import modelVertexAttribEnum

def convertOBJModel(convertContext, path):
	"""
	Converts an OBJ model for use with ModelNodeConvert.

	Each object and group is a separate group name. The name will be:
	* object.group (for object name and group name )when an object contains multiple groups.
	* object name for objects that have no group.
	* "<unnamed>" for no object or group.

	Limitations:
	* Faces must be triangles.
	* Lines may only have two indices each.
	* Smooth groups aren't supported.
	* Free-form geometry (with "vp") isn't supported.
	* Materials aren't read, and are instead provided in the DeepSea scene configuration.
	"""

	data = object()
	data.positions = bytearray()
	data.positionCount = 0
	data.positionIndices = bytearray()

	data.texCoords = bytearray()
	data.texCoordCount = 0
	data.texCoordIndices = bytearray()

	data.normals = bytearray()
	data.normalCount = 0
	data.normalIndices = bytearray()

	data.primitiveType = None

	objectName = '<unnamed>'
	groupName = None

	geometry = []
	def addGeometry():
		if not data.primitiveType:
			return

		streams = [ModelNodeVertexStream((modelVertexAttribEnum['Position'], 'X32Y32Z32W32',
			'Float'), data.positions, 4, data.positionIndices)]
		data.positionIndices = bytearray()

		if data.texCoordIndices:
			streams.append(ModelNodeVertexStream((modelVertexAttribEnum['TexCoord0'], 'X32Y32Z32',
				'Float'), data.texCoords, 4, data.texCoordIndices))
			data.texCoordIndices = bytearray()

		if data.normalIndices:
			streams.append(ModelNodeVertexStream((modelVertexAttribEnum['Normal'], 'X32Y32Z32',
				'Float'), data.texCoords, 4, data.texCoordIndices))
			data.normalIndices = bytearray()

		if groupName:
			name = objectName + '.' + groupName
		else:
			name = objectName
		geometry.append(ModelNodeGeometryData(name, streams, data.primitiveType))
		data.primitiveType = None

	def resolveIndex(index, count, indexType):
		if index < 0:
			realIndex = count + index
		else:
			realIndex = index - 1

		if not 0 <= realIndex < count:
			raise Exception('Invalid ' + indexType + ' index value "' + str(index) +
				'" in obj file "' + path + '".')
		return realIndex

	def setPrimitiveType(primitiveType):
		if not data.primitiveType:
			data.primitiveType = primitiveType
		elif data.primitiveType != primitiveType:
			raise Exception(
				'Cannot mix primitive types in the same group in obj file "' + path + '".')

	with open(path) as f:
		for line in f:
			tokens = line.split()
			if not tokens:
				continue

			try:
				command = tokens[0]
				if command == 'o':
					addGeometry()
					objectName = tokens[1]
					groupName = None
				elif command == 'g':
					addGeometry()
					groupName = tokens[1]
				elif command == 'v':
					x = float(tokens[1])
					y = float(tokens[2])
					z = float(tokens[3])
					if len(tokens) > 4:
						w = float(tokens[4])
					else:
						w = 1.0
					struct.pack_into(data.positions, len(data.positions), '4f', x, y, z, w)
					data.positionCount += 1
				elif command == 'vt':
					u = float(tokens[1])
					if len(tokens) > 2:
						v = float(tokens[2])
					else:
						v = 0.0
					if len(tokens) > 3:
						w = float(tokens[3])
					else:
						w = 0.0
					struct.pack_into(data.texCoords, len(data.texCoords), '3f', u, v, w)
					data.texCoordCount += 1
				elif command == 'vn':
					x = float(tokens[1])
					y = float(tokens[2])
					z = float(tokens[3])
					struct.pack_into(data.normals, len(data.normals), '3f', x, y, z)
					data.normalCount += 1
				elif command == 'f':
					setPrimitiveType('TriangleList')
					for vertex in (tokens[1], tokens[2], tokens[3]):
						indices = vertex.split('/')
						if not indices or len(indices) > 3:
							raise IndexError()

						struct.pack_into(data.positionIndices, len(data.positionIndices), 'u',
							resolveIndex(int(indices[0]), data.positionCount, 'position'))
						if len(indices) > 1 and indices[1]:
							struct.pack_into(data.texCoordIndices, len(data.texCoordIndices), 'u',
								resolveIndex(int(indices[1]), data.texCoordCount, 'tex coord'))
						if len(indices) > 2:
							struct.pack_into(data.normalIndices, len(data.normalIndices), 'u',
								resolveIndex(int(indices[2]), data.normalCount, 'normal'))
				elif command == 'l':
					setPrimitiveType('LineList')
					for index in (tokens[1], tokens[2]):
						struct.pack_into(data.positionIndices, len(data.positionIndices), 'u',
							resolveIndex(int(index), data.positionCount, 'position'))
			except (IndexError, TypeError):
				raise Exception('Invalid OBJ file "' + path + '".')

	# Add the last geometry element.
	addGeometry()
	return geometry

def registerOBJType(convertContext):
	"""
	Registers the OBJ model type under the name "obj".
	"""
	addModelType(convertContext, 'obj', convertOBJModel)
