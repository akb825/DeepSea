# Copyright 2020-2025 Aaron Barany
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
from .. import VectorItemList
from DeepSeaScene.Convert.DynamicRenderStatesConvert import convertDynamicRenderStates

def convertVectorItemList(convertContext, data):
	"""
	Converts a VectorItemList. The data map is expected to contain the following elements:
	- instanceData: optional list of instance data to include with the item list. Each element of
	  the array has the following members:
	  - type: the name of the instance data type.
	  - Remaining members depend on the value of "type".
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
						'VectorItemList "instanceData" doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('VectorItemList "instanceData" must be an array of objects.')

		dynamicRenderStateInfo = data.get('dynamicRenderStates')
		if dynamicRenderStateInfo:
			dynamicRenderStatesOffset = convertDynamicRenderStates(dynamicRenderStateInfo, builder)
		else:
			dynamicRenderStatesOffset = 0

		views = data.get('views', [])
		if not isinstance(views, list):
			raise Exception('VectorItemList "views" must be an array of strings.')
	except KeyError as e:
		raise Exception('VectorItemList doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('VectorItemList must be an object.')

	instanceDataOffsets = []
	for instanceType, instance in instanceData:
		instanceDataOffsets.append(convertContext.convertInstanceData(builder, instanceType,
			instance))

	if instanceDataOffsets:
		VectorItemList.StartInstanceDataVector(builder, len(instanceDataOffsets))
		for offset in reversed(instanceDataOffsets):
			builder.PrependUOffsetTRelative(offset)
		instanceDataOffset = builder.EndVector()
	else:
		instanceDataOffset = 0

	if views:
		viewOffsets = []
		for view in views:
			viewOffsets.append(builder.CreateString(str(view)))
		VectorItemList.StartViewsVector(builder, len(viewOffsets))
		for offset in reversed(viewOffsets):
			builder.PrependUOffsetTRelative(offset)
		viewsOffset = builder.EndVector()
	else:
		viewsOffset = 0

	VectorItemList.Start(builder)
	VectorItemList.AddInstanceData(builder, instanceDataOffset)
	VectorItemList.AddDynamicRenderStates(builder, dynamicRenderStatesOffset)
	VectorItemList.AddViews(builder, viewsOffset)
	builder.Finish(VectorItemList.End(builder))
	return builder.Output()
