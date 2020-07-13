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
from ..VectorImage import *
from DeepSeaScene.Convert.FileOrDataConvert import convertFileOrData, readDataOrPath
from DeepSeaScene.Vector2f import *

def convertVectorImage(convertContext, data):
	"""
	Converts a VectorImage. The data map is expected to contain the following elements:
	- image: path to the vector image or base64 encoded data prefixed with "base64:".
	- output: the path to the output the vector image. This can be omitted if the vector image is
	  embedded.
	- outputRelativeDir: the directory relative to output path. This will be removed from the path
	  before adding the reference.
	- resourceType: the resource type. See the dsFileResourceType for values, removing the type
	  prefix. Defaults to "Embedded".
	- size: the size of the vector image as an array of two floats.
	- sharedMaterials: the name of the vector material set for shared material data.
	- shader: the name of the vector material set for shared material data.
	- resources: list of strings for the names of the vector resources to get textures and fonts
	  from.
	- srgb: bool for whether or not the embedded materials should be treated as sRGB and converted
	  to linear when drawing. Defaults to false.
	"""
	builder = flatbuffers.Builder(0)

	try:
		imageStr = str(data['image'])
		try:
			imagePath, imageContents = readDataOrPath(imageStr)
		except TypeError:
			raise Exception('VectorImage "data" uses incorrect base64 encoding.')
		imageType, imageOffset = convertFileOrData(builder, imagePath, imageContents,
			data.get('output'), data.get('outputRelativeDir'), data.get('resourceType'))

		size = data['size']
		try:
			if len(size) != 2:
				raise Exception() # Common error handling in except block.
			size[0] = float(size[0])
			size[1] = float(size[1])
		except:
			raise Exception('Invalid vector image size "' + str(size) + '".')

		sharedMaterials = str(data['sharedMaterials'])
		shader = str(data['shader'])
		resources = data['resources']
		if not isinstance(resources, list):
			raise Exception('Invalid vector image resources "' + str(resources) + '".')

		srgb = bool(data.get('srgb'))
	except KeyError as e:
		raise Exception('VectorImage doesn\'t contain element "' + str(e) + '".')
	except (AttributeError, TypeError, ValueError):
		raise Exception('VectorImage must be an object.')

	sizeOffset = CreateVector2f(builder, size[0], size[1])
	sharedMaterialsOffset = builder.CreateString(sharedMaterials)
	shaderOffset = builder.CreateString(shader)

	resourceOffsets = []
	for resource in resources:
		resourceOffsets.append(builder.CreateString(resource))

	VectorImageStartResourcesVector(builder, len(resourceOffsets))
	for offset in reversed(resourceOffsets):
		builder.PrependUOffsetTRelative(offset)
	resourcesOffset = builder.EndVector(len(resourceOffsets))

	VectorImageStart(builder)
	VectorImageAddImageType(builder, imageType)
	VectorImageAddImage(builder, imageOffset)
	VectorImageAddSharedMaterials(builder, sharedMaterialsOffset)
	VectorImageAddShader(builder, shaderOffset)
	VectorImageAddResources(builder, resourcesOffset)
	VectorImageAddSrgb(builder, srgb)
	builder.Finish(VectorImageEnd(builder))
	return builder.Output()
