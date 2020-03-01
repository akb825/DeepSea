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

from .ObjectData import *
from .TransformNodeConvert import convertTransformNode
from .SceneNodeRefConvert import convertReferenceNode

class ConvertContext:
	"""
	Class containing information used when converting scene data.

	Builtin types will all be registered with this automatically. Custom types may be registered to
	extend scene conversion.
	"""
	def __init__(self, cuttlefishTool = 'cuttlefish', vfcTool = 'vfc'):
		"""
		Initializes this with the paths to the cuttlefish tool (for texture conversion) and vfc tool
		(for vertex format conversion). By default the tool names to use them on the PATH.
		"""
		self.cuttlefish = cuttlefishTool
		self.vfc = vfcTool

		self.nodeTypeMap = {
			'TransformNode': convertTransformNode,
			'ReferenceNode': convertReferenceNode
		}

	def addNodeType(self, typeName, loadFunc):
		"""
		Adds a node type with the name and the load function. The function should take the
		ConvertContext and dict for the data as parameters and return the flatbuffer bytes.

		An exception will be raised if the type is already registered.
		"""
		if typeName in self.nodeTypeMap:
			raise Exception('Node type "' + typeName + '" is already registered.')
		self.nodeTypeMap[typeName] = loadFunc

	def convertNode(self, builder, typeName, data):
		"""
		Converts a node based on its type and dict for the data. This will return the offset to the
		ObjectData added to the builder.
		"""
		if typeName not in self.nodeTypeMap:
			raise Exception('Node type "' + typeName + '" hasn\'t been registered.')

		convertedData = self.nodeTypeMap[typeName](self, data)

		typeNameOffset = builder.CreateString(typeName)
		dataOffset = builder.CreateByteVector(convertedData)

		ObjectDataStart(builder)
		ObjectDataAddType(builder, typeNameOffset)
		ObjectDataAddData(builder, dataOffset)
		return ObjectDataEnd(builder)
