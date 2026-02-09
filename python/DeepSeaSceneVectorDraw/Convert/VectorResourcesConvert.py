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
import shutil

from .. import VectorResources
from CreateVectorResources import VectorResources as VectorResourcesConvert
from DeepSeaScene.Convert.FileOrDataConvert import convertFileOrData, finalOutputPath

def convertVectorResources(convertContext, data, inputDir, outputDir):
	"""
	Converts vector resources used in a scene. The data map is expected to contain the following
	elements:
	- resources: array of objects for the vector resources. Each element is expected to have the
	  following members:
	  - type: the type of the resource. The following sections document what types are supported and
	    the members it contains:
	    - "Texture": texture referenced within a vector image or text icon.
	      - name: name used to reference the texture. Defaults to path filename without
	        extension
	      - path: path to the input image.
	      - format: texture format. See cuttlefish help for supported formats.
	      - channelType: texture channel type. See cuttlefish help for supported types.
	      - srgb: whether to perform sRGB to linear conversion when interpreting the texture.
	        Defaults to false.
	      - size: array with the target width and height. Defaults to the dimensions of the image.
	      - quality: quality of encoding, one of Lowest, Low, Normal, High, or Highest. Defaults to
	        Normal.
	      - container: the texture container format, one of pvr, dds, or ktx. Defaults to pvr.
	      - embed: whether to embed embed directly in the resources file. Defaults to false.
	    - "VectorImage": vector image that can be looked up by name.
	      - name: name used to reference the vector image. Defaults to the path filename without
	        extension.
	      - path: path to the input SVG.
	      - defaultFont: the default font to use when none is specified for a text element. Defaults
	        to serif.
	      - targetSize: array with the target width and height for tessellation quality.
			Defaults to the dimensions of the image.
		  - embed: whether to embed embed directly in the resources file. Defaults to false.
	    - "TextureTextIcons", "VectorTextIcons": icons used to replace specific codepoints with
	       either a texture or vector image rather than the glyph from a font face.
	      - name: name used to reference the text icons.
	      - icons: array of array of objects for the individual icons. The outer array groups icons
	        together for faster lookup, where all values between the minimum and maximum codepoints
	        are considered one block of icon glyphs. Each element of the inner array is expected to
	        have the following members:
	        - codepoint: the integer codepoint for the Unicode value to assign the icon to. This may
	          also be provided as a hexidecimal string.
	        - icon: the name of the texture or vector image (depending on the parent object's type)
	          to use for the icon.
	        - advance: the amount to advance text after the icon, normalized to be typically in the
	          range [0, 1].
	        - bounds: 2D array for the minimum and maximum bounds for the icon in normalized values
	          typically in the range [0, 1].
	    - "FaceGroup": font faces that can be used by a font.
	      - name: name used to reference the face group.
	      - faces: array of objects for the faces. Each element is expected to have the following
	        members:
	        - name: the name of the face to reference by a font. Defaults to the filename of the
	          font file without the extension.
	        - path: the path to the font file.
	        - embed: whether to embed embed directly in the resources file. Defaults to false.
	    - "Font": font for displaying of text.
	      - name: the name used to reference the font.
	      - faceGroup: the face group that provides the faces used by the font.
	      - faces: array of strings for the names of the faces to use within the face group.
	      - icons: the name of text icons to use. Defaults to unset, meaning no icons are used.
	      - quality: the quality of the font as one of Low, Medium, High, or VeryHigh.
	      - cacheSize: the size of the cache as one of Small or Large. Defaults to Large.
	- output: the path to the output the vector resources. When omitted, the vector resources will
	  be embedded and all embed members will be forced to true. If resourceType is "Relative",
	  this will be treated as relative to the scene resource file.
	- outputRelativeDir: the directory relative to output path. This will be removed from the path
	  before adding the reference.
	- resourceType: the resource type. See the dsFileResourceType for values, removing the type
	  prefix, in addition to "Relative" for a path relative to the scene resources file. Defaults
	  to "Relative".
	"""
	builder = flatbuffers.Builder(0)
	vectorResources = VectorResourcesConvert(convertContext.cuttlefish)

	try:
		vectorResources.load(data, inputDir)
		outputPath = data.get('output')
		resourceType = data.get('resourceType')
		if outputPath:
			writePath = finalOutputPath(outputPath, resourceType, outputDir)
		else:
			writePath =  None

		resourceData = vectorResources.save(writePath, True, convertContext.multithread)
		inputPath = writePath if resourceData is None else None

		dataType, dataOffset = convertFileOrData(builder, inputPath, resourceData, outputPath,
			data.get('outputRelativeDir'), resourceType, outputDir)
	except KeyError as e:
		raise Exception('VectorResources doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('VectorResources must be an object.')

	VectorResources.Start(builder)
	VectorResources.AddResourcesType(builder, dataType)
	VectorResources.AddResources(builder, dataOffset)
	builder.Finish(VectorResources.End(builder))
	return builder.Output()
