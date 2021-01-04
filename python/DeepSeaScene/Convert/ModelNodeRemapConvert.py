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

import flatbuffers
from ..MaterialRemap import *
from ..ModelNodeRemap import *

def convertModelNodeRemap(convertContext, data):
	"""
	Converts a ModelNodeRemap, which clones an existing model node with optional remapping of
	materials. The data map is expected to contain the following elements:
	- baseName: the name of the model node to clone.
	- materialRemaps: optional array of material remaps to apply. Each element of the array has the
	  following members:
	  - name: the name of the model inside the node to replace the material with.
	  - modelList: the name of the item list the model is drawn with. If unset, all models matching
	    the name will be remapped.
	  - shader: the name of the shader to use. If unset, the shader will remain unchanged.
	  - material: the name of the material to use. If unset, the material will remain unchanged.
	"""
	try:
		name = data['baseName']

		remaps = []
		try:
			for remap in data.get('materialRemaps', []):
				remaps.append((str(remap['name']), str(remap.get('modelList', '')),
					str(remap.get('shader', '')), str(remap.get('material', ''))))
		except (TypeError, ValueError):
			raise Exception(
				'ModelNodeRemap "materialRemaps" must be an array of objects.')
		except KeyError as e:
			raise Exception(
				'ModelNodeRemap material remaps doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('ModelNodeRemap data must be an object.')
	except KeyError as e:
		raise Exception('ModelNodeRemap data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	nameOffset = builder.CreateString(name)

	if remaps:
		remapsOffsets = []
		for modelName, modelList, shader, material in remaps:
			modelNameOffset = builder.CreateString(modelName)

			if modelList:
				modelListOffset = builder.CreateString(modelList)
			else:
				modelListOffset = 0

			if shader:
				shaderOffset = builder.CreateString(shader)
			else:
				shaderOffset = 0

			if material:
				materialOffset = builder.CreateString(material)
			else:
				materialOffset = 0

			MaterialRemapStart(builder)
			MaterialRemapAddName(builder, modelNameOffset)
			MaterialRemapAddModelList(builder, modelListOffset)
			MaterialRemapAddShader(builder, shaderOffset)
			MaterialRemapAddMaterial(builder, materialOffset)
			remapsOffsets.append(MaterialRemapEnd(builder))

		ModelNodeRemapStartMaterialRemapsVector(builder, len(remapsOffsets))
		for offset in reversed(remapsOffsets):
			builder.PrependUOffsetTRelative(offset)
		remapsOffset = builder.EndVector(len(remapsOffsets))
	else:
		remapsOffset = 0

	ModelNodeRemapStart(builder)
	ModelNodeRemapAddName(builder, nameOffset)
	ModelNodeRemapAddMaterialRemaps(builder, remapsOffset)
	builder.Finish(ModelNodeRemapEnd(builder))
	return builder.Output()
