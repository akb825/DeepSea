# Copyright 2020 Aaron Barany
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
from .Matrix44f import *
from .TransformNode import *

def convertTransformNode(convertContext, data):
	"""
	Converts a TransformNode. The data map is expected to contain the following elements:
	- transform: a 4x4 array of floats for a matrix. Each inner array is a column of the matrix.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - type: the name of the node type.
	  - data: the data for the node.
	"""
	builder = flatbuffers.Builder(0)

	try:
		transform = data['transform']
		try:
			values = list()
			for i in range(0, 4):
				for j in range(0, 4):
					values.append(transform[i][j])
			transformOffset = CreateMatrix44f(builder, *values)
		except:
			raise Exception('TransformNode "transform" must be a 4x4 array of floats.')

		children = data['children']
		childOffsets = list()
		try:
			for child in children:
				try:
					childType = str(child['type'])
					childData = child['data']
					childOffsets.append(convertContext.convertNode(builder, childType, childData))
				except KeyError as e:
					raise Exception('Child node data doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('TransformNode "children" must be an array of objects.')
	except (TypeError, ValueError):
		raise Exception('TransformNode data must be an object.')
	except KeyError as e:
		raise Exception('TransformNode data doesn\'t contain element "' + str(e) + '".')

	TransformNodeStartChildrenVector(builder, len(childOffsets))
	for offset in reversed(childOffsets):
		builder.PrependUOffsetTRelative(offset)
	childrenOffset = builder.EndVector(len(childOffsets))

	TransformNodeStart(builder)
	TransformNodeAddTransform(builder, transformOffset)
	TransformNodeAddChildren(builder, childrenOffset)
	builder.Finish(TransformNodeEnd(builder))
	return builder.Output()
