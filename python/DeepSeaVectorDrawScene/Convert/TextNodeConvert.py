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
from DeepSeaScene.Convert.SceneResourcesConvert import convertSceneResources
from ..TextAlign import *
from ..TextNode import *

FLT_MAX = 3.402823466e38

def convertTextNode(convertContext, data):
	"""
	Converts a TextNode. The data map is expected to contain the following elements:
	- embeddedResources: optional set of resources to embed with the node. This is a map containing
	  the elements as expected by SceneResourcesConvert.convertSceneResources().
	- text: the name of the text element to draw. 
	- alignment: the alignment of the text. May be Start, End, Left, Right, or Center. Defaults to
	  Start.
	- maxWidth: the maximum width of the text. Defaults to no maximum.
	- lineScale: the scale to apply to the distance between each line. Defaults to 1.
	- z: the Z value used for sorting text and vector elements as a signed int.
	- firstChar: the first character to display. Defaults to 0.
	- charCount: the number of characters to display. Defaults to all characters.
	- shader: the name of the shader to draw with.
	- material: the name of the material to draw with.
	- itemLists: array of item list names to add the node to.
	"""
	def readFloat(value, name, minValue = 0.0):
		try:
			f = float(value)
			if f < minValue:
				raise Exception() # Shared error handling.
			return f
		except:
			raise Exception('Invalid ' + name + ' float value "' + str(value) + '".')

	def readInt(value, name):
		try:
			return int(value)
		except:
			raise Exception('Invalid ' + name + ' int value "' + str(value) + '".')

	def readUInt(value, name):
		try:
			i = int(value)
			if i < 0:
				raise Exception() # Shared error handling.
			return i
		except:
			raise Exception('Invalid ' + name + ' uint value "' + str(value) + '".')

	try:
		embeddedResources = data.get('embeddedResources', dict())
		if not isinstance(embeddedResources, dict):
			raise Exception ('TextNode "embeddedResources" must be an object.')

		text = str(data['text'])

		alignmentStr = str(data.get('alignment', 'Start'))
		try:
			alignment = getattr(TextAlign, alignmentStr)
		except AttributeError:
			raise Exception('Invalid text alignment "' + alignmentStr + '".')

		maxWidth = readFloat(data.get('maxWidth', FLT_MAX), 'maxWidth')
		lineScale = readFloat(data.get('lineScale', 1.0), 'lineScale')
		z = readInt(data['z'], 'z')
		firstChar = readUInt(data.get('firstChar', 0), 'firstChar')
		charCount = readUInt(data.get('charCount', 0xFFFFFFFF), 'charCount')
		shader = str(data['shader'])
		material = str(data['material'])
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('TextNode data must be an object.')
	except KeyError as e:
		raise Exception('TextNode data doesn\'t contain element "' + str(e) + '".')

	builder = flatbuffers.Builder(0)
	if embeddedResources:
		embeddedResourcesData = convertSceneResources(convertContext, embeddedResources)
		embeddedResourcesOffset = builder.CreateByteVector(embeddedResourcesData)
	else:
		embeddedResourcesOffset = 0

	textOffset = builder.CreateString(text)
	shaderOffset = builder.CreateString(shader)
	materialOffset = builder.CreateString(material)

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('TextNode "itemLists" must be an array of strings.')

		TextNodeStartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector(len(itemListOffsets))
	else:
		itemListsOffset = 0

	TextNodeStart(builder)
	TextNodeAddEmbeddedResources(builder, embeddedResourcesOffset)
	TextNodeAddText(builder, textOffset)
	TextNodeAddAlignment(builder, alignment)
	TextNodeAddMaxWidth(builder, maxWidth)
	TextNodeAddLineScale(builder, lineScale)
	TextNodeAddZ(builder, z)
	TextNodeAddFirstChar(builder, firstChar)
	TextNodeAddCharCount(builder, charCount)
	TextNodeAddShader(builder, shaderOffset)
	TextNodeAddMaterial(builder, materialOffset)
	TextNodeAddItemLists(builder, itemListsOffset)
	builder.Finish(TextNodeEnd(builder))
	return builder.Output()
