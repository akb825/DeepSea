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
from .. import DeferredLightInfo
from .. import DeferredLightResolve
from .. import DeferredShadowLightInfo

class Object:
	pass

def convertDeferredLightResolve(convertContext, data, inputDir):
	"""
	Converts a DeferredLightResolve. The data map is expected to contain the following elements:
	- lightSet: name of the light set to draw the lights from.
	- shadowManager: name of the shadow manager when drawing shadowed lights.
	- ambient: object containing info for the ambient light. If omitted, the ambient light won't be
	  drawn. It is expected to contain the following elements:
	  - shader: the name of the shader to draw the light.
	  - material: the name of the material to use with the light shader.
	- directional: object containing info for non-shadowed directional lights. If omitted,
	  non-shadowed directional lights won't be drawn. It is expected to contain the following
	  elements:
	  - shader: the name of the shader to draw the light.
	  - material: the name of the material to use with the light shader.
	- point: object containing info for non-shadowed point lights. If omitted, non-shadowed spot
	  lights won't be drawn. It is expected to contain the following elements:
	  - shader: the name of the shader to draw the light.
	  - material: the name of the material to use with the light shader.
	- spot: object containing info for non-shadowed spot lights. If omitted, non-shadowed spot
	  lights won't be drawn. It is expected to contain the following elements:
	  - shader: the name of the shader to draw the light.
	  - material: the name of the material to use with the light shader.
	- shadowDirectional: object containing info for shadowed directional lights. If omitted,
	  shadowed directional lights won't be drawn. It is expected to contain the following elements:
	  - shader: the name of the shader to draw the light.
	  - material: the name of the material to use with the light shader.
	  - transformGroup: name of the shader variable group containing the shadow transform.
	  - shadowTexture: name of the shader variable for the the shadow texture.
	- shadowPoint: object containing info for shadowed point lights. If omitted, shadowed spot
	  lights won't be drawn. It is expected to contain the following elements:
	  - shader: the name of the shader to draw the light.
	  - material: the name of the material to use with the light shader.
	  - transformGroup: name of the shader variable group containing the shadow transform.
	  - shadowTexture: name of the shader variable for the the shadow texture.
	- spot: object containing info for shadowed spot lights. If omitted, shadowed spot lights won't
	  be drawn. It is expected to contain the following elements:
	  - shader: the name of the shader to draw the light.
	  - material: the name of the material to use with the light shader.
	  - transformGroup: name of the shader variable group containing the shadow transform.
	  - shadowTexture: name of the shader variable for the the shadow texture.
	- intensityThreshold: the threshold below which the light is considered out of view. If unset
	  this will use the default.
	"""
	def readLightInfo(lightData, name):
		lightInfo = Object()
		try:
			lightInfo.shader = str(lightData['shader'])
			lightInfo.material = str(lightData['material'])
			return lightInfo
		except KeyError as e:
			raise Exception('DeferredLightResolve ' + name + ' doesn\'t contain element ' + str(e) +
				'.')
		except (AttributeError, TypeError, ValueError):
			raise Exception('DeferredLightResolve ' + name + ' must be an object.')

	def readShadowLightInfo(lightData, name):
		lightInfo = Object()
		try:
			lightInfo.shader = str(lightData['shader'])
			lightInfo.material = str(lightData['material'])
			lightInfo.transformGroup = str(lightData['transformGroup'])
			lightInfo.shadowTexture = str(lightData['shadowTexture'])
			return lightInfo
		except KeyError as e:
			raise Exception('DeferredLightResolve ' + name + ' doesn\'t contain element ' + str(e) +
				'.')
		except (AttributeError, TypeError, ValueError):
			raise Exception('DeferredLightResolve ' + name + ' must be an object.')

	try:
		lightSet = str(data['lightSet'])
		shadowManager = str(data.get('shadowManager', ''))

		ambientData = data.get('ambient')
		if ambientData:
			ambient = readLightInfo(ambientData, 'ambient')
		else:
			ambient = None

		directionalData = data.get('directional')
		if directionalData:
			directional = readLightInfo(directionalData, 'directional')
		else:
			directional = None

		pointData = data.get('point')
		if pointData:
			point = readLightInfo(pointData, 'point')
		else:
			point = None

		spotData = data.get('spot')
		if spotData:
			spot = readLightInfo(spotData, 'spot')
		else:
			spot = None

		shadowDirectionalData = data.get('shadowDirectional')
		if shadowDirectionalData:
			shadowDirectional = readShadowLightInfo(shadowDirectionalData, 'shadowDirectional')
		else:
			shadowDirectional = None

		shadowPointData = data.get('shadowPoint')
		if shadowPointData:
			shadowPoint = readShadowLightInfo(shadowPointData, 'shadowPoint')
		else:
			shadowPoint = None

		shadowSpotData = data.get('shadowSpot')
		if shadowSpotData:
			shadowSpot = readShadowLightInfo(shadowSpotData, 'shadowSpot')
		else:
			shadowSpot = None

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
	if shadowManager:
		shadowManagerOffset = builder.CreateString(shadowManager)
	else:
		shadowManagerOffset = 0

	if ambient:
		shaderOffset = builder.CreateString(ambient.shader)
		materialOffset = builder.CreateString(ambient.material)
		DeferredLightInfo.Start(builder)
		DeferredLightInfo.AddShader(builder, shaderOffset)
		DeferredLightInfo.AddMaterial(builder, materialOffset)
		ambientOffset = DeferredLightInfo.End(builder)
	else:
		ambientOffset = 0

	if directional:
		shaderOffset = builder.CreateString(directional.shader)
		materialOffset = builder.CreateString(directional.material)
		DeferredLightInfo.Start(builder)
		DeferredLightInfo.AddShader(builder, shaderOffset)
		DeferredLightInfo.AddMaterial(builder, materialOffset)
		directionalOffset = DeferredLightInfo.End(builder)
	else:
		directionalOffset = 0

	if point:
		shaderOffset = builder.CreateString(point.shader)
		materialOffset = builder.CreateString(point.material)
		DeferredLightInfo.Start(builder)
		DeferredLightInfo.AddShader(builder, shaderOffset)
		DeferredLightInfo.AddMaterial(builder, materialOffset)
		pointOffset = DeferredLightInfo.End(builder)
	else:
		pointOffset = 0

	if spot:
		shaderOffset = builder.CreateString(spot.shader)
		materialOffset = builder.CreateString(spot.material)
		DeferredLightInfo.Start(builder)
		DeferredLightInfo.AddShader(builder, shaderOffset)
		DeferredLightInfo.AddMaterial(builder, materialOffset)
		spotOffset = DeferredLightInfo.End(builder)
	else:
		spotOffset = 0

	if shadowDirectional:
		shaderOffset = builder.CreateString(shadowDirectional.shader)
		materialOffset = builder.CreateString(shadowDirectional.material)
		transformGroupOffset = builder.CreateString(shadowDirectional.transformGroup)
		shadowTextureOffset = builder.CreateString(shadowDirectional.shadowTexture)
		DeferredShadowLightInfo.Start(builder)
		DeferredShadowLightInfo.AddShader(builder, shaderOffset)
		DeferredShadowLightInfo.AddMaterial(builder, materialOffset)
		DeferredShadowLightInfo.AddTransformGroup(builder, transformGroupOffset)
		DeferredShadowLightInfo.AddShadowTexture(builder, shadowTextureOffset)
		shadowDirectionalOffset = DeferredShadowLightInfo.End(builder)
	else:
		shadowDirectionalOffset = 0

	if shadowPoint:
		shaderOffset = builder.CreateString(shadowPoint.shader)
		materialOffset = builder.CreateString(shadowPoint.material)
		transformGroupOffset = builder.CreateString(shadowPoint.transformGroup)
		shadowTextureOffset = builder.CreateString(shadowPoint.shadowTexture)
		DeferredShadowLightInfo.Start(builder)
		DeferredShadowLightInfo.AddShader(builder, shaderOffset)
		DeferredShadowLightInfo.AddMaterial(builder, materialOffset)
		DeferredShadowLightInfo.AddTransformGroup(builder, transformGroupOffset)
		DeferredShadowLightInfo.AddShadowTexture(builder, shadowTextureOffset)
		shadowPointOffset = DeferredShadowLightInfo.End(builder)
	else:
		shadowPointOffset = 0

	if shadowSpot:
		shaderOffset = builder.CreateString(shadowSpot.shader)
		materialOffset = builder.CreateString(shadowSpot.material)
		transformGroupOffset = builder.CreateString(shadowSpot.transformGroup)
		shadowTextureOffset = builder.CreateString(shadowSpot.shadowTexture)
		DeferredShadowLightInfo.Start(builder)
		DeferredShadowLightInfo.AddShader(builder, shaderOffset)
		DeferredShadowLightInfo.AddMaterial(builder, materialOffset)
		DeferredShadowLightInfo.AddTransformGroup(builder, transformGroupOffset)
		DeferredShadowLightInfo.AddShadowTexture(builder, shadowTextureOffset)
		shadowSpotOffset = DeferredShadowLightInfo.End(builder)
	else:
		shadowSpotOffset = 0

	DeferredLightResolve.Start(builder)
	DeferredLightResolve.AddLightSet(builder, lightSetOffset)
	DeferredLightResolve.AddShadowManager(builder, shadowManagerOffset)
	DeferredLightResolve.AddAmbient(builder, ambientOffset)
	DeferredLightResolve.AddDirectional(builder, directionalOffset)
	DeferredLightResolve.AddPoint(builder, pointOffset)
	DeferredLightResolve.AddSpot(builder, spotOffset)
	DeferredLightResolve.AddShadowDirectional(builder, shadowDirectionalOffset)
	DeferredLightResolve.AddShadowPoint(builder, shadowPointOffset)
	DeferredLightResolve.AddShadowSpot(builder, shadowSpotOffset)
	DeferredLightResolve.AddIntensityThreshold(builder, intensityThreshold)
	builder.Finish(DeferredLightResolve.End(builder))
	return builder.Output()
