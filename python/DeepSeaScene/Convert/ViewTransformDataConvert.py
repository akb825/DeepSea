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
from ..ViewTransformData import *

def convertViewTransformData(convertContext, data):
	"""
	Converts an ViewTransformData. The data map is expected to contain the following elements:
	- variableGroupDescName: string name for the shader variable group to use.
	"""
	try:
		variableGroupDescName = str(data['variableGroupDescName'])
	except KeyError as e:
		raise Exception('ViewTransform doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('ViewTransform must be an object.')

	builder = flatbuffers.Builder(0)
	variableGroupDescNameOffset = builder.CreateString(variableGroupDescName)
	ViewTransformDataStart(builder)
	ViewTransformDataAddVariableGroupDescName(builder, variableGroupDescNameOffset)
	builder.Finish(ViewTransformDataEnd(builder))
	return builder.Output()
