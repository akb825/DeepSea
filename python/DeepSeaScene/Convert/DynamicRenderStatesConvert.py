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

from ..Color4f import *
from ..DynamicRenderStates import *
from ..Vector2f import *

def convertDynamicRenderStates(data, builder):
	"""
	Converts dynamic render states. The data map is expected to contain the following elements:
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
	def readFloat(value, name):
		try:
			return float(value)
		except:
			raise Exception('Invalid ' + name + ' float value "' + str(value) + '".')

	def readUInt(value, name):
		try:
			intVal = int(value)
			if intVal < 0:
				raise Exception()
			return intVal
		except:
			raise Exception('Invalid ' + name + ' unsigned int value "' + str(value) + '".')

	lineWidth = readFloat(data.get('lineWidth', 1.0), 'line width')
	depthBiasConstantFactor = readFloat(data.get('depthBiasConstantFactor', 0.0),
		'depth bias constant factor')
	depthBiasClamp = readFloat(data.get('depthBiasClamp', 0.0), 'depth bias clamp')
	depthBiasSlopeFactor = readFloat(data.get('depthBiasSlopeFactor', 0.0),
		'depth bias slope factor')

	colorValue = data.get('blendConstants', [0.0, 0.0, 0.0, 0.0])
	try:
		if len(colorValue) != 4:
			raise Exception()
	except:
		raise Exception('Blend constants value must be an array of 4 floats.')
	blendConstants = []
	for c in colorValue:
		blendConstants.append(readFloat(c, 'blend constant'))

	depthBoundsValue = data.get('depthBounds', [0.0, 1.0])
	try:
		if len(depthBoundsValue) != 2:
			raise Exception()
	except:
		raise Exception('Depth bounds value must be an array of 2 floats.')
	depthBounds = []
	for b in depthBoundsValue:
		depthBounds.append(readFloat(b, 'depth bounds'))

	stencilCompareMask = data.get('stencilCompareMask', 0xFFFFFFFF)
	frontStencilCompareMask = readUInt(data.get('frontStencilCompareMask', stencilCompareMask),
		'stencil compare mask')
	backStencilCompareMask = readUInt(data.get('backStencilCompareMask', stencilCompareMask),
		'stencil compare mask')

	stencilWriteMask = data.get('stencilWriteMask', 0)
	frontStencilWriteMask = readUInt(data.get('frontStencilWriteMask', stencilWriteMask),
		'stencil write mask')
	backStencilWriteMask = readUInt(data.get('backStencilWriteMask', stencilWriteMask),
		'stencil write mask')

	stencilReference = data.get('stencilReference', 0)
	frontStencilReference = readUInt(data.get('frontStencilReference', stencilReference),
		'stencil reference')
	backStencilReference = readUInt(data.get('backStencilReference', stencilReference),
		'stencil reference')

	DynamicRenderStatesStart(builder)
	DynamicRenderStatesAddLineWidth(builder, lineWidth)
	DynamicRenderStatesAddDepthBiasConstantFactor(builder, depthBiasConstantFactor)
	DynamicRenderStatesAddDepthBiasClamp(builder, depthBiasClamp)
	DynamicRenderStatesAddDepthBiasSlopeFactor(builder, depthBiasSlopeFactor)
	DynamicRenderStatesAddBlendConstants(builder, CreateColor4f(builder, *blendConstants))
	DynamicRenderStatesAddDepthBounds(builder, CreateVector2f(builder, *depthBounds))
	DynamicRenderStatesAddFrontStencilCompareMask(builder, frontStencilCompareMask)
	DynamicRenderStatesAddBackStencilCompareMask(builder, backStencilCompareMask)
	DynamicRenderStatesAddFrontStencilWriteMask(builder, frontStencilWriteMask)
	DynamicRenderStatesAddBackStencilWriteMask(builder, backStencilWriteMask)
	DynamicRenderStatesAddFrontStencilReference(builder, frontStencilReference)
	DynamicRenderStatesAddBackStencilReference(builder, backStencilReference)
	return DynamicRenderStatesEnd(builder)
