# Copyright 2025 Aaron Barany
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
from .. import ShiftNode
from ..Vector3d import CreateVector3d

def convertShiftNode(convertContext, data, outputDir):
	"""
	Converts a ShiftNode. The data map is expected to contain the following elements:
	- origin: the origin of the node, or zero if unset.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	- itemLists: array of item list names to add the node to.
	"""
	def convertFloat(value):
		try:
			return float(value)
		except:
			raise Exception('Invalid float value "' + str(value) + '".')

	builder = flatbuffers.Builder(0)
	try:
		originData = data.get('origin')
		if originData is None:
			origin = None
		else:
			if not isinstance(originData, list) or len(originData) != 3:
				raise Exception('Shift node origin must be an array of three floats.')
			origin = (convertFloat(originData[0]), convertFloat(originData[1]),
				convertFloat(originData[2]))

		children = data.get('children', [])
		childOffsets = []
		try:
			for child in children:
				try:
					childType = str(child['nodeType'])
					childOffsets.append(
						convertContext.convertNode(builder, childType, child, outputDir))
				except KeyError as e:
					raise Exception('Child node data doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('ShiftNode "children" must be an array of objects.')

		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('ShiftNode data must be an object.')
	except KeyError as e:
		raise Exception('ShiftNode data doesn\'t contain element ' + str(e) + '.')

	if childOffsets:
		ShiftNode.StartChildrenVector(builder, len(childOffsets))
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
			raise Exception('ShiftNode "itemLists" must be an array of strings.')

		ShiftNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	ShiftNode.Start(builder)
	ShiftNode.AddOrigin(builder, CreateVector3d(builder, *origin) if origin else None)
	ShiftNode.AddChildren(builder, childrenOffset)
	ShiftNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(ShiftNode.End(builder))
	return builder.Output()
