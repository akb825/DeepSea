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
from .. import RigidBodyGroupNode
from DeepSeaPhysics.MotionType import MotionType

def convertRigidBodyGroupNode(convertContext, data, inputDir, outputDir):
	"""
	Converts a RigidBodyGroupNode. The data map is expected to contain the following elements:
	- motionType: the motion type for rigid bodies within the group. See dsPhysicsMotionType for
	  valid enum values, omitting the type prefix.
	- rigidBodyTemplates: array of string names for the rigid body templates that will be
	  instantiated for each instance of the group node within the scene graph.
	- constraints: array of string names for the constraints that will be instantiated for each
	  instance of the group node within the scene graph.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	- itemLists: array of item list names to add the node to.
	"""
	builder = flatbuffers.Builder(0)
	try:
		motionTypeStr = str(data['motionType'])
		try:
			motionType = getattr(MotionType, motionTypeStr)
		except AttributeError:
			raise Exception('Invalid motion type "' + motionTypeStr + '".')

		rigidBodyTemplates = data.get('rigidBodyTemplates')
		constraints = data.get('constraints')

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
			raise Exception('RigidBodyGroupNode "children" must be an array of objects.')

		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('RigidBodyGroupNode data must be an object.')
	except KeyError as e:
		raise Exception('RigidBodyGroupNode data doesn\'t contain element ' + str(e) + '.')

	if rigidBodyTemplates:
		rigidBodyTemplateOffsets = []
		try:
			for item in rigidBodyTemplates:
				rigidBodyTemplateOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('RigidBodyGroupNode "rigidBodyTemplates" must be an array of strings.')

		RigidBodyGroupNode.StartItemListsVector(builder, len(rigidBodyTemplateOffsets))
		for offset in reversed(rigidBodyTemplateOffsets):
			builder.PrependUOffsetTRelative(offset)
		rigidBodyTemplatesOffset = builder.EndVector()
	else:
		rigidBodyTemplatesOffset = 0

	if constraints:
		constraintOffsets = []
		try:
			for item in constraints:
				constraintOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('RigidBodyGroupNode "constraints" must be an array of strings.')

		RigidBodyGroupNode.StartItemListsVector(builder, len(constraintOffsets))
		for offset in reversed(constraintOffsets):
			builder.PrependUOffsetTRelative(offset)
		constraintsOffset = builder.EndVector()
	else:
		constraintsOffset = 0

	if childOffsets:
		RigidBodyGroupNode.StartChildrenVector(builder, len(childOffsets))
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
			raise Exception('RigidBodyGroupNode "itemLists" must be an array of strings.')

		RigidBodyGroupNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	RigidBodyGroupNode.Start(builder)
	RigidBodyGroupNode.AddMotionType(builder, motionType)
	RigidBodyGroupNode.AddRigidBodyTemplates(builder, rigidBodyTemplatesOffset)
	RigidBodyGroupNode.AddConstraints(builder, constraintsOffset)
	RigidBodyGroupNode.AddChildren(builder, childrenOffset)
	RigidBodyGroupNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(RigidBodyGroupNode.End(builder))
	return builder.Output()
