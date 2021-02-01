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
	- ambientShader: the name of the shader used for the ambient light.
	- ambientMaterial: the name of the material used for the ambient light.
	- drectionalShader: the name of the shader used for directional lights.
	- drectionalMaterial: the name of the material used for directional lights.
	- pointShader: the name of the shader used for point lights.
	- pointMaterial: the name of the material used for point lights.
	- spotShader: the name of the shader used for spot lights.
	- spotMaterial: the name of the material used for spot lights.
	- intensityThreshold: the threshold below which the light is considered out of view. If unset
	  this will use the default.
	"""
	try:
		lightSet = str(data['lightSet'])
		ambientShader = str(data['ambientShader'])
		ambientMaterial = str(data['ambientMaterial'])
		directionalShader = str(data['directionalShader'])
		directionalMaterial = str(data['directionalMaterial'])
		pointShader = str(data['pointShader'])
		pointMaterial = str(data['pointMaterial'])
		spotShader = str(data['spotShader'])
		spotMaterial = str(data['spotMaterial'])

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
	ambientShaderOffset = builder.CreateString(ambientShader)
	ambientMaterialOffset = builder.CreateString(ambientMaterial)
	directionalShaderOffset = builder.CreateString(directionalShader)
	directionalMaterialOffset = builder.CreateString(directionalMaterial)
	pointShaderOffset = builder.CreateString(pointShader)
	pointMaterialOffset = builder.CreateString(pointMaterial)
	spotShaderOffset = builder.CreateString(spotShader)
	spotMaterialOffset = builder.CreateString(spotMaterial)

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
