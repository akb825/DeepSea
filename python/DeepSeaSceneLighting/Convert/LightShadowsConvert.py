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
import math

from .. import SceneLightShadows
from ..LightType import LightType

def convertLightShadows(convertContext, data):
	"""
	Converts a light shadows for a scene. The data map is expected to contain the following
	elements:
	- lightSet: name of the light set to query the light from.
	- lightType: type of the light to shadow. See dsSceneLightType enum for values, removing the
	  type prefix.
	- light: name of the light to shadow. May be unset to disable initially until set at runtime.
	- transformGroupDesc: name of the shader variable group description for the transform group.
	- maxCascades: the maximum number of cascades for cascaded directional light shadows. Defaults
	  to 4.
	- maxFirstSplitDistance: maximum distance for the first split for cascaded shadows. Defaults to
	  100.
	- cascadeExpFactor: exponential factor for cascaded shadows in the range [0, 1], where 0 uses
	  linear distances between the splits and 1 is fully exponential. Defaults to 0.5.
	- fadeStartDistance: the distance to start fading out shadows. Defaults to 1000000, which is a
	  large distance less likely to break GPUs that use limited precision floats.
	- maxDistance: the maximum distance to display shadows. Defaults to 1000000, which is a large
	  distance less likely to break GPUs that use limited precision floats.
	"""
	largeDistance = 100000000.0

	def readFloat(value, name, minVal = None, maxVal = None):
		try:
			floatVal = float(value)
			if (minVal is not None and floatVal < minVal) or \
					(maxVal is not None and floatVal > maxVal):
				raise Exception() # Common error handling in except block.
			return floatVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	try:
		lightSet = str(data['lightSet'])

		lightTypeStr = str(data['lightType'])
		try:
			lightType = getattr(LightType, lightTypeStr)
		except AttributeError:
			raise Exception('Invalid light type "' + lightTypeStr + '".')

		light = str(data.get('light', ''))
		transformGroupDesc = data['transformGroupDesc']

		maxCascadesVal = data.get('maxCascades', 4)
		try:
			maxCascades = int(maxCascadesVal)
			if maxCascades < 1 or maxCascades > 4:
				raise Exception() # Common error handling in except block.
		except:
			raise Exception('Invalid max cascade count "' + str(maxCascadesVal) + '".')

		maxFirstSplitDistance = readFloat(data.get('maxFirstSplitDistance', 100.0),
			'max first split distance', 0.1)
		cascadeExpFactor = readFloat(data.get('cascadeExpFactor', 0.5), 'cascade exp factor', 0.0,
			1.0)
		fadeStartDistance = readFloat(data.get('fadeStartDistance', largeDistance),
			'fade start distance', 0.0)
		maxDistance = readFloat(data.get('maxDistance', largeDistance), 'max distance', 0.1)
	except KeyError as e:
		raise Exception('LightShadows doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('LightShadows must be an object.')

	builder = flatbuffers.Builder(0)

	lightSetOffset = builder.CreateString(lightSet)
	if light:
		lightOffset = builder.CreateString(light)
	else:
		lightOffset = 0
	transformGroupDescOffset = builder.CreateString(transformGroupDesc)

	SceneLightShadows.Start(builder)
	SceneLightShadows.AddLightSet(builder, lightSetOffset)
	SceneLightShadows.AddLightType(builder, lightType)
	SceneLightShadows.AddLight(builder, lightOffset)
	SceneLightShadows.AddTransformGroupDesc(builder, transformGroupDescOffset)
	SceneLightShadows.AddMaxCascades(builder, maxCascades)
	SceneLightShadows.AddMaxFirstSplitDistance(builder, maxFirstSplitDistance)
	SceneLightShadows.AddCascadeExpFactor(builder, cascadeExpFactor)
	SceneLightShadows.AddFadeStartDistance(builder, fadeStartDistance)
	SceneLightShadows.AddMaxDistance(builder, maxDistance)
	builder.Finish(SceneLightShadows.End(builder))
	return builder.Output()
