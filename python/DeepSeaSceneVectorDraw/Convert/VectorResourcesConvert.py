# Copyright 2020-2025 Aaron Barany
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

from .. import VectorResources
from DeepSeaScene.Convert.FileOrDataConvert import convertFileOrData, readDataOrPath

def convertVectorResources(convertContext, data, outputDir):
	"""
	Converts vector resources used in a scene. The data map is expected to contain the following
	elements:
	- resources: path to the vector resources.
	- output: the path to the output the vector resources. This can be omitted if vector resources
	  are embedded. If resourceType is "Relative", this will be treated as relative to the scene
	  resource file.
	- outputRelativeDir: the directory relative to output path. This will be removed from the path
	  before adding the reference.
	- resourceType: the resource type. See the dsFileResourceType for values, removing the type
	  prefix, in addition to "Relative" for a path relative to the scene resources file. Defaults
	  to "Relative".
	"""
	builder = flatbuffers.Builder(0)

	try:
		path = str(data['resources'])

		fileName = os.path.splitext(os.path.basename(path))[0]
		parentDir = os.path.dirname(path)
		resourcesDirName = fileName + '_resources'
		resourcesDir = os.path.abspath(os.path.join(parentDir, resourcesDirName))
		hasResourcesDir = os.path.isdir(resourcesDir)

		outputPath = data.get('output')
		if not outputPath and hasResourcesDir:
			raise Exception("Can't embed vector resources that has non-embedded resources.")

		dataType, dataOffset = convertFileOrData(builder, path, None, outputPath,
			data.get('outputRelativeDir'), data.get('resourceType'), outputDir)
	except KeyError as e:
		raise Exception('VectorResources doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('VectorResources must be an object.')

	# Copy resources directory if it's not to the same location.
	if hasResourcesDir:
		outputDir = os.path.dirname(outputPath)
		outputResourcesDir = os.path.abspath(os.path.join(outputDir, resourcesDirName))
		if resourcesDir != outputResourcesDir:
			shutil.rmtree(outputResourcesDir, ignore_errors=True)
			shutil.copytree(resourcesDir, outputResourcesDir)

	VectorResources.Start(builder)
	VectorResources.AddResourcesType(builder, dataType)
	VectorResources.AddResources(builder, dataOffset)
	builder.Finish(VectorResources.End(builder))
	return builder.Output()
