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

from .. import AnimationNode

def convertAnimationNode(convertContext, data):
	"""
	Converts a AnimationNode. The data map is expected to contain the following elements:
	- nodeMapCache: the name of the animation node map cache to use with the animation tree.
	- itemLists: array of item list names to add the node to.
	"""
	try:
		nodeMapCache = str(data['nodeMapcache'])
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('AnimationNode data must be an object.')
	except KeyError as e:
		raise Exception('AnimationNode data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)
	nodeMapCacheOffset = builder.CreateString(nodeMapCache)

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('AnimationNode "itemLists" must be an array of strings.')

		AnimationNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	AnimationNode.Start(builder)
	AnimationNode.AddNodeMapCache(builder, nodeMapCacheOffset)
	AnimationNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(AnimationNode.End(builder))
	return builder.Output()
