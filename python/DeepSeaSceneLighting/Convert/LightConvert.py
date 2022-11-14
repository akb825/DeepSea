# Copyright 2022 Aaron Barany
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

import math

from .. import DirectionalLight
from ..LightUnion import LightUnion
from .. import PointLight
from .. import SpotLight

from .ColorConvert import readFloat, readColor

from DeepSeaScene.Color3f import CreateColor3f
from DeepSeaScene.Vector3f import CreateVector3f

def readLight(light, lightData, srgb):
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
		light.color = readColor(lightData['color'], 'light color', srgb)
		light.intensity = readFloat(lightData['intensity'], 'light intensity', 0.0)
		lightType = str(lightData['type'])
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
		raise Exception('Light doesn\'t contain element ' + str(e) + '.')

def writeLight(builder, light):
	if light.type == LightUnion.DirectionalLight:
		DirectionalLight.Start(builder)
		DirectionalLight.AddDirection(builder, CreateVector3f(builder, light.direction[0],
			light.direction[1], light.direction[2]))
		DirectionalLight.AddColor(builder, CreateColor3f(builder, light.color[0],
			light.color[1], light.color[2]))
		DirectionalLight.AddIntensity(builder, light.intensity)
		return light.type, DirectionalLight.End(builder)
	elif light.type == LightUnion.PointLight:
		PointLight.Start(builder)
		PointLight.AddPosition(builder, CreateVector3f(builder, light.position[0],
			light.position[1], light.position[2]))
		PointLight.AddColor(builder, CreateColor3f(builder, light.color[0], light.color[1],
			light.color[2]))
		PointLight.AddIntensity(builder, light.intensity)
		PointLight.AddLinearFalloff(builder, light.linearFalloff)
		PointLight.AddQuadraticFalloff(builder, light.quadraticFalloff)
		return light.type, PointLight.End(builder)
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
		return light.type, SpotLight.End(builder)
	else:
		assert False
