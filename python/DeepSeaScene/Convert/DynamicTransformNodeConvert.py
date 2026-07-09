# Copyright 2020-2026 Aaron Barany
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
import math
from .. import DynamicTransformNode
from ..Vector3f import CreateVector3f
from ..Quaternion4f import CreateQuaternion4f
from .Quaternion import eulerToQuaternion

def convertDynamicTransformNode(convertContext, data, inputDir, outputDir):
	"""
	Converts a DynamicTransformNode. The data map is expected to contain the following elements:
	- scale: array of 3 floats for the scale along the X, Y, and Z axes. Defaults to [1, 1, 1].
	- rotate: array of 3 floats for the X, Y, and Z rotation in degrees. Defaults to [0, 0, 0].
	- translate: array of 3 floats for the translation along X, Y, and Z. Defaults to [0, 0, 0].
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	- itemLists: array of item list names to process the node with.
	"""
	def readFloat3Array(value, name):
		try:
			if len(value) != 3:
				raise Exception()

			return [float(value[0]), float(value[1]), float(value[2])]
		except:
			raise Exception('DynamicTransformNode "' + name + '" must be an array of 3 floats.')

	builder = flatbuffers.Builder(0)
	try:
		scaleData = data.get('scale')
		if scaleData:
			scale = readFloat3Array(scaleData, 'scale')
		else:
			scale = None

		rotateData = data.get('rotate')
		if rotateData:
			rotate = readFloat3Array(rotateData, 'rotate')
		else:
			rotate = None

		translateData = data.get('translate')
		if translateData:
			translate = readFloat3Array(translateData, 'translate')
		else:
			translate = None

		children = data.get('children', [])
		childOffsets = []
		try:
			for child in children:
				try:
					childType = str(child['nodeType'])
					childOffsets.append(
						convertContext.convertNode(builder, childType, child, inputDir, outputDir))
				except KeyError as e:
					raise Exception('Child node data doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('DynamicTransformNode "children" must be an array of objects.')

		itemLists = data.get('itemLists')
	except KeyError as e:
		raise Exception('DynamicTransformNode data doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('DynamicTransformNode data must be an object.')

	if childOffsets:
		DynamicTransformNode.StartChildrenVector(builder, len(childOffsets))
		for offset in reversed(childOffsets):
			builder.PrependUOffsetTRelative(offset)
		childrenOffset = builder.EndVector()
	else:
		childrenOffset = 0

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('DynamicTransformNode "itemLists" must be an array of strings.')

		DynamicTransformNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	DynamicTransformNode.Start(builder)
	DynamicTransformNode.AddScale(builder, CreateVector3f(builder, *scale) if scale else 0)
	DynamicTransformNode.AddOrientation(
		builder, CreateQuaternion4f(builder, *eulerToQuaternion(*rotate)) if rotate else 0)
	DynamicTransformNode.AddPosition(
		builder, CreateVector3f(builder, *translate) if translate else 0)
	DynamicTransformNode.AddChildren(builder, childrenOffset)
	DynamicTransformNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(DynamicTransformNode.End(builder))
	return builder.Output()
