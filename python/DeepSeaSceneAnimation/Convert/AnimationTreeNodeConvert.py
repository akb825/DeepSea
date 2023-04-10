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
	- itemLists: array of item list names to add the node to.
	"""
	try:
		animationTree = str(data['animationTree'])
		nodeMapCache = str(data['nodeMapcache'])
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('AnimationTreeNode data must be an object.')
	except KeyError as e:
		raise Exception('AnimationTreeNode data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)
	animationTreeOffset = builder.CreateString(animationTree)
	nodeMapCacheOffset = builder.CreateString(nodeMapCache)

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
	AnimationTreeNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(AnimationTreeNode.End(builder))
	return builder.Output()
