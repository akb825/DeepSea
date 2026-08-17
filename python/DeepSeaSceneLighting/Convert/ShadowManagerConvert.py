# Copyright 2021-2026 Aaron Barany
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

def convertShadowManager(convertContext, data, inputDir, outputDir):
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
	  - paddingRatios: ratio to apply to each dimension to padd the bounds for each cascade. For
	    cascaded shadows, the same value will be used for each cascade.
	  - paddingRatio: ratio to apply to each dimension to pad the bounds.
	  - minPaddings: the minimum padding to apply to each direction of the bounds for each cascade.
	  - minPadding: the minimum padding to apply to each direction of the bounds. For cascaded
	    shadows, the same value will be used for each cascade.
	  - minDepthRanges: minimum distance between the near and far planes for each cascade.
	  - minDepthRange: minimum distance between the  near and far planes. For cascaded shadows, the
	    same value will be used for each cascade.
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
					shadow.name = str(shadowData['name'])
					shadow.lightSet = str(shadowData.get('lightSet', defaultLightSet))
					if not shadow.lightSet:
						raise KeyError('lightSet')

					lightTypeStr = str(shadowData['lightType'])
					try:
						shadow.lightType = getattr(LightType, lightTypeStr)
					except AttributeError:
						raise Exception('Invalid light type "' + lightTypeStr + '".')

					shadow.light = str(shadowData.get('light', ''))
					shadow.transformGroupDesc = str(shadowData['transformGroupDesc'])
					shadow.transformGroupName = str(shadowData.get('transformGroupName', ''))

					maxCascadesVal = shadowData.get('maxCascades', 4)
					try:
						shadow.maxCascades = int(maxCascadesVal)
						if shadow.maxCascades < 1 or shadow.maxCascades > 4:
							raise Exception() # Common error handling in except block.
					except:
						raise Exception('Invalid max cascade count "' + str(maxCascadesVal) + '".')

					shadow.paddingRatios = []
					paddingRatiosVal = shadowData.get('paddingRatios')
					if paddingRatiosVal is not None:
						if not isinstance(paddingRatiosVal, list):
							raise Exception(
								'ShadowManager paddingRatios must be an array of floats.')
						for val in paddingRatiosVal:
							shadow.paddingRatios.append(readFloat(val, 'paddingRatios', 0.0))
					else:
						paddingRatioVal = shadowData.get('paddingRatio')
						if paddingRatioVal is not None:
							floatVal = readFloat(paddingRatioVal, 'paddingRatio', 0.0)
							shadow.paddingRatios = [floatVal, floatVal, floatVal, floatVal]

					shadow.minPaddings = []
					minPaddingsVal = shadowData.get('minPaddings')
					if minPaddingsVal is not None:
						if not isinstance(minPaddingsVal, list):
							raise Exception(
								'ShadowManager minPaddings must be an array of floats.')
						for val in minPaddingsVal:
							shadow.minPaddings.append(readFloat(val, 'minPaddings', 0.0))
					else:
						minPaddingVal = shadowData.get('minPadding')
						if minPaddingVal is not None:
							floatVal = readFloat(minPaddingVal, 'minPadding', 0.0)
							shadow.minPaddings = [floatVal, floatVal, floatVal, floatVal]

					shadow.minDepthRanges = []
					minDepthRangesVal = shadowData.get('minDepthRanges')
					if minDepthRangesVal is not None:
						if not isinstance(minDepthRangesVal, list):
							raise Exception(
								'ShadowManager minDepthRanges must be an array of floats.')
						for val in minDepthRangesVal:
							shadow.minDepthRanges.append(readFloat(val, 'minDepthRanges', 0.0))
					else:
						minDepthRangeVal = shadowData.get('minDepthRange')
						if minDepthRangeVal is not None:
							floatVal = readFloat(minDepthRangeVal, 'minDepthRange', 0.0)
							shadow.minDepthRanges = [floatVal, floatVal, floatVal, floatVal]

					shadow.maxFirstSplitDistance = readFloat(
						shadowData.get('maxFirstSplitDistance', 100.0), 'max first split distance',
						0.1)
					shadow.cascadeExpFactor = readFloat(
						shadowData.get('cascadeExpFactor', 0.5), 'cascade exp factor', 0.0, 1.0)
					shadow.fadeStartDistance = readFloat(
						shadowData.get('fadeStartDistance', largeDistance), 'fade start distance',
						0.0)
					shadow.maxDistance = readFloat(
						shadowData.get('maxDistance', largeDistance), 'max distance', 0.1)
					shadows.append(shadow)
				except KeyError as e:
					raise Exception(
						'ShadowManager shadows doesn\'t contain element ' + str(e) + '.')
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

		if shadow.paddingRatios:
			SceneLightShadows.StartPaddingRatiosVector(builder, len(shadow.paddingRatios))
			for val in reversed(shadow.paddingRatios):
				builder.PrependFloat32(val)
			paddingRatiosOffset = builder.EndVector()
		else:
			paddingRatiosOffset = 0

		if shadow.minPaddings:
			SceneLightShadows.StartMinPaddingsVector(builder, len(shadow.minPaddings))
			for val in reversed(shadow.minPaddings):
				builder.PrependFloat32(val)
			minPaddingsOffset = builder.EndVector()
		else:
			minPaddingsOffset = 0

		if shadow.minDepthRanges:
			SceneLightShadows.StartMinDepthRangesVector(builder, len(shadow.minDepthRanges))
			for val in reversed(shadow.minDepthRanges):
				builder.PrependFloat32(val)
			minDepthRangesOffset = builder.EndVector()
		else:
			minDepthRangesOffset = 0

		SceneLightShadows.Start(builder)
		SceneLightShadows.AddName(builder, nameOffset)
		SceneLightShadows.AddLightSet(builder, lightSetOffset)
		SceneLightShadows.AddLightType(builder, shadow.lightType)
		SceneLightShadows.AddLight(builder, lightOffset)
		SceneLightShadows.AddTransformGroupDesc(builder, transformGroupDescOffset)
		SceneLightShadows.AddTransformGroupName(builder, transformGroupNameOffset)
		SceneLightShadows.AddMaxCascades(builder, shadow.maxCascades)
		SceneLightShadows.AddMaxFirstSplitDistance(builder, shadow.maxFirstSplitDistance)
		SceneLightShadows.AddCascadeExpFactor(builder, shadow.cascadeExpFactor)
		SceneLightShadows.AddPaddingRatios(builder, paddingRatiosOffset)
		SceneLightShadows.AddMinPaddings(builder, minPaddingsOffset)
		SceneLightShadows.AddMinDepthRanges(builder, minDepthRangesOffset)
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
