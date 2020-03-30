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

import base64
from copy import copy
import json
import flatbuffers
from subprocess import Popen, PIPE
from .FormatDecoration import *
from .ModelDrawRange import *
from .ModelNode import *
from .PrimitiveType import *
from .SceneResourcesConvert import convertSceneResources, readVertexAttrib
from .VertexFormat import *

validModelVertexTransforms = {
	'Identity',
	'Bounds',
	'UNormToSNorm',
	'SNormToUNorm'
}

class ModelNodeVertexStream:
	"""
	Class containing information for a vertex stream of a model. The following members are present:
	- vertexFormat: array of vertex attributes defining the vertex format. Each attribute is a tuple
	  containing the attribute index, format, and type decoration.
	- vertexData: bytes for the vertices.
	- indexSize: the number of bytes for each index. A value of 0 indicates no indices.
	- indexData: bytes for the indices, or None if no indices.
	"""
	def __init__(self, vertexFormat, vertexData, indexSize = 0, indexData = None):
		self.vertexFormat = vertexFormat
		self.vertexData = vertexData
		self.indexSize = indexSize
		self.indexData = indexData

class ModelNodeGeometryData:
	"""
	Class containing data for a piece of geometry. The following members are present:
	- name: the name of the geometry.
	- vertexStreams: array of ModelVertexStream objects for the vertices.
	"""
	def __init__(self, name, vertexStreams):
		self.name = name
		self.vertexStreams = vertexStreams
		

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

def validateModelDistanceRange(distanceRange):
	try:
		if len(distanceRange) != 2:
			raise Exception()
		for dist in distanceRange:
			if not isinstance(dist, float):
				raise Exception()
	except:
		raise Exception('Invalid model draw distance range "' + str(distanceRange) + '".')

def convertModelNodeGeometry(convertContext, modelGeometry, embeddedResources):
	def appendBuffer(combinedBuffer, data, pad = True):
		# Some graphics APIs require offsets to be aligned to 4 bytes.
		if pad:
			for i in range(len(combinedBuffer) % 4):
				combinedBuffer.append(0)

		offset = len(combinedBuffer)
		combinedBuffer.extend(combinedBuffer)
		return offset

	def addModelEmbeddedResources(embeddedResources, resourceType, resources):
		try:
			if resourceType in embeddedResources:
				embeddedResources[resourceType].extend(resources)
			else:
				embeddedResources[resourceType] = resources
		except:
			raise Exception('ModelNode embedded resource type "' + resourceType +
				'" must be an array of objects.')

	def convertGeometry(convertContext, modelType, path, vertexFormat, indexSize, primitiveType,
			patchPoints, transforms, combinedBuffer):
		def getIndexType(indexSize):
			if indexSize == 2:
				return 'UInt16'
			elif indexSize == 4:
				return 'UInt32'
			else:
				return None

		if not hasattr(convertContext, 'modelTypeMap') or \
				modelType not in convertContext.modelTypeMap:
			raise Exception('Model type"' + modelType + '" hasn\'t been registered.')

		vfcVertexFormat = []
		for attrib, attribFormat, decoration in vertexFormat:
			vfcVertexFormat.append({
				'name': str(attrib),
				'layout': attribFormat,
				'type': decoration
			})

		indexType = getIndexType(indexSize)

		vfcTransforms = []
		for attrib, transform in transforms:
			vfcTransforms.append({'name': str(attrib), 'transform': transform})

		geometryDataList = convertContext.modelTypeMap[modelType](convertContext, path)
		convertedGeometry = dict()
		try:
			for geometryData in geometryDataList:
				if geometryData.name in convertedGeometry:
					raise Exception('Geometry data "' + geometryData.name + '" appears multiple '
						'times in model "' + path + '".')

				# Prepare the input for the vfc tool.
				vertexStreams = []
				for vertexStream in geometryData.vertexStreams:
					streamVertexFormat = []
					for attrib, attribFormat, decoration in vertexStream.vertexFormat:
						streamVertexFormat.append({
							'name': str(attrib),
							'layout': attribFormat,
							'type': decoration
						})

					vfcVertexStream = {
						'vertexFormat': streamVertexFormat,
						'vertexData': 'base64:' + base64.b64encode(vertexStream.vertexData)
					}
					if vertexStream.indexSize > 0:
						vfcVertexStream['indexType'] = getIndexType(vertexStream.indexSize)
						vfcVertexStream['indexData'] = 'base64:' + \
							base64.b64encode(vertexStream.indexData)
					vertexStreams.append(vfcVertexStream)

				vfcInput = {
					'vertexFormat': vfcVertexFormat,
					'indexType': indexType,
					'primitiveType': primitiveType,
					'patchPoints': patchPoints,
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
					if vfc.returnCode is None:
						continue

					if vfc.returnCode == 0:
						vfcOutput = json.loads(stdinStr)
						break
					else:
						raise Exception('Error converting geometry data "' + path + '":\n' + 
							stderrStr.replace('stdin: ', ''))

				try:
					# Parse the final geometry info.
					geometry = object()
					geometry.vertexCount = vfcOutput['vertexCount']
					geometry.vertexData = appendBuffer(combinedBuffer,
						base64.b64decode(vfcOutput['vertexData'][7:]))
					geometry.vertexFormat = vertexFormat
					geometry.primitiveType = primitiveType

					geometry.indexBuffers = []
					if indexType:
						for vfcIndexBuffer in vfcOutput.indexBuffers:
							indexBuffer = object()
							indexBuffer.indexCount = vfcIndexBuffer['indexCount']
							indexBuffer.vertexOffset = vfcIndexBuffer['baseVertex']
							indexData = appendBuffer(combinedBuffer,
								base64.b64decode(vfcIndexBuffer['indexData'][7:],
								len(geometry.indexBuffers) == 0))
							if geometry.indexBuffers:
								indexBuffer.indexData = geometry.indexBuffers[0].indexData
								indexBuffer.firstIndex = \
									(indexData - indexBuffer.indexData)/indexSize
							else:
								indexBuffer.indexData = indexData
								indexBuffer.firstIndex = 0
							indexBuffer.indexSize = indexSize
							geometry.indexBuffers.append(indexBuffer)
				except:
					raise Exception('Internal error: unexpected output from vfc.')

				convertedGeometry[geometryData.name] = geometry
		except (ValueError, AttributeError):
			raise Exception('Unexpected data from conversion function for model type "' +
				modelType + '".')

		return convertedGeometry

	embeddedBufferName = '_DSEmbeddedModelBuffer'
	embeddedGeometryName = '_DSEmbeddedModelGeometry'
	combinedBuffer = bytearray()
	geometries = []
	models = []
	try:
		for geometryData in modelGeometry:
			try:
				name = str(geometryData['name'])
				modelType = str(geometryData['type'])
				path = str(geometryData['path'])

				vertexFormat = geometryData['vertexFormat']
				vfcVertexFormat = []
				try:
					for vertexAttrib in vertexFormat:
						try:
							attrib = readVertexAttrib(vertexAttrib['attrib'])

							attribFormat = str(vertexAttrib['format'])
							if not hasattr(VertexFormat, attribFormat):
								raise Exception('Invalid vertex format "' + attribFormat + '".')

							decoration = str(vertexAttrib['decoration'])
							if not hasattr(FormatDecoration, decoration):
								raise Exception(
									'Invalid vertex format decoration "' + decoration + '".')

							vfcVertexFormat.append((attrib, attribFormat, decoration))
						except KeyError as e:
							raise Exception(
								'ModelNode geometry vertex format doesn\'t contain element "' +
								str(e) + '".')
				except (TypeError, ValueError):
					raise Exception(
						'ModelNode geometry "vertexFormat" must be an array of objects.')

				indexSize = geometryData.get('indexSize', 0)
				if indexSize not in (0, 2, 4):
					raise Exception('Invalid geometry indexSize "' + str(indexSize) + '".')

				primitiveType = geometryData.get('primitiveType', 'TriangleList')
				if not hasattr(PrimitiveType, primitiveType):
					raise Exception('Invalid geometry primitive type "' + str(primitiveType) + '".')

				if primitiveType == 'PatchList':
					patchPointsStr = geometryData['patchPoints']
					try:
						patchPoints = int(patchPointsStr)
						if patchPoints < 1:
							raise Exception() # Common error handling in except block.
					except:
						raise Exception(
							'Invalid geometry patch points "' + str(patchPointsStr) + '".')
				else:
					patchPoints = 0

				transforms = geometryData.get('transforms')
				vfcTransforms = []
				if transforms:
					try:
						for transform in transforms:
							try:
								attrib = readVertexAttrib(transform['attrib'])
								transformType = transform['transform']
								if transformType not in validModelVertexTransforms:
									raise Exception('Invalid geometry transform "' +
										str(transformType) + '".')

								vfcTransforms.append((attrib, transformType))
							except KeyError as e:
								raise Exception(
									'ModelNode geometry transform doesn\'t contain element "' +
									str(e) + '".')
					except (TypeError, ValueError):
						raise Exception(
							'ModelNode geometry "transforms" must be an array of objects.')

				drawInfo = geometryData['drawInfo']
			except KeyError as e:
				raise Exception(
					'ModelNode "modelGeometry" doesn\'t contain element "' + str(e) + '".')

			convertedGeometry = convertGeometry(convertContext, modelType, path, vfcVertexFormat,
				indexSize, primitiveType, patchPoints, transforms, combinedBuffer)

			# Geometries to be added to the embedded resources.
			for name, geometry in convertedGeometry:
				vertexAttributes = []
				for attrib, attribFormat, decoration in geometry.vertexFormat:
					vertexAttributes.append({
						'attrib': attrib,
						'format': attribFormat,
						'decoration': decoration
					})
				vertexBuffer = {
					'name': embeddedBufferName,
					'offset': geometry.vertexData,
					'count': geometry.vertexCount,
					'format': {
						'attributes': vertexAttributes,
					}
				}

				geometryName = embeddedGeometryName + str(len(geometries))
				geometryInfo = {
					'name': geometryName,
					'vertexBuffers': [vertexBuffer]
				}
				if geometry.indexBuffers:
					totalIndexCount = 0
					for indexBuffer in geometry.indexBuffers:
						totalIndexCount += indexBuffer.indexCount
					geometryInfo['indexBuffer'] = {
						'name': embeddedBufferName,
						'offset': geometry.indexBuffers[0].offset,
						'count': totalIndexCount,
						'indexSize': geometry.indexBuffers[0].indexSize
					}

				geometries.append(geometryInfo)
				convertedGeometry[name].geometryName = geometryName

			try:
				for info in drawInfo:
					try:
						modelInfo = object()
						name = str(info['name'])
						modelInfo.shader = str(info['shader'])
						modelInfo.material = str(info['material'])
						modelInfo.listName = str(info['listName'])

						modelInfo.distanceRange = info.get('distanceRange',
							[0.0, 3.402823466e38])
						validateModelDistanceRange(modelInfo.distanceRange)
					except KeyError as e:
						raise Exception('Model geometry draw info doesn\'t contain element "' +
							str(e) + '".')

					baseGeometry = convertedGeometry.get(name)
					if not baseGeometry:
						raise Exception('Model node geometry "' + name + '" isn\'t in the model "' +
							path + '".')

					modelInfo.geometry = baseGeometry.geometryName
					modelInfo.primitiveType = baseGeometry.primitiveType

					if baseGeometry.indexBuffers:
						for indexBuffer in baseGeometry.indexBuffers:
							curModelInfo = copy(modelInfo)
							drawRange = object()
							drawRange.rangeType = ModelDrawRange.DrawIndexedRange
							drawRange.indexCount = indexBuffer.indexCount
							drawRange.instanceCount = 1
							drawRange.firstIndex = indexBuffer.firstIndex
							drawRange.vertexOffset = indexBuffer.vertexOffset
							drawRange.firstInstance = 0
							curModelInfo.drawRange = drawRange
							models.append(curModelInfo)
					else:
						drawRange = object()
						drawRange.rangeType = ModelDrawRange.DrawRange
						drawRange.vertexCount = baseGeometry.vertexCount
						drawRange.instanceCount = 1
						drawRange.firstVertex = 0
						drawRange.firstInstance = 1
						modelInfo.drawRange = drawRange
						models.append(modelInfo)
			except (TypeError, ValueError):
				raise Exception('Model geometry draw info must be an array of objects.')
	except (TypeError, ValueError):
		raise Exception('ModelNode "modelGeometry" must be an array of objects.')

	embeddedBuffer = {
		'name': embeddedBufferName,
		'usage': ['Index', 'Vertex'],
		'memoryHints': ['GPUOnly', 'Static', 'Draw'],
		'data': 'base64:' + base64.b64encode(combinedBuffer),
	}
	addModelEmbeddedResources(embeddedResources, 'buffers', [embeddedBuffer])
	addModelEmbeddedResources(embeddedResources, 'drawGeometries', geometries)
	return models

def convertModelNode(convertContext, data):
	"""
	Converts a ModelNode. The data map is expected to contain the following elements:
	- embeddedResources: optional set of resources to embed with the node. This is a map containing
	  the elements as expected by SceneResourcesConvert.convertSceneResources().
	- modelGeometry: array of model geometry. Each element of the array has the following members:
	  - type: the name of the geometry type, such as "obj" or "gltf".
	  - path: the path to the geometry.
	  - vertexFormat: array of vertex attributes defining the vertex format. Each element of the
	    array has the following members:
		- attrib: the attribute. This can either be an enum value from dsVertexAttrib, removing
		  the type prefix, or the integer for the attribute.
	    - format: the attribute format. See the dsGfxFormat enum for values, removing the type
	      prefix. Only the "standard" formats may be used.
	    - decoration: the decoration for the format. See the dsGfxFormat enum for values,
	      removing the type prefix. Only the decorator values may be used.
	  - indexSize: the size of the index in bytes. This must be either 2 or 4. If not set, no
	    indices will be produced.
	  - primitiveType: the type of primitives. See the dsPrimitiveType enum for values, removing the
	    type prefix. Defaults to "TriangleList".
	  - patchPoints: the number of points when primitiveType is "PatchList".
	  - transforms: optional array of transforms to perform on the vertex values. Each element of
	    the array has the following members:
	    - attrib: the attribute, matching one of the attributes in vertexFormat.
	    - transform: transform to apply on the attribute. Valid values are:
	      - Identity: leaves the values un-transformed.
	      - Bounds: normalizes the values based on the original value's bounds
	      - UNormToSNorm: converts UNorm values to SNorm values.
	      - SNormToUNorm: converts SNorm values to UNorm values.
	  - drawInfo: array of definitions for drawing components of the geometry. Each element of the
	    array has the following members:
	    - name: the name of the model component.
	    - shader: te name of the shader to draw with.
	    - material: the name of the material to draw with.
	    - distanceRange: array of two floats for the minimum and maximum distance to draw at.
	      Defaults to [0, 3.402823466e38].
	    - listName The name of the item list to draw the model with.
	- models: array of models to draw with manually provided geometry. (i.e. not converted from
	  the modelGeometry array) Each element of the array has the following members:
	  - shader: the name of the shader to draw with.
	  - material: the name of the material to draw with.
	  - geometry: the name of the geometry to draw.
	  - distanceRange: array of two floats for the minimum and maximum distance to draw at. Defaults
	    to [0, 3.402823466e38].
	  - drawRange: the range of the geometry to draw. This is an object with the following members,
	    depending on if the geometry is indexed or not:
	    Indexed geometry:
	    - indexCount: the number of indices to draw.
	    - instanceCount: the number of instances to draw. Defaults to 1.
	    - firstIndex: the first index to draw. Defaults to 0.
	    - vertexOffset: the offset to apply to each index value. Defaults to 0.
	    - firstInstance: the first instance to draw. Defaults to 0.
	    Non-indexed geometry:
	    - vertexCount: the number of vertices to draw.
	    - instanceCount: the number of instances to draw. Defaults to 1.
	    - firstVertex: the first vertex to draw. Defaults to 0.
	    - firstIstance: the first instance to draw. Defaults to 0.
	  - primitiveType: the primitive type to draw with. See the dsPrimitiveType enum for values,
	    removing the type prefix. Defaults to "TriangleList".
	  - listName The name of the item list to draw the model with.
	- extraItemLists: array of extra item list names to add the node to.
	- bounds: 2x3 array of float values for the minimum and maximum values for the positions. This
	  will be automatically calculated from geometry in modelGeometry if unset. Otherwise if unset
	  the model will have no explicit bounds for culling.
	"""
	builder = flatbuffers.Builder(0)

	try:
		embeddedResources = data.get('embeddedResources', dict())
		if not isinstance(embeddedResources, dict):
			raise Exception ('ModelNode "embeddedResources" must be an object.')
		
		modelGeometry = data.get('modelGeometry')
		if modelGeometry:
			models = convertModelNodeGeometry(convertContext, modelGeometry, embeddedResources)
		else:
			models = []
	except (TypeError, ValueError):
		raise Exception('ModelNode data must be an object.')
	except KeyError as e:
		raise Exception('ModelNode data doesn\'t contain element "' + str(e) + '".')
