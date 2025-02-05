# Copyright 2020-2024 Aaron Barany
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

import base64
import json
import os
import struct
from .ModelConvert import ModelVertexStream, ModelGeometryData, VertexAttrib, addModelType, \
	modelVertexAttribEnum

class Object:
	pass

class Buffer:
	def __init__(self, path, bufferInfo, binData, binDataOffset):
		uri = bufferInfo.get('uri')
		if uri:
			dataPrefix = 'data:application/octet-stream;base64,'
			if uri.startswith(dataPrefix):
				try:
					self.uri = None
					self.dataRef = None
					self.data = base64.b64decode(uri[len(dataPrefix):])
				except:
					raise Exception('Invalid buffer data for GLTF file "' + path + '".')
			else:
				parentDir = os.path.dirname(path)
				self.uri = os.path.join(parentDir, uri)
				self.dataRef = None
				self.data = None
		elif binData:
			length = bufferInfo['byteLength']
			if not isinstance(length, int) or binDataOffset + length > len(binData):
				raise Exception('Invalid buffer data for GLTF file "' + path + '".')
			self.uri = None
			self.dataRef = (binData, binDataOffset, length)
			self.data = None
		else:
			raise KeyError('uri')

	def load(self):
		if self.data is not None:
			return self.data
		elif self.uri:
			with open(self.uri, 'rb') as f:
				self.data = f.read()
			self.uri = None
		elif self.dataRef:
			offset = self.dataRef[1]
			length = self.dataRef[2]
			self.data = self.dataRef[0][offset:offset + length]
			self.dataRef = None
		return self.data

class BufferView:
	def __init__(self, buffer, offset, length, stride):
		self.buffer = buffer
		self.offset = offset
		self.length = length
		self.stride = stride

class Accessor:
	def __init__(self, bufferView, itemType, decorator, itemSize, offset, count):
		self.bufferView = bufferView
		self.type = itemType
		self.decorator = decorator
		self.itemSize = itemSize
		self.offset = offset
		self.count = count

	def extractData(self):
		offset = self.offset + self.bufferView.offset
		bufferData = self.bufferView.buffer.load()
		if not self.bufferView.stride:
			return bufferData[offset:offset + self.itemSize*self.count]

		data = bytearray()
		for i in range(0, self.count):
			curOffset = offset + i*self.bufferView.stride
			data += self.bufferView.buffer[curOffset:curOffset + self.itemSize]
		return data

gltfVertexAttribEnum = {
	'POSITION': modelVertexAttribEnum['Position'],
	'NORMAL': modelVertexAttribEnum['Normal'],
	'TANGENT': modelVertexAttribEnum['Tangent'],
	'TEXCOORD_0': modelVertexAttribEnum['TexCoord0'],
	'TEXCOORD`1': modelVertexAttribEnum['TexCoord1'],
	'TEXCOORD`2': modelVertexAttribEnum['TexCoord2'],
	'TEXCOORD`3': modelVertexAttribEnum['TexCoord3'],
	'TEXCOORD`4': modelVertexAttribEnum['TexCoord4'],
	'TEXCOORD`5': modelVertexAttribEnum['TexCoord5'],
	'TEXCOORD`6': modelVertexAttribEnum['TexCoord6'],
	'TEXCOORD`7': modelVertexAttribEnum['TexCoord7'],
	'COLOR_0': modelVertexAttribEnum['Color0'],
	'COLOR_1': modelVertexAttribEnum['Color1'],
	'JOINTS_0': modelVertexAttribEnum['BlendIndices'],
	'WEIGHTS_0': modelVertexAttribEnum['BlendWeights'],
}

gltfTypeMap = {
	('SCALAR', 5120, True): ('X8', 'SNorm', 1),
	('SCALAR', 5120, False): ('X8', 'Int', 1),
	('SCALAR', 5121, True): ('X8', 'UNorm', 1),
	('SCALAR', 5121, False): ('X8', 'UInt', 1),
	('SCALAR', 5122, True): ('X16', 'SNorm', 2),
	('SCALAR', 5122, False): ('X16', 'Int', 2),
	('SCALAR', 5123, True): ('X16', 'UNorm', 2),
	('SCALAR', 5123, False): ('X16', 'UInt', 2),
	('SCALAR', 5125, False): ('X32', 'UInt', 4),
	('SCALAR', 5126, False): ('X32', 'Float', 4),

	('VEC2', 5120, True): ('X8Y8', 'SNorm', 2),
	('VEC2', 5120, False): ('X8Y8', 'Int', 2),
	('VEC2', 5121, True): ('X8Y8', 'UNorm', 2),
	('VEC2', 5121, False): ('X8Y8', 'UInt', 2),
	('VEC2', 5122, True): ('X16Y16', 'SNorm', 4),
	('VEC2', 5122, False): ('X16Y16', 'Int', 4),
	('VEC2', 5123, True): ('X16Y16', 'UNorm', 4),
	('VEC2', 5123, False): ('X16Y16', 'UInt', 4),
	('VEC2', 5126, False): ('X32Y32', 'Float', 8),

	('VEC3', 5120, True): ('X8Y8Z8', 'SNorm', 3),
	('VEC3', 5120, False): ('X8Y8Z8', 'Int', 3),
	('VEC3', 5121, True): ('X8Y8Z8', 'UNorm', 3),
	('VEC3', 5121, False): ('X8Y8Z8', 'UInt', 3),
	('VEC3', 5122, True): ('X16Y16Z16', 'SNorm', 6),
	('VEC3', 5122, False): ('X16Y16Z16', 'Int', 6),
	('VEC3', 5123, True): ('X16Y16Z16', 'UNorm', 6),
	('VEC3', 5123, False): ('X16Y16Z16', 'UInt', 6),
	('VEC3', 5126, False): ('X32Y32Z32', 'Float', 12),

	('VEC4', 5120, True): ('X8Y8Z8W8', 'SNorm', 4),
	('VEC4', 5120, False): ('X8Y8Z8W8', 'Int', 4),
	('VEC4', 5121, True): ('X8Y8Z8W8', 'UNorm', 4),
	('VEC4', 5121, False): ('X8Y8Z8W8', 'UInt', 4),
	('VEC4', 5122, True): ('X16Y16Z16W16', 'SNorm', 8),
	('VEC4', 5122, False): ('X16Y16Z16W16', 'Int', 8),
	('VEC4', 5123, True): ('X16Y16Z16W16', 'UNorm', 8),
	('VEC4', 5123, False): ('X16Y16Z16W16', 'UInt', 8),
	('VEC4', 5126, False): ('X32Y32Z32W32', 'Float', 16),

	('MAT4', 5126, False): ('MAT4', 'Float', 64)
}

gltfPrimitiveTypeMap = ['PointList', 'LineList', 'LineStrip', 'LineStrip', 'TriangleList',
	'TriangleStrip', 'TriangleFan']

def extractGLTFOrGLBBuffersViewsAccessors(path, jsonData, binData):
	binOffset = 0
	try:
		# Read the buffers.
		buffers = []
		bufferInfos = jsonData['buffers']
		try:
			for bufferInfo in bufferInfos:
				curBuffer = Buffer(path, bufferInfo, binData, binOffset)
				if curBuffer.dataRef:
					paddedLength = ((curBuffer.dataRef[2] + 3)//4)*4
					binOffset += paddedLength
				buffers.append(curBuffer)
		except (TypeError, ValueError):
			raise Exception('Buffers must be an array of objects for GLTF file "' + path + '".')
		except KeyError as e:
			raise Exception('Buffer doesn\'t contain element "' + str(e) +
				'" for GLTF file "' + path + '".')

		# Read the buffer views.
		bufferViews = []
		bufferViewInfos = jsonData['bufferViews']
		try:
			for bufferViewInfo in bufferViewInfos:
				try:
					buffer = buffers[bufferViewInfo['buffer']]
				except (IndexError, TypeError):
					raise Exception('Invalid buffer index for GLTF file "' + path + '".')
				offset = bufferViewInfo['byteOffset']
				length = bufferViewInfo['byteLength']
				stride = bufferViewInfo.get('byteStride', 0)
				bufferViews.append(BufferView(buffer, offset, length, stride))
		except (TypeError, ValueError):
			raise Exception(
				'Buffer views must be an array of objects for GLTF file "' + path + '".')
		except KeyError as e:
			raise Exception('Buffer view doesn\'t contain element "' + str(e) +
				'" for GLTF file "' + path + '".')

		# Read the accessors.
		accessors = []
		accessorInfos = jsonData['accessors']
		try:
			for accessorInfo in accessorInfos:
				try:
					bufferView = bufferViews[accessorInfo['bufferView']]
				except (IndexError, TypeError):
					raise Exception('Invalid buffer view index for GLTF file "' + path + '".')

				gltfType = accessorInfo['type']
				componentType = accessorInfo['componentType']
				normalized = accessorInfo.get('normalized', False)
				try:
					accessorType, decorator, itemSize = \
						gltfTypeMap[(gltfType, componentType, normalized)]
				except (KeyError, TypeError):
					raise Exception('Invalid accessor type (' + str(gltfType) + ', ' +
						str(componentType) + ') for GLTF file "' + path + '".')

				offset = accessorInfo.get('byteOffset', 0)
				count = accessorInfo['count']
				accessors.append(
					Accessor(bufferView, accessorType, decorator, itemSize, offset, count))
		except (TypeError, ValueError):
			raise Exception('Accessors must be an array of objects for GLTF file "' + path + '".')
		except KeyError as e:
			raise Exception('Accessor doesn\'t contain element "' + str(e) +
				'" for GLTF file "' + path + '".')
	except (TypeError, ValueError):
		raise Exception('Root value in GLTF file "' + path + '" must be an object.')
	except KeyError as e:
		raise Exception('GLTF file "' + path + '" doesn\'t contain element "' + str(e) + '".')

	return buffers, bufferViews, accessors

def convertGLTFOrGLBModel(convertContext, path, jsonData, binData):
	buffers, bufferViews, accessors = extractGLTFOrGLBBuffersViewsAccessors(path, jsonData, binData)

	try:
		# Read the meshes.
		meshes = []
		meshInfos = jsonData['meshes']
		try:
			meshIndex = 0
			for meshInfo in meshInfos:
				meshName = meshInfo.get('name', 'mesh' + str(meshIndex))

				primitiveInfos = meshInfo['primitives']
				try:
					primitiveIndex = 0
					for primitiveInfo in primitiveInfos:
						mesh = Object()
						mesh.attributes = []
						mesh.name = meshName
						if len(primitiveInfos) > 1:
							mesh.name += '.' + str(primitiveIndex)
							primitiveIndex += 1

						try:
							for attrib, index in primitiveInfo['attributes'].items():
								if attrib not in gltfVertexAttribEnum:
									raise Exception('Unsupported attribute "' + str(attrib) +
										'" for GLTF file "' + path + '".')
								try:
									mesh.attributes.append((gltfVertexAttribEnum[attrib],
										accessors[index]))
								except (IndexError, TypeError):
									raise Exception('Invalid accessor index for GLTF file "' +
										path + '".')
						except (TypeError, ValueError):
							raise Exception(
								'Mesh primitives attributes must be an object containing attribute '
								'mappings for GLTF file "' + path + '".')

						if 'indices' in primitiveInfo:
							try:
								mesh.indices = accessors[primitiveInfo['indices']]
							except (IndexError, TypeError):
								raise Exception(
									'Invalid accessor index for GLTF file "' + path + '".')
						else:
							mesh.indices = None

						mode = primitiveInfo.get('mode', 4)
						try:
							mesh.primitiveType = gltfPrimitiveTypeMap[mode]
						except (IndexError, TypeError):
							raise Exception('Unsupported primitive mode for GLTF file "' + path + '".')

						meshes.append(mesh)
				except (TypeError, ValueError):
					raise Exception(
						'Mesh primitives must be an array of objects for GLTF file "' + path + '".')
				except KeyError as e:
					raise Exception('Mesh primitives doesn\'t contain element "' + str(e) +
						'" for GLTF file "' + path + '".')

				meshIndex += 1
		except (TypeError, ValueError):
			raise Exception('Meshes must be an array of objects for GLTF file "' + path + '".')
		except KeyError as e:
			raise Exception('Mesh doesn\'t contain element "' + str(e) + '" for GLTF file "' +
				path + '".')

	except (TypeError, ValueError):
		raise Exception('Root value in GLTF file "' + path + '" must be an object.')
	except KeyError as e:
		raise Exception('GLTF file "' + path + '" doesn\'t contain element "' + str(e) + '".')

	# Convert meshes to geometry list. GLTF uses separate vertex streams rather than interleved
	# vertices, so the index buffer will need to be separate for each. This will have some
	# data duplication during processing, but isn't expected to be a large amount in practice.
	geometry = []
	for mesh in meshes:
		if mesh.indices:
			if mesh.indices.type == 'X16':
				indexSize = 2
			elif mesh.indices.type == 'X32':
				indexSize = 4
			else:
				raise Exception('Unsupported index type "' + mesh.indices.type +
					'" for GLTF file "' + path + '".')
			indexData = mesh.indices.extractData()
		else:
			indexData = None
			indexSize = 0

		vertexStreams = []
		for attrib, accessor in mesh.attributes:
			if not accessor.type.startswith('X'):
				raise Exception('Unsupported vertex type "' + accessor.type + '" for GLTF file "' +
					path + ".'")
			vertexFormat = [VertexAttrib(attrib, accessor.type, accessor.decorator)]
			vertexStreams.append(ModelVertexStream(vertexFormat, accessor.extractData(),
				indexSize, indexData))

		geometry.append(ModelGeometryData(mesh.name, vertexStreams, mesh.primitiveType))
	return geometry

def readGLTFData(path):
	"""
	Reads glTF data, returning the JSON data as a dict.
	"""
	with open(path) as f:
		try:
			return json.load(f)
		except:
			raise Exception('Invalid glTF file "' + path + '".')

def readGLBData(path):
	"""
	Reads GLB data, returning a tuple for the JSON data as a dict and the binary data.
	"""
	with open(path, 'rb') as f:
		try:
			if f.read(4) != b'glTF' or struct.unpack('I', f.read(4))[0] != 2 or not f.read(4):
				raise Exception()

			length = struct.unpack('I', f.read(4))[0]
			if f.read(4) != b'JSON':
				raise Exception()

			jsonData = json.loads(f.read(length))

			length = struct.unpack('I', f.read(4))[0]
			if f.read(4) != b'BIN\0':
				raise Exception()

			binData = f.read(length)
			return jsonData, binData
		except:
			raise Exception('Invalid GLTF file "' + path + '".')

def convertGLTFModel(convertContext, path):
	"""
	Converts an glTF model for use with ModelConvert.

	If the "name" element is provided for a mesh, it will be used for the name of the model
	geometry. Otherwise, the name will be "mesh#", where # is the index of the mesh. If multiple
	sets of primitives are used, the index will be appended to the name, separated with '.'.

	Limitations:
	- Only meshes and dependent data (accessors, buffer views, and buffers) are extracted. All other
	  parts of the scene are ignored, including transforms.
	- Morph targets aren't supported.
	- Materials aren't read, and are instead provided in the DeepSea scene configuration.
	- Buffer data may either be embedded or a file path relative to the main model file. General
	  URIs are not supported.
	"""
	data = readGLTFData(path)
	return convertGLTFOrGLBModel(convertContext, path, data, None)

def convertGLBModel(convertContext, path):
	"""
	Converts an GLB model for use with ModelConvert.

	If the "name" element is provided for a mesh, it will be used for the name of the model
	geometry. Otherwise, the name will be "mesh#", where # is the index of the mesh. If multiple
	sets of primitives are used, the index will be appended to the name, separated with '.'.

	Limitations:
	- Only meshes and dependent data (accessors, buffer views, and buffers) are extracted. All other
	  parts of the scene are ignored, including transforms.
	- Morph targets aren't supported.
	- Materials aren't read, and are instead provided in the DeepSea scene configuration.
	- Buffer data may either be embedded or a file path relative to the main model file. General
	  URIs are not supported.
	"""
	jsonData, binData = readGLBData(path)
	return convertGLTFOrGLBModel(convertContext, path, jsonData, binData)

def registerGLTFModelType(convertContext):
	"""
	Registers the GLTF model type under the name "gltf" and GLB under the name "glb".
	"""
	addModelType(convertContext, 'gltf', convertGLTFModel)
	addModelType(convertContext, 'glb', convertGLBModel)
