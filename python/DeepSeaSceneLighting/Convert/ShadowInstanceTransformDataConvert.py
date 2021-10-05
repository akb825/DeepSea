# Copyright 2021 Aaron Barany
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
from .. import ShadowInstanceTransformData

def convertShadowInstanceTransformData(convertContext, data):
	"""
	Converts a ShadowInstanceTransformData. The data map is expected to contain the following
	elements:
	- shadowManager: name of the shadow manager that contains the shadows to get the transform from.
	- shadows: name of the shadows within the shadow manager to get the transform from.
	- surface: index of the surface within the shadows to get the transform from.
	- variableGroupDesc:  name for the shader variable group to use.
	"""
	try:
		shadowManager = str(data['shadowManager'])
		shadows = str(data['shadows'])
		variableGroupDesc = str(data['variableGroupDesc'])
		surfaceVal = data['surface']
		try:
			surface = int(surfaceVal)
			if surface < 0 or surface > 6:
				raise Exception() # Common error handling in except block.
		except:
			raise Exception('Invalid surface index "' + str(surfaceVal) + '".')
	except KeyError as e:
		raise Exception('ShadowInstanceData doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('ShadowInstanceData must be an object.')

	builder = flatbuffers.Builder(0)
	shadowManagerOffset = builder.CreateString(shadowManager)
	shadowsOffset = builder.CreateString(shadows)
	variableGroupDescOffset = builder.CreateString(variableGroupDesc)

	ShadowInstanceTransformData.Start(builder)
	ShadowInstanceTransformData.AddShadowManager(builder, shadowManagerOffset)
	ShadowInstanceTransformData.AddShadows(builder, shadowsOffset)
	ShadowInstanceTransformData.AddSurface(builder, surface)
	ShadowInstanceTransformData.AddVariableGroupDesc(builder, variableGroupDescOffset)
	builder.Finish(ShadowInstanceTransformData.End(builder))
	return builder.Output()
