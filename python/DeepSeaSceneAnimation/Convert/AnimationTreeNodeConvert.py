# Copyright 2023 Aaron Barany
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

from .. import AnimationTreeNode

def convertAnimationTreeNode(convertContext, data):
	"""
	Converts a AnimationTreeNode. The data map is expected to contain the following elements:
	- animationTree: the name of the animation tree to use for the node.
	- nodeMapCache: the name of the animation node map cache to use with the animation tree.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	- itemLists: array of item list names to add the node to.
	"""
	builder = flatbuffers.Builder(0)
	try:
		animationTree = str(data['animationTree'])
		nodeMapCache = str(data['nodeMapcache'])

		children = data.get('children', [])
		childOffsets = []
		try:
			for child in children:
				try:
					childType = str(child['nodeType'])
					childOffsets.append(convertContext.convertNode(builder, childType, child))
				except KeyError as e:
					raise Exception('Child node data doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('AnimationTreeNode "children" must be an array of objects.')

		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('AnimationTreeNode data must be an object.')
	except KeyError as e:
		raise Exception('AnimationTreeNode data doesn\'t contain element ' + str(e) + '.')

	animationTreeOffset = builder.CreateString(animationTree)
	nodeMapCacheOffset = builder.CreateString(nodeMapCache)

	if childOffsets:
		AnimationTreeNode.StartChildrenVector(builder, len(childOffsets))
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
			raise Exception('AnimationTreeNode "itemLists" must be an array of strings.')

		AnimationTreeNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	AnimationTreeNode.Start(builder)
	AnimationTreeNode.AddAnimationTree(builder, animationTreeOffset)
	AnimationTreeNode.AddNodeMapCache(builder, nodeMapCacheOffset)
	AnimationTreeNode.AddChildren(builder, childrenOffset)
	AnimationTreeNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(AnimationTreeNode.End(builder))
	return builder.Output()
