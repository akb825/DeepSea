# Copyright 2026 Aaron Barany
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

from .. import DiscardBoundsNode
from DeepSeaScene.AlignedBox2f import CreateAlignedBox2f

def convertDiscardBoundsNode(convertContext, data, inputDir, outputDir):
	"""
	Converts a DiscardBoundsNode. The data map is expected to contain the following elements:
	- discardBounds: the discard bounds as a 2x2 array of floats for the minimum and maximum points.
	  If omitted, an empty bounds will be used to initially disable bounds discarding.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	"""
	builder = flatbuffers.Builder(0)
	try:
		boundsData = data.get('discardBounds')
		if boundsData:
			try:
				if len(boundsData) != 2:
					raise ValueError('Invalid length')
				for boundExtent in boundsData:
					if len(boundExtent) != 2:
						raise ValueError('Invalid length')

				bounds = (float(boundsData[0][0]), float(boundsData[0][1]), float(boundsData[1][0]),
					float(boundsData[1][1]))
			except (TypeError, AttributeError, ValueError):
				raise Exception('DiscardBoundsNode "discardBounds" must be a 2x2 array of floats.')
		else:
			bounds = None

		children = data.get('children', [])
		childOffsets = []
		try:
			for child in children:
				try:
					childType = str(child['nodeType'])
					childOffsets.append(
						convertContext.convertNode(builder, childType, child, inputDir, outputDir))
				except KeyError as e:
					raise Exception("Child node data doesn't contain element " + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('DiscardBoundsNode "children" must be an array of objects.')
	except (TypeError, ValueError):
		raise Exception('DiscardBoundsNode data must be an object.')
	except KeyError as e:
		raise Exception("DiscardBoundsNode data doesn't contain element " + str(e) + '.')

	if childOffsets:
		DiscardBoundsNode.StartChildrenVector(builder, len(childOffsets))
		for offset in reversed(childOffsets):
			builder.PrependUOffsetTRelative(offset)
		childrenOffset = builder.EndVector()
	else:
		childrenOffset = 0

	DiscardBoundsNode.Start(builder)
	DiscardBoundsNode.AddDiscardBounds(
		builder, CreateAlignedBox2f(builder, *bounds) if bounds else 0)
	DiscardBoundsNode.AddChildren(builder, childrenOffset)
	builder.Finish(DiscardBoundsNode.End(builder))
	return builder.Output()
