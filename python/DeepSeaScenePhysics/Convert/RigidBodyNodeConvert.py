# Copyright 2024-2026 Aaron Barany
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
from .. import RigidBodyNode
from DeepSeaPhysics.MotionType import MotionType

def convertRigidBodyNode(convertContext, data, inputDir, outputDir):
	"""
	Converts a RigidBodyNode. The data map is expected to contain the following elements:
	- rigidBody: the name of the rigid body.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	- itemLists: array of item list names to add the node to.
	"""
	builder = flatbuffers.Builder(0)
	try:
		rigidBody = str(data['rigidBody'])

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
			raise Exception('RigidBodyNode "children" must be an array of objects.')

		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('RigidBodyNode data must be an object.')
	except KeyError as e:
		raise Exception('RigidBodyNode data doesn\'t contain element ' + str(e) + '.')

	rigidBodyOffset = builder.CreateString(rigidBody)

	if childOffsets:
		RigidBodyNode.StartChildrenVector(builder, len(childOffsets))
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
			raise Exception('RigidBodyNode "itemLists" must be an array of strings.')

		RigidBodyNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	RigidBodyNode.Start(builder)
	RigidBodyNode.AddRigidBody(builder, rigidBodyOffset)
	RigidBodyNode.AddChildren(builder, childrenOffset)
	RigidBodyNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(RigidBodyNode.End(builder))
	return builder.Output()
