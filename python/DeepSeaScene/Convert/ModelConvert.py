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

import base64
from copy import copy
import json
import os
from subprocess import Popen, PIPE

from ..PrimitiveType import PrimitiveType

validModelVertexTransforms = {
	'Identity',
	'Bounds',
	'UNormToSNorm',
	'SNormToUNorm'
}

modelVertexAttribEnum = {
	'Position': 0,
	'Position0': 0,
	'Position1': 1,
	'Normal': 2,
	'Color': 3,
	'Color0': 3,
	'Color1': 4,
	'FogCoord': 5,
	'Tangent': 6,
	'Bitangent': 7,
	'TexCoord0': 8,
	'TexCoord1': 9,
	'TexCoord2': 10,
	'TexCoord3': 11,
	'TexCoord4': 12,
	'TexCoord5': 13,
	'TexCoord6': 14,
	'TexCoord7': 15,
	'BlendIndices': 14,
	'BlendWeights': 15
}

class VertexAttrib:
	"""
	Class containing the format for a single vertex attribute. The following members are present:
	- attrib: the vertex abbtribute.
	- format: the vertex format.
	- decoration: the decoration to apply to the vertex.
	"""
	def __init__(self, attrib, attribFormat, decoration):
		self.attrib = attrib
		self.format = attribFormat
		self.decoration =decoration

class ModelVertexStream:
	"""
	Class containing information for a vertex stream of a model. The following members are present:
	- vertexFormat: array of vertex attributes defining the vertex format.
	- vertexData: bytes for the vertices.
	- indexSize: the number of bytes for each index. A value of 0 indicates no indices.
	- indexData: bytes for the indices, or None if no indices.
	"""
	def __init__(self, vertexFormat, vertexData, indexSize = 0, indexData = None):
		self.vertexFormat = vertexFormat
		self.vertexData = vertexData
		self.indexSize = indexSize
		self.indexData = indexData

class ModelGeometryData:
	"""
	Class containing data for a piece of geometry. The following members are present:
	- name: the name of the geometry.
	- vertexStreams: array of ModelVertexStream objects for the vertices.
	- primitiveType: the type of primitives to use.
	- patchPoints: the number of patch points. This must be non-zero of primitiveType is
	  'PatchList'.
	"""
	def __init__(self, name, vertexStreams, primitiveType = 'TriangleList', patchPoints = 0):
		self.name = name
		self.vertexStreams = vertexStreams
		if not hasattr(PrimitiveType, primitiveType):
			raise Exception('Invalid geometry primitive type "' + primitiveType + '".')
		self.primitiveType = primitiveType
		if primitiveType == 'PatchList' and patchPoints < 1:
			raise Exception(
				'Geometry patch points must be provided when primitiveType is "PatchPoints".')
		self.patchPoints = patchPoints

class VertexBuffer:
	"""
	Class containing data for a vertex buffer. The following members are present:
	- vertexFormat: list of VertexAttrib instances to describe the vertex buffer.
	- offset: the offset into the buffer.
	"""
	def __init__(self, vertexFormat, offset):
		self.vertexFormat = vertexFormat
		self.offset = offset

class IndexBuffer:
	"""
	Class containing data for an index buffer. The following members are present:
	- indexSize: the size of each index.
	- firstIndex: the first index to use within the buffer.
	- indexCount: the number of indices.
	- vertexOffset: the index of the first vertex, offsetting the index values.
	- offset: the offset into the buffer.
	"""
	def __init__(self, indexSize, firstIndex, indexCount, vertexOffset, offset):
		self.indexSize = indexSize
		self.firstIndex = firstIndex
		self.indexCount = indexCount
		self.vertexOffset = vertexOffset
		self.offset = offset

class ConvertedModelGeometry:
	"""
	Class containing data for the model geometry. The following members are present:
	- vertexCount: the number of vertices.
	- vertexBuffers: list of vertex buffers for the model.
	- primitiveType: the type of primitive to render as.
	- indexBuffers: the index buffers for the model.
	"""
	def __init__(self, vertexCount, vertexBuffers, primitiveType, indexBuffers):
		self.vertexCount = vertexCount
		self.vertexBuffers = vertexBuffers
		self.primitiveType = primitiveType
		self.indexBuffers = indexBuffers

def addModelType(convertContext, typeName, convertFunc):
	"""
	Adds a model type with the name and the convert function.

	The function should take the ConvertContext and path to the model to convert, and should return
	an array of ModelNodeGeometryData objects for the contents of the model.
	An exception will be raised if the type is already registered.
	"""
	if not hasattr(convertContext, 'modelTypeMap'):
		convertContext.modelTypeMap = dict()

	if typeName in convertContext.modelTypeMap:
		raise Exception('Model type "' + typeName + '" is already registered.')
	convertContext.modelTypeMap[typeName] = convertFunc

def readVertexAttrib(attrib):
	if not isinstance(attrib, int):
		attribStr = str(attrib)
		attrib = modelVertexAttribEnum.get(attribStr)
		if attrib is None:
			raise Exception('Invalid vertex attribute "' + attribStr + '".')
	return attrib


def loadModel(convertContext, modelType, path):
	"""
	Loads a model, returning a list of ModelGeometryData for the components of the model.
	"""
	convertFunc = convertContext.modelTypeMap.get(modelType) \
		if hasattr(convertContext, 'modelTypeMap') else None
	if not convertFunc:
		raise Exception('Model type "' + modelType + '" hasn\'t been registered.')
	return convertFunc(convertContext, path)
	

def convertModelGeometry(convertContext, geometryDataList, includedComponents, vertexFormat,
		indexSize, transforms, combinedBuffer, modelBounds = None, pointsOnly = False,
		modelType = None, path = None):
	"""
	Converts geometry from a model, where geometryDataList is the list of geometry data from
	loadModel(). The model type and path may be provided for error reporting.

	vertexFormat provides a 2D list of VertexAttrib instances, describing the vertex buffers and
	which attributes they privde. indexSize should be 2 or 4 when indices are used, or None if no
	indices are used. transforms is a list of pairs for a vertex attribute and transform as defined
	by VertexAttribConvert. The converted data will be appended to combinedBuffer (as a bytearray),
	and the bounds will be added to modelBounds as a 2x3 array of floats for the min and max
	positions.

	If pointsOnly is set to True, then the vertices will be converted to a point array. This can
	be useful when the primitives are discarded.

	The results of this function will be a mapping from component name to ConvertedModelGeometry
	instances.
	"""
	def appendBuffer(combinedBuffer, data, pad = True):
		# Some graphics APIs require offsets to be aligned to 4 bytes.
		if pad:
			for i in range(len(combinedBuffer) % 4):
				combinedBuffer.append(0)

		offset = len(combinedBuffer)
		combinedBuffer.extend(data)
		return offset

	def getIndexType(indexSize):
		if indexSize == 2:
			return 'UInt16'
		elif indexSize == 4:
			return 'UInt32'
		else:
			return None

	vfcVertexAttrib = []
	for streamFormat in vertexFormat:
		vfcStreamFormat = []
		for vertexAttrib in streamFormat:
			vfcStreamFormat.append({
				'name': str(vertexAttrib.attrib),
				'layout': vertexAttrib.format,
				'type': vertexAttrib.decoration
			})
		vfcVertexAttrib.append(vfcStreamFormat)

	indexType = getIndexType(indexSize)

	vfcTransforms = []
	if transforms:
		for attrib, transform in transforms:
			if transform not in validModelVertexTransforms:
				raise Exception('Invalid geometry transform "' + str(transform) + '".')
			vfcTransforms.append({'name': str(attrib), 'transform': transform})

	convertedGeometry = dict()
	try:
		for geometryData in geometryDataList:
			if geometryData.name not in includedComponents:
				continue

			if geometryData.name in convertedGeometry:
				baseErrorStr = 'Geometry data "' + geometryData.name + \
					'" appears multiple times in model'
				if path:
					raise Exception(baseErrorStr + ' "' + path + '".')
				else:
					raise Exception(baseErrorStr + '.')

			# Prepare the input for the vfc tool.
			vertexStreams = []
			for vertexStream in geometryData.vertexStreams:
				streamVertexAttrib = []
				for attribFormat in vertexStream.vertexFormat:
					streamVertexAttrib.append({
						'name': str(attribFormat.attrib),
						'layout': attribFormat.format,
						'type': attribFormat.decoration
					})

				vfcVertexStream = {
					'vertexFormat': streamVertexAttrib,
					'vertexData': 'base64:' + base64.b64encode(vertexStream.vertexData).decode()
				}
				if vertexStream.indexSize > 0:
					vfcVertexStream['indexType'] = getIndexType(vertexStream.indexSize)
					vfcVertexStream['indexData'] = 'base64:' + \
						base64.b64encode(vertexStream.indexData).decode()
				vertexStreams.append(vfcVertexStream)

			primitiveType = 'PointList' if pointsOnly else geometryData.primitiveType
			vfcPrimitiveType = primitiveType
			if vfcPrimitiveType == 'TriangleListAdjacency':
				vfcPrimitiveType = 'TriangleList'
			elif vfcPrimitiveType == 'TriangleStripAdjacency':
				vfcPrimitiveType = 'TriangleStrip'
			vfcInput = {
				'vertexFormat': vfcVertexAttrib,
				'indexType': indexType,
				'primitiveType': vfcPrimitiveType,
				'patchPoints': geometryData.patchPoints,
				'vertexStreams': vertexStreams,
				'vertexTransforms': vfcTransforms
			}

			# Call into vfc as a subprocess to convert the vertex format.
			vfc = Popen([convertContext.vfc], stdin = PIPE, stdout = PIPE, stderr = PIPE,
				universal_newlines = True)
			stdinStr = json.dumps(vfcInput)
			stdoutStr = ''
			stderrStr = ''
			while True:
				stdoutData, stderrData = vfc.communicate(stdinStr)
				stdinStr = None
				stdoutStr += stdoutData
				stderrStr += stderrData
				if vfc.returncode is None:
					continue

				if vfc.returncode == 0:
					vfcOutput = json.loads(stdoutStr)
					break
				else:
					stderrStr = stderrStr.replace('stdin: ', '').replace('error: ', '')
					if path:
						raise Exception('Error converting geometry data "' + path + '":\n' + 
							stderrStr)
					else:
						raise Exception('Error converting geometry data:\n' + stderrStr)

			try:
				# Parse the final geometry info.
				vertexCount = vfcOutput['vertexCount']
				vertexBuffers = []

				vfcVertices = vfcOutput['vertices']
				for i in range(len(vfcVertices)):
					offset = appendBuffer(combinedBuffer,
						base64.b64decode(vfcVertices[i]['vertexData'][7:]))
					vertexBuffers.append(VertexBuffer(vertexFormat[i], offset))

					# Update the bounds.
					if modelBounds:
						for attribFormat in vfcVertices[i]['vertexFormat']:
							if attribFormat['name'] == '0':
								minValue = attribFormat['minValue']
								maxValue = attribFormat['maxValue']
								for i in range(3):
									modelBounds[0][i] = min(modelBounds[0][i], minValue[i])
									modelBounds[1][i] = max(modelBounds[1][i], maxValue[i])

				indexBuffers = []
				if indexType:
					for vfcIndexBuffer in vfcOutput['indexBuffers']:
						indexCount = vfcIndexBuffer['indexCount']
						vertexOffset = vfcIndexBuffer['baseVertex']
						offset = appendBuffer(combinedBuffer,
							base64.b64decode(vfcIndexBuffer['indexData'][7:]),
							len(indexBuffers) == 0)
						if indexBuffers:
							firstIndex = (offset - indexBuffers[0].indexData)/indexSize
							offset = indexBuffers[0].indexData
						else:
							firstIndex = 0
						indexBuffers.append(IndexBuffer(indexSize, firstIndex, indexCount,
							vertexOffset, offset))
			except Exception as e:
				raise Exception('Internal error: unexpected output from vfc.')

			convertedGeometry[geometryData.name] = ConvertedModelGeometry(vertexCount,
				vertexBuffers, primitiveType, indexBuffers)
	except (ValueError, AttributeError):
		if modelType:
			raise Exception('Unexpected data from conversion function for model type "' +
				modelType + '".')
		else:
			raise Exception('Unexpected data from conversion function.')

	return convertedGeometry

def loadAndConvertModelGeometry(convertContext, modelType, path, includedComponents, vertexFormat,
		indexSize, transforms, combinedBuffer, modelBounds = None, pointsOnly = False):
	"""
	Convenience function to load a model as with loadModel(), then convert it as with
	convertModelGeometry().
	"""
	geometryDataList = loadModel(convertContext, modelType, path)
	return convertModelGeometry(convertContext, geometryDataList, includedComponents, vertexFormat,
		indexSize, transforms, combinedBuffer, modelBounds, pointsOnly, modelType, path)
