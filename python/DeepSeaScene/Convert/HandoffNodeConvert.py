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
from .. import HandoffNode

def convertHandoffNode(convertContext, data, outputDir):
	"""
	Converts a HandoffNode. The data map is expected to contain the following elements:
	- transitionTime: the time in seconds to transition from one transform to another.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	- itemLists: array of item list names to add the node to.
	"""
	builder = flatbuffers.Builder(0)
	try:
		transitionTimeStr = data['transitionTime']
		try:
			transitionTime = float(transitionTimeStr)
			if transitionTimeStr < 0:
				raise Exception()
		except:
			raise Exception('Invalid transition time value "' + transitionTimeStr + '".')

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
			raise Exception('HandoffNode "children" must be an array of objects.')

		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('HandoffNode data must be an object.')
	except KeyError as e:
		raise Exception('HandoffNode data doesn\'t contain element ' + str(e) + '.')

	if childOffsets:
		HandoffNode.StartChildrenVector(builder, len(childOffsets))
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
			raise Exception('HandoffNode "itemLists" must be an array of strings.')

		HandoffNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	HandoffNode.Start(builder)
	HandoffNode.AddTransitionTime(builder, transitionTime)
	HandoffNode.AddChildren(builder, childrenOffset)
	HandoffNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(HandoffNode.End(builder))
	return builder.Output()
