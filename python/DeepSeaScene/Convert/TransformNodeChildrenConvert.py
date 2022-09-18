# Copyright 2022 Aaron Barany
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
from .. import TransformNodeChildren

def convertTransformNodeChildren(convertContext, data):
	"""
	Adds an action to insert children into an existing TransformNode. The data map is expected to
	contain the following elements:
	- node: the name of the transform node to add children to.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	"""
	builder = flatbuffers.Builder(0)
	try:
		node = str(data['node'])
		children = data['children']
		childOffsets = []
		try:
			for child in children:
				try:
					childType = str(child['nodeType'])
					childOffsets.append(convertContext.convertNode(builder, childType, child))
				except KeyError as e:
					raise Exception('Child node data doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('TransformNodeChildren "children" must be an array of objects.')
	except KeyError as e:
		raise Exception('TransformNodeChildren data doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('TransformNodeChildren data must be an object.')

	nodeOffset = builder.CreateString(node)

	TransformNodeChildren.StartChildrenVector(builder, len(childOffsets))
	for offset in reversed(childOffsets):
		builder.PrependUOffsetTRelative(offset)
	childrenOffset = builder.EndVector()

	TransformNodeChildren.Start(builder)
	TransformNodeChildren.AddNode(builder, nodeOffset)
	TransformNodeChildren.AddChildren(builder, childrenOffset)
	builder.Finish(TransformNodeChildren.End(builder))
	return builder.Output()
