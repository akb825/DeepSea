# Copyright 2020-2025 Aaron Barany
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

from .FullScreenResolveConvert import convertFullScreenResolve
from .GLTFModel import registerGLTFModelType
from .HandoffListConvert import convertHandoffList
from .HandoffNodeConvert import convertHandoffNode
from .InstanceTransformDataConvert import convertInstanceTransformData
from .ModelListConvert import convertModelList
from .ModelNodeReconfigConvert import convertModelNodeReconfig
from .ModelNodeRemapConvert import convertModelNodeRemap
from .ModelNodeConvert import convertModelNode
from .NodeChildrenConvert import convertNodeChildren
from .OBJModel import registerOBJModelType
from .SceneNodeRefConvert import convertReferenceNode
from .ShiftNodeConvert import convertShiftNode
from .TransformNodeConvert import convertTransformNode
from .UserDataListConvert import convertUserDataList
from .ViewCullListConvert import convertViewCullList
from .ViewMipmapListConvert import convertViewMipmapList
from .ViewTransformDataConvert import convertViewTransformData
from .. import ObjectData
from .. import SceneItemList

class ConvertContext:
	"""
	Class containing information used when converting scene data.

	Builtin types will all be registered with this automatically. Custom types may be registered to
	extend scene conversion.
	"""
	def __init__(self, cuttlefishTool = 'cuttlefish', vfcTool = 'vfc', multithread = True):
		"""
		Initializes this with the paths to the cuttlefish tool (for texture conversion) and vfc tool
		(for vertex format conversion). By default the tool names to use them on the PATH.
		"""
		self.cuttlefish = cuttlefishTool
		self.vfc = vfcTool
		self.multithread = multithread

		self.itemListTypeMap = {
			'FullScreenResolve': convertFullScreenResolve,
			'HandoffList': convertHandoffList,
			'ModelList': convertModelList,
			'UserDataList': convertUserDataList,
			'ViewCullList': convertViewCullList,
			'ViewMipmapList': convertViewMipmapList,
			'ViewTransformData': convertViewTransformData
		}

		self.instanceDataTypeMap = {
			'InstanceTransformData': convertInstanceTransformData
		}

		self.nodeTypeMap = {
			'HandoffNode': convertHandoffNode,
			'ModelNode': convertModelNode,
			'ModelNodeReconfig': convertModelNodeReconfig,
			'ModelNodeRemap': convertModelNodeRemap,
			'ReferenceNode': convertReferenceNode,
			'ShiftNode': convertShiftNode,
			'TransformNode': convertTransformNode
		}

		self.customResourceTypeMap = dict()

		self.resourceActionTypeMap = {
			'NodeChildren': convertNodeChildren
		}

		# Model types are considered an extension. However, register the builtin model types here
		# for convenience similar to the node and item list types.
		registerGLTFModelType(self)
		registerOBJModelType(self)

	def addItemListType(self, typeName, convertFunc):
		"""
		Adds an item list type with the name and the convert function. The function should take the
		ConvertContext and dict for the data as parameters and return the flatbuffer bytes.

		An exception will be raised if the type is already registered.
		"""
		if typeName in self.itemListTypeMap:
			raise Exception('Item list type "' + typeName + '" is already registered.')
		self.itemListTypeMap[typeName] = convertFunc

	def convertItemList(self, builder, typeName, name, data):
		"""
		Converts an item list based on its type and dict for the data. This will return the offset
		to the ObjectData added to the builder.
		"""
		if typeName not in self.itemListTypeMap:
			raise Exception('Item list type "' + typeName + '" hasn\'t been registered.')

		convertedData = self.itemListTypeMap[typeName](self, data)

		typeNameOffset = builder.CreateString(typeName)
		nameOffset = builder.CreateString(name)
		dataOffset = builder.CreateByteVector(convertedData)

		SceneItemList.Start(builder)
		SceneItemList.AddType(builder, typeNameOffset)
		SceneItemList.AddName(builder, nameOffset)
		SceneItemList.AddData(builder, dataOffset)
		return SceneItemList.End(builder)

	def addInstanceDataType(self, typeName, convertFunc):
		"""
		Adds an instance data type with the name and the convert function. The function should take
		the ConvertContext and dict for the data as parameters and return the flatbuffer bytes.

		An exception will be raised if the type is already registered.
		"""
		if typeName in self.instanceDataTypeMap:
			raise Exception('Instance data type "' + typeName + '" is already registered.')
		self.instanceDataTypeMap[typeName] = convertFunc

	def convertInstanceData(self, builder, typeName, data):
		"""
		Converts an instance based on its type and dict for the data. This will return the offset to
		the ObjectData added to the builder.
		"""
		if typeName not in self.instanceDataTypeMap:
			raise Exception('Instance data type "' + typeName + '" hasn\'t been registered.')

		convertedData = self.instanceDataTypeMap[typeName](self, data)

		typeNameOffset = builder.CreateString(typeName)
		dataOffset = builder.CreateByteVector(convertedData)

		ObjectData.Start(builder)
		ObjectData.AddType(builder, typeNameOffset)
		ObjectData.AddData(builder, dataOffset)
		return ObjectData.End(builder)

	def addNodeType(self, typeName, convertFunc):
		"""
		Adds a node type with the name and the convert function. The function should take the
		ConvertContext, dict for the data as parameters, and the output directory for the resources,
		and return the flatbuffer bytes.

		An exception will be raised if the type is already registered.
		"""
		if typeName in self.nodeTypeMap:
			raise Exception('Node type "' + typeName + '" is already registered.')
		self.nodeTypeMap[typeName] = convertFunc

	def convertNode(self, builder, typeName, data, outputDir):
		"""
		Converts a node based on its type and dict for the data. This will return the offset to the
		ObjectData added to the builder.
		"""
		if typeName not in self.nodeTypeMap:
			raise Exception('Node type "' + typeName + '" hasn\'t been registered.')

		convertedData = self.nodeTypeMap[typeName](self, data, outputDir)

		typeNameOffset = builder.CreateString(typeName)
		dataOffset = builder.CreateByteVector(convertedData)

		ObjectData.Start(builder)
		ObjectData.AddType(builder, typeNameOffset)
		ObjectData.AddData(builder, dataOffset)
		return ObjectData.End(builder)

	def addCustomResourceType(self, typeName, convertFunc, flatbufferTypeName = None):
		"""
		Adds a custom resource type with the name and the convert function. The function should
		take the ConvertContext, dict for the data as parameters, and output directory for the
		resources, and return the flatbuffer bytes.

		flatbufferTypeName may be specified if the type name written to flatbuffers is different
		from the Python type name for looking up the convert function.

		An exception will be raised if the type is already registered.
		"""
		if typeName in self.customResourceTypeMap:
			raise Exception('Custom resource type "' + typeName + '" is already registered.')
		if not flatbufferTypeName:
			flatbufferTypeName = typeName
		self.customResourceTypeMap[typeName] = convertFunc, flatbufferTypeName

	def convertCustomResource(self, builder, typeName, data, outputDir):
		"""
		Converts a custom resource based on its type and dict for the data. This will return the
		offset to the ObjectData added to the builder.
		"""
		if typeName not in self.customResourceTypeMap:
			raise Exception('Custom resource type "' + typeName + '" hasn\'t been registered.')

		convertFunc, flatbufferTypeName = self.customResourceTypeMap[typeName]
		convertedData = convertFunc(self, data, outputDir)

		typeNameOffset = builder.CreateString(flatbufferTypeName)
		dataOffset = builder.CreateByteVector(convertedData)

		ObjectData.Start(builder)
		ObjectData.AddType(builder, typeNameOffset)
		ObjectData.AddData(builder, dataOffset)
		return ObjectData.End(builder)

	def addResourceActionType(self, typeName, convertFunc):
		"""
		Adds a resource action type with the name and the convert function. The function should
		take the ConvertContext, dict for the data as parameters, and output directory for the
		resources, and return the flatbuffer bytes.

		An exception will be raised if the type is already registered.
		"""
		if typeName in self.resourceActionTypeMap:
			raise Exception('Resource action type "' + typeName + '" is already registered.')
		self.resourceActionTypeMap[typeName] = convertFunc

	def convertResourceAction(self, builder, typeName, data, outputDir):
		"""
		Converts a resource action based on its type and dict for the data. This will return the
		offset to the ObjectData added to the builder.
		"""
		if typeName not in self.resourceActionTypeMap:
			raise Exception('Resource action type "' + typeName + '" hasn\'t been registered.')

		convertedData = self.resourceActionTypeMap[typeName](self, data, outputDir)

		typeNameOffset = builder.CreateString(typeName)
		dataOffset = builder.CreateByteVector(convertedData)

		ObjectData.Start(builder)
		ObjectData.AddType(builder, typeNameOffset)
		ObjectData.AddData(builder, dataOffset)
		return ObjectData.End(builder)
