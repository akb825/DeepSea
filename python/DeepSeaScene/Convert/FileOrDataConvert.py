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

import base64
import os
import shutil

from ..FileOrData import FileOrData
from .. import FileReference
from ..FileResourceType import FileResourceType
from .. import RawData
from .. import RelativePathReference

def readDataOrPath(dataStr, inputDir):
	"""
	Reads in either base64 data or a path.
	"""
	if dataStr.startswith('base64:'):
		dataPath = None
		dataContents = base64.b64decode(dataStr[7:])
	else:
		dataPath = dataStr
		with open(os.path.join(inputDir, dataStr), 'rb') as stream:
			dataContents = stream.read()
	return dataPath, dataContents

def convertFileOrData(builder, inputPath, data, outputPath, outputRelativeDir, resourceType,
		parentOutputDir):
	"""
	Converts to a file reference or raw data. Either inputPath or data may be None if not
	applicable. resourceType should be the name of the resource type or None for the default.
	If inputPath and outputPath are the same then the data will not be copied or written to
	outputPath. outputPath will be treated relative to parentOutputDir if resourceType is
	"Relative" or None.

	A tuple with the type and structure offset will be returned.
	"""
	if not inputPath and not data:
		return FileOrData.NONE, 0

	if outputPath:
		if not resourceType or resourceType == 'Relative':
			fbResourceType = None
		else:
			try:
				fbResourceType = getattr(FileResourceType, resourceType)
			except AttributeError:
				raise Exception('Invalid resource type "' + str(resourceType) + '".')

		if fbResourceType is None:
			outputWritePath = os.path.join(parentOutputDir, outputPath)
		else:
			outputWritePath = outputPath

		if not inputPath or os.path.abspath(inputPath) != os.path.abspath(outputWritePath):
			if data:
				with open(outputWritePath, 'wb') as stream:
					stream.write(data)
			else:
				shutil.copy(inputPath, outputWritePath)

		if outputRelativeDir:
			outputPath = os.path.relpath(outputPath, outputRelativeDir)

		if os.path.isabs(outputPath):
			if fbResourceType != FileResourceType.External:
				raise Exception('Resource path "' + outputPath + '" must not be absolute.')
		elif os.sep != '/':
			outputPath = outputPath.replace(os.sep, '/')

		pathOffset = builder.CreateString(outputPath)

		if fbResourceType is None:
			RelativePathReference.Start(builder)
			RelativePathReference.AddPath(builder, pathOffset)
			return FileOrData.RelativePathReference, RelativePathReference.End(builder)

		FileReference.Start(builder)
		FileReference.AddType(builder, fbResourceType)
		FileReference.AddPath(builder, pathOffset)
		return FileOrData.FileReference, FileReference.End(builder)
	else:
		if not data:
			with open(inputPath, 'rb') as stream:
				data = stream.read()

		dataOffset = builder.CreateByteVector(data)
		RawData.Start(builder)
		RawData.AddData(builder, dataOffset)
		return FileOrData.RawData, RawData.End(builder)
