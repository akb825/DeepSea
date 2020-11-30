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
from .ModelNodeConvert import validateModelDistanceRange, FLT_MAX
from ..ModelReconfig import *
from ..ModelNodeReconfig import *
from ..Vector2f import *

def convertModelNodeReconfig(convertContext, data):
	"""
	Converts a ModelNodeReconfig, which clones an existing model node while reconfiguring its
	layout. The data map is expected to contain the following elements:
	- name: the name of the model node to clone.
	- models: array of models to reconfigure to apply. Each element of the array has the
	  following members:
	  - name: the name of the model inside the node to use.
	  - shader: the name of the shader to use.
	  - material: the name of the material to use.
	  - distanceRange: array of two floats for the minimum and maximum distance to draw at.
	    Defaults to [0, 3.402823466e38].
	  - listName: the name of the item list the model is drawn with.
	- extraItemLists: array of extra item list names to add the node to.
	"""
	try:
		name = data['name']

		models = []
		try:
			for model in data[models]:
				distanceRange = model.get('distanceRange', [0.0, FLT_MAX])
				validateModelDistanceRange(distanceRange)

				models.append((str(model['name']), str(model['shader']),
					str(model['material']), distanceRange, str(model.get('listName', ''))))

			extraItemLists = data.get('extraItemLists')
			if extraItemLists and not isinstance(extraItemLists, list):
				raise Exception('ModelNodeReconfig "extraItemLists" must be an array of strings.')

			if not models:
				raise Exception("ModelNodeReconfig doesn't contain any models.")
		except (TypeError, ValueError):
			raise Exception(
				'ModelNodeReconfig "imodels" must be an array of objects.')
		except KeyError as e:
			raise Exception(
				'ModelNodeReconfig model doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('ModelNodeReconfig data must be an object.')
	except KeyError as e:
		raise Exception('ModelNodeReconfig data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	nameOffset = builder.CreateString(name)

	modelsOffsets = []
	for modelName, shader, material, distanceRange, listName in model:
		modelNameOffset = builder.CreateString(modelName)
		shaderOffset = builder.CreateString(shader)
		materialOffset = builder.CreateString(material)

		if listName:
			listNameOffset = builder.CreateString(listName)
		else:
			listNameOffset = 0

		ModelReconfigStart(builder)
		ModelReconfigAddName(builder, modelNameOffset)
		ModelReconfigAddShader(builder, shaderOffset)
		ModelReconfigAddMaterial(builder, materialOffset)
		ModelReconfigAddDistanceRange(builder,
			CreateVector2f(builder, distanceRange[0], distanceRange[1]))
		ModelReconfigAddListName(builder, listNameOffset)
		modelsOffsets.append(ModelReconfigEnd(builder))

	ModelNodeReconfigStartModelsVector(builder, len(modelsOffsets))
	for offset in reversed(modelsOffsets):
		builder.PrependUOffsetTRelative(offset)
	modelsOffset = builder.EndVector(len(modelsOffsets))

	extraItemListsOffsets = []
	for itemList in extraItemLists:
		extraItemListsOffsets.append(builder.CreateString(str(itemList)))

	ModelNodeReconfigStartExtraItemListsVector(builder, len(extraItemListsOffsets))
	for offset in reversed(extraItemListsOffsets):
		builder.PrependUOffsetTRelative(offset)
	extraItemListsOffset = builder.EndVector(len(extraItemListsOffsets))

	ModelNodeReconfigStart(builder)
	ModelNodeReconfigAddName(builder, nameOffset)
	ModelNodeReconfigAddModels(builder, modelsOffset)
	ModelNodeReconfigAddExtraItemLists(builder, extraItemListsOffset)
	builder.Finish(ModelNodeReconfigEnd(builder))
	return builder.Output()
