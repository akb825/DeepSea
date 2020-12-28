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
from DeepSeaScene.Vector2f import *
from ..VectorImageNode import *

def convertVectorImageNode(convertContext, data):
	"""
	Converts a VectorImageNode. The data map is expected to contain the following elements:
	- embeddedResources: optional set of resources to embed with the node. This is an array of maps
	  as expected by SceneResourcesConvert.convertSceneResources().
	- vectorImage: the name of the vector image to draw.
	- size: the size to draw the vector image as an array of two floats. Defaults to the original
	  image size.
	- z: the Z value used for sorting text and vector elements as a signed int.
	- vectorShaders: the name of the vector shaders to draw with.
	- material: the name of the material to draw with.
	- itemLists: array of item list names to add the node to.
	"""
	def readInt(value, name):
		try:
			return int(value)
		except:
			raise Exception('Invalid ' + name + ' int value "' + str(value) + '".')

	try:
		embeddedResources = data.get('embeddedResources', list())
		if not isinstance(embeddedResources, list):
			raise Exception ('TextNode "embeddedResources" must be an object.')

		size = data.get('size')
		if size:
			try:
				if len(size) != 2:
					raise Exception() # Common error handling in except block.
				size[0] = float(size[0])
				size[1] = float(size[1])
			except:
				raise Exception('Invalid vector image node size "' + str(size) + '".')

		vectorImage = str(data['vectorImage'])
		z = readInt(data['z'], 'z')
		vectorShaders = str(data['vectorShaders'])
		material = str(data['material'])
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('VectorImageNode data must be an object.')
	except KeyError as e:
		raise Exception('VectorImageNode data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)
	if embeddedResources:
		embeddedResourcesData = convertSceneResources(convertContext, embeddedResources)
		embeddedResourcesOffset = builder.CreateByteVector(embeddedResourcesData)
	else:
		embeddedResourcesOffset = 0

	vectorImageOffset = builder.CreateString(vectorImage)
	vectorShadersOffset = builder.CreateString(vectorShaders)
	materialOffset = builder.CreateString(material)

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('VectorImageNode "itemLists" must be an array of strings.')

		VectorImageNodeStartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector(len(itemListOffsets))
	else:
		itemListsOffset = 0

	VectorImageNodeStart(builder)
	VectorImageNodeAddEmbeddedResources(builder, embeddedResourcesOffset)
	VectorImageNodeAddVectorImage(builder, vectorImageOffset)

	if size:
		sizeOffset = CreateVector2f(builder, size[0], size[1])
	else:
		sizeOffset = 0
	VectorImageNodeAddSize(builder, sizeOffset)

	VectorImageNodeAddZ(builder, z)
	VectorImageNodeAddVectorShaders(builder, vectorShadersOffset)
	VectorImageNodeAddMaterial(builder, materialOffset)
	VectorImageNodeAddItemLists(builder, itemListsOffset)
	builder.Finish(VectorImageNodeEnd(builder))
	return builder.Output()
