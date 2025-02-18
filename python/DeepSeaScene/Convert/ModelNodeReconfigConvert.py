# Copyright 2020-2021 Aaron Barany
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

from copy import copy
import flatbuffers
from .ModelNodeConvert import validateModelDistanceRange, FLT_MAX
from .. import ModelReconfig
from .. import ModelNodeReconfig
from ..Vector2f import CreateVector2f

def convertModelNodeReconfig(convertContext, data, outputDir):
	"""
	Converts a ModelNodeReconfig, which clones an existing model node while reconfiguring its
	layout. The data map is expected to contain the following elements:
	- baseName: the name of the model node to clone.
	- models: array of models to reconfigure to apply. Each element of the array has the
	  following members:
	  - baseName: the name of the model inside the node to use.
	  - distanceRange: array of two floats for the minimum and maximum distance to draw at.
	    Defaults to [0, 3.402823466e38].
	  - modelLists: array of objects describing the lists to draw the model with the shader and
	    material to draw with. Each element of the array has the following members:
	      - shader: the name of the shader to draw with
	      - material: the name of the material to draw with.
	      - list: the name of the item list to draw the model with.
	- extraItemLists: array of extra item list names to add the node to.
	"""
	try:
		name = data['baseName']

		modelInfos = data['models']
		models = []
		try:
			for model in modelInfos:
				distanceRange = model.get('distanceRange', [0.0, FLT_MAX])
				validateModelDistanceRange(distanceRange)
				baseName = str(model['baseName'])

				modelListInfos = model['modelLists']
				try:
					for modelListInfo in modelListInfos:
						try:
							models.append((baseName, str(modelListInfo['shader']),
								str(modelListInfo['material']), distanceRange,
								str(modelListInfo['list'])))
						except KeyError as e:
							raise Exception(
								'ModelNodeReconfig model list doesn\'t contain element ' +
									str(e) + '.')
				except (TypeError, ValueError):
					raise Exception(
						'ModelNodeReconfig "modelLists" must be an array of objects.')

			extraItemLists = data.get('extraItemLists')
			if extraItemLists and not isinstance(extraItemLists, list):
				raise Exception('ModelNodeReconfig "extraItemLists" must be an array of strings.')

			if not models:
				raise Exception("ModelNodeReconfig doesn't contain any models.")
		except (TypeError, ValueError):
			raise Exception(
				'ModelNodeReconfig "models" must be an array of objects.')
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
	for modelName, shader, material, distanceRange, modelList in models:
		modelNameOffset = builder.CreateString(modelName)
		shaderOffset = builder.CreateString(shader)
		materialOffset = builder.CreateString(material)
		modelListOffset = builder.CreateString(modelList)

		ModelReconfig.Start(builder)
		ModelReconfig.AddName(builder, modelNameOffset)
		ModelReconfig.AddShader(builder, shaderOffset)
		ModelReconfig.AddMaterial(builder, materialOffset)
		ModelReconfig.AddDistanceRange(builder,
			CreateVector2f(builder, distanceRange[0], distanceRange[1]))
		ModelReconfig.AddModelList(builder, modelListOffset)
		modelsOffsets.append(ModelReconfig.End(builder))

	ModelNodeReconfig.StartModelsVector(builder, len(modelsOffsets))
	for offset in reversed(modelsOffsets):
		builder.PrependUOffsetTRelative(offset)
	modelsOffset = builder.EndVector()

	extraItemListsOffsets = []
	for itemList in extraItemLists:
		extraItemListsOffsets.append(builder.CreateString(str(itemList)))

	ModelNodeReconfig.StartExtraItemListsVector(builder, len(extraItemListsOffsets))
	for offset in reversed(extraItemListsOffsets):
		builder.PrependUOffsetTRelative(offset)
	extraItemListsOffset = builder.EndVector()

	ModelNodeReconfig.Start(builder)
	ModelNodeReconfig.AddName(builder, nameOffset)
	ModelNodeReconfig.AddModels(builder, modelsOffset)
	ModelNodeReconfig.AddExtraItemLists(builder, extraItemListsOffset)
	builder.Finish(ModelNodeReconfig.End(builder))
	return builder.Output()
