# Copyright 2020-2026 Aaron Barany
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
import os

from .. import VectorImage

from DeepSeaScene.Convert.FileOrDataConvert import convertFileOrData
from DeepSeaScene.Vector2f import CreateVector2f
from DeepSeaVectorDraw.Convert.SVG import convertSVG

def convertVectorImage(convertContext, data, inputDir, outputDir):
	"""
	Converts a VectorImage. The data map is expected to contain the following elements:
	- image: path to the input SVG.
	- defaultFont: the default font to use when none is specified for a text element. Defaults to
	  serif.
	- output: the path to the output the vector image. This can be omitted if the vector image is
	  embedded. If resourceType is "Relative", this will be treated as relative to the scene
	  resource file.
	- outputRelativeDir: the directory relative to output path. This will be removed from the path
	  before adding the reference.
	- resourceType: the resource type. See the dsFileResourceType for values, removing the type
	  prefix, in addition to "Relative" for a path relative to the scene resources file. Defaults
	  to "Relative".
	- targetSize: the target size of the vector image for the tessellation quality as an array of
	  two floats. Defaults to the original image size.
	- sharedMaterials: the name of the vector material set for shared material data.
	- vectorShaders: the name of the vector material set for shared material data.
	- vectorResources: list of strings for the names of the vector resources to get textures and
	  fonts from.
	- srgb: bool for whether or not the embedded materials should be treated as sRGB and converted
	  to linear when drawing. Defaults to false.
	"""
	builder = flatbuffers.Builder(0)

	try:
		imagePath = str(data['image'])
		imageData = convertSVG(
			os.path.join(inputDir, imagePath), data['name'], str(data.get('defaultFont', 'serif')))
		imageType, imageOffset = convertFileOrData(builder, None, imageData, data.get('output'),
			data.get('outputRelativeDir'), data.get('resourceType'), outputDir)

		size = data.get('targetSize')
		if size:
			try:
				if len(size) != 2:
					raise Exception() # Common error handling in except block.
				size[0] = float(size[0])
				size[1] = float(size[1])
			except:
				raise Exception('Invalid vector image target size "' + str(size) + '".')

		sharedMaterials = str(data.get('sharedMaterials', ''))
		shaders = str(data['vectorShaders'])
		resources = data.get('resources', [])
		if not isinstance(resources, list):
			raise Exception('Invalid vector image resources "' + str(resources) + '".')

		srgb = bool(data.get('srgb'))
	except KeyError as e:
		raise Exception('VectorImage doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('VectorImage must be an object.')

	if sharedMaterials:
		sharedMaterialsOffset = builder.CreateString(sharedMaterials)
	else:
		sharedMaterialsOffset = 0

	shadersOffset = builder.CreateString(shaders)

	resourceOffsets = []
	for resource in resources:
		resourceOffsets.append(builder.CreateString(resource))

	VectorImage.StartResourcesVector(builder, len(resourceOffsets))
	for offset in reversed(resourceOffsets):
		builder.PrependUOffsetTRelative(offset)
	resourcesOffset = builder.EndVector()

	VectorImage.Start(builder)
	VectorImage.AddImageType(builder, imageType)
	VectorImage.AddImage(builder, imageOffset)

	if size:
		sizeOffset = CreateVector2f(builder, size[0], size[1])
	else:
		sizeOffset = 0
	VectorImage.AddTargetSize(builder, sizeOffset)

	VectorImage.AddSharedMaterials(builder, sharedMaterialsOffset)
	VectorImage.AddVectorShaders(builder, shadersOffset)
	VectorImage.AddResources(builder, resourcesOffset)
	VectorImage.AddSrgb(builder, srgb)
	builder.Finish(VectorImage.End(builder))
	return builder.Output()
