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

import flatbuffers

from .. import LightNode
from .LightConvert import readLight, writeLight

class Object:
	pass

def convertLightNode(convertContext, data):
	"""
	Converts a LightNode. The data map is expected to contain the following elements:
	- light: The light used as a template for all the lights created in the scene. It is expected
	  to contain the following members:
	  - color: the color of the light as an array of three float values, typically in the range
	    [0, 1].
	  - intensity: the intensity of the light, which multiplies the color.
	  - type: the type of the light. The following types are supported with the members they expect:
	  	- "Directional"
	      - direction: direction of the light as an array of three float values.
	    - "Point"
	      - position: position of the light as an array of three float values.
	      - linearFalloff: amount the light falls off based on distance.  Defaults to 1.
	      - quadraticFalloff: amount the light falls off based on squared distance. Defaults to 1.
	    - "Spot"
	      - position: position of the light as an array of three float values.
	      - direction: direction of the light as an array of three float values.
	      - linearFalloff: amount the light falls off based on distance. Defaults to 1.
	      - quadraticFalloff: amount the light falls off based on squared distance. Defaults to 1.
	      - innerSpotAngle: the angle in degrees of the spot light where it starts to fade out.
	      - outerSpotAngle: the angle in degrees of the spot light where it finishes fade out.
	- srgb: true to treat all color values as sRGB values to be converted to linear space. Defaults
	  to false.
	- lightBaseName: The base name for the lights added to the scene. The lights will have ".n"
	  appended to the name, where n is an index incremented for new instances.
	- itemLists: array of item list names to add the node to.
	"""
	try:
		lightData = data['light']
		srgb = data.get('srgb', False)
		light = Object()
		try:
			readLight(light, lightData, srgb)
		except KeyError as e:
			raise Exception('LightNode templateLight doesn\'t contain element ' + str(e) + '.')
		lightBaseName = str(data['lightBaseName'])
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('LightNode data must be an object.')
	except KeyError as e:
		raise Exception('LightNode data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	lightType, lightOffset = writeLight(builder, light)
	lightBaseNameOffset = builder.CreateString(lightBaseName)

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('LightNode "itemLists" must be an array of strings.')

		LightNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	LightNode.Start(builder)
	LightNode.AddTemplateLightType(builder, lightType)
	LightNode.AddTemplateLight(builder, lightOffset)
	LightNode.AddLightBaseName(builder, lightBaseNameOffset)
	LightNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(LightNode.End(builder))
	return builder.Output()
