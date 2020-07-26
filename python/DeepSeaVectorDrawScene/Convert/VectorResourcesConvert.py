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

import os
import shutil

import flatbuffers
from DeepSeaScene.Convert.FileOrDataConvert import convertFileOrData, readDataOrPath
from ..VectorResources import *

def convertVectorResources(convertContext, data):
	"""
	Converts vector resources used in a scene. The data map is expected to contain the following
	elements:
	- path: path to the vector resources.
	- output: the path to the output the vector resources. This can be omitted if vector resources
	  are embedded.
	- outputRelativeDir: the directory relative to output path. This will be removed from the path
	  before adding the reference.
	- resourceType: the resource type. See the dsFileResourceType for values, removing the type
	  prefix. Defaults to "Embedded".
	"""
	builder = flatbuffers.Builder(0)

	try:
		path = str(data['path'])

		fileName = os.path.splitext(os.path.basename(path))[0]
		parentDir = os.path.dirname(path)
		resourcesDirName = fileName + '_resources'
		resourcesDir = os.path.join(parentDir, resourcesDirName)
		hasResourcesDir = os.path.isdir(resourcesDir)

		outputPath = data.get('output')
		if outputPath and hasResourcesDir:
			raise Exception("Can't embed vector resources that has non-embedded resources.")

		dataType, dataOffset = convertFileOrData(builder, path, None, outputPath,
			data.get('outputRelativeDir'), data.get('resourceType'))
		if hasResourcesDir:
			outputDir = os.path.dirname(outputPath)
			outputResourcesDir = os.path.join(outputDir, resourcesDirName)
			shutil.rmtree(outputResourcesDir, ignore_errors=True)
			shutil.copytree(resourcesDir, outputResourcesDir)
	except KeyError as e:
		raise Exception('VectorResources doesn\'t contain element "' + str(e) + '".')
	except (AttributeError, TypeError, ValueError):
		raise Exception('VectorResources must be an object.')

	VectorResourcesStart(builder)
	VectorResourcesAddResourcesType(builder, dataType)
	VectorResourcesAddResources(builder, dataOffset)
	builder.Finish(VectorResourcesEnd(builder))
	return builder.Output()
