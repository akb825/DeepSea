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
import math

from .. import DirectionalLight
from .. import Light
from ..LightUnion import LightUnion
from .. import PointLight
from .. import SceneLightSet
from .. import SpotLight

from DeepSeaScene.Color3f import CreateColor3f
from DeepSeaScene.Vector3f import CreateVector3f

class Object:
	pass

def convertLightSet(convertContext, data):
	"""
	Converts a light set for a scene. The data map is expected to contain the following elements:
	- lights: array of lights to initially populate the light set with. Each member of the array
	  has the following members:
	  - name: the name of the light.
	  - color: the color of the light as an array of three float values, typically in the range
	    [0, 1].
	  - intensity: the intensity of the light, which multiplies the color.
	  - type: the type of the light. The following types are supported with the members they expect:
	  	- "Directional"
	      - direction: direction of the light as an array of three float values.
	    - "Point"
	      - position: position of the light as an array of three float values.
	      - linearFalloff: amount the light falls off based on distance.
	      - quadraticFalloff: amount the light falls off based on squared distance. Defaults to 1.
	    - "Spot"
	      - position: position of the light as an array of three float values.
	      - direction: direction of the light as an array of three float values.
	      - linearFalloff: amount the light falls off based on distance. Defaults to 1.
	      - quadraticFalloff: amount the light falls off based on squared distance. Defaults to 1.
	      - innerSpotAngle: the angle in degrees of the spot light where it starts to fade out.
	      - outerSpotAngle: the angle in degrees of the spot light where it finishes fade out.
	- maxLights: the maximum number of lights that can be stored. If unset, the number of elements
	  in lights will be used.
	- ambientColor: the color of the ambient light as an array of three floats, typically in the
	  range [0,1]. Defaults to all 0.
	- ambientIntensity: the intensity of the ambient light, which multiplies the color. Defaults
	  to 0.
	- mainLight: the name of the main light. If omitted no light will be considered the main light.
	- srgb: true to treat all color values as sRGB values to be converted to linear space. Defaults
	  to false.
	"""
	def readFloat(value, name, minVal = None, maxVal = None):
		try:
			floatVal = float(value)
			if (minVal is not None and floatVal < minVal) or \
					(maxVal is not None and floatVal > maxVal):
				raise Exception() # Common error handling in except block.
			return floatVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	def readInt(value, name, minVal):
		try:
			intVal = int(value)
			if intVal < minVal:
				raise Exception() # Common error handling in except block.
			return intVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	def readColor(value, name, srgb):
		if not isinstance(value, list) or len(value) != 3:
			raise Exception('SceneLight ' + name + ' must be an array of three floats.')

		color = [readFloat(value[0], name + ' red channel'),
			readFloat(value[1], name + ' green channel'),
			readFloat(value[2], name + ' blue channel')]
		if srgb:
			for i in range(0, 3):
				if color[i] <= 0.04045:
					color[i] = color[i]/12.92
				else:
					color[i] = pow((color[i] + 0.055)/1.055, 2.4)
		return color

	def readVector(value, name):
		if not isinstance(value, list) or len(value) != 3:
			raise Exception('SceneLight ' + name + ' must be an array of three floats.')

		return [readFloat(value[0], name + ' x'),
			readFloat(value[1], name + ' y'),
			readFloat(value[2], name + ' z')]

	try:
		srgb = data.get('srgb', False)
		lightsData = data.get('lights', [])
		lights = []
		try:
			for lightData in lightsData:
				try:
					light = Object()
					light.name = str(lightData['name'])
					light.color = readColor(lightData['color'], 'light color', srgb)
					light.intensity = readFloat(lightData['intensity'], 'light intensity', 0.0)
					lightType = lightData['type']
					if lightType == 'Directional':
						light.type = LightUnion.DirectionalLight
						light.direction = readVector(lightData['direction'], 'light direction')
					elif lightType == 'Point':
						light.type = LightUnion.PointLight
						light.position = readVector(lightData['position'], 'light position')
						light.linearFalloff = readFloat(lightData.get('linearFalloff', 1.0),
							'light linear falloff', 0.0)
						light.quadraticFalloff = readFloat(lightData.get('quadraticFalloff', 1.0),
							'light quadratic falloff', 0.0)
					elif lightType == 'Spot':
						light.type = LightUnion.SpotLight
						light.position = readVector(lightData['position'], 'light position')
						light.direction = readVector(lightData['direction'], 'light direction')
						light.linearFalloff = readFloat(lightData.get('linearFalloff', 1.0),
							'light linear falloff', 0.0)
						light.quadraticFalloff = readFloat(lightData.get('quadraticFalloff', 1.0),
							'light quadratic falloff', 0.0)
						light.innerSpotAngle = math.radians(readFloat(lightData['innerSpotAngle'],
							'inner spot angle', 0.0, 180.0))
						light.outerSpotAngle = math.radians(readFloat(lightData['outerSpotAngle'],
							'outer spot angle', 0.0, 180.0))
						if light.innerSpotAngle > light.outerSpotAngle:
							raise Exception(
								'Spot light inner spot angle must be less than outer spot angle.')
				except KeyError as e:
					raise Exception('LightSet light doesn\'t contain element ' + str(e) + '.')

				lights.append(light)
		except (TypeError, ValueError):
			raise Exception('SceneLights "lights" must be an array of objects.')

		maxLights = readInt(data.get('maxLights', 0), 'maxLights', 0)
		if not maxLights and not lights:
			raise Exception('SceneLights cannot have zero max lights.')

		ambientColorData = data.get('ambientColor')
		if ambientColorData:
			ambientColor = readColor(ambientColorData, 'ambient color', srgb)
		else:
			ambientColor = None

		ambientIntensity = readFloat(data.get('ambientIntensity', 0.0), 'ambient intensity', 0.0)
		mainLight = str(data.get('mainLight', ''))
	except KeyError as e:
		raise Exception('LightSet doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('LightSet must be an object.')

	builder = flatbuffers.Builder(0)

	lightOffsets = []
	for light in lights:
		nameOffset = builder.CreateString(light.name)
		if light.type == LightUnion.DirectionalLight:
			DirectionalLight.Start(builder)
			DirectionalLight.AddDirection(builder, CreateVector3f(builder, light.direction[0],
				light.direction[1], light.direction[2]))
			DirectionalLight.AddColor(builder, CreateColor3f(builder, light.color[0],
				light.color[1], light.color[2]))
			DirectionalLight.AddIntensity(builder, light.intensity)
			lightUnionOffset = DirectionalLight.End(builder)
		elif light.type == LightUnion.PointLight:
			PointLight.Start(builder)
			PointLight.AddPosition(builder, CreateVector3f(builder, light.position[0],
				light.position[1], light.position[2]))
			PointLight.AddColor(builder, CreateColor3f(builder, light.color[0], light.color[1],
				light.color[2]))
			PointLight.AddIntensity(builder, light.intensity)
			PointLight.AddLinearFalloff(builder, light.linearFalloff)
			PointLight.AddQuadraticFalloff(builder, light.quadraticFalloff)
			lightUnionOffset = PointLight.End(builder)
		elif light.type == LightUnion.SpotLight:
			SpotLight.Start(builder)
			SpotLight.AddPosition(builder, CreateVector3f(builder, light.position[0],
				light.position[1], light.position[2]))
			SpotLight.AddDirection(builder, CreateVector3f(builder, light.direction[0],
				light.direction[1], light.direction[2]))
			SpotLight.AddColor(builder, CreateColor3f(builder, light.color[0], light.color[1],
				light.color[2]))
			SpotLight.AddIntensity(builder, light.intensity)
			SpotLight.AddLinearFalloff(builder, light.linearFalloff)
			SpotLight.AddQuadraticFalloff(builder, light.quadraticFalloff)
			SpotLight.AddInnerSpotAngle(builder, light.innerSpotAngle)
			SpotLight.AddOuterSpotAngle(builder, light.outerSpotAngle)
			lightUnionOffset = SpotLight.End(builder)

		Light.Start(builder)
		Light.AddName(builder, nameOffset)
		Light.AddLightType(builder, light.type)
		Light.AddLight(builder, lightUnionOffset)
		lightOffsets.append(Light.End(builder))

	if lightOffsets:
		SceneLightSet.StartLightsVector(builder, len(lightOffsets))
		for offset in reversed(lightOffsets):
			builder.PrependUOffsetTRelative(offset)
		lightsOffset = builder.EndVector()
	else:
		lightsOffset

	mainLightOffset = 0
	if mainLight:
		mainLightOffset = builder.CreateString(mainLight)

	SceneLightSet.Start(builder)
	SceneLightSet.AddLights(builder, lightsOffset)
	SceneLightSet.AddMaxLights(builder, maxLights)
	SceneLightSet.AddAmbientColor(builder,
		CreateColor3f(builder, ambientColor[0], ambientColor[1], ambientColor[2]) if ambientColor
			else 0)
	SceneLightSet.AddAmbientIntensity(builder, ambientIntensity)
	SceneLightSet.AddMainLight(builder, mainLightOffset)
	builder.Finish(SceneLightSet.End(builder))
	return builder.Output()
