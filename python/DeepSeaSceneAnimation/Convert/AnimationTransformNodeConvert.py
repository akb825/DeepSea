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

from .. import AnimationTransformNode

def convertAnimationTransformNode(convertContext, data):
	"""
	Converts a AnimationTransformNode. The data map is expected to contain the following elements:
	- animationNode: the name of the node within the animation to retrieve the transform from.
	- itemLists: array of item list names to add the node to.
	"""
	try:
		animationNode = str(data['animationNode'])
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('AnimationTransformNode data must be an object.')
	except KeyError as e:
		raise Exception('AnimationTransformNode data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)
	animationNodeOffset = builder.CreateString(animationNode)

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('AnimationTransformNode "itemLists" must be an array of strings.')

		AnimationTransformNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	AnimationTransformNode.Start(builder)
	AnimationTransformNode.AddAnimationNode(builder, animationNodeOffset)
	AnimationTransformNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(AnimationTransformNode.End(builder))
	return builder.Output()
