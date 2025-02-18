# Copyright 2020-2023 Aaron Barany
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

from .. import Light
from ..LightUnion import LightUnion
from .. import SceneLightSet

from .ColorConvert import readFloat, readColor
from .LightConvert import readLight, writeLight

from DeepSeaScene.Color3f import CreateColor3f

class Object:
	pass

def convertLightSet(convertContext, data, outputDir):
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
	      - linearFalloff: amount the light falls off based on distance. Defaults to 1.
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
	def readInt(value, name, minVal):
		try:
			intVal = int(value)
			if intVal < minVal:
				raise Exception() # Common error handling in except block.
			return intVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	def readVector(value, name):
		if not isinstance(value, list) or len(value) != 3:
			raise Exception('SceneLight ' + name + ' must be an array of three floats.')

		return [readFloat(value[0], name + ' x'),
			readFloat(value[1], name + ' y'),
			readFloat(value[2], name + ' z')]

	try:
		srgb = bool(data.get('srgb', False))
		lightsData = data.get('lights', [])
		lights = []
		try:
			for lightData in lightsData:
				try:
					light = Object()
					light.name = str(lightData['name'])
					readLight(light, lightData, srgb)
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
		lightType, lightUnionOffset = writeLight(builder, light)

		Light.Start(builder)
		Light.AddName(builder, nameOffset)
		Light.AddLightType(builder, lightType)
		Light.AddLight(builder, lightUnionOffset)
		lightOffsets.append(Light.End(builder))

	if lightOffsets:
		SceneLightSet.StartLightsVector(builder, len(lightOffsets))
		for offset in reversed(lightOffsets):
			builder.PrependUOffsetTRelative(offset)
		lightsOffset = builder.EndVector()
	else:
		lightsOffset = 0

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
