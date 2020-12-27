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
from .DynamicRenderStatesConvert import convertDynamicRenderStates
from ..FullScreenResolve import *
from ..DynamicRenderStates import *

def convertFullScreenResolve(convertContext, data):
	"""
	Converts a FullScreenResolve. The data map is expected to contain the following elements:
	- shaderName: the name of the shader to draw with.
	- materialName: the name of the material to draw with.
	- dynamicRenderStates: dynamic render states to apply when drawing. This may be ommitted if no
	  dynamic render states are used. This is expected to contain any of the following members:
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
	  - backStencilReference: int reference for just the back stencil
	"""
	builder = flatbuffers.Builder(0)
	try:
		shaderName = str(data['shaderName'])
		materialName = str(data['materialName'])

		dynamicRenderStateInfo = data.get('dynamicRenderStates')
		if dynamicRenderStateInfo:
			dynamicRenderStatesOffset = convertDynamicRenderStates(dynamicRenderStateInfo, builder)
		else:
			dynamicRenderStatesOffset = 0
	except KeyError as e:
		raise Exception('FullScreenResolve doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('FullScreenResolve must be an object.')

	shaderNameOffset = builder.CreateString(shaderName)
	materialNameOffset = builder.CreateString(materialName)

	FullScreenResolveStart(builder)
	FullScreenResolveAddShaderName(builder, shaderNameOffset)
	FullScreenResolveAddMaterialName(builder, materialNameOffset)
	FullScreenResolveAddDynamicRenderStates(builder, dynamicRenderStatesOffset)
	builder.Finish(FullScreenResolveEnd(builder))
	return builder.Output()
