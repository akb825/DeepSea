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
from ..DeferredLightResolve import *

def convertDeferredLightResolve(convertContext, data):
	"""
	Converts a DeferredLightResolve. The data map is expected to contain the following elements:
	- lightSet: name of the light set to draw the lights from.
	- ambientShader: the name of the shader used for the ambient light. This may be null or ommitted
	  to not draw ambient.
	- ambientMaterial: the name of the material used for the ambient light.
	- drectionalShader: the name of the shader used for directional lights. This may be null or
	  ommitted to not draw directional lights.
	- drectionalMaterial: the name of the material used for directional lights.
	- pointShader: the name of the shader used for point lights. This may be null or ommitted to not
	  draw point lights.
	- pointMaterial: the name of the material used for point lights.
	- spotShader: the name of the shader used for spot lights. This may be null or ommitted to not
	  draw spot lights.
	- spotMaterial: the name of the material used for spot lights.
	- intensityThreshold: the threshold below which the light is considered out of view. If unset
	  this will use the default.
	"""
	try:
		lightSet = str(data['lightSet'])
		ambientShader = str(data.get('ambientShader', ''))
		ambientMaterial = str(data.get('ambientMaterial', ''))
		if ambientShader and not ambientMaterial:
			raise Exception('DeferredLightResolve ambientShader requires ambientMaterial')

		directionalShader = str(data.get('directionalShader', ''))
		directionalMaterial = str(data.get('directionalMaterial', ''))
		if directionalShader and not directionalMaterial:
			raise Exception('DeferredLightResolve directionalShader requires directionalMaterial')

		pointShader = str(data.get('pointShader', ''))
		pointMaterial = str(data.get('pointMaterial', ''))
		if pointShader and not pointMaterial:
			raise Exception('DeferredLightResolve pointShader requires pointMaterial')

		spotShader = str(data.get('spotShader', ''))
		spotMaterial = str(data.get('spotMaterial', ''))
		if spotShader and not spotMaterial:
			raise Exception('DeferredLightResolve spotShader requires spotMaterial')

		try:
			intensityThresholdStr = data.get('intensityThreshold', 0.0)
			intensityThreshold = float(intensityThresholdStr)
		except:
			raise Exception('Invalid intensityThreshold float value "' +
				str(intensityThresholdStr) + '".')
	except KeyError as e:
		raise Exception('DeferredLightResolve doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('DeferredLightResolve must be an object.')

	builder = flatbuffers.Builder(0)

	lightSetOffset = builder.CreateString(lightSet)
	if ambientShader:
		ambientShaderOffset = builder.CreateString(ambientShader)
		ambientMaterialOffset = builder.CreateString(ambientMaterial)
	else:
		ambientShaderOffset = 0
		ambientMaterialOffset = 0

	if directionalShader:
		directionalShaderOffset = builder.CreateString(directionalShader)
		directionalMaterialOffset = builder.CreateString(directionalMaterial)
	else:
		directionalShaderOffset = 0
		directionalMaterialOffset = 0

	if pointShader:
		pointShaderOffset = builder.CreateString(pointShader)
		pointMaterialOffset = builder.CreateString(pointMaterial)
	else:
		pointShaderOffset = 0
		pointMaterialOffset = 0

	if spotShader:
		spotShaderOffset = builder.CreateString(spotShader)
		spotMaterialOffset = builder.CreateString(spotMaterial)
	else:
		spotShaderOffset = 0
		spotMaterialOffset = 0

	DeferredLightResolveStart(builder)
	DeferredLightResolveAddLightSet(builder, lightSetOffset)
	DeferredLightResolveAddAmbientShader(builder, ambientShaderOffset)
	DeferredLightResolveAddAmbientMaterial(builder, ambientMaterialOffset)
	DeferredLightResolveAddDirectionalShader(builder, directionalShaderOffset)
	DeferredLightResolveAddDirectionalMaterial(builder, directionalMaterialOffset)
	DeferredLightResolveAddPointShader(builder, pointShaderOffset)
	DeferredLightResolveAddPointMaterial(builder, pointMaterialOffset)
	DeferredLightResolveAddSpotShader(builder, spotShaderOffset)
	DeferredLightResolveAddSpotMaterial(builder, spotMaterialOffset)
	builder.Finish(DeferredLightResolveEnd(builder))
	return builder.Output()
