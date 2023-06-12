# Copyright 2020-2023 Aaron Barany
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
from .DynamicRenderStatesConvert import convertDynamicRenderStates
from ..Color4f import CreateColor4f
from ..DynamicRenderStates import DynamicRenderStates
from .. import ModelList
from ..SortType import SortType
from ..Vector2f import CreateVector2f

def convertModelList(convertContext, data):
	"""
	Converts a ModelList. The data map is expected to contain the following elements:
	- instanceData: optional list of instance data to include with the model list. Each element of
	  the array has the following members:
	  - type: the name of the instance data type.
	  - Remaining members depend on the value of "type".
	- sortType: the method to sort the models. See the dsModelSortType enum for values, removing the
	  type prefix. Defaults to None.
	- dynamicRenderStates: dynamic render states to apply when rendering. This may be omitted if
	  no dynamic render states are used. This is expected to contain any of the following members:
	  - lineWidth: float width for the line. Defaults to 1.
	  - depthBiasConstantFactor: float value for the depth bias constant factor. Defaults to 0.
	  - depthBiasClamp: float value for the depth bias clamp. Defaults to 0.
	  - depthBiasSlopeFactor: float value for the depth bias slope factor. Defaults to 0.
	  - blendConstants: array of 4 floats for the blend color. Defaults to [0, 0, 0, 0].
	  - depthBounds: array of 2 floats for the min and max depth value. Defaults to [0, 1].
	  - stencilCompareMask: int compare mask for both the front and back stencil. Defaults to
	    0xFFFFFFFF.
	  - frontStencilCompareMask: int compare mask for just the front stencil.
	  - backStencilCompareMask: int compare mask for just the back stencil.
	  - stencilWriteMask: int write mask for both the front and back stencil. Defaults to 0.
	  - frontStencilWriteMask: int write mask for just the front stencil.
	  - backStencilWriteMask: int write mask for just the back stencil.
	  - stencilReference: int reference for both the front and back stencil. Defaults to 0.
	  - frontStencilReference: int reference for just the front stencil.
	  - backStencilReference: int reference for just the back stencil.
	- cullList: optional name for the item list to handle culling.
	"""
	builder = flatbuffers.Builder(0)
	try:
		instanceDataInfo = data.get('instanceData', [])
		instanceData = []
		try:
			for info in instanceDataInfo:
				try:
					instanceData.append((str(info['type']), info))
				except KeyError as e:
					raise Exception(
						'ModelList "instanceData" doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('ModelList "instanceData" must be an array of objects.')

		sortTypeStr = data.get('sortType', 'None')
		if sortTypeStr == 'None':
			sortTypeStr += '_'
		if not hasattr(SortType, sortTypeStr):
			raise Exception('Invalid model sort type "' + str(sortTypeStr) + '".')
		sortType = getattr(SortType, sortTypeStr)

		dynamicRenderStateInfo = data.get('dynamicRenderStates')
		if dynamicRenderStateInfo:
			dynamicRenderStatesOffset = convertDynamicRenderStates(dynamicRenderStateInfo, builder)
		else:
			dynamicRenderStatesOffset = 0

		cullList = data.get('cullList')
	except KeyError as e:
		raise Exception('ModelList doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('ModelList must be an object.')

	instanceDataOffsets = []
	for instanceType, instance in instanceData:
		instanceDataOffsets.append(convertContext.convertInstanceData(builder, instanceType,
			instance))

	if instanceDataOffsets:
		ModelList.StartInstanceDataVector(builder, len(instanceDataOffsets))
		for offset in reversed(instanceDataOffsets):
			builder.PrependUOffsetTRelative(offset)
		instanceDataOffset = builder.EndVector()
	else:
		instanceDataOffset = 0

	if cullList:
		cullListOffset = builder.CreateString(str(cullList))
	else:
		cullListOffset = 0

	ModelList.Start(builder)
	ModelList.AddInstanceData(builder, instanceDataOffset)
	ModelList.AddSortType(builder, sortType)
	ModelList.AddDynamicRenderStates(builder, dynamicRenderStatesOffset)
	ModelList.AddCullList(builder, cullListOffset)
	builder.Finish(ModelList.End(builder))
	return builder.Output()
