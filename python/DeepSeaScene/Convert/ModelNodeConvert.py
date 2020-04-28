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
import os
from subprocess import Popen, PIPE

import flatbuffers
from .SceneResourcesConvert import convertSceneResources, readVertexAttrib
from ..DrawIndexedRange import *
from ..DrawRange import *
from ..FormatDecoration import *
from ..ModelDrawRange import *
from ..ModelInfo import *
from ..ModelNode import *
from ..OrientedBox3f import *
from ..PrimitiveType import *
from ..Vector2f import *
from ..VertexElementFormat import *
from ..VertexFormat import *

class Object:
	pass

FLT_MAX = 3.402823466e38

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

	def convertGeometry(convertContext, modelType, path, vertexFormat, indexSize, transforms,
			includedComponents, combinedBuffer, modelBounds):
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
				if geometryData.name not in includedComponents:
					continue

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
						'vertexData': 'base64:' + base64.b64encode(vertexStream.vertexData).decode()
					}
					if vertexStream.indexSize > 0:
						vfcVertexStream['indexType'] = getIndexType(vertexStream.indexSize)
						vfcVertexStream['indexData'] = 'base64:' + \
							base64.b64encode(vertexStream.indexData).decode()
					vertexStreams.append(vfcVertexStream)

				primitiveType = geometryData.primitiveType
				if primitiveType == 'TriangleListAdjacency':
					primitiveType = 'TriangleList'
				elif primitiveType == 'TriangleStripAdjacency':
					primitiveType = 'TriangleStrip'
				vfcInput = {
					'vertexFormat': vfcVertexFormat,
					'indexType': indexType,
					'primitiveType': primitiveType,
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
						raise Exception('Error converting geometry data "' + path + '":\n' + 
							stderrStr.replace('stdin: ', '').replace('error: ', ''))

				try:
					# Parse the final geometry info.
					geometry = Object()
					geometry.vertexCount = vfcOutput['vertexCount']
					geometry.vertexData = appendBuffer(combinedBuffer,
						base64.b64decode(vfcOutput['vertexData'][7:]))
					geometry.vertexFormat = vertexFormat
					geometry.primitiveType = geometryData.primitiveType

					geometry.indexBuffers = []
					if indexType:
						for vfcIndexBuffer in vfcOutput['indexBuffers']:
							indexBuffer = Object()
							indexBuffer.indexCount = vfcIndexBuffer['indexCount']
							indexBuffer.vertexOffset = vfcIndexBuffer['baseVertex']
							indexBuffer.offset = len(combinedBuffer)
							indexData = appendBuffer(combinedBuffer,
								base64.b64decode(vfcIndexBuffer['indexData'][7:]),
								len(geometry.indexBuffers) == 0)
							if geometry.indexBuffers:
								indexBuffer.indexData = geometry.indexBuffers[0].indexData
								indexBuffer.firstIndex = \
									(indexData - indexBuffer.indexData)/indexSize
							else:
								indexBuffer.indexData = indexData
								indexBuffer.firstIndex = 0
							indexBuffer.indexSize = indexSize
							geometry.indexBuffers.append(indexBuffer)

					# Update the bounds.
					for attribFormat in vfcOutput['vertexFormat']:
						if attribFormat['name'] == '0':
							minValue = attribFormat['minValue']
							maxValue = attribFormat['maxValue']
							for i in range(3):
								modelBounds[0][i] = min(modelBounds[0][i], minValue[i])
								modelBounds[1][i] = max(modelBounds[1][i], maxValue[i])
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
	modelBounds = [[FLT_MAX, FLT_MAX, FLT_MAX], [-FLT_MAX, -FLT_MAX, -FLT_MAX]]
	try:
		for geometryData in modelGeometry:
			try:
				path = str(geometryData['path'])
				modelType = str(geometryData.get('type', ''))
				if not modelType:
					modelType = os.path.splitext(path)[1]
					if modelType:
						modelType = modelType[1:]
					if not modelType:
						raise Exception('Model geometry has no known model type.')

				vertexFormat = geometryData['vertexFormat']
				vfcVertexFormat = []
				try:
					for vertexAttrib in vertexFormat:
						try:
							attrib = readVertexAttrib(vertexAttrib['attrib'])

							attribFormat = str(vertexAttrib['format'])
							if not hasattr(VertexElementFormat, attribFormat):
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

				includedComponents = set()
				drawInfos = geometryData['drawInfos']
				try:
					for info in drawInfos:
						try:
							includedComponents.add(str(info['name']))
						except KeyError as e:
							raise Exception('Model geometry draw info doesn\'t contain element "' +
								str(e) + '".')
				except (TypeError, ValueError):
					raise Exception('Model geometry draw info must be an array of objects.')
			except KeyError as e:
				raise Exception(
					'ModelNode "modelGeometry" doesn\'t contain element "' + str(e) + '".')

			convertedGeometry = convertGeometry(convertContext, modelType, path, vfcVertexFormat,
				indexSize, vfcTransforms, includedComponents, combinedBuffer, modelBounds)

			# Geometries to be added to the embedded resources.
			for geometry in convertedGeometry.values():
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
				geometry.geometryName = geometryName

			# Add the models associating the shader, material, draw list, and draw range with the
			# converted geometry.
			for info in drawInfos:
				try:
					modelInfo = Object()
					name = str(info['name'])
					modelInfo.shader = str(info['shader'])
					modelInfo.material = str(info['material'])
					modelInfo.listName = str(info['listName'])

					modelInfo.distanceRange = info.get('distanceRange', [0.0, FLT_MAX])
					validateModelDistanceRange(modelInfo.distanceRange)
				except KeyError as e:
					raise Exception('Model geometry draw info doesn\'t contain element "' +
						str(e) + '".')

				baseGeometry = convertedGeometry.get(name)
				if not baseGeometry:
					raise Exception('Model node geometry "' + name + '" isn\'t in the model "' +
						path + '".')

				modelInfo.geometry = baseGeometry.geometryName
				modelInfo.primitiveType = getattr(PrimitiveType, baseGeometry.primitiveType)

				if baseGeometry.indexBuffers:
					for indexBuffer in baseGeometry.indexBuffers:
						curModelInfo = copy(modelInfo)
						drawRange = Object()
						drawRange.rangeType = ModelDrawRange.DrawIndexedRange
						drawRange.indexCount = indexBuffer.indexCount
						drawRange.instanceCount = 1
						drawRange.firstIndex = indexBuffer.firstIndex
						drawRange.vertexOffset = indexBuffer.vertexOffset
						drawRange.firstInstance = 0
						curModelInfo.drawRange = drawRange
						models.append(curModelInfo)
				else:
					drawRange = Object()
					drawRange.rangeType = ModelDrawRange.DrawRange
					drawRange.vertexCount = baseGeometry.vertexCount
					drawRange.instanceCount = 1
					drawRange.firstVertex = 0
					drawRange.firstInstance = 1
					modelInfo.drawRange = drawRange
					models.append(modelInfo)
	except (TypeError, ValueError):
		raise Exception('ModelNode "modelGeometry" must be an array of objects.')

	embeddedBuffer = {
		'name': embeddedBufferName,
		'usage': ['Index', 'Vertex'],
		'memoryHints': ['GPUOnly', 'Static', 'Draw'],
		'data': 'base64:' + base64.b64encode(combinedBuffer).decode(),
	}
	addModelEmbeddedResources(embeddedResources, 'buffers', [embeddedBuffer])
	addModelEmbeddedResources(embeddedResources, 'drawGeometries', geometries)
	return models, modelBounds

def convertModelNodeModels(modelInfoList):
	def convertDrawRange(drawRangeInfo):
		def readInt(value, name, minVal):
			try:
				intVal = int(value)
				if intVal < minVal:
					raise Exception() # Common error handling in except block.
				return intVal
			except:
				raise Exception('Invalid draw range ' + name + ' "' + str(value) + '".')

		drawRange = Object()
		try:
			hasIndexCount = 'indexCount' in drawRangeInfo
			hasVertexCount = 'vertexCount' in drawRangeInfo
			if hasIndexCount and not hasVertexCount:
				drawRange.rangeType = ModelDrawRange.DrawIndexedRange
				drawRange.indexCount = readInt(drawRangeInfo['indexCount'], 'indexCount', 1)
				drawRange.firstIndex = readInt(drawRangeInfo.get('firstIndex', 0), 'firstIndex', 0)
				drawRange.vertexOffset = readInt(drawRangeInfo.get('vertexOffset', 0),
					'vertexOffset', 0)
			elif not hasIndexCount and hasVertexCount:
				drawRange.rangeType = ModelDrawRange.DrawRange
				drawRange.vertexCount = readInt(drawRangeInfo['vertexCount'], 'vertexCount', 1)
				drawRange.firstVertex = readInt(drawRangeInfo.get('firstVertex', 0),
					'firstVertex', 0)
			else:
				raise Exception('Model draw range must have either "indexCount" or "vertexCount".')

			drawRange.instanceCount = readInt(drawRangeInfo.get('instanceCount', 1),
				'instanceCount', 1)
			drawRange.firstInstance = readInt(drawRangeInfo.get('firstInstance', 0),
				'firstInstance', 0)
		except KeyError as e:
			raise Exception('Model draw range doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('Model draw range must be an object.')

		return drawRange

	models = []
	try:
		for info in modelInfoList:
			try:
				model = Object()
				model.shader = info['shader']
				model.material = info['material']
				model.geometry = info['geometry']
				model.distanceRange = info.get('distanceRange', [0.0, FLT_MAX])
				validateModelDistanceRange(model.distanceRange)
				model.drawRange = convertDrawRange(info['drawRange'])

				primitiveTypeStr = info.get('primitiveType', 'TriangleList')
				try:
					model.primitiveType = getattr(PrimitiveType, primitiveTypeStr)
				except AttributeError:
					raise Exception(
						'Invalid geometry primitive type "' + str(primitiveTypeStr) + '".')

				model.listName = info['listName']
			except KeyError as e:
				raise Exception('ModelNode "models" doesn\'t contain element "' + str(e) + '".')
	except (TypeError, ValueError):
		raise Exception('ModelNode "models" must be an array of objects.')

	return models

def convertModelNode(convertContext, data):
	"""
	Converts a ModelNode. The data map is expected to contain the following elements:
	- embeddedResources: optional set of resources to embed with the node. This is a map containing
	  the elements as expected by SceneResourcesConvert.convertSceneResources().
	- modelGeometry: array of model geometry. Each element of the array has the following members:
	  - type: the name of the geometry type, such as "obj" or "gltf". If ommitted, the type is
	    inferred from the path extension.
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
	  - transforms: optional array of transforms to perform on the vertex values. Each element of
	    the array has the following members:
	    - attrib: the attribute, matching one of the attributes in vertexFormat.
	    - transform: transform to apply on the attribute. Valid values are:
	      - Identity: leaves the values un-transformed.
	      - Bounds: normalizes the values based on the original value's bounds
	      - UNormToSNorm: converts UNorm values to SNorm values.
	      - SNormToUNorm: converts SNorm values to UNorm values.
	  - drawInfos: array of definitions for drawing components of the geometry. Each element of the
	    array has the following members:
	    - name: the name of the model component. Note that only model components referenced in the
		  drawInfo array will be included in the final model.
	    - shader: te name of the shader to draw with.
	    - material: the name of the material to draw with.
	    - distanceRange: array of two floats for the minimum and maximum distance to draw at.
	      Defaults to [0, 3.402823466e38].
	    - listName: the name of the item list to draw the model with.
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
	try:
		embeddedResources = data.get('embeddedResources', dict())
		if not isinstance(embeddedResources, dict):
			raise Exception ('ModelNode "embeddedResources" must be an object.')
		
		modelGeometry = data.get('modelGeometry')
		if modelGeometry:
			models, modelBounds = convertModelNodeGeometry(convertContext, modelGeometry,
				embeddedResources)
		else:
			models = []
			modelBounds = None

		modelInfoList = data.get('models')
		if modelInfoList:
			models.extend(convertModelNodeModels(modelInfoList))

		extraItemLists = data.get('extraItemLists')
		if 'bounds' in data:
			modelBounds = data['bounds']
			try:
				if len(modelBounds) != 2:
					raise Exception()
				for bound in modelBounds:
					if len(bound) != 3:
						raise Exception()
					for val in bound:
						if not isinstance(val, float):
							raise Exception()
			except:
				raise Exception('Invalid model bounds "' + str(modelBounds) + '".')
	except (TypeError, ValueError):
		raise Exception('ModelNode data must be an object.')
	except KeyError as e:
		raise Exception('ModelNode data doesn\'t contain element "' + str(e) + '".')

	builder = flatbuffers.Builder(0)
	if embeddedResources:
		embeddedResourcesData = convertSceneResources(convertContext, embeddedResources)
		embeddedResourcesOffset = builder.CreateByteVector(embeddedResourcesData)
	else:
		embeddedResourcesOffset = 0

	if extraItemLists:
		extraItemListOffsets = []
		try:
			for item in extraItemLists:
				extraItemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('ModelNode "extraItemLists" must be an array of strings.')

		ModelNodeStartExtraItemListsVector(builder, len(extraItemListOffsets))
		for offset in reversed(extraItemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		extraItemListsOffset = builder.EndVector(len(extraItemListOffsets))
	else:
		extraItemListsOffset = 0

	modelOffsets = []
	for model in models:
		shaderOffset = builder.CreateString(model.shader)
		materialOffset = builder.CreateString(model.material)
		geometryOffset = builder.CreateString(model.geometry)

		drawRange = model.drawRange
		if drawRange.rangeType == ModelDrawRange.DrawIndexedRange:
			DrawIndexedRangeStart(builder)
			DrawIndexedRangeAddIndexCount(builder, drawRange.indexCount)
			DrawIndexedRangeAddInstanceCount(builder, drawRange.instanceCount)
			DrawIndexedRangeAddFirstIndex(builder, drawRange.firstIndex)
			DrawIndexedRangeAddVertexOffset(builder, drawRange.vertexOffset)
			DrawIndexedRangeAddFirstInstance(builder, drawRange.firstInstance)
			drawRangeOffset = DrawIndexedRangeEnd(builder)
		else:
			DrawRangeStart(builder)
			DrawRangeAddVertexCount(builder, drawRange.vertexCount)
			DrawRangeAddInstanceCount(builder, drawRange.instanceCount)
			DrawRangeAddFirstVertex(builder, drawRange.firstVertex)
			DrawRangeAddFirstInstance(builder, drawRange.firstInstance)
			drawRangeOffset = DrawRangeEnd(builder)

		listNameOffset = builder.CreateString(model.listName)

		ModelInfoStart(builder)
		ModelInfoAddShader(builder, shaderOffset)
		ModelInfoAddMaterial(builder, materialOffset)
		ModelInfoAddGeometry(builder, geometryOffset)
		ModelInfoAddDistanceRange(builder, CreateVector2f(builder, model.distanceRange[0],
			model.distanceRange[1]))
		ModelInfoAddDrawRangeType(builder, drawRange.rangeType)
		ModelInfoAddDrawRange(builder, drawRangeOffset)
		ModelInfoAddPrimitiveType(builder, model.primitiveType)
		ModelInfoAddListName(builder, listNameOffset)
		modelOffsets.append(ModelInfoEnd(builder))

	ModelNodeStartModelsVector(builder, len(modelOffsets))
	for offset in reversed(modelOffsets):
		builder.PrependUOffsetTRelative(offset)
	modelsOffset = builder.EndVector(len(modelOffsets))

	ModelNodeStart(builder)
	ModelNodeAddEmbeddedResources(builder, embeddedResourcesOffset)
	ModelNodeAddExtraItemLists(builder, extraItemListsOffset)
	ModelNodeAddModels(builder, modelsOffset)

	if modelBounds:
		center = []
		halfExtents = []
		for i in range(0, 3):
			center.append((modelBounds[0][i] + modelBounds[1][i])/2)
			halfExtents.append((modelBounds[1][i] - modelBounds[0][i])/2)
		boundsOffset = CreateOrientedBox3f(builder, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,
			center[0], center[1], center[2], halfExtents[0], halfExtents[1], halfExtents[2])
	else:
		boundsOffset = 0
	ModelNodeAddBounds(builder, boundsOffset)

	builder.Finish(ModelNodeEnd(builder))
	return builder.Output()
