# Copyright 2020-2026 Aaron Barany
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
import math
from .TransformConvert import convertTransform
from ..Matrix44f import CreateMatrix44f
from .. import TransformNode

def convertTransformNode(convertContext, data, inputDir, outputDir):
	"""
	Converts a TransformNode. The data map is expected to contain the following elements:
	- transformList: array of transforms to apply. The matrices for the transforms provided are
	  multiplied in reverse order given (i.e. in logical transform order), starting from the
	  identity matrix. Each member of the array has the following elements:
	  - type: the type of transform. May be Rotate, Scale, Translate, or Matrix.
	  - value: the value of transform based on the type:
	    - Rotate: array of 3 floats for the X, Y, and Z rotation in degrees.
	    - Scale: array of 3 floats for the scale value along the X, Y, and Z axes.
	    - Translate: array of 3 floats for the translation along X, Y, and Z.
	    - Matrix: a 4x4 array of floats for a matrix. Each inner array is a column of the matrix.
	- children: an array of child nodes. Each element is an object with the following elements:
	  - nodeType: the name of the node type.
	  - data: the data for the node.
	"""
	builder = flatbuffers.Builder(0)
	try:
		matrix = convertTransform(data.get('transformList', []), 'TransformNode', 'transformList')
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
			raise Exception('TransformNode "children" must be an array of objects.')
	except KeyError as e:
		raise Exception('TransformNode data doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('TransformNode data must be an object.')

	if childOffsets:
		TransformNode.StartChildrenVector(builder, len(childOffsets))
		for offset in reversed(childOffsets):
			builder.PrependUOffsetTRelative(offset)
		childrenOffset = builder.EndVector()
	else:
		childrenOffset = 0

	TransformNode.Start(builder)
	TransformNode.AddTransform(builder, CreateMatrix44f(builder, *matrix))
	TransformNode.AddChildren(builder, childrenOffset)
	builder.Finish(TransformNode.End(builder))
	return builder.Output()
