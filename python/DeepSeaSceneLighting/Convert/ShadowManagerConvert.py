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
from .. import SceneShadowManager
from ..LightType import LightType

class Object:
	pass

def convertShadowManager(convertContext, data):
	"""
	Converts a shadow manager for a scene. The data map is expected to contain the following
	elements:
	- lightSet: the name of the light set to query the light from. If set, this will be the default
	  for elements in the shadows array.
	- shadows: array of objects for the shadows the shadow manager will manage. Each element is
	  expected to have the following members:
	  - name: name of the shadows.
	  - lightSet: name of the light set to query the light from.
	  - lightType: type of the light to shadow. See dsSceneLightType enum for values, removing the
	    type prefix.
	  - light: name of the light to shadow. May be unset to disable initially until set at runtime.
	  - transformGroupDesc: name of the shader variable group description for the transform group.
	  - transformGroupName: name of the transform group to set as view global data. This may be
	    omitted if not used as global data on a view.
	  - maxCascades: the maximum number of cascades for cascaded directional light shadows. Defaults
	    to 4.
	  - maxFirstSplitDistance: maximum distance for the first split for cascaded shadows. Defaults
	    to 100.
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
		shadowsData = data['shadows']
		defaultLightSet = data.get('lightSet', '')
		shadows = []
		try:
			for shadowData in shadowsData:
				try:
					shadow = Object()
					shadow.name = str(data['name'])
					shadow.lightSet = str(data.get('lightSet', defaultLightSet))
					if not shadow.lightSet:
						raise KeyError('lightSet')

					lightTypeStr = str(data['lightType'])
					try:
						shadow.lightType = getattr(LightType, lightTypeStr)
					except AttributeError:
						raise Exception('Invalid light type "' + lightTypeStr + '".')

					shadow.light = str(data.get('light', ''))
					shadow.transformGroupDesc = str(data['transformGroupDesc'])
					shadow.transformGroupName = str(data.get('transformGroupName', ''))

					maxCascadesVal = data.get('maxCascades', 4)
					try:
						shadow.maxCascades = int(maxCascadesVal)
						if shadow.maxCascades < 1 or shadow.maxCascades > 4:
							raise Exception() # Common error handling in except block.
					except:
						raise Exception('Invalid max cascade count "' + str(maxCascadesVal) + '".')

					shadow.maxFirstSplitDistance = readFloat(
						data.get('maxFirstSplitDistance', 100.0), 'max first split distance', 0.1)
					shadow.cascadeExpFactor = readFloat(
						data.get('cascadeExpFactor', 0.5), 'cascade exp factor', 0.0, 1.0)
					shadow.fadeStartDistance = readFloat(
						data.get('fadeStartDistance', largeDistance), 'fade start distance', 0.0)
					shadow.maxDistance = readFloat(
						data.get('maxDistance', largeDistance), 'max distance', 0.1)
				except KeyError as e:
					raise Exception('ShadowManager shadows doesn\'t contain element ' + str(e) +
						'.')
		except (AttributeError, TypeError, ValueError):
			raise Exception('ShadowManager shadows must be an array of objects.')
	except KeyError as e:
		raise Exception('ShadowManager doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('ShadowManager must be an object.')

	builder = flatbuffers.Builder(0)

	shadowOffsets = []
	for shadow in shadows:
		nameOffset = builder.CreateString(shadow.name)
		lightSetOffset = builder.CreateString(shadow.lightSet)
		if shadow.light:
			lightOffset = builder.CreateString(shadow.light)
		else:
			lightOffset = 0
		transformGroupDescOffset = builder.CreateString(shadow.transformGroupDesc)
		if shadow.transformGroupName:
			transformGroupNameOffset = builder.CreateString(shadow.transformGroupName)
		else:
			transformGroupNameOffset = 0

		SceneLightShadows.Start(builder)
		SceneLightShadows.AddLightSet(builder, lightSetOffset)
		SceneLightShadows.AddLightType(builder, shadow.lightType)
		SceneLightShadows.AddLight(builder, lightOffset)
		SceneLightShadows.AddTransformGroupDesc(builder, transformGroupDescOffset)
		SceneLightShadows.AddMaxCascades(builder, shadow.maxCascades)
		SceneLightShadows.AddMaxFirstSplitDistance(builder, shadow.maxFirstSplitDistance)
		SceneLightShadows.AddCascadeExpFactor(builder, shadow.cascadeExpFactor)
		SceneLightShadows.AddFadeStartDistance(builder, shadow.fadeStartDistance)
		SceneLightShadows.AddMaxDistance(builder, shadow.maxDistance)
		shadowOffsets.append(SceneLightShadows.End(builder))

	SceneShadowManager.StartShadowsVector(builder, len(shadowOffsets))
	for offset in reversed(shadowOffsets):
		builder.PrependUOffsetTRelative(offset)
	shadowsOffset = builder.EndVector()

	SceneShadowManager.Start(builder)
	SceneShadowManager.AddShadows(builder, shadowsOffset)
	builder.Finish(SceneShadowManager.End(builder))
	return builder.Output()
