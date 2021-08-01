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
from .. import ShadowCullList

def convertShadowCullList(convertContext, data):
	"""
	Converts a ShadowCullList. The data map is expected to contain the following elements:
	- lightShadows: name of the light shadows that will be drawn for.
	- surface: index of the surface within the light shadows.
	"""
	try:
		lightShadows = str(data['lightShadows'])
		surfaceVal = data['surface']
		try:
			surface = int(surfaceVal)
			if surface < 0 or surface > 6:
				raise Exception() # Common error handling in except block.
		except:
			raise Exception('Invalid surface index "' + str(surfaceVal) + '".')
	except KeyError as e:
		raise Exception('ShadowCullList doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('ShadowCullList must be an object.')

	builder = flatbuffers.Builder(0)

	lightShadowsOffset = builder.CreateString(lightShadows)

	ShadowCullList.Start(builder)
	ShadowCullList.AddLightShadows(builder, lightShadowsOffset)
	ShadowCullList.AddSurface(builder, surface)
	builder.Finish(ShadowCullList.End(builder))
	return builder.Output()
